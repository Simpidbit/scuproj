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
    std::vector<std::string> values1 = {"1", "20246030", "password", "张三", "0"};
    if (db.addOne(TableName::ACCOUNT, values1)) {
        std::cout << "First data added successfully." << std::endl;
    } else {
        std::cerr << "Failed to add first data: " << db.getErrorMsg() << std::endl;
    }

    db.updateOneById(TableName::ACCOUNT, "ACCOUNT_NAME", "王五", "20246030");

     db.deleteOneById(TableName::ACCOUNT,"20246030");


  

    return 0;
}
