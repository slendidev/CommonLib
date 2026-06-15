#include <gtest/gtest.h>

#include <array>
#include <initializer_list>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

import CommonLib;

using namespace CL;

enum class TestFlags : u8 {
	Read = 1 << 0,
	Write = 1 << 1,
	Exec = 1 << 2,
};

static_assert(SameAs<uint, unsigned int>);
static_assert(SameAs<u8, unsigned char>);
static_assert(SameAs<usize, decltype(sizeof(0))>);
static_assert(IsIntegralV<int>);
static_assert(!IsIntegralV<float>);
static_assert(IsFloatingPointV<double>);
static_assert(IsUnsignedIntegralV<u32>);
static_assert(IsSignedIntegralV<int>);
static_assert(!IsEnumV<int>);
static_assert(IsTypeListV<TypeList<int, char>>);
static_assert(TypeListSizeV<TypeList<int, char, bool>> == 3);
static_assert(SameAs<TypeListAtT<1, TypeList<int, char, bool>>, char>);
static_assert(SameAs<MakeIndexSequence<3>, IndexSequence<0, 1, 2>>);
static_assert(SameAs<InitializerList<int>, std::initializer_list<int>>);
static_assert(SameAs<decltype(move(std::declval<int &>())), int &&>);
static_assert(SameAs<decltype(forward<int &>(std::declval<int &>())), int &>);
using PlusOneFn = int (*)(int);
static_assert(SameAs<InvokeResultT<PlusOneFn, int>, int>);
static_assert(Memory::align_down<usize>(37, 8) == 32);
static_assert(Memory::align_up<usize>(37, 8) == 40);
static_assert(Memory::ranges_overlap<usize>(0, 8, 4, 8));
static_assert(Memory::range_contains<usize>(0, 16, 4, 4));
static_assert(Math::ceil(3) == 3);
static_assert(Math::ceil(3.2) == 4.0);
static_assert(Math::div_ceil(7, 3) == 3);
static_assert(SameAs<
    decltype(Flags<TestFlags> { TestFlags::Read, TestFlags::Exec }.raw()),
    typename Flags<TestFlags>::Underlying>);

constexpr Flags<TestFlags> kReadExec { TestFlags::Read, TestFlags::Exec };
static_assert(kReadExec.contains(TestFlags::Read));
static_assert(!kReadExec.contains(TestFlags::Write));
static_assert(kReadExec.any());
static_assert(!Flags<TestFlags> { }.any());
static_assert(Pair<int, int> { 1, 2 } == Pair<int, int> { 1, 2 });
static_assert(Iterable<Range<int>>);
static_assert(DoubleEndedIterator<Range<int>>);

TEST(Utility, Swap)
{
	int a { 1 };
	int b { 2 };
	swap(a, b);

	EXPECT_EQ(a, 2);
	EXPECT_EQ(b, 1);
}

TEST(IteratorAndRange, Pipelines)
{
	auto values { range(0, 5).collect<ArrayList>() };
	ASSERT_EQ(values.size(), 5u);
	EXPECT_EQ(values[0], 0);
	EXPECT_EQ(values[4], 4);

	auto reversed { range(0, 5).rev().collect<ArrayList>() };
	ASSERT_EQ(reversed.size(), 5u);
	EXPECT_EQ(reversed[0], 4);
	EXPECT_EQ(reversed[4], 0);

	EXPECT_TRUE(range(4).every([](int value) { return value < 4; }));
	EXPECT_TRUE(range(4).any([](int value) { return value == 3; }));
	EXPECT_TRUE(range(3).eq(range(3)));
	EXPECT_FALSE(range(3).eq(range(4)));
	EXPECT_EQ(Math::median3(9, 3, 5), 5);

	auto mapped {
		range(4).map([](int value) { return value + 1; }).collect<ArrayList>()
	};
	ASSERT_EQ(mapped.size(), 4u);
	EXPECT_EQ(mapped[0], 1);

	auto filtered {
		range(8)
		    .filter([](int value) { return value % 2 == 0; })
		    .collect<ArrayList>(),
	};
	ASSERT_EQ(filtered.size(), 4u);
	EXPECT_EQ(filtered[3], 6);

	auto enumerated { range(3).enumerate().collect<ArrayList>() };
	ASSERT_EQ(enumerated.size(), 3u);
	EXPECT_EQ(enumerated[1].first, 1u);
	EXPECT_EQ(enumerated[1].second, 1);

	std::vector<int> source { 1, 2, 3 };
	auto borrowed { borrow_iter(source) };
	auto first { borrowed.next() };
	ASSERT_TRUE(first);
	*first += 10;

	auto doubled {
		borrow_iter(source)
		    .map([](int &value) { return value * 2; })
		    .collect<ArrayList>(),
	};
	ASSERT_EQ(doubled.size(), 3u);
	EXPECT_EQ(doubled[0], 22);
	EXPECT_EQ(source[0], 11);
}

