#include <catch2/catch_test_macros.hpp>
#include <string>
#include <cstddef>
#include <cstdint>
#include <cstring>

size_t writecallback(void* contents, size_t size, size_t nmemb, void* userp);

TEST_CASE("writecallback appends bytes to std::string", "[api][telegram][writecallback]") {
    std::string out;
    const char data[] = "abc";
    auto written = writecallback((void*)data, 1, 3, &out);
    REQUIRE(written == 3);
    REQUIRE(out == "abc");
}

TEST_CASE("writecallback appends when string already has content", "[api][telegram][writecallback]") {
    std::string out = "hello";
    const char data[] = " world";
    auto written = writecallback((void*)data, 1, 6, &out);
    REQUIRE(written == 6);
    REQUIRE(out == "hello world");
}

TEST_CASE("writecallback appends multiple times accumulating result", "[api][telegram][writecallback]") {
    std::string out;
    const char a[] = "12";
    const char b[] = "345";
    const char c[] = "6";

    REQUIRE(writecallback((void*)a, 1, 2, &out) == 2);
    REQUIRE(writecallback((void*)b, 1, 3, &out) == 3);
    REQUIRE(writecallback((void*)c, 1, 1, &out) == 1);

    REQUIRE(out == "123456");
}

TEST_CASE("writecallback works with size>1 and nmemb>1", "[api][telegram][writecallback]") {
    std::string out;
    const char data[] = "ABCDEFGH";
    auto written = writecallback((void*)data, 2, 4, &out);
    REQUIRE(written == 8);
    REQUIRE(out == "ABCDEFGH");
}

TEST_CASE("writecallback can append binary data with zero bytes", "[api][telegram][writecallback]") {
    std::string out;

    unsigned char bytes[] = { 0x41, 0x00, 0x42, 0x00, 0x43 };
    auto written = writecallback((void*)bytes, 1, 5, &out);

    REQUIRE(written == 5);
    REQUIRE(out.size() == 5);
    REQUIRE(static_cast<unsigned char>(out[0]) == 0x41);
    REQUIRE(static_cast<unsigned char>(out[1]) == 0x00);
    REQUIRE(static_cast<unsigned char>(out[2]) == 0x42);
    REQUIRE(static_cast<unsigned char>(out[3]) == 0x00);
    REQUIRE(static_cast<unsigned char>(out[4]) == 0x43);
}

TEST_CASE("writecallback returns 0 and does not change string when nmemb is 0", "[api][telegram][writecallback]") {
    std::string out = "x";
    const char data[] = "abc";
    auto written = writecallback((void*)data, 1, 0, &out);
    REQUIRE(written == 0);
    REQUIRE(out == "x");
}

TEST_CASE("writecallback returns 0 and does not change string when size is 0", "[api][telegram][writecallback]") {
    std::string out = "x";
    const char data[] = "abc";
    auto written = writecallback((void*)data, 0, 3, &out);
    REQUIRE(written == 0);
    REQUIRE(out == "x");
}