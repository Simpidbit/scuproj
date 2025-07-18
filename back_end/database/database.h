#include "sqlite3.h"
#include <iostream>
#include <vector>
#include <string>

// 枚举类定义
enum class TableName {
    ACCOUNT,
    COURSE,
    COURSE_SELECT
};

// Helper function to convert TableName to string
const char* tableNameToString(TableName name) {
    switch (name) {
        case TableName::ACCOUNT:
            return "ACCOUNT";
        case TableName::COURSE:
            return "COURSE";
        case TableName::COURSE_SELECT:
            return "COURSE_SELECT";
        default:
            return "NULL";
    }
}

/**
* @brief 数据库工具类
*
* 提供数据库的基本操作
*
* @author lxy(shed4329@163.com)
*
* @date 2025-7-15
**/
class Database{
    // 定义常量表达式
    static constexpr int STATUS_OK = SQLITE_OK;
    public:
        /**
         * @brief 数据库构造函数
         * 
         */
        Database(){
            //打开数据库
            rc = sqlite3_open("SCU.db", &db);
            if (rc != SQLITE_OK) {
                std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
                // 此时 db 可能未正确初始化，不关闭
                db = nullptr;
            }
        }

        /**
         * @brief 获取错误信息
         * 
         * @return const char* 错误信息
         */
        const char* getErrorMsg(){
            return zErrMsg;
        }
        
        /**
        * @brief 搜索所有数据
        * 
        * @param table 表名，使用枚举类
        * @param field 字段名
        * @param value 字段值
        *
        * @return std::vector<std::vector<std::string>> 搜索结果
        */
        std::vector<std::vector<std::string>> searchALL(TableName table,const std::string field,const std::string value){
            // 转换枚举为字符串
            std::string tableName = tableNameToString(table);
            //结果
            std::vector<std::vector<std::string>> ans;
            //sql语句
            std::string sql = "select * from "+ tableName + " where " + field + " = ?;";
            // 调用私有预处理函数
            auto result = executeSQL(sql, {value}, true);
            if (!result.first) {
                std::cerr << "Search all failed: " << getErrorMsg() << std::endl;
                return ans;
            }
            sqlite3_stmt* stmt = result.second;

            while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
                //行
                std::vector<std::string> row;
                //读取全部字段
                for(int col=0;col<sqlite3_column_count(stmt);col++){
                    //字段值,返回字符串
                    const char* colValue = (const char*)sqlite3_column_text(stmt,col);
                    //添加到行,处理NULL
                    row.push_back(colValue?colValue:"");
                }
                ans.push_back(row);
            }

