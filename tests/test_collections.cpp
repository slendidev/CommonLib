#include <gtest/gtest.h>

#include <vector>

import CommonLib;

using namespace CL;

namespace {
struct Tracker {
	int value { 0 };
	static inline int copies { 0 };
	static inline int moves { 0 };

	Tracker() = default;
	explicit Tracker(int v)
	    : value { v }
	{
	}
	Tracker(Tracker const &other)
	    : value { other.value }
	{
		++copies;
	}
	Tracker(Tracker &&other) noexcept
	    : value { other.value }
	{
		++moves;
		other.value = -1;
	}
	auto operator=(Tracker const &other) -> Tracker &
	{
		value = other.value;
		++copies;
		return *this;
	}
	auto operator=(Tracker &&other) noexcept -> Tracker &
	{
		value = other.value;
		++moves;
		other.value = -1;
		return *this;
	}
	friend auto operator==(Tracker const &, Tracker const &) -> bool = default;
};

auto reset_tracker() -> void
{
	Tracker::copies = 0;
	Tracker::moves = 0;
}

}

TEST(Collections, Array)
{
	int raw[] { 1, 2, 3 };
	Array<int, 3> a;
	EXPECT_EQ(a.size(), 3u);

	Array<int, 3> from_raw { raw };
	EXPECT_EQ(from_raw[0], 1);
	EXPECT_EQ(from_raw.last(), 3);

	Array<int, 3> from_list { { 4, 5, 6 } };
	EXPECT_EQ(from_list.get(1).unwrap(), 5);
	EXPECT_FALSE(from_list.get(99));

	Array<int, 3> copy { from_list };
	EXPECT_EQ(copy[2], 6);
	Array<int, 3> moved { std::move(copy) };
	EXPECT_EQ(moved[0], 4);

	from_list = from_raw;
	EXPECT_EQ(from_list[1], 2);
	from_raw = std::move(from_list);
	EXPECT_EQ(from_raw[2], 3);

	std::vector<int> values;
	for (auto v : raw)
		values.push_back(v);
	EXPECT_EQ(values.size(), 3u);
}

TEST(Collections, Span)
{
	int raw[] { 1, 2, 3 };
	Span<int> view { raw, 3 };
	EXPECT_FALSE(view.empty());
	EXPECT_EQ(view.size(), 3u);
	EXPECT_EQ(view.data(), raw);
	EXPECT_EQ(view[1], 2);

	auto iter { view.iter() };
	ASSERT_TRUE(iter.next());
	EXPECT_EQ(*iter.next_back(), 3);
	ASSERT_TRUE(iter.next());
	EXPECT_FALSE(iter.next());

	Span<int> copy { view };
	EXPECT_EQ(copy[0], 1);
	Span<int> moved { std::move(copy) };
	EXPECT_EQ(moved[2], 3);
	EXPECT_EQ(copy[1], 2);

	Span<int> assigned { };
	assigned = view;
	EXPECT_EQ(assigned[1], 2);
	Span<int> moved_source { raw, 3 };
	assigned = std::move(moved_source);
	EXPECT_EQ(assigned[0], 1);

	EXPECT_EQ(moved_source.size(), 3u);
	EXPECT_EQ(moved_source[2], 3);
}

TEST(Collections, ArrayList)
{
	reset_tracker();
	ArrayList<Tracker> list;
	EXPECT_TRUE(list.is_empty());
	EXPECT_EQ(list.size(), 0u);
	EXPECT_EQ(list.capacity(), 0u);

	list.reserve(2);
	EXPECT_GE(list.capacity(), 2u);

	list.emplace(1);
	list.push(Tracker { 2 });
	list.push(Tracker { 3 });
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(list.last().value, 3);
	EXPECT_TRUE(list.get(1));
	EXPECT_EQ(list.get(1).unwrap().value, 2);
	EXPECT_FALSE(list.get(99));

	auto span { list.span() };
	EXPECT_EQ(span.size(), 3u);
	EXPECT_EQ(span[0].value, 1);
	EXPECT_EQ(list.span(1).size(), 1u);
	EXPECT_EQ(list.span(1, 99).size(), 2u);
	EXPECT_EQ(list.span(99, 1).size(), 0u);

	auto iterated { list.iter().collect<ArrayList>() };
	EXPECT_EQ(iterated.size(), 3u);
	EXPECT_EQ(iterated[2].value, 3);

	list.remove_at(1);
	EXPECT_EQ(list.size(), 2u);
	EXPECT_EQ(list[1].value, 3);
	list.remove_at(99);
	EXPECT_EQ(list.size(), 2u);

	list.pop();
	EXPECT_EQ(list.size(), 1u);
	list.clear();
	EXPECT_TRUE(list.is_empty());

	ArrayList<Tracker> init {
		Tracker { 4 },
		Tracker { 5 },
	};
	EXPECT_EQ(init.size(), 2u);
	ArrayList<Tracker> copy { init };
	EXPECT_EQ(copy[0].value, 4);
	ArrayList<Tracker> moved { std::move(copy) };
	EXPECT_EQ(moved[1].value, 5);
	ArrayList<Tracker> assigned { };
	assigned = init;
	EXPECT_EQ(assigned.size(), 2u);
	assigned = std::move(init);
	EXPECT_EQ(assigned.size(), 2u);
}

