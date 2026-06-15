#include <print>
#include <vector>

import CommonLib;

using namespace CL;

enum class DemoFlags : u8 {
	Read = 1 << 0,
	Write = 1 << 1,
	Exec = 1 << 2,
};

auto main() -> int
{
	static_assert(IsIntegralV<int>);
	static_assert(IsFloatingPointV<double>);
	static_assert(IsTypeListV<TypeList<int, char>>);
	static_assert(TypeListSizeV<TypeList<int, char, bool>> == 3);
	static_assert(SameAs<TypeListAtT<1, TypeList<int, char, bool>>, char>);

	std::println("CommonLib demo");

	auto aligned_down { Memory::align_down<usize>(37, 8) };
	auto aligned_up { Memory::align_up<usize>(37, 8) };
	auto overlaps { Memory::ranges_overlap<usize>(0, 8, 4, 8) };
	auto contains { Memory::range_contains<usize>(0, 16, 4, 4) };
	std::println(
	    "memory: {} {} {} {}", aligned_down, aligned_up, overlaps, contains);

	std::println("math: {:.1f} {} {}", Math::ceil(3.2), Math::div_ceil(7, 3),
	    Math::median3(9, 3, 5));

	int raw_array[] { 1, 2, 3 };
	Array<int, 3> fixed_array(raw_array);
	Span<int> raw_span(raw_array, 3);
	std::print("array:");
	for (usize i { }; i < fixed_array.size(); ++i)
		std::print(" {}", fixed_array[i]);
	std::println(" span={}", raw_span.size());

	ArrayList<int> numbers;
	numbers.push(10);
	numbers.push(20);
	numbers.emplace(30);
	std::print("arraylist:");
	numbers.iter().for_each([](int &value) { std::print(" {}", value); });
	std::println(" size={} span={}", numbers.size(), numbers.span().size());

	LinkedList<int> linked { 4, 5, 6 };
	linked.push(7);
	std::print("linkedlist:");
	linked.iter().for_each([](int &value) { std::print(" {}", value); });
	auto first_linked { linked.first() };
	auto last_linked { linked.last() };
	std::println(" first={} last={}", first_linked ? *first_linked : -1,
	    last_linked ? *last_linked : -1);

	String text { "hello world" };
	StringView view { text.c_str() };
	FixedString<32> fixed { view };
	fixed += "!";
	std::println("string: {} view={} fixed={}", text.c_str(), view.size(),
	    fixed.c_str());
	std::println(
	    "string starts_with hello: {}", text.starts_with(StringView("hello")));
	std::println(
	    "string contains world: {}", text.contains(StringView("world")));
	std::println("substring: {}", text.substring(6).data());

	usize rune_count { 0 };
	view.iter().for_each([&](Rune) { ++rune_count; });
	std::println("string runes: {}", rune_count);

	Option<int> maybe { 42 };
	Option<int> empty { };
	std::println("option: {} {}", maybe.unwrap_or(-1), empty.unwrap_or(7));

	auto ok_result { Result<int, String>::Ok(99) };
	auto err_result { Result<int, String>::Err(String("boom")) };
	std::println("result ok={} err={}", ok_result.unwrap(),
	    err_result.unwrap_err().c_str());

	Variant<int, String, double> variant { String("variant") };
	std::print("variant: ");
	variant.visit([](auto const &value) {
		using Value = RemoveConstRef<decltype(value)>;
		if constexpr (SameAs<Value, String>)
			std::print("{}", value.c_str());
		else if constexpr (SameAs<Value, double>)
			std::print("{:.1f}", value);
		else
			std::print("{}", value);
	});
	std::println();

	Flags<DemoFlags> flags { DemoFlags::Read, DemoFlags::Exec };
	std::println("flags: {} {} {}", static_cast<unsigned>(flags.raw()),
	    flags.contains(DemoFlags::Write), flags.contains(DemoFlags::Read));

	HashMap<String, int> map;
	map.insert_or_replace(String("one"), 1);
	map.insert_or_replace(String("two"), 2);
	auto duplicate { map.insert(String("one"), 11) };
	auto one_value { map.get(String("one")) };
	std::println("hashmap: {} {} {}", map.size(), map.contains(String("two")),
	    one_value ? *one_value : -1);
	if (!duplicate) {
		auto erased { ErasedError::from(duplicate.unwrap_err()) };
		std::println("hashmap duplicate: {}", erased.message().data());
	}

	BitArray<16> bits;
	bits.set(3);
	bits.toggle(4);
	bits.set_range(8, 3);
	std::println("bitarray: {} {} {}", bits.test(3), bits.test(4),
	    bits.find_first_set().unwrap_or(usize(-1)));

	BitList bit_list;
	bit_list.push(true);
	bit_list.push(false);
	bit_list.push(true);
	auto bit_pop { bit_list.pop() };
	std::println("bitlist: {} {}", bit_list.size(), bit_pop.unwrap_or(false));

	MpscQueue<int, 8> queue;
	queue.push_blocking(7);
	queue.push_blocking(9);
	std::print("queue:");
	while (auto value { queue.try_pop() })
		std::print(" {}", *value);
	std::println();

	Function<int(int)> doubler { [](int value) { return value * 2; } };
	std::println("function: {}", doubler(21));

	Box<Pair<int, int>> boxed { 3, 4 };
	Rc<Pair<int, int>> shared { 5, 6 };
	Arc<Pair<int, int>> atomic_shared { 7, 8 };
	auto shared_copy = shared;
	auto atomic_shared_copy = atomic_shared;
	std::println("box: {},{}", boxed->first, boxed->second);
	std::println("rc: {} {}", shared.ref_count(), shared_copy.ref_count());
	std::println("arc: {} {}", atomic_shared.ref_count(),
	    atomic_shared_copy.ref_count());

	Atomic<int> atomic_value { 10 };
	atomic_value.fetch_add(5);
	std::println("atomic: {}", atomic_value.load());

	SpinLock spin_lock;
	int guarded { 0 };
	{
		ScopedSpinLock guard { spin_lock };
		guarded = 123;
	}
	std::println("sync: {}", guarded);

	auto filtered {
		range(8)
		    .filter([](int value) { return value % 2 == 0; })
		    .collect<ArrayList>(),
	};
	std::print("range/filter:");
	filtered.iter().for_each([](int &value) { std::print(" {}", value); });
	std::println();

	auto reversed { range(0, 5).rev().collect<ArrayList>() };
	std::print("range/rev:");
	reversed.iter().for_each([](int &value) { std::print(" {}", value); });
	std::println();

	std::println("iterator: {} {} {}",
	    range(4)
	        .map([](int value) { return value + 1; })
	        .collect<ArrayList>()
	        .size(),
	    range(4).every([](int value) { return value < 4; }),
	    range(4).any([](int value) { return value == 3; }));

	std::vector<int> values { 1, 2, 3 };
	auto borrowed {
		borrow_iter(values),
	}; // coud use (values.begin(), values.end()) as well
	if (auto first { borrowed.next() })
		*first += 10;
	auto doubled_values {
		borrow_iter(values.begin(), values.end())
		    .map([](int &value) { return value * 2; })
		    .collect<ArrayList>(),
	};
	std::print("std::vector borrow:");
	doubled_values.iter().for_each(
	    [](int &value) { std::print(" {}", value); });
	std::println(" first={}", values.front());

	std::println("done'd");
}