            // 检查执行状态，对于查询操作，SQLITE_DONE 是正常结束状态
            if(rc != SQLITE_DONE && rc != STATUS_OK){
                std::cerr << "An error happened when searching all in the database: " << getErrorMsg() << std::endl;
            }
            sqlite3_finalize(stmt);
            return ans;
        }

        /**
        * @brief 返回第一个匹配对象的全部属性值
        *
        * @param table 表名，使用枚举类
        * @param field 字段名
        * @param value 字段值
        *
        * @return std::vector<std::string> 搜索结果
        */
       std::vector<std::string> searchOne(TableName table,const std::string field,const std::string value){
            // 转换枚举为字符串
            std::string tableName = tableNameToString(table);
            //结果
           std::vector<std::string> ans;
            //sql语句
            std::string sql = "select * from "+ tableName + " where " + field + " = ? limit 1;";
            // 调用私有预处理函数
            auto result = executeSQL(sql, {value}, true);
            if (!result.first) {
                std::cerr << "Search one failed: " << getErrorMsg() << std::endl;
                return ans;
            }
            sqlite3_stmt* stmt = result.second;

           if((rc = sqlite3_step(stmt)) == SQLITE_ROW){
                //行
                std::vector<std::string> row;
                //读取全部字段
                for(int col=0;col<sqlite3_column_count(stmt);col++){
                    //字段值,返回字符串
                    const char* colValue = (const char*)sqlite3_column_text(stmt,col);
                    //添加到行,处理NULL
                    ans.push_back(colValue?colValue:"");
                }
            }

            // 检查执行状态，对于查询操作，SQLITE_DONE 是正常结束状态
            if(rc != SQLITE_DONE&&rc != SQLITE_ROW){
                std::cout<<"rc: "<<rc<<std::endl;
                std::cerr << "An error happened when searching one in the database: " << getErrorMsg() << std::endl;
            }
            sqlite3_finalize(stmt);
            return ans;
        }

        /**
        * @brief 添加一条数据到数据库，所有属性都要添加
        * 
        * @param table 表名，使用枚举类
        * @param values 字段值
        *
        * @return bool 执行状态
        */
        bool addOne(TableName table,std::vector<std::string> values){
            if(values.empty()){
                std::cerr<<"No value to add"<<std::endl;
                return false;
            }
            // 转换枚举为字符串
            std::string tableName = tableNameToString(table);
            //1.预构建
            // 构建 SQL 语句：INSERT INTO table (field1, field2) VALUES (?, ?)
            // 注意：这里假设插入所有字段，按默认顺序插入
            std::string sql = "insert into " + tableName + " values ("; 
            for (size_t i = 0; i < values.size(); i++) {
                sql += "?";
                if (i != values.size() - 1) {
                    sql += ",";
                }
            }
            sql += ")";
            // 调用私有预处理函数
            auto result = executeSQL(sql, values, false);
            if (!result.first) {
                std::cerr << "Add one failed: " << getErrorMsg() << std::endl;
            }
            return result.first;
        }
        
        /**
         * @brief 根据id删除一条数据
         * 
         * @param table 表名，使用枚举类
         * @param id 字段值
         * 
         * @return bool 执行状态
         */
        bool deleteOneById(TableName table,std::string id){
            if(id.empty()){
                std::cerr<<"Id could not be NULL"<<std::endl;
                return false;
            }
            // 转换枚举为字符串
            std::string tableName = tableNameToString(table);
            //1.预构建
            std::string sql = "delete from "+ tableName + " where ACCOUNT_ID = ?";
            // 调用私有预处理函数
            auto result = executeSQL(sql, {id}, false);
            if (!result.first) {
                std::cerr << "Delete one by id failed: " << getErrorMsg() << std::endl;
            }
            return result.first;
        }


        /**
        * @brief 根据id更新一条数据
        * 
        * @param table 表名，使用枚举类
        * @param field 字段名
        * @param value 字段值
        * @param id 字段值
        * 
        * @return bool 执行状态
        *
        */
        bool updateOneById(TableName table, const std::string field, const std::string value, const std::string id) {
            if(id.empty()){
                std::cerr<<"Id could not be NULL"<<std::endl;
                return false;
            }
            // 转换枚举为字符串
            std::string tableName = tableNameToString(table);
            //1.预构建
            // 注释内容与实际操作不符，更新为正确的注释
            // 构建 SQL 语句：UPDATE table SET field = ? WHERE id = ?
            std::string sql = "update "+ tableName + " set "+ field + " = ? where ACCOUNT_ID = ?";
            // 调用私有预处理函数
            auto result = executeSQL(sql, {value, id}, false);
            if (!result.first) {
                std::cerr << "Update one by id failed: " << getErrorMsg() << std::endl;
            }
            return result.first;
        }

        /**
         * @brief 获取错误码
         * 当错误码为SQLITE_OK时，说明执行成功
         * @return int 错误码
         */
        int getErrorCode(){
            return rc;
        }
          
    private:
        /**
         * @brief 执行 SQL 语句的私有预处理函数
         * 
         * @param sql SQL 语句
         * @param values 要绑定的参数值
         * @param isQuery 是否为查询语句
         * @return std::pair<bool, sqlite3_stmt*> 执行结果和预处理语句对象
         */
        std::pair<bool, sqlite3_stmt*> executeSQL(const std::string& sql, const std::vector<std::string>& values, bool isQuery) {
            sqlite3_stmt* stmt = nullptr;
            // 执行预编译
            rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            if(rc != STATUS_OK) {
                zErrMsg = sqlite3_errmsg(db);
                std::cerr << "Fail to prebuild sql: " << zErrMsg << std::endl;
                return {false, nullptr};
            }

            // 执行替换，绑定参数
            for(size_t i = 0; i < values.size(); i++){
                rc = sqlite3_bind_text(stmt, i + 1, values[i].c_str(), -1, SQLITE_STATIC);
                if(rc != STATUS_OK){
                    zErrMsg = sqlite3_errmsg(db);
                    std::cerr << "Fail to bind value: " << zErrMsg << std::endl;
                    sqlite3_finalize(stmt);
                    return {false, nullptr};
                }
            }

            // 执行 SQL 语句
            if (isQuery) {
                return {true, stmt};
            } else {
                rc = sqlite3_step(stmt);
                if(rc != SQLITE_DONE){
                    zErrMsg = sqlite3_errmsg(db);
                    std::cerr << "Fail to execute sql: " << zErrMsg << std::endl;
                    sqlite3_finalize(stmt);
                    return {false, nullptr};
                }
                // 释放资源
                sqlite3_finalize(stmt);
                return {true, nullptr};
            }
        }

        //数据库
        sqlite3* db;
        //错误信息
        const char* zErrMsg = nullptr;
        //状态
        int rc;
};
