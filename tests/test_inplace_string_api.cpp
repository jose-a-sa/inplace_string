#include <qx/inplace_string.h>

#include <gtest/gtest.h>

#include "string_api_tester.h"



TEST(InplaceStringSApiTest, FindFirstNotOfAndFindLastOf)
{
    qx::inplace_string<20> const s("hello world");
    auto const tester = qx::string_api_tester(s);

    // find_first_not_of matches first char not present in target set
    tester.result_eq([](auto& s) { return s.find_first_not_of("helo"); }, 5)
        .result_eq([](auto& s) { return s.find_first_not_of("hello world"); }, qx::inplace_string<20>::npos)
        .result_eq([](auto& s) { return s.find_last_of("aeiou"); }, 7)
        .result_eq([](auto& s) { return s.find_last_of("xyz"); }, qx::inplace_string<20>::npos);
}