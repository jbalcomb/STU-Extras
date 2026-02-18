#include <gtest/gtest.h>

#include "FuncExtract.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::filesystem::path WriteTempSourceFile(const std::string& contents)
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto file_name = "func_extract_parse_test_" + std::to_string(now) + ".c";
    const auto path = std::filesystem::temp_directory_path() / file_name;

    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open temporary source file for writing.");
    }
    out << contents;
    out.close();
    return path;
}

class TempSourceFile {
public:
    explicit TempSourceFile(const std::string& contents) : path_(WriteTempSourceFile(contents)) {}
    ~TempSourceFile() { std::error_code ec; std::filesystem::remove(path_, ec); }

    std::string PathString() const { return path_.string(); }

private:
    std::filesystem::path path_;
};

} // namespace

TEST(ParseCSourceFileTest, ReturnsErrorForNullArguments)
{
    FUNCTION_LIST list{};
    EXPECT_EQ(Parse_C_Source_File(nullptr, &list), FE_ERROR_FILE);
    EXPECT_EQ(Parse_C_Source_File("dummy.c", nullptr), FE_ERROR_FILE);
}

TEST(ParseCSourceFileTest, ReturnsErrorWhenFileCannotBeOpened)
{
    FUNCTION_LIST list{};
    ASSERT_EQ(Init_Function_List(&list, 1), FE_SUCCESS);

    EXPECT_EQ(Parse_C_Source_File("__file_that_should_not_exist__.c", &list), FE_ERROR_FILE);

    Free_Function_List(&list);
}

TEST(ParseCSourceFileTest, ExtractsDefinitionsAndSkipsDeclarationsAndComments)
{
    const std::string source =
        "int proto(int a);\n"
        "static int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "// void commented_out(void) { }\n"
        "\n"
        "int split_signature(int x)\n"
        "{\n"
        "    return x;\n"
        "}\n"
        "\n"
        "/*\n"
        "int also_commented(int x) {\n"
        "    return x;\n"
        "}\n"
        "*/\n";

    TempSourceFile file(source);
    FUNCTION_LIST list{};

    ASSERT_EQ(Init_Function_List(&list, 1), FE_SUCCESS);
    ASSERT_EQ(Parse_C_Source_File(file.PathString().c_str(), &list), FE_SUCCESS);

    ASSERT_EQ(list.count, 2ULL);

    EXPECT_STREQ(list.functions[0].name, "add");
    EXPECT_STREQ(list.functions[0].return_type, "static int");
    EXPECT_EQ(list.functions[0].line_number, 2ULL);

    EXPECT_STREQ(list.functions[1].name, "split_signature");
    EXPECT_STREQ(list.functions[1].return_type, "int");
    EXPECT_EQ(list.functions[1].line_number, 8ULL);

    Free_Function_List(&list);
}
