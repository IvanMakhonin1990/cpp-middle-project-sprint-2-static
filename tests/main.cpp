#include <cstdint>
#include <iostream>
#include <string_view>

#include <gtest/gtest.h>

#include "scan.hpp"

using namespace stdx;
using namespace stdx::details;

// ============================================================================
// Тесты для fixed_string
// ============================================================================

TEST(FixedStringTest, ExactSizeInitialization) {
    constexpr fixed_string<6> fs("hello");
    EXPECT_EQ(fs.size(), 5);
    EXPECT_EQ(fs[0], 'h');
    EXPECT_EQ(fs[4], 'o');
}

TEST(FixedStringTest, SmallerInitialization) {
    constexpr fixed_string<10> fs("hi");
    EXPECT_EQ(fs.size(), 2);
    EXPECT_EQ(fs[0], 'h');
    EXPECT_EQ(fs[1], 'i');
    EXPECT_EQ(fs[2], '\0');
}

TEST(FixedStringTest, InitializationFromPointers) {
    constexpr const char *str = "test";
    constexpr fixed_string<10> fs(str, str + 4);
    EXPECT_EQ(fs.size(), 4);
    EXPECT_EQ(fs[0], 't');
    EXPECT_EQ(fs[3], 't');
}

TEST(FixedStringTest, AccessOperator) {
    constexpr fixed_string<6> fs("hello");
    for (size_t i = 0; i < fs.size(); ++i) {
        EXPECT_NE(fs[i], '\0');
    }
}

// ============================================================================
// Тесты для format_string
// ============================================================================

TEST(FormatStringTest, SinglePlaceholder) {
    constexpr fixed_string<12> fmt_str("Value: {%d}");
    constexpr format_string<fmt_str> fmt;
    EXPECT_EQ(fmt.number_placeholders, 1);
}

TEST(FormatStringTest, MultiplePlaceholders) {
    constexpr fixed_string<32> fmt_str("Name: {%s}, Age: {%d}, ID: {%u}");
    constexpr format_string<fmt_str> fmt;
    EXPECT_EQ(fmt.number_placeholders, 3);
}

TEST(FormatStringTest, NoDuplicates) {
    constexpr fixed_string<20> fmt_str("A{%d}B{%d}C{%d}");
    constexpr format_string<fmt_str> fmt;
    EXPECT_EQ(fmt.number_placeholders, 3);
}

// ============================================================================
// Тесты для scan - целые числа
// ============================================================================

TEST(ScanTest, SingleInt32) {
    constexpr fixed_string<6> fmt("x{%d}");
    constexpr fixed_string<5> src("x42");
    constexpr auto result = scan<fmt, src, int32_t>();
    EXPECT_EQ(result.template get<0>(), 42);
}

TEST(ScanTest, SingleUint32) {
    constexpr fixed_string<6> fmt("{%u}");
    constexpr fixed_string<4> src("99");
    constexpr auto result = scan<fmt, src, uint32_t>();
    EXPECT_EQ(result.template get<0>(), 99);
}

TEST(ScanTest, Int8Type) {
    constexpr fixed_string<8> fmt("v{%d}");
    constexpr fixed_string<5> src("v127");
    constexpr auto result = scan<fmt, src, int8_t>();
    EXPECT_EQ(result.template get<0>(), 127);
}

TEST(ScanTest, Uint8Type) {
    constexpr fixed_string<8> fmt("v{%u}");
    constexpr fixed_string<5> src("v255");
    constexpr auto result = scan<fmt, src, uint8_t>();
    EXPECT_EQ(result.template get<0>(), 255);
}

TEST(ScanTest, Int16Type) {
    constexpr fixed_string<10> fmt("i16:{%d}");
    constexpr fixed_string<10> src("i16:32767");
    constexpr auto result = scan<fmt, src, int16_t>();
    EXPECT_EQ(result.template get<0>(), 32767);
}

TEST(ScanTest, Uint16Type) {
    constexpr fixed_string<10> fmt("u16:{%u}");
    constexpr fixed_string<10> src("u16:65535");
    constexpr auto result = scan<fmt, src, uint16_t>();
    EXPECT_EQ(result.template get<0>(), 65535);
}

// ============================================================================
// Тесты для scan - строки
// ============================================================================

TEST(ScanTest, SingleStringView) {
    constexpr fixed_string<10> fmt("str:{%s}");
    constexpr fixed_string<15> src("str:hello");
    constexpr auto result = scan<fmt, src, std::string_view>();
    EXPECT_EQ(result.template get<0>(), "hello");
}

