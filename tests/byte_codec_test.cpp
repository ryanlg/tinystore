#include <tinystore/byte_codec.hpp>

#include <bit>
#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("single integral types", "[ByteCodec]", char, int, long, uint64_t) {
    tinystore::ByteCodec bw = {};
    const TestType num = 3;

    bw.write_le(num);
    const auto result = bw.read_le<TestType>(0);
    REQUIRE(result);
    REQUIRE(*result == num);
}

TEST_CASE("single string", "[ByteCodec]") {
    tinystore::ByteCodec bw = {};
    const std::string str = "test string";

    bw.write(str);
    const auto result = bw.read_string(0, str.length());
    REQUIRE(result);
    REQUIRE(*result == str);
}

TEST_CASE("multiple values", "[ByteCodec]") {
    const std::string str1 = "test string 1";
    const std::string str2 = "str test 2";
    const uint64_t l = 10203040;
    const int i = 42;
    const char c = 'b';

    tinystore::ByteCodec bw = {};
    bw.write(str1);
    bw.write(str2);
    bw.write_le(l);
    bw.write_le(i);
    bw.write_le(c);

    std::size_t offset = 0;
    REQUIRE(bw.read_string(offset, str1.length()) == str1);
    offset += str1.length();
    REQUIRE(bw.read_string(offset, str2.length()) == str2);
    offset += str2.length();
    REQUIRE(bw.read_le<uint64_t>(offset) == l);
    offset += sizeof(uint64_t);
    REQUIRE(bw.read_le<int>(offset) == i);
    offset += sizeof(int);
    REQUIRE(bw.read_le<char>(offset) == c);
}
