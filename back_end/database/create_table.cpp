#include <iostream>
#include "sqlite3.h"

int main(){
    // 数据库
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;
    const char* sql;

    // 打开数据库
    rc = sqlite3_open("SCU.db", &db);

    if(rc != SQLITE_OK){
        std::cerr << "open database failed:" << zErrMsg << std::endl;
        return -1;
    }
    
    std::cout << "open database successed" << std::endl;

    // 建 ACCOUNT 表
    sql = "CREATE TABLE IF NOT EXISTS ACCOUNT("         \
            "ID INT PRIMARY KEY NOT NULL,"              \
            "ACCOUNT_ID         CHAR(8)     NOT NULL,"  \
            "ACCOUNT_PASSWD     CHAR(64)    NOT NULL,"  \
            "ACCOUNT_NAME       CHAR(255)   NOT NULL,"  \
            "ACCOUNT_LEVEL      INT         NOT NULL);";

    // 执行 SQL 语句，没有 callback
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);

    if(rc != SQLITE_OK){
        std::cerr << "create ACCOUNT table failed:" << zErrMsg << std::endl;
        sqlite3_free(zErrMsg); // 释放错误信息内存
        sqlite3_close(db);
        return -1;
    }

    // 建 COURSE 表
    sql = "CREATE TABLE IF NOT EXISTS COURSE("          \
            "ID INT PRIMARY KEY NOT NULL,"              \
            "COURSE_ID        CHAR(10)      NOT NULL,"  \
            "COURSE_NAME      CHAR(255)     NOT NULL,"  \
            "COURSE_CAPACITY  INT           NOT NULL,"  \
            "COURSE_SPARE     INT           NOT NULL,"  \
            "COURSE_WEEK      CHAR(1023)    NOT NULL,"  \
            "COURSE_DAY       CHAR(1023)    NOT NULL);";

    // 执行 SQL 语句，没有 callback
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);

    if(rc != SQLITE_OK){
        std::cerr << "create COURSE table failed:" << zErrMsg << std::endl;
        sqlite3_free(zErrMsg); // 释放错误信息内存
        sqlite3_close(db);
        return -1;
    }

     // 建 COURSE_SELECT 表
    sql = "CREATE TABLE IF NOT EXISTS COURSE_SELECT(" \
            "ID INT PRIMARY KEY NOT NULL,"            \
            "ACCOUNT_ID       CHAR(8)     NOT NULL,"  \
            "COURSE_ID_LIST   CHAR(1000)  NOT NULL);";

    // 执行 SQL 语句，没有 callback
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);

    if(rc != SQLITE_OK){
        std::cerr << "create COURSE table failed:" << zErrMsg << std::endl;
        sqlite3_free(zErrMsg); // 释放错误信息内存
        sqlite3_close(db);
        return -1;
    }

    std::cout << "create tables successed" << std::endl;
    // 关闭数据库
    sqlite3_close(db);
    return 0;
}