TEST(ScanTest, StringExtraction) {
    constexpr fixed_string<40> fmt("Name: {%s}, Email: {%s}");
    constexpr fixed_string<60> src("Name: Alice, Email: alice@example.com");
    constexpr auto result = scan<fmt, src, std::string_view, std::string_view>();
    EXPECT_EQ(result.template get<0>(), "Alice");
    EXPECT_EQ(result.template get<1>(), "alice@example.com");
}

// ============================================================================
// Тесты для scan - смешанные типы
// ============================================================================

TEST(ScanTest, MultipleTypes) {
    constexpr fixed_string<30> fmt("Name:{%s},Age:{%d}");
    constexpr fixed_string<30> src("Name:John,Age:25");
    constexpr auto result = scan<fmt, src, std::string_view, int32_t>();

    EXPECT_EQ(result.template get<0>(), "John");
    EXPECT_EQ(result.template get<1>(), 25);
}

TEST(ScanTest, ComplexFormat) {
    constexpr fixed_string<50> fmt("ID:{%d},Score:{%u},Name:{%s}");
    constexpr fixed_string<50> src("ID:123,Score:999,Name:Alice");
    constexpr auto result = scan<fmt, src, int32_t, uint32_t, std::string_view>();

    EXPECT_EQ(result.template get<0>(), 123);
    EXPECT_EQ(result.template get<1>(), 999);
    EXPECT_EQ(result.template get<2>(), "Alice");
}

TEST(ScanTest, ThreeIntegers) {
    constexpr fixed_string<30> fmt("Point({%d},{%d},{%d})");
    constexpr fixed_string<30> src("Point(10,20,30)");
    constexpr auto result = scan<fmt, src, int32_t, int32_t, int32_t>();

    EXPECT_EQ(result.template get<0>(), 10);
    EXPECT_EQ(result.template get<1>(), 20);
    EXPECT_EQ(result.template get<2>(), 30);
}

// ============================================================================
// Тесты для scan - специальные случаи
// ============================================================================

TEST(ScanTest, CustomSeparators) {
    constexpr fixed_string<20> fmt("({%d},{%d},{%d})");
    constexpr fixed_string<20> src("(1,2,3)");
    constexpr auto result = scan<fmt, src, int32_t, int32_t, int32_t>();
    EXPECT_EQ(result.template get<0>(), 1);
    EXPECT_EQ(result.template get<1>(), 2);
    EXPECT_EQ(result.template get<2>(), 3);
}

TEST(ScanTest, WhitespaceSeparator) {
    constexpr fixed_string<20> fmt("a {%d} b {%d}");
    constexpr fixed_string<20> src("a 5 b 10");
    constexpr auto result = scan<fmt, src, int32_t, int32_t>();
    EXPECT_EQ(result.template get<0>(), 5);
    EXPECT_EQ(result.template get<1>(), 10);
}

TEST(ScanTest, NegativeNumbers) {
    constexpr fixed_string<30> fmt("Temperature: {%d}C");
    constexpr fixed_string<30> src("Temperature: -40C");
    constexpr auto result = scan<fmt, src, int32_t>();
    EXPECT_EQ(result.template get<0>(), -40);
}

TEST(ScanTest, LargeValues) {
    constexpr fixed_string<10> fmt("val:{%u}");
    constexpr fixed_string<20> src("val:4294967295");
    constexpr auto result = scan<fmt, src, uint32_t>();
    EXPECT_EQ(result.template get<0>(), 4294967295u);
}

// ============================================================================
// Параметризованные тесты для различных форматов
// ============================================================================

struct ScanParamTest : public ::testing::TestWithParam<std::pair<int, const char *>> {};

TEST_P(ScanParamTest, ParseIntegerWithPrefix) {
    auto [expected, src_str] = GetParam();

    constexpr fixed_string<15> fmt("val:{%d}");
    constexpr fixed_string<20> src("val:42");

    if (expected == 42) {
        constexpr auto result = scan<fmt, src, int32_t>();
        EXPECT_EQ(result.template get<0>(), 42);
    }
}

INSTANTIATE_TEST_SUITE_P(ScanIntegerTests, ScanParamTest,
                         ::testing::Values(std::make_pair(42, "val:42"), std::make_pair(100, "val:100"),
                                           std::make_pair(-50, "val:-50")));

class CompileTimeTest : public ::testing::Test {
protected:
    static constexpr bool all_compile_time_tests_passed = true;
};

TEST_F(CompileTimeTest, CompileTimeIntegrity) {
    static_assert(CompileTimeTest::all_compile_time_tests_passed);
    SUCCEED();
}

// ============================================================================
// Главная функция
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
