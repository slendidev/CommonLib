#include <gtest/gtest.h>

#include <initializer_list>
#include <type_traits>

import CommonLib;

using namespace CL;

enum class TraitFlags : u8 {
	A = 1 << 0,
	B = 1 << 1,
	C = 1 << 2,
};

static_assert(SameAs<u8, unsigned char>);
static_assert(SameAs<u16, unsigned short>);
static_assert(SameAs<u32, unsigned int>);
static_assert(SameAs<u64, unsigned long>);
static_assert(SameAs<i8, signed char>);
static_assert(SameAs<i16, short>);
static_assert(SameAs<i32, int>);
static_assert(SameAs<i64, long>);
static_assert(SameAs<f32, float>);
static_assert(SameAs<f64, double>);
static_assert(SameAs<usize, decltype(sizeof(0))>);
static_assert(SameAs<nullptr_t, decltype(nullptr)>);

static_assert(SameAs<RemoveReferenceT<int &>, int>);
static_assert(SameAs<RemoveReferenceT<int &&>, int>);
static_assert(SameAs<RemoveConstT<int const>, int>);
static_assert(SameAs<RemoveConstRef<int const &>, int>);
static_assert(SameAs<AddConstT<int>, int const>);
static_assert(IsReferenceV<int &>);
static_assert(IsReferenceV<int &&>);
static_assert(!IsReferenceV<int>);
static_assert(IsIntegralV<bool>);
static_assert(IsIntegralV<char>);
static_assert(IsIntegralV<unsigned long long>);
static_assert(!IsIntegralV<float>);
static_assert(IsFloatingPointV<float>);
static_assert(IsFloatingPointV<double>);
static_assert(IsFloatingPointV<long double>);
static_assert(IsEnumV<TraitFlags>);
static_assert(IsUnsignedIntegralV<u32>);
static_assert(!IsUnsignedIntegralV<int>);
static_assert(IsSignedIntegralV<int>);
static_assert(!IsSignedIntegralV<u32>);
static_assert(SameAs<TypeIdentityT<long>, long>);
static_assert(SameAs<InvokeResultT<int (*)(int), int>, int>);
static_assert(
    SameAs<InvokeResultT<decltype(+[](int value) { return value + 1; }), int>,
        int>);
static_assert(SameAs<IndexSequence<0, 1, 2>, MakeIndexSequence<3>>);
static_assert(SameAs<InitializerList<int>, std::initializer_list<int>>);
static_assert(SameAs<TypeListAtT<0, TypeList<int, char, bool>>, int>);
static_assert(SameAs<TypeListAtT<1, TypeList<int, char, bool>>, char>);
static_assert(SameAs<TypeListAtT<2, TypeList<int, char, bool>>, bool>);
static_assert(TypeListSizeV<TypeList<>> == 0);
static_assert(TypeListSizeV<TypeList<int, char, bool>> == 3);
static_assert(IsTypeListV<TypeList<int>>);
static_assert(!IsTypeListV<int>);
static_assert(SameAs<decltype(declval<int &>()), int &>);
static_assert(SameAs<decltype(declval<int &&>()), int &&>);
static_assert(OneOf<int, char, int, double>);
static_assert(!OneOf<long, char, int, double>);
static_assert(SameAs<Pair<int, char>, Pair<int, char>>);
static_assert(Pair<int, int> { 1, 2 } == Pair<int, int> { 1, 2 });
static_assert(SameAs<decltype(Flags<TraitFlags> { TraitFlags::A }.raw()),
    Flags<TraitFlags>::Underlying>);
static_assert(Flags<TraitFlags> { TraitFlags::A }.contains(TraitFlags::A));
static_assert(!Flags<TraitFlags> { TraitFlags::A }.contains(TraitFlags::B));

TEST(Traits, UtilityHelpers)
{
	int a { 1 };
	int b { 2 };
	EXPECT_EQ(to_hash(7u), 7u);
	EXPECT_EQ(to_hash(7), static_cast<usize>(7));
	EXPECT_EQ(to_hash(StringView("abc")), to_hash(String("abc")));
	EXPECT_EQ(to_display_string("abc"), String("abc"));
	EXPECT_EQ(to_debug_string(StringView("abc")), String("abc"));
	swap(a, b);
	EXPECT_EQ(a, 2);
	EXPECT_EQ(b, 1);

	auto moved { move(a) };
	EXPECT_EQ(moved, 2);
	ignore_unused(moved, a, b);
}

TEST(Traits, MathMemoryFlags)
{
	EXPECT_EQ(Math::ceil(9), 9);
	EXPECT_EQ(Math::ceil(3.1), 4.0);
	EXPECT_EQ(Math::trunc(3.9), 3.0);
	EXPECT_EQ(Math::div_ceil(7, 3), 3);
	EXPECT_EQ(Math::div_ceil(-7, 3), -2);
	EXPECT_EQ(Math::median3(9, 3, 5), 5);

	EXPECT_EQ(Memory::align_down<usize>(37, 8), 32u);
	EXPECT_EQ(Memory::align_up<usize>(37, 8), 40u);
	EXPECT_TRUE(Memory::ranges_overlap<usize>(0, 8, 4, 8));
	EXPECT_FALSE(Memory::ranges_overlap<usize>(0, 4, 4, 4));
	EXPECT_TRUE(Memory::range_contains<usize>(0, 16, 4, 4));
	EXPECT_FALSE(Memory::range_contains<usize>(0, 4, 4, 4));

	Flags<TraitFlags> flags { TraitFlags::A, TraitFlags::C };
	EXPECT_TRUE(flags.any());
	EXPECT_FALSE(flags.none());
	EXPECT_TRUE(flags.contains(TraitFlags::A));
	EXPECT_FALSE(flags.contains(TraitFlags::B));
	EXPECT_TRUE(
	    flags.contains_any(Flags<TraitFlags> { TraitFlags::B, TraitFlags::C }));
	EXPECT_TRUE(
	    flags.contains_all(Flags<TraitFlags> { TraitFlags::A, TraitFlags::C }));

	flags |= Flags<TraitFlags> { TraitFlags::B };
	EXPECT_TRUE(flags.contains(TraitFlags::B));
	flags &= Flags<TraitFlags> { TraitFlags::A, TraitFlags::B };
	EXPECT_TRUE(flags.contains(TraitFlags::A));
	EXPECT_TRUE(flags.contains(TraitFlags::B));
	EXPECT_FALSE(flags.contains(TraitFlags::C));
	flags ^= Flags<TraitFlags> { TraitFlags::A };
	EXPECT_FALSE(flags.contains(TraitFlags::A));
	flags += Flags<TraitFlags> { TraitFlags::C };
	EXPECT_TRUE(flags.contains(TraitFlags::C));
	flags -= Flags<TraitFlags> { TraitFlags::C };
	EXPECT_FALSE(flags.contains(TraitFlags::C));
	EXPECT_EQ((Flags<TraitFlags> { TraitFlags::A } | TraitFlags::B).raw(), 3u);
	EXPECT_EQ((Flags<TraitFlags> { TraitFlags::A } & TraitFlags::A).raw(), 1u);
	EXPECT_EQ((Flags<TraitFlags> { TraitFlags::A } ^ TraitFlags::A).raw(), 0u);
	EXPECT_TRUE(Flags<TraitFlags>::all().any());
}
