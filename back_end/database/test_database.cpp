#include "database.h"
#include <iostream>
#include <windows.h> // 引入 Windows API 头文件

// 辅助函数：打印查询结果
void printQueryResult(const std::vector<std::vector<std::string>>& result) {
    for (const auto& row : result) {
        for (const auto& value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}

// 辅助函数：打印单行查询结果
void printSingleQueryResult(const std::vector<std::string>& result) {
    for (const auto& value : result) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

int main() {
    // 设置控制台输出编码为 UTF - 8
    SetConsoleOutputCP(CP_UTF8);

    Database db;

    // 检查数据库是否成功打开
    if (db.getErrorCode() != SQLITE_OK) {
        std::cerr << "Failed to open database." << std::endl;
        return 1;
    }

    // 测试添加第一条数据
    std::vector<std::string> values1 = {"1", "张三", "1", "abcdef123456", "计算机科学"};
    if (db.addOne(TableName::STUDENT, values1)) {
        std::cout << "First data added successfully." << std::endl;
    } else {
        std::cerr << "Failed to add first data: " << db.getErrorMsg() << std::endl;
    }

    // 测试添加第二条数据
    std::vector<std::string> values2 = {"2", "李四", "2", "qwertyuiop", "电子工程"};
    if (db.addOne(TableName::STUDENT, values2)) {
        std::cout << "Second data added successfully." << std::endl;
    } else {
        std::cerr << "Failed to add second data: " << db.getErrorMsg() << std::endl;
    }

    // 测试搜索所有数据
    auto allResults = db.searchALL(TableName::STUDENT, "NAME", "张三");
    std::cout << "Search all results for Zhang San:" << std::endl;
    printQueryResult(allResults);

    // 测试搜索单条数据
    auto singleResult = db.searchOne(TableName::STUDENT, "NAME", "李四");
    std::cout << "Search single result for Li Si:" << std::endl;
    printSingleQueryResult(singleResult);

    // 测试更新第一条数据
    if (db.updateOneById(TableName::STUDENT, "PROFESSION", "软件工程", "1")) {
        std::cout << "First data updated successfully." << std::endl;
    } else {
        std::cerr << "Failed to update first data: " << db.getErrorMsg() << std::endl;
    }

    // 测试更新第二条数据
    if (db.updateOneById(TableName::STUDENT, "GENDER", "1", "2")) {
        std::cout << "Second data updated successfully." << std::endl;
    } else {
        std::cerr << "Failed to update second data: " << db.getErrorMsg() << std::endl;
    }

    // 测试删除第一条数据
    if (db.deleteOneById(TableName::STUDENT, "1")) {
        std::cout << "First data deleted successfully." << std::endl;
    } else {
        std::cerr << "Failed to delete first data: " << db.getErrorMsg() << std::endl;
    }

    // 测试删除第二条数据
    if (db.deleteOneById(TableName::STUDENT, "2")) {
        std::cout << "Second data deleted successfully." << std::endl;
    } else {
        std::cerr << "Failed to delete second data: " << db.getErrorMsg() << std::endl;
    }

    return 0;
}