TEST(Collections, LinkedList)
{
	LinkedList<int> list { };
	EXPECT_TRUE(list.is_empty());
	EXPECT_EQ(list.size(), 0u);
	EXPECT_FALSE(list.first());
	EXPECT_FALSE(list.last());

	list.emplace(1);
	list.push(2);
	list.push(3);
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(list.first().unwrap(), 1);
	EXPECT_EQ(list.last().unwrap(), 3);

	auto iterated { list.iter().collect<ArrayList>() };
	ASSERT_EQ(iterated.size(), 3u);
	EXPECT_EQ(iterated[0], 1);
	EXPECT_EQ(iterated[2], 3);

	list.pop();
	EXPECT_EQ(list.last().unwrap(), 2);
	list.pop();
	list.pop();
	EXPECT_TRUE(list.is_empty());
	list.pop();

	LinkedList<int> init { 4, 5, 6 };
	EXPECT_EQ(init.size(), 3u);
	LinkedList<int> copy { init };
	EXPECT_EQ(copy.first().unwrap(), 4);
	LinkedList<int> moved { std::move(copy) };
	EXPECT_EQ(moved.last().unwrap(), 6);
	LinkedList<int> assigned { };
	assigned = init;
	EXPECT_EQ(assigned.size(), 3u);
	assigned = std::move(init);
	EXPECT_EQ(assigned.size(), 3u);
}

TEST(Collections, RangeAndIterator)
{
	auto r { range(0, 5) };
	EXPECT_EQ(r.next().unwrap(), 0);
	EXPECT_EQ(r.next().unwrap(), 1);

	auto back { range(0, 5) };
	EXPECT_EQ(back.next_back().unwrap(), 4);
	EXPECT_EQ(back.next_back().unwrap(), 3);
	EXPECT_EQ(back.next_back().unwrap(), 2);
	EXPECT_EQ(back.next_back().unwrap(), 1);
	EXPECT_EQ(back.next_back().unwrap(), 0);
	EXPECT_FALSE(back.next_back());

	auto collected { range(5).collect<ArrayList>() };
	ASSERT_EQ(collected.size(), 5u);
	EXPECT_EQ(collected[4], 4);

	auto reversed { range(0, 4).rev().collect<ArrayList>() };
	ASSERT_EQ(reversed.size(), 4u);
	EXPECT_EQ(reversed[0], 3);

	auto filtered {
		range(10)
		    .filter([](int value) { return value % 3 == 0; })
		    .collect<ArrayList>(),
	};
	ASSERT_EQ(filtered.size(), 4u);
	EXPECT_EQ(filtered[3], 9);

	auto mapped {
		range(3)
		    .map([](int value) { return value * value; })
		    .collect<ArrayList>(),
	};
	ASSERT_EQ(mapped.size(), 3u);
	EXPECT_EQ(mapped[2], 4);

	auto enumerated { range(3).enumerate().collect<ArrayList>() };
	ASSERT_EQ(enumerated.size(), 3u);
	EXPECT_EQ(enumerated[0].first, 0u);
	EXPECT_EQ(enumerated[2].second, 2);

	EXPECT_TRUE(range(4).every([](int value) { return value < 4; }));
	EXPECT_TRUE(range(4).any([](int value) { return value == 2; }));
	EXPECT_TRUE(range(4).eq(range(4)));
	EXPECT_FALSE(range(4).eq(range(3)));

	std::vector<int> values { 1, 2, 3 };
	auto borrowed { borrow_iter(values.begin(), values.end()) };
	EXPECT_EQ(borrowed.next().unwrap(), 1);
	EXPECT_EQ(borrowed.next_back().unwrap(), 3);
	EXPECT_EQ(values.front(), 1);
}
