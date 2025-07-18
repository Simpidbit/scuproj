#include "database.h"
#include "handler.h"
#include <cstring>

std::unordered_map<std::string, void *>
login_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    auto account_id = *(std::string *)data.at("account_id");
    auto account_passwd = *(std::string *)data.at("account_passwd");
    std::unordered_map<std::string, void *> result;
    auto user = db.searchOne(TableName::ACCOUNT, "ACCOUNT_ID", account_id);
    unsigned char *res = new unsigned char;
    if (!user.empty() && user.size() >= 3 && user[1] == account_passwd) {
        *res = 1; // 登录成功
        result["account_level"] = new unsigned char((unsigned char)std::stoi(user[3]));
        result["account_name"] = new std::string(user[2]);
    } else {
        *res = 0; // 登录失败
    }
    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
modify_password_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    auto account_id = *(std::string *)data.at("account_id");
    auto old_passwd = *(std::string *)data.at("account_passwd");
    auto new_passwd = *(std::string *)data.at("new_passwd");
    std::unordered_map<std::string, void *> result;
    auto user = db.searchOne(TableName::ACCOUNT, "ACCOUNT_ID", account_id);
    unsigned char *res = new unsigned char;
    if (!user.empty() && user.size() >= 2 && user[1] == old_passwd) {
        if (db.updateOneById(TableName::ACCOUNT, "ACCOUNT_PASSWD", new_passwd, account_id)) {
            *res = 1; // 修改成功
        } else {
            *res = 0; // 数据库更新失败
        }
    } else {
        *res = 0; // 原密码错误
    }
    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
add_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::vector<std::string> values = {
        *(std::string *)data.at("counse_id"),
        *(std::string *)data.at("course_name"),
        std::to_string(*(unsigned int *)data.at("course_capacity")),
        std::to_string(*(unsigned int *)data.at("course_spare")),
        *(std::string *)data.at("course_week"),
        *(std::string *)data.at("course_day")
    };
    std::unordered_map<std::string, void *> result;
    unsigned char *res = new unsigned char;
    if (db.addOne(TableName::COURSE, values)) {
        *res = 1;
    } else {
        *res = 0;
    }
    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
choose_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    auto student_id = *(std::string *)data.at("account_id");
    auto course_id = *(std::string *)data.at("course_id");
    std::unordered_map<std::string, void *> result;
    unsigned char *res = new unsigned char;
    // 这里假设 COURSE_SELECT 只存一条选课记录，实际可根据表结构调整
    std::vector<std::string> values = {student_id, course_id};
    // 先查 spare
    auto course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
    if (!course.empty() && std::stoi(course[3]) > 0) {
        // 更新 spare
        int spare = std::stoi(course[3]) - 1;
        db.updateOneById(TableName::COURSE, "COURSE_SPARE", std::to_string(spare), course_id);
        // 插入选课
        std::vector<std::string> select_values = {student_id + "_" + course_id, student_id, course_id};
        if (db.addOne(TableName::COURSE_SELECT, select_values)) {
            *res = 1;
        } else {
            *res = 0;
        }
    } else {
        *res = 0;
    }
    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
query_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    auto course_id = *(std::string *)data.at("course_id");
    std::unordered_map<std::string, void *> result;
    auto course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
    unsigned char *res = new unsigned char;
    if (!course.empty()) {
        *res = 1;
        result["course_name"] = new std::string(course[1]);
        result["course_capacity"] = new unsigned int(std::stoi(course[2]));
        result["course_spare"] = new unsigned int(std::stoi(course[3]));
        result["course_week"] = new std::string(course[4]);
        result["course_day"] = new std::string(course[5]);
    } else {
        *res = 0;
    }
    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
remove_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    auto student_id = *(std::string *)data.at("account_id");
    auto course_id = *(std::string *)data.at("course_id");
    std::unordered_map<std::string, void *> result;
    unsigned char *res = new unsigned char;
    // 删除选课
    std::string select_id = student_id + "_" + course_id;
    if (db.deleteOneById(TableName::COURSE_SELECT, select_id)) {
        // spare+1
        auto course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
        if (!course.empty()) {
            int spare = std::stoi(course[3]) + 1;
            db.updateOneById(TableName::COURSE, "COURSE_SPARE", std::to_string(spare), course_id);
        }
        *res = 1;
    } else {
        *res = 0;
    }
    result["result"] = res;
    return result;
}
