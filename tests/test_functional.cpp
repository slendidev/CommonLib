#include <gtest/gtest.h>

#include <thread>

import CommonLib;

using namespace CL;

TEST(Functional, OptionAllPublicApis)
{
	Option<int> empty { };
	Option<int> value { 42 };
	int backing { 7 };
	Option<int &> ref { backing };

	EXPECT_TRUE(empty.is_none());
	EXPECT_FALSE(value.is_none());
	EXPECT_TRUE(value.is_some());
	EXPECT_FALSE(empty.is_some());
	EXPECT_TRUE(value.is_some_and([](int v) { return v == 42; }));
	EXPECT_TRUE(empty.is_none_or([](int) { return false; }));
	EXPECT_EQ(value.unwrap_or_else([] { return 0; }), 42);
	EXPECT_EQ(empty.unwrap_or_else([] { return 5; }), 5);
	EXPECT_EQ(value.unwrap_or(1), 42);
	EXPECT_EQ(empty.unwrap_or(1), 1);
	EXPECT_EQ(value.unwrap(), 42);
	EXPECT_EQ(*value, 42);
	EXPECT_EQ(ref.unwrap(), 7);
	ref.unwrap() = 8;
	EXPECT_EQ(backing, 8);

	auto mapped { value.map([](int v) { return v + 1; }) };
	ASSERT_TRUE(mapped);
	EXPECT_EQ(mapped.unwrap(), 43);

	auto anded { value.and_then([](int v) { return Option<int> { v * 2 }; }) };
	ASSERT_TRUE(anded);
	EXPECT_EQ(anded.unwrap(), 84);

	bool inspected { false };
	value.inspect([&](int v) { inspected = (v == 42); });
	EXPECT_TRUE(inspected);

	auto ok { empty.ok_or<String>(String("missing")) };
	ASSERT_TRUE(ok.is_err());
	EXPECT_EQ(ok.unwrap_err(), String("missing"));

	auto ok2 { value.ok_or_else<String>([] { return String("missing"); }) };
	ASSERT_TRUE(ok2.is_ok());
	EXPECT_EQ(ok2.unwrap(), 42);

	auto fallback { empty.or_else([] { return Option<int> { 9 }; }) };
	ASSERT_TRUE(fallback);
	EXPECT_EQ(fallback.unwrap(), 9);

	auto assigned { value };
	assigned.reset();
	EXPECT_FALSE(assigned);
	assigned.emplace(11);
	EXPECT_EQ(assigned.unwrap(), 11);
	assigned = value;
	EXPECT_EQ(assigned.unwrap(), 42);
	assigned = std::move(value);
	EXPECT_EQ(assigned.unwrap(), 42);
}

TEST(Functional, ResultAllPublicApis)
{
	auto ok { Result<int, String>::Ok(7) };
	auto err { Result<int, String>::Err(String("bad")) };

	EXPECT_TRUE(ok.is_ok());
	EXPECT_FALSE(ok.is_err());
	EXPECT_FALSE(err.is_ok());
	EXPECT_TRUE(err.is_err());
	EXPECT_TRUE(ok.is_ok_and([](int v) { return v == 7; }));
	EXPECT_TRUE(
	    err.is_err_and([](String const &v) { return v == String("bad"); }));
	EXPECT_EQ(ok.unwrap(), 7);
	EXPECT_EQ(*ok, 7);
	EXPECT_EQ(ok.ok().unwrap(), 7);
	EXPECT_FALSE(ok.err());
	EXPECT_TRUE(err.err());
	EXPECT_EQ(err.err().unwrap(), String("bad"));
	EXPECT_EQ(err.unwrap_err(), String("bad"));
	EXPECT_EQ(ok.unwrap_or(1), 7);
	EXPECT_EQ(err.unwrap_or(1), 1);
	EXPECT_EQ(ok.unwrap_or_else([](String const &) { return 0; }), 7);
	EXPECT_EQ(err.unwrap_or_else([](String const &) { return 5; }), 5);

	auto mapped { ok.map([](int v) { return v + 1; }) };
	EXPECT_EQ(mapped.unwrap(), 8);
	auto err_mapped { err.map_err([](String const &s) { return s.size(); }) };
	EXPECT_EQ(err_mapped.unwrap_err(), 3u);

	auto chained {
		ok.and_then([](int v) {
		    return Result<String, String>::Ok(String(v == 7 ? "yes" : "no"));
		}),
	};
	EXPECT_EQ(chained.unwrap(), String("yes"));

	bool seen_ok { false };
	bool seen_err { false };
	ok.inspect([&](int v) {
		  seen_ok = (v == 7);
	  }).inspect_err([&](String const &) { seen_err = true; });
	err.inspect([&](int) { seen_ok = false; })
	    .inspect_err([&](String const &v) { seen_err = (v == String("bad")); });
	EXPECT_TRUE(seen_ok);
	EXPECT_TRUE(seen_err);

	auto assigned { ok };
	assigned = err;
	EXPECT_TRUE(assigned.is_err());
	assigned = Result<int, String>::Ok(99);
	EXPECT_EQ(assigned.unwrap(), 99);
	assigned = std::move(ok);
	EXPECT_EQ(assigned.unwrap(), 7);
	EXPECT_TRUE(std::move(err).err());

	auto void_ok { Result<void, String>::Ok() };
	auto void_err { Result<void, String>::Err(String("oops")) };
	EXPECT_TRUE(void_ok.is_ok());
	EXPECT_TRUE(void_ok);
	void_ok.unwrap();
	EXPECT_TRUE(void_err.is_err());
	EXPECT_EQ(void_err.unwrap_err(), String("oops"));
	EXPECT_TRUE(void_err.err());

	auto transpose_src { Result<Option<int>, String>::Ok(Option<int> { 5 }) };
	auto transposed { transpose(std::move(transpose_src)) };
	EXPECT_TRUE(transposed);
	EXPECT_EQ(transposed.unwrap().unwrap(), 5);
}

