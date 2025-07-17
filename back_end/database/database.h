#include "sqlite3.h"
#include <iostream>
#include <vector>
#include <string>

namespace db {

// 枚举类定义
enum class TableName {
    TN_ACCOUNT = 0,
    TN_COURSE,
    TN_COURSE_SELECT
};

// Helper function to convert TableName to string
std::string tableNameToString(TableName name);

/**
* @brief 数据库工具类
*
* 提供数据库的基本操作
*
* @author lxy<shed4329@163.com>, Simpidbit<8155530@gmail.com>
*
* @date 2025.7
**/
class Database {
  // 定义常量表达式
  static constexpr int STATUS_OK = SQLITE_OK;

  sqlite3 *     db;                   // 数据库
  const char *  zErrMsg = nullptr;    // 错误信息
  int           rc;                   // 状态

public:
  Database();
        
  /**
   * @brief 搜索所有数据
   * 
   * @param table 表名，使用枚举类
   * @param field 字段名
   * @param value 字段值
   *
   * @return std::vector<std::vector<std::string>> 搜索结果
   */
  std::vector<std::vector<std::string> >
  searchALL(TableName         table,
            const std::string field,
            const std::string value);

  /**
   * @brief 返回第一个匹配对象的全部属性值
   *
   * @param table 表名，使用枚举类
   * @param field 字段名
   * @param value 字段值
   *
   * @return std::vector<std::string> 搜索结果
   */
  std::vector<std::string> 
  searchOne(TableName         table,
            const std::string field,
            const std::string value);

  /**
   * @brief 添加一条数据到数据库，所有属性都要添加
   * 
   * @param table 表名，使用枚举类
   * @param values 字段值
   *
   * @return bool 执行状态
   */
  bool
  addOne(TableName table, std::vector<std::string> values);
  
  /**
   * @brief 根据id删除一条数据
   * 
   * @param table 表名，使用枚举类
   * @param id 字段值
   * 
   * @return bool 执行状态
   */
  bool
  deleteOneById(TableName table,std::string id);


  /**
   * @brief 根据id更新一条数据
   * 
   * @param table 表名，使用枚举类
   * @param field 字段名
   * @param value 字段值
   * @param id 字段值
   * 
   * @return bool 执行状态
   */
  bool updateOneById(
    TableName         table, 
    const std::string field,
    const std::string value,
    const std::string id);

  /**
   * @brief 获取错误码
   * 当错误码为SQLITE_OK时，说明执行成功
   * @return int 错误码
   */
  int getErrorCode();

  const char* getErrorMsg();
    
private:
  /**
   * @brief 执行 SQL 语句的私有预处理函数
   * 
   * @param sql SQL 语句
   * @param values 要绑定的参数值
   * @param isQuery 是否为查询语句
   * @return std::pair<bool, sqlite3_stmt*> 执行结果和预处理语句对象
   */
  std::pair<bool, sqlite3_stmt *>
  executeSQL(
    const std::string& sql,
    const std::vector<std::string>& values,
    bool isQuery);
};


} // namespace db
