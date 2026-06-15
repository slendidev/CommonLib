#include <gtest/gtest.h>

import CommonLib;

using namespace CL;

TEST(BitsAndQueue, BitArrayAllPublicApis)
{
	BitArray<16> bits { };
	EXPECT_EQ(bits.size(), 16u);
	EXPECT_EQ(bits.words() != nullptr, true);
	EXPECT_FALSE(bits.test(0));
	EXPECT_FALSE(bits.test(99));

	bits.set(0);
	bits.set(3);
	bits.set(15);
	EXPECT_TRUE(bits.test(0));
	EXPECT_TRUE(bits.test(3));
	EXPECT_TRUE(bits.test(15));

	bits.clear(3);
	EXPECT_FALSE(bits.test(3));
	bits.toggle(3);
	EXPECT_TRUE(bits.test(3));
	bits.toggle(3);
	EXPECT_FALSE(bits.test(3));

	bits.set_range(4, 4);
	EXPECT_TRUE(bits.test_range(4, 4));
	bits.clear_range(5, 2);
	EXPECT_FALSE(bits.test_range(4, 4));
	bits.toggle_range(4, 4);
	EXPECT_FALSE(bits.test(4));
	EXPECT_TRUE(bits.test(5));
	EXPECT_TRUE(bits.test(6));
	EXPECT_FALSE(bits.test(7));

	EXPECT_EQ(bits.find_first_set().unwrap(), 0u);
	EXPECT_EQ(bits.find_first_clear().unwrap(), 1u);
	bits.clear_all();
	EXPECT_FALSE(bits.find_first_set());
	EXPECT_EQ(bits.find_first_clear().unwrap(), 0u);
	bits.set_all();
	EXPECT_TRUE(bits.test_range(0, 16));
	EXPECT_FALSE(bits.find_first_clear());
	bits.clear(15);
	EXPECT_EQ(bits.find_first_clear().unwrap(), 15u);
}

TEST(BitsAndQueue, BitListAllPublicApis)
{
	BitList bits;
	EXPECT_TRUE(bits.is_empty());
	EXPECT_EQ(bits.size(), 0u);
	EXPECT_FALSE(bits.test(0));
	EXPECT_TRUE(bits.set(0).is_err());
	EXPECT_TRUE(bits.clear(0).is_err());
	EXPECT_TRUE(bits.toggle(0).is_err());

	for (int i { 0 }; i < 10; ++i)
		bits.push((i % 2) == 0);
	EXPECT_EQ(bits.size(), 10u);
	EXPECT_FALSE(bits.is_empty());
	EXPECT_TRUE(bits.set(0).is_ok());
	EXPECT_TRUE(bits.test(0).unwrap());
	EXPECT_TRUE(bits.clear(0).is_ok());
	EXPECT_FALSE(bits.test(0).unwrap());
	EXPECT_TRUE(bits.toggle(0).is_ok());
	EXPECT_TRUE(bits.test(0).unwrap());
	EXPECT_TRUE(bits.test(8).unwrap());
	EXPECT_FALSE(bits.test(9).unwrap());

	bits.reserve(100);
	EXPECT_EQ(bits.size(), 10u);

	bits.set_all();
	EXPECT_TRUE(bits.test(1).unwrap());
	bits.clear_all();
	EXPECT_FALSE(bits.test(1).unwrap());

	auto popped { bits.pop() };
	ASSERT_TRUE(popped);
	EXPECT_FALSE(popped.unwrap());

	auto error { BitList { }.pop() };
	EXPECT_TRUE(error.is_err());
	EXPECT_EQ(ErasedError::from(error.unwrap_err()).message(),
	    StringView("Popping empty list"));
	EXPECT_TRUE(BitList { }.test(99).is_err());
	EXPECT_EQ(ErasedError::from(BitList { }.test(99).unwrap_err()).message(),
	    StringView("Invalid index"));
	EXPECT_TRUE(BitList { }.set(99).is_err());
	EXPECT_TRUE(BitList { }.clear(99).is_err());
	EXPECT_TRUE(BitList { }.toggle(99).is_err());
}

TEST(BitsAndQueue, HashMapAllPublicApis)
{
	HashMap<String, int> map { };
	EXPECT_TRUE(map.is_empty());
	EXPECT_EQ(map.size(), 0u);
	EXPECT_FALSE(map.contains(String("one")));
	EXPECT_FALSE(map.get(String("one")));

	for (int i { 0 }; i < 7; ++i) {
		String key { "k" };
		key += static_cast<char>('0' + i);
		EXPECT_TRUE(map.insert(key, i).is_ok());
	}
	EXPECT_EQ(map.size(), 7u);
	for (int i { 0 }; i < 7; ++i) {
		String key { "k" };
		key += static_cast<char>('0' + i);
		ASSERT_TRUE(map.get(key));
		EXPECT_EQ(map.get(key).unwrap(), i);
	}
	EXPECT_TRUE(map.insert(String("k0"), 99).is_err());

	HashMap<String, int> other {
		{ String("one"), 1 },
		{ String("two"), 2 },
		{ String("three"), 3 },
	};
	EXPECT_EQ(other.size(), 3u);
	EXPECT_EQ(other.get(String("one")).unwrap(), 1);
	EXPECT_EQ(other.get(String("two")).unwrap(), 2);
	EXPECT_EQ(other.get(String("three")).unwrap(), 3);
	EXPECT_TRUE(other.contains(String("two")));

	auto duplicate { other.insert(String("two"), 22) };
	EXPECT_TRUE(duplicate.is_err());
	other.insert_or_replace(String("two"), 22);
	EXPECT_EQ(other.get(String("two")).unwrap(), 22);

	auto removed { other.remove(String("one")) };
	ASSERT_TRUE(removed);
	EXPECT_EQ(removed.unwrap(), 1);
	EXPECT_FALSE(other.contains(String("one")));
	EXPECT_FALSE(other.remove(String("missing")));

	usize count { 0 };
	auto iter { other.iter() };
	while (auto entry { iter.next() }) {
		EXPECT_FALSE(entry->key.is_empty());
		++count;
	}
	EXPECT_EQ(count, other.size());

	auto const const_other { other };
	count = 0;
	auto citer { const_other.iter() };
	while (auto entry { citer.next() }) {
		EXPECT_FALSE(entry->key.is_empty());
		++count;
	}
	EXPECT_EQ(count, const_other.size());

	other.clear();
	EXPECT_TRUE(other.is_empty());
}

TEST(BitsAndQueue, QueueAllPublicApis)
{
	MpscQueue<int, 8> queue { };
	queue.init();
	EXPECT_FALSE(queue.try_pop());

	for (int i { 0 }; i < 8; ++i)
		EXPECT_TRUE(queue.try_push(i));
	EXPECT_FALSE(queue.try_push(99));

	for (int i { 0 }; i < 8; ++i) {
		auto value { queue.try_pop() };
		ASSERT_TRUE(value);
		EXPECT_EQ(*value, i);
	}
	EXPECT_FALSE(queue.try_pop());

	queue.push_blocking(10);
	queue.push_blocking(11);
	int sum { 0 };
	queue.drain([&](int v) { sum += v; });
	EXPECT_EQ(sum, 21);
	EXPECT_FALSE(queue.try_pop());
	queue.init();
	EXPECT_FALSE(queue.try_pop());
}