TEST(Containers, ArrayAndSpan)
{
	int raw[] { 1, 2, 3 };
	Array<int, 3> array_from_c_array(raw);
	Array<int, 3> array_from_init { { 4, 5, 6 } };

	EXPECT_EQ(array_from_c_array.size(), 3u);
	EXPECT_EQ(array_from_c_array[1], 2);
	EXPECT_EQ(array_from_c_array.last(), 3);
	EXPECT_EQ(array_from_init[2], 6);

	Span<int> view(raw, 3);
	EXPECT_FALSE(view.empty());
	EXPECT_EQ(view[0], 1);

	Span<int> owned(raw, 3);
	EXPECT_EQ(owned.size(), 3u);
	EXPECT_EQ(owned[1], 2);

	Span<int> copied = owned;
	EXPECT_EQ(copied[2], 3);

	Span<int> moved = std::move(copied);
	EXPECT_EQ(moved[0], 1);
	EXPECT_EQ(copied.size(), 3u);
}

TEST(Containers, ArrayListAndLinkedList)
{
	ArrayList<int> list;
	EXPECT_TRUE(list.is_empty());

	auto made_list { ArrayList<int>::make(4) };
	ASSERT_TRUE(made_list);
	EXPECT_GE(made_list.unwrap().capacity(), 4u);

	list.push(1);
	list.emplace(2);
	list.push(3);
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(list.last(), 3);
	ASSERT_TRUE(list.get(1));
	EXPECT_EQ(list.get(1).unwrap(), 2);

	list.remove_at(1);
	EXPECT_EQ(list.size(), 2u);
	EXPECT_EQ(list[1], 3);

	auto list_values { list.iter().collect<ArrayList>() };
	ASSERT_EQ(list_values.size(), 2u);
	EXPECT_EQ(list_values[0], 1);
	EXPECT_EQ(list_values[1], 3);

	auto prefix { list.span(1) };
	EXPECT_EQ(prefix.size(), 1u);
	EXPECT_EQ(prefix[0], 1);

	LinkedList<int> linked { 4, 5, 6 };
	linked.push(7);
	EXPECT_EQ(linked.size(), 4u);
	ASSERT_TRUE(linked.first());
	ASSERT_TRUE(linked.last());
	EXPECT_EQ(linked.first().unwrap(), 4);
	EXPECT_EQ(linked.last().unwrap(), 7);

	auto linked_values { linked.iter().collect<ArrayList>() };
	ASSERT_EQ(linked_values.size(), 4u);
	EXPECT_EQ(linked_values[0], 4);
	EXPECT_EQ(linked_values[3], 7);

	linked.pop();
	EXPECT_EQ(linked.size(), 3u);
	EXPECT_EQ(linked.last().unwrap(), 6);
}

