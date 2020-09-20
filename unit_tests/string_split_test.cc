#include "../src/common.h"
#include "unit_tests_common.h"

TEST(StringSplitText, SplittingStringsIntoTokensByAGivenDelimiter) {
    using namespace std::string_literals;
    std::vector<std::string> expected{"this", "is", "sample"};

    ASSERT_THAT(expected, split("this is sample"s, ' '));
}
