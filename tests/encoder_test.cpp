#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <tinystore/byte_writer.hpp>

template <typename T>
    requires std::is_fundamental_v<T>
T reconstruct_primitive(const std::vector<std::byte> &bvec, off_t offset = 0) {
    T result;
    std::memcpy(&result, bvec.data() + offset, sizeof(T));
    return result;
}

std::string reconstruct_string(const std::vector<std::byte> &bvec, size_t size, off_t offset = 0) {
    return std::string(reinterpret_cast<const char *>(bvec.data() + offset), size);
}

TEMPLATE_TEST_CASE("single fundamental data types", "[ByteWriter]", char, int, long, uint64_t) {
    tinystore::ByteWriter bw = {};
    const TestType num = 3;

    bw.write(num);
    const auto result = bw.finalize();
    REQUIRE(reconstruct_primitive<TestType>(result) == 3);
}

TEST_CASE("single string", "[ByteWriter]") {
    tinystore::ByteWriter bw = {};
    const std::string str = "test string";

    bw.write(str);
    const auto result = bw.finalize();
    REQUIRE(reconstruct_string(result, str.length()) == str);
}

TEST_CASE("multiple values", "[ByteWriter]") {
    const std::string str1 = "test string 1";
    const std::string str2 = "str test 2";
    const uint64_t l = 10203040;
    const int i = 42;
    const char c = 'b';

    tinystore::ByteWriter bw = {};
    bw.write(str1);
    bw.write(str2);
    bw.write(l);
    bw.write(i);
    bw.write(c);
    const auto result = bw.finalize();

    off_t offset = 0;
    REQUIRE(reconstruct_string(result, str1.length()) == str1);
    offset += str1.length();
    REQUIRE(reconstruct_string(result, str2.length(), offset) == str2);
    offset += str2.length();
    REQUIRE(reconstruct_primitive<uint64_t>(result, offset) == l);
    offset += sizeof(uint64_t);
    REQUIRE(reconstruct_primitive<int>(result, offset) == i);
    offset += sizeof(int);
    REQUIRE(reconstruct_primitive<char>(result, offset) == c);
}