TEST(Strings, ViewStringAndFixedString)
{
	StringView ascii { "hello" };
	EXPECT_EQ(ascii.size(), 5u);
	EXPECT_TRUE(ascii.starts_with(StringView("he")));
	EXPECT_TRUE(ascii.ends_with(StringView("lo")));
	EXPECT_TRUE(ascii.contains(StringView("ell")));
	EXPECT_EQ(ascii.find(StringView("ll")), 2u);
	EXPECT_EQ(ascii.substring(1, 3), StringView("ell"));

	auto ascii_bytes { ascii.iter_bytes().collect<ArrayList>() };
	ASSERT_EQ(ascii_bytes.size(), 5u);
	EXPECT_EQ(ascii_bytes[0], 'h');

	StringView utf8 {
		"a\xC3\xA9"
		"b",
	};
	EXPECT_EQ(utf8.size(), 4u);
	usize rune_count { 0 };
	auto rune_iter { utf8.iter() };
	auto first_rune { rune_iter.next() };
	ASSERT_TRUE(first_rune);
	EXPECT_EQ(static_cast<u32>(*first_rune), static_cast<u32>('a'));
	++rune_count;
	auto second_rune { rune_iter.next() };
	ASSERT_TRUE(second_rune);
	EXPECT_EQ(static_cast<u32>(*second_rune), 0xe9u);
	++rune_count;
	auto third_rune { rune_iter.next() };
	ASSERT_TRUE(third_rune);
	EXPECT_EQ(static_cast<u32>(*third_rune), static_cast<u32>('b'));
	++rune_count;
	EXPECT_EQ(rune_count, 3u);

	String text { "hello" };
	auto made_text { String::make("hello") };
	ASSERT_TRUE(made_text);
	EXPECT_EQ(made_text.unwrap(), StringView("hello"));
	text += StringView(" world");
	text += '!';
	EXPECT_EQ(text, StringView("hello world!"));
	EXPECT_EQ(text.size(), 12u);
	EXPECT_EQ(text.c_str()[text.size()], '\0');
	EXPECT_EQ(text.substring(6), StringView("world!"));

	FixedString<5> fixed { "hello world" };
	EXPECT_EQ(fixed.size(), 5u);
	EXPECT_EQ(fixed, StringView("hello"));
	fixed += '!';
	EXPECT_EQ(fixed, StringView("hello"));
}

TEST(Functional, OptionAndResult)
{
	Option<int> some { 42 };
	Option<int> none { };
	EXPECT_TRUE(some);
	EXPECT_FALSE(none);
	EXPECT_EQ(some.unwrap_or(7), 42);
	EXPECT_EQ(none.unwrap_or(7), 7);

	auto mapped { some.map([](int value) { return value + 1; }) };
	ASSERT_TRUE(mapped);
	EXPECT_EQ(mapped.unwrap(), 43);

	int value { 5 };
	Option<int &> ref { value };
	ref.unwrap() = 6;
	EXPECT_EQ(value, 6);

	auto ok { Result<int, String>::Ok(9) };
	auto err { Result<int, String>::Err(String("boom")) };
	EXPECT_TRUE(ok);
	EXPECT_FALSE(err);
	EXPECT_EQ(ok.unwrap(), 9);
	EXPECT_EQ(err.unwrap_err(), String("boom"));
	EXPECT_TRUE(ok.ok());
	EXPECT_FALSE(ok.err());
	EXPECT_EQ(ok.ok().unwrap(), 9);
	EXPECT_EQ(err.err().unwrap(), String("boom"));

	auto ok_void { Result<void, String>::Ok() };
	EXPECT_TRUE(ok_void);
	ok_void.unwrap();

	auto promoted { Option<int> { 7 }.ok_or<String>(String("missing")) };
	ASSERT_TRUE(promoted);
	EXPECT_EQ(promoted.unwrap(), 7);

	auto promoted_empty {
		Option<int> { }.ok_or_else<String>([] { return String("missing"); }),
	};
	ASSERT_TRUE(promoted_empty.is_err());
	EXPECT_EQ(promoted_empty.unwrap_err(), String("missing"));

	auto nested { Option<Result<int, String>> { Result<int, String>::Ok(3) } };
	auto transposed { transpose(nested) };
	ASSERT_TRUE(transposed);
	EXPECT_TRUE(transposed.unwrap());
	EXPECT_EQ(transposed.unwrap().unwrap(), 3);

	auto nested_err {
		Option<Result<int, String>> {
		    Result<int, String>::Err(String("nested")) },
	};
	auto transposed_err { transpose(std::move(nested_err)) };
	ASSERT_TRUE(transposed_err.is_err());
	EXPECT_EQ(transposed_err.unwrap_err(), String("nested"));
}