TEST(Functional, VariantAndErrorAllPublicApis)
{
	Variant<int, String, double> v { 1 };
	EXPECT_TRUE(v.is<int>());
	EXPECT_EQ(v.tag(), 0);
	EXPECT_EQ(v.get<0>().unwrap(), 1);
	EXPECT_EQ(v.get<int>().unwrap(), 1);

	auto v2 { Variant<int, String, double>::make<1>(String("abc")) };
	EXPECT_TRUE(v2.is<String>());
	EXPECT_EQ(v2.get<String>().unwrap(), String("abc"));

	auto const cv { v2 };
	EXPECT_EQ(cv.get<1>().unwrap(), String("abc"));
	EXPECT_EQ(cv.get<String>().unwrap(), String("abc"));

	auto mapped { v2.map([](auto &value) {
		using Value = RemoveConstRef<decltype(value)>;
		if constexpr (SameAs<Value, String>)
			return value.size();
		else
			return static_cast<usize>(value);
	}) };
	EXPECT_EQ(mapped, 3u);
	bool inspected { false };
	v2.inspect([&](auto const &) { inspected = true; });
	EXPECT_TRUE(inspected);

	auto moved { std::move(v2) };
	EXPECT_TRUE(moved.is<String>());
	EXPECT_EQ(moved.get<String>().unwrap(), String("abc"));

	auto from_tag { Variant<int, String, double>::from_tag_unsafe(2) };
	EXPECT_TRUE(from_tag.is<double>());
	EXPECT_EQ(from_tag.tag(), 2);

	Variant<int, String, double> copied { moved };
	EXPECT_TRUE(copied.is<String>());
	Variant<int, String, double> assigned
	    = Variant<int, String, double>::make<0>(0);
	assigned = copied;
	EXPECT_TRUE(assigned.is<String>());
	assigned = std::move(copied);
	EXPECT_TRUE(assigned.is<String>());

	auto err { ErrorsV::InvalidIndex { } };
	EXPECT_EQ(ErasedError::from(err).message(), StringView("Invalid index"));
	EXPECT_EQ(ErasedError::msg(StringView("hey")).message(), StringView("hey"));

	Errors variant_err = ErrorsV::HashMapDuplicateKeyError { };
	EXPECT_EQ(ErasedError::from(variant_err).message(),
	    StringView("HashMap duplicate key"));

	Fallible<int> fallible {
		Result<int, ErasedError>::Err(ErasedError::msg("boom")),
	};
	EXPECT_TRUE(fallible.is_err());
}

