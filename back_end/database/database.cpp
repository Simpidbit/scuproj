#include "database.h"

namespace db {

std::string
tableNameToString(TableName name)
{
  std::string tabname;
  switch (name) {
    case db::TN_ACCOUNT:
      tabname = "ACCOUNT";
      break;
    case db::TN_COURSE:
      tabname = "COURSE";
      break;
    case db::TN_COURSE_SELECT:
      tabname = "COURSE_SELECT";
      break;
  }
  return tabname;
}


Database::Database()
{
  //打开数据库
  rc = sqlite3_open("SCU.db", &db);
  if (rc != SQLITE_OK) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    // 此时 db 可能未正确初始化，不关闭
    db = nullptr;
  }
}


const char *
Database::getErrorMsg()
{ return zErrMsg; }


std::vector<std::vector<std::string> >
Database::searchAll(
  TableName table,
  const std::string field,
  const std::string value)
{
  std::string tableName = tableNameToString(table);
  std::vector<std::vector<std::string>> ans;

  std::string sql = 
        "select * from " + tableName 
      + " where " + field + " = ?;";

  // 执行SQL语句
  auto result = executeSQL(sql, {value}, true);
  if (!result.first) {
      std::cerr << "Search all failed: " << getErrorMsg() << std::endl;
      return ans;
  }
  sqlite3_stmt* stmt = result.second;

  while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
    std::vector<std::string> row;
    for(int col = 0; col < sqlite3_column_count(stmt); col++) {
      //字段值,返回字符串
      const char * colValue =
        (const char *)sqlite3_column_text(stmt, col);

      //添加到行,处理NULL
      row.push_back(colValue ? colValue : "");
    }
    ans.push_back(row);
  }

  // 检查执行状态，对于查询操作，SQLITE_DONE 是正常结束状态
  if(rc != SQLITE_DONE && rc != STATUS_OK)
      std::cerr << "An error happened when searching all in the database: " << getErrorMsg() << std::endl;

  sqlite3_finalize(stmt);

  return ans;
}


std::vector<std::string>
Database::searchOne(
  TableName         table,
  const std::string field,
  const std::string value)
{
  std::string tableName = tableNameToString(table);
  std::vector<std::string> ans;
  std::string sql = "select * from "+ tableName + " where " + field + " = ? limit 1;";

  // 调用私有预处理函数
  auto result = executeSQL(sql, {value}, true);
  if (!result.first) {
    std::cerr << "Search one failed: " << getErrorMsg() << std::endl;
    return ans;
  }

  sqlite3_stmt* stmt = result.second;

 if((rc = sqlite3_step(stmt)) == SQLITE_ROW){
    std::vector<std::string> row;
    for(int col = 0; col < sqlite3_column_count(stmt); col++) {
      //字段值,返回字符串
      const char* colValue = 
        (const char*)sqlite3_column_text(stmt, col);
      //添加到行,处理NULL
      ans.push_back(colValue ? colValue : "");
    }
  }

  // 检查执行状态，对于查询操作，SQLITE_DONE 是正常结束状态
  if(rc != SQLITE_DONE && rc != SQLITE_ROW){
      std::cout << "rc: " << rc << std::endl;
      std::cerr << "An error happened when searching one in the database: " << getErrorMsg() << std::endl;
  }

  sqlite3_finalize(stmt);
  return ans;
}


bool
Database::addOne(
  TableName table,
  std::vector<std::string> values)
{
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


bool
Database::deleteOneById(TableName table, std::string id)
{
  if(id.empty()){
    std::cerr<<"Id could not be NULL"<<std::endl;
    return false;
  }
  // 转换枚举为字符串
  std::string tableName = tableNameToString(table);
  //1.预构建
  std::string sql = "delete from "+ tableName + " where id = ?";
  // 调用私有预处理函数
  auto result = executeSQL(sql, {id}, false);

  if (!result.first) 
    std::cerr << "Delete one by id failed: " << getErrorMsg() << std::endl;

  return result.first;
}


bool
Database::updateOneById(
  TableName table,
  const std::string field,
  const std::string value,
  const std::string id)
{
  if(id.empty()){
    std::cerr<<"Id could not be NULL"<<std::endl;
    return false;
  }

  // 转换枚举为字符串
  std::string tableName = tableNameToString(table);

  //1.预构建
  // 注释内容与实际操作不符，更新为正确的注释
  // 构建 SQL 语句：UPDATE table SET field = ? WHERE id = ?
  std::string sql = "update "+ tableName + " set "+ field + " = ? where id = ?";

  // 执行SQL语句
  auto result = executeSQL(sql, {value, id}, false);
  if (!result.first)
    std::cerr << "Update one by id failed: " << getErrorMsg() << std::endl;

  return result.first;
}


int
Database::getErrorCode()
{ return rc; }


std::pair<bool, sqlite3_stmt *>
executeSQL(
  const std::string& sql,
  const std::vector<std::string>& values,
  bool isQuery) 
{
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


} // namespace db