TEST(Functional, VariantAndError)
{
	Variant<int, String, double> variant { String("abc") };
	EXPECT_TRUE(variant.is<String>());
	auto got { variant.get<String>() };
	ASSERT_TRUE(got);
	EXPECT_EQ(got.unwrap(), String("abc"));

	int visited { -1 };
	variant.visit([&](auto const &value) {
		using Value = RemoveConstRef<decltype(value)>;
		if constexpr (SameAs<Value, String>) {
			visited = 1;
			EXPECT_EQ(value, StringView("abc"));
		} else if constexpr (SameAs<Value, double>) {
			visited = 2;
		} else {
			visited = 0;
		}
	});
	EXPECT_EQ(visited, 1);

	auto made { Variant<int, String, double>::make<0>(7) };
	EXPECT_TRUE(made.is<int>());
	EXPECT_EQ(made.get<int>().unwrap(), 7);

	auto display { ErasedError::from(ErrorsV::InvalidIndex { }) };
	EXPECT_EQ(display.message(), StringView("Invalid index"));

	auto duplicate { ErasedError::from(ErrorsV::HashMapDuplicateKeyError { }) };
	EXPECT_EQ(duplicate.message(), StringView("HashMap duplicate key"));

	auto popping { ErasedError::from(ErrorsV::PoppingEmptyList { }) };
	EXPECT_EQ(popping.message(), StringView("Popping empty list"));

	auto oom { ErasedError::from(ErrorsV::OutOfMemory { }) };
	EXPECT_EQ(oom.message(), StringView("Out of memory"));
}

TEST(Containers, HashMapBitArrayBitListAndQueue)
{
	HashMap<String, int> map { };
	EXPECT_TRUE(map.is_empty());

	auto inserted { map.insert(String("one"), 1) };
	ASSERT_TRUE(inserted);
	EXPECT_TRUE(map.contains(String("one")));
	ASSERT_TRUE(map.get(String("one")));
	EXPECT_EQ(map.get(String("one")).unwrap(), 1);

	auto duplicate { map.insert(String("one"), 2) };
	ASSERT_TRUE(duplicate.is_err());
	EXPECT_EQ(ErasedError::from(duplicate.unwrap_err()).message(),
	    StringView("HashMap duplicate key"));

	map.insert_or_replace(String("one"), 11);
	EXPECT_EQ(map.get(String("one")).unwrap(), 11);

	map.insert_or_replace(String("two"), 2);
	EXPECT_EQ(map.size(), 2u);

	auto removed { map.remove(String("one")) };
	ASSERT_TRUE(removed);
	EXPECT_EQ(removed.unwrap(), 11);
	EXPECT_FALSE(map.contains(String("one")));

	BitArray<16> bits { };
	EXPECT_FALSE(bits.test(3));
	bits.set(3);
	bits.toggle(4);
	bits.set_range(8, 3);
	EXPECT_TRUE(bits.test(3));
	EXPECT_TRUE(bits.test(4));
	EXPECT_TRUE(bits.test_range(8, 3));
	EXPECT_EQ(bits.find_first_set().unwrap(), 3u);
	EXPECT_EQ(bits.find_first_clear().unwrap(), 0u);
	bits.clear_range(8, 3);
	EXPECT_FALSE(bits.test_range(8, 3));
	bits.set_all();
	EXPECT_TRUE(bits.test_range(0, 16));
	bits.clear_all();
	EXPECT_FALSE(bits.test(3));

	BitList bit_list { };
	EXPECT_TRUE(bit_list.is_empty());
	bit_list.push(true);
	bit_list.push(false);
	bit_list.push(true);
	EXPECT_EQ(bit_list.size(), 3u);
	ASSERT_TRUE(bit_list.test(0));
	ASSERT_TRUE(bit_list.test(1));
	ASSERT_TRUE(bit_list.test(2));
	EXPECT_TRUE(bit_list.test(0).unwrap());
	EXPECT_FALSE(bit_list.test(1).unwrap());

	auto popped { bit_list.pop() };
	ASSERT_TRUE(popped);
	EXPECT_TRUE(popped.unwrap());
	EXPECT_EQ(bit_list.size(), 2u);

	auto bad_index { bit_list.set(99) };
	ASSERT_TRUE(bad_index.is_err());
	EXPECT_EQ(ErasedError::from(bad_index.unwrap_err()).message(),
	    StringView("Invalid index"));

	auto empty_pop { BitList { }.pop() };
	ASSERT_TRUE(empty_pop.is_err());
	EXPECT_EQ(ErasedError::from(empty_pop.unwrap_err()).message(),
	    StringView("Popping empty list"));

	MpscQueue<int, 8> queue { };
	queue.push_blocking(7);
	EXPECT_TRUE(queue.try_push(9));
	EXPECT_EQ(queue.try_pop().unwrap(), 7);
	EXPECT_EQ(queue.try_pop().unwrap(), 9);
	EXPECT_FALSE(queue.try_pop());

	queue.push_blocking(1);
	queue.push_blocking(2);
	int sum { 0 };
	queue.drain([&](int value) { sum += value; });
	EXPECT_EQ(sum, 3);
	EXPECT_FALSE(queue.try_pop());
}