TEST(Functional, BoxRcArcAtomicSyncFunction)
{
	Box<Pair<int, int>> box { 3, 4 };
	EXPECT_EQ(box->first, 3);
	EXPECT_EQ((*box).second, 4);
	EXPECT_EQ(box.get()->first, 3);
	auto *leaked = box.leak();
	EXPECT_EQ(leaked->first, 3);
	delete leaked;

	Rc<Pair<int, int>> rc { 5, 6 };
	EXPECT_EQ(rc.ref_count(), 1u);
	Rc<Pair<int, int>> rc2 { rc };
	EXPECT_EQ(rc.ref_count(), 2u);
	EXPECT_EQ(rc2.ref_count(), 2u);
	rc2 = rc;
	EXPECT_EQ(rc.ref_count(), 2u);
	Rc<Pair<int, int>> rc3 { std::move(rc2) };
	EXPECT_EQ(rc3.ref_count(), 2u);
	Rc<Pair<int, int>> rc4 { 9, 10 };
	rc3 = std::move(rc4);
	EXPECT_EQ(rc3.ref_count(), 1u);
	EXPECT_EQ(rc4.ref_count(), 0u);
	EXPECT_EQ(rc3.get()->first, 9);

	Arc<Pair<int, int>> arc { 7, 8 };
	EXPECT_EQ(arc.ref_count(), 1u);
	Arc<Pair<int, int>> arc2 { arc };
	EXPECT_EQ(arc.ref_count(), 2u);
	EXPECT_EQ(arc2.ref_count(), 2u);
	Arc<Pair<int, int>> arc3 { std::move(arc2) };
	EXPECT_EQ(arc3.ref_count(), 2u);
	Arc<Pair<int, int>> arc4 { 11, 12 };
	arc3 = std::move(arc4);
	EXPECT_EQ(arc3.ref_count(), 1u);
	EXPECT_EQ(arc4.ref_count(), 0u);

	Atomic<int> atomic { 10 };
	EXPECT_EQ(static_cast<int>(atomic), 10);
	EXPECT_EQ(atomic.load(MemoryOrder::Relaxed), 10);
	atomic.store(12, MemoryOrder::Release);
	EXPECT_EQ(atomic.load(), 12);
	EXPECT_EQ(atomic.exchange(13), 12);
	int expected { 13 };
	EXPECT_TRUE(atomic.compare_exchange(expected, 14));
	expected = 99;
	EXPECT_FALSE(atomic.compare_exchange(expected, 15));
	EXPECT_EQ(expected, 14);
	EXPECT_EQ(atomic.fetch_add(2), 14);
	EXPECT_EQ(atomic.fetch_sub(1), 16);
	EXPECT_EQ(atomic.fetch_or(0b0100), 15);
	EXPECT_EQ(atomic.fetch_and(0b0111), 0b1111);
	EXPECT_EQ(atomic.fetch_xor(0b0011), 0b0111);
	atomic = 77;
	EXPECT_EQ(atomic.load(), 77);

	SpinLock lock;
	lock.lock();
	lock.unlock();

	int count { 0 };
	auto worker { [&]() {
		for (int i { 0 }; i < 500; ++i) {
			ScopedSpinLock guard { lock };
			++count;
		}
	} };
	std::thread t1(worker);
	std::thread t2(worker);
	t1.join();
	t2.join();
	EXPECT_EQ(count, 1000);

	Function<int(int)> fn { [](int v) { return v * 2; } };
	EXPECT_TRUE(fn);
	EXPECT_EQ(fn(3), 6);
	fn.clear();
	EXPECT_FALSE(fn);
	fn = Function<int(int)> { [](int v) { return v + 1; } };
	EXPECT_EQ(fn(4), 5);
	fn = nullptr;
	EXPECT_FALSE(fn);

	Function<int(int) const> cfn { [](int v) { return v + 2; } };
	EXPECT_EQ(cfn(3), 5);
	Function<int(int) noexcept> nfn { [](int v) noexcept { return v - 1; } };
	EXPECT_EQ(nfn(3), 2);
	Function<int(int) &> lfn { [](int v) { return v + 3; } };
	EXPECT_EQ(lfn(3), 6);
	Function<int(int) &&> rfn { [](int v) { return v + 4; } };
	EXPECT_EQ(std::move(rfn)(3), 7);
	Function<int(int) const &> clf { [](int v) { return v + 5; } };
	EXPECT_EQ(clf(3), 8);
	Function<int(int) const &&> crf { [](int v) { return v + 6; } };
	EXPECT_EQ(std::move(crf)(3), 9);
	Function<int(int) const noexcept> cnf { [](int v) noexcept {
		return v + 7;
	} };
	EXPECT_EQ(cnf(3), 10);
	Function<int(int) const & noexcept> clnf { [](int v) noexcept {
		return v + 8;
	} };
	EXPECT_EQ(clnf(3), 11);
	Function<int(int) const && noexcept> crnf { [](int v) noexcept {
		return v + 9;
	} };
	EXPECT_EQ(std::move(crnf)(3), 12);
}
