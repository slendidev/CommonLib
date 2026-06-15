#include <gtest/gtest.h>

import CommonLib;

using namespace CL;

TEST(Strings, StringViewConstructionAndSearch)
{
	StringView empty;
	EXPECT_TRUE(empty.is_empty());
	EXPECT_EQ(empty.size(), 0u);

	StringView text { "hello world" };
	EXPECT_FALSE(text.is_empty());
	EXPECT_EQ(text.size(), 11u);
	EXPECT_EQ(text.data()[0], 'h');
	EXPECT_TRUE(text.starts_with(StringView("he")));
	EXPECT_TRUE(text.ends_with(StringView("ld")));
	EXPECT_TRUE(text.contains(StringView("lo wo")));
	EXPECT_EQ(text.find(StringView("world")), 6u);
	EXPECT_EQ(text.find(StringView("missing")), StringView::npos);
	EXPECT_EQ(text.find(StringView("")), 0u);
	EXPECT_EQ(text.substring(6), StringView("world"));
	EXPECT_EQ(text.substring(6, 3), StringView("wor"));
	EXPECT_EQ(text.substring(99), StringView(""));
	EXPECT_EQ(text, StringView("hello world"));
	EXPECT_NE(text, StringView("hello"));
}

TEST(Strings, StringViewUtf8Iteration)
{
	StringView text {
		"a\xC3\xA9"
		"b",
	};
	auto bytes { text.iter_bytes() };
	ASSERT_TRUE(bytes.next());
	EXPECT_EQ(*bytes.next_back(), 'b');

	auto runes { text.iter() };
	ASSERT_TRUE(runes.next());
	EXPECT_EQ(static_cast<u32>(*runes.next()), 0xe9u);
	EXPECT_EQ(static_cast<u32>(*runes.next()), static_cast<u32>('b'));
	EXPECT_FALSE(runes.next());

	auto reverse { text.iter() };
	ASSERT_TRUE(reverse.next_back());
	EXPECT_EQ(static_cast<u32>(*reverse.next_back()), 0xe9u);
	EXPECT_EQ(static_cast<u32>(*reverse.next_back()), static_cast<u32>('a'));
	EXPECT_FALSE(reverse.next_back());
}

TEST(Strings, StringConstructionMutationAndViews)
{
	String empty;
	EXPECT_EQ(empty.size(), 0u);
	EXPECT_EQ(empty.c_str()[0], '\0');

	String text { "hello" };
	EXPECT_EQ(text.size(), 5u);
	EXPECT_EQ(text.capacity() >= text.size(), true);
	EXPECT_EQ(text[1], 'e');
	EXPECT_EQ(text.view(), StringView("hello"));

	String copy { text };
	EXPECT_EQ(copy, StringView("hello"));
	String moved { std::move(copy) };
	EXPECT_EQ(moved, StringView("hello"));

	text.append('!');
	EXPECT_EQ(text, StringView("hello!"));
	text.append(StringView(" world"));
	EXPECT_EQ(text, StringView("hello! world"));
	text += '?';
	text += StringView(" yes");
	EXPECT_EQ(text, StringView("hello! world? yes"));
	EXPECT_EQ(text.substring(7), StringView("world? yes"));
	auto span { text.span(1, 4) };
	EXPECT_EQ(span.size(), 4u);
	EXPECT_EQ(span[0], 'e');
	EXPECT_EQ(span[3], 'o');

	auto bytes { text.iter_bytes().collect<ArrayList>() };
	ASSERT_EQ(bytes.size(), text.size());
	EXPECT_EQ(bytes[0], 'h');

	usize rune_count { 0 };
	text.iter().for_each([&](Rune) { ++rune_count; });
	EXPECT_EQ(rune_count, text.size());

	text.clear();
	EXPECT_EQ(text.size(), 0u);
	EXPECT_EQ(text.c_str()[0], '\0');
}

TEST(Strings, FixedStringTruncationAndAppend)
{
	FixedString<5> text;
	EXPECT_EQ(text.capacity(), 5u);
	EXPECT_EQ(text.size(), 0u);
	EXPECT_EQ(text.c_str()[0], '\0');

	text = FixedString<5>("hello world");
	EXPECT_EQ(text.size(), 5u);
	EXPECT_EQ(text, StringView("hello"));
	EXPECT_EQ(text[4], 'o');

	text.append('!');
	EXPECT_EQ(text, StringView("hello"));
	text.append(StringView(" world"));
	EXPECT_EQ(text, StringView("hello"));

	text.clear();
	EXPECT_TRUE(text.is_empty());
	text += 'a';
	text += StringView("bcdefg");
	EXPECT_EQ(text, StringView("abcde"));
	EXPECT_EQ(text.substring(1, 2), StringView("bc"));
	auto fixed_span { text.span(2, 3) };
	EXPECT_EQ(fixed_span.size(), 3u);
	EXPECT_EQ(fixed_span[0], 'c');
	EXPECT_EQ(fixed_span[2], 'e');
	EXPECT_EQ(text.view(), StringView("abcde"));

	FixedString<5> copy { text };
	EXPECT_EQ(copy, StringView("abcde"));
	FixedString<5> moved { std::move(copy) };
	EXPECT_EQ(moved, StringView("abcde"));
	EXPECT_EQ(copy.size(), 5u);
}