TEST(Functional, PointersAtomicFunctionAndSync)
{
	Box<Pair<int, int>> boxed { 3, 4 };
	EXPECT_EQ(boxed->first, 3);
	EXPECT_EQ(boxed->second, 4);
	auto *raw = boxed.leak();
	EXPECT_EQ(raw->first, 3);
	EXPECT_EQ(raw->second, 4);
	delete raw;

	Rc<Pair<int, int>> shared { 5, 6 };
	auto shared_copy { shared };
	EXPECT_EQ(shared.ref_count(), 2u);
	EXPECT_EQ(shared_copy.ref_count(), 2u);
	auto shared_moved { std::move(shared_copy) };
	EXPECT_EQ(shared_moved.ref_count(), 2u);
	EXPECT_EQ(shared_copy.ref_count(), 0u);

	Arc<Pair<int, int>> atomic_shared { 7, 8 };
	auto atomic_copy { atomic_shared };
	EXPECT_EQ(atomic_shared.ref_count(), 2u);
	EXPECT_EQ(atomic_copy.ref_count(), 2u);
	auto atomic_moved { std::move(atomic_copy) };
	EXPECT_EQ(atomic_moved.ref_count(), 2u);
	EXPECT_EQ(atomic_copy.ref_count(), 0u);

	Atomic<int> atomic_value { 10 };
	EXPECT_EQ(atomic_value.load(), 10);
	EXPECT_EQ(atomic_value.exchange(12), 10);
	int expected { 12 };
	EXPECT_TRUE(atomic_value.compare_exchange(expected, 20));
	EXPECT_EQ(atomic_value.load(), 20);
	EXPECT_EQ(atomic_value.fetch_add(5), 20);
	EXPECT_EQ(atomic_value.load(), 25);
	EXPECT_EQ(atomic_value.fetch_sub(3), 25);
	EXPECT_EQ(atomic_value.load(), 22);

	Atomic<int> bitwise { 0b1100 };
	EXPECT_EQ(bitwise.fetch_or(0b0011), 0b1100);
	EXPECT_EQ(bitwise.load(), 0b1111);
	EXPECT_EQ(bitwise.fetch_and(0b1010), 0b1111);
	EXPECT_EQ(bitwise.load(), 0b1010);
	EXPECT_EQ(bitwise.fetch_xor(0b0110), 0b1010);
	EXPECT_EQ(bitwise.load(), 0b1100);

	Function<int(int)> doubler { [](int value) { return value * 2; } };
	EXPECT_TRUE(doubler);
	EXPECT_EQ(doubler(21), 42);
	auto copied { doubler };
	EXPECT_EQ(copied(3), 6);
	copied.clear();
	EXPECT_FALSE(copied);

	SpinLock lock;
	int counter { 0 };
	auto worker { [&]() {
		for (int i { 0 }; i < 1000; ++i) {
			ScopedSpinLock guard { lock };
			++counter;
		}
	} };

	std::thread t1(worker);
	std::thread t2(worker);
	t1.join();
	t2.join();
	EXPECT_EQ(counter, 2000);
}
