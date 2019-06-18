#include "gtest/gtest.h"

#include "../FileReader.hpp"

namespace {

void verify_prefix(std::string_view str, std::string_view prefix)
{
    ASSERT_TRUE(str.compare(0, prefix.size(), prefix) == 0)
        << "Expected \"" << str << "\" to have the prefix \"" << prefix << '"';
}

class FileReaderTest : public testing::Test
{
};

TEST_F(FileReaderTest, TestReadingSnekFile)
{
    FileReader reader("../test/input/snek.txt");

    ASSERT_EQ("hsssss ~~~~~:>~\n", reader.data());
    ASSERT_EQ(16u, reader.size());
}

TEST_F(FileReaderTest, TestOpeningFileThatDoesNotExist)
{
    try {
        FileReader reader("../test/input/doesnotexist.txt");
        ASSERT_TRUE(false) << "Expected a std::runtime_error to be thrown";
    } catch (const std::runtime_error& err) {
        verify_prefix(err.what(), "Unable to open file");
    }
}

TEST_F(FileReaderTest, TestOpeningEmptyFile)
{
    FileReader reader("../test/input/empty.txt");
    ASSERT_EQ("", reader.data());
}

}
