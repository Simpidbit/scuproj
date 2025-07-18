#include "database.h"
#include "handler.h"
#include "picosha2.h"

#include <cstring>

#include <vector>
#include <string>
#include <chrono>
#include <utility>

static std::unordered_map<std::string, std::pair<std::string, long long> > tokens;

static long long
get_timestamp()
{
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  long long millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  return millis;
}

// 判断此account_id是否有tokens存在
static bool
check_token(std::string account_id)
{
  return ::tokens.count(account_id) ? true : false;
}

// 刷新token有效时间
static void
update_token(std::string account_id)
{
  if (!check_token(account_id)) return;
  ::tokens[account_id].second = get_timestamp();
}

// 检查token是否已经超过有效时间
// 必须已有此token，否则返回true
// 已超过有效时间返回true
static bool
check_token_timeout(std::string account_id)
{
  if (!check_token(account_id)) return true;

  long long last_update = ::tokens[account_id].second;
  long long now = get_timestamp();

  if (now - last_update > 15 * 60 * 1000) return true;
  else return false;
}

// 检查token是否存在且有效
// 有效返回 3
static long long
check_token_correct(std::string account_id, std::string token)
{
  if (!check_token(account_id)) return 0;// token 不存在
  else if (::tokens[account_id].first != token) return 1;// token 不匹配
  else if(check_token_timeout(account_id)) return 2;// token 已过期
  else return 3; // token 有效  
}


// 删除token
static void
remove_token(std::string account_id)
{
  if (!check_token(account_id)) return;
  ::tokens.erase(account_id);
}

namespace handler {

std::unordered_map<std::string, void *>
login_request_handle(const std::unordered_map<std::string, void *> &data)
{
  Database db;
  std::unordered_map<std::string, void *> result;

  std::string   account_id      = *(std::string *)data.at("account_id");
  std::string   account_passwd  = *(std::string *)data.at("account_passwd");
  unsigned char account_level   = *(unsigned char *)data.at("account_level");

  delete (unsigned char *)data.at("type");
  delete (std::string *)data.at("account_id");
  delete (std::string *)data.at("account_passwd");
  delete (unsigned char *)data.at("account_level");

  std::vector<std::string> user = db.searchOne(TableName::ACCOUNT, "ACCOUNT_ID", account_id);
  unsigned char *res = new unsigned char;

  if (user.empty())
    *res = 1;   // 账号不存在
  else if (user[1] != account_passwd)
    *res = 2;   // 账号存在但密码错误
  else if (user[3] != std::to_string(account_level))
    *res = 3;   // 账号、密码都正确但权限错误
  else {  // 登录成功
    *res = 0;
    long long timestamp = get_timestamp();
    std::string timestamp_str = std::to_string(timestamp);
    std::string token = picosha2::sha256(timestamp_str + account_id + account_passwd);
    result["token"] = new std::string(token);

    ::tokens[account_id] = std::make_pair(token, timestamp);
  }

  result["result"] = res;
  return result;
}

std::unordered_map<std::string, void *>
modify_password_request_handle(const std::unordered_map<std::string, void *> &data)
{
  Database db;
  std::unordered_map<std::string, void *> result;

  std::string account_id  = *(std::string *)data.at("account_id");
  std::string new_passwd  = *(std::string *)data.at("account_passwd");
  std::string token       = *(std::string *)data.at("token");

  delete (std::string *)data.at("account_id");
  delete (std::string *)data.at("account_passwd");
  delete (std::string *)data.at("token");

  std::vector<std::string> user = db.searchOne(TableName::ACCOUNT, "ACCOUNT_ID", account_id);
  unsigned char *res = new unsigned char;
  if (!user.empty() && check_token_correct(account_id, token)==3) {
    if (db.updateOneById(TableName::ACCOUNT, "ACCOUNT_PASSWD", new_passwd, account_id)) {
      *res = 0; // 修改成功
      ::tokens[account_id].second = get_timestamp();
    }
  } else if(check_token_correct(account_id, token)==1) {
    *res = 100; // token 不匹配
  } else if(check_token_correct(account_id, token)==2) {
    *res = 101; // token 已过期
  }else {
    *res = 1;//其他错误
  }

  result["result"] = res;
  return result;
}

std::unordered_map<std::string, void *>
add_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::unordered_map<std::string, void *> result;


    std::vector<std::string> values = {
        *(std::string *)data.at("course_id"),
        *(std::string *)data.at("course_name"),
        std::to_string(*(unsigned int *)data.at("course_capacity")),
        std::to_string(*(unsigned int *)data.at("course_spare")),
        std::string((char *)((std::vector<unsigned char> *)data.at("course_week"))->data(), ((std::vector<unsigned char> *)data.at("course_week"))->size()),
        std::string((char *)((std::vector<unsigned char> *)data.at("course_day"))->data(), ((std::vector<unsigned char> *)data.at("course_day"))->size())
    };
    std::string token = *(std::string *)data.at("token");

    delete (std::string *)data.at("course_id");
    delete (std::string *)data.at("token");
    delete (std::string *)data.at("course_name");
    delete (unsigned int *)data.at("course_capacity");
    delete (unsigned int *)data.at("course_spare");
    delete (std::vector<unsigned char> *)data.at("course_week");
    delete (std::vector<unsigned char> *)data.at("course_day");

    unsigned char *res = new unsigned char;

    if (check_token_correct(values[0], token) == 1) {
        *res = 100; // token 错误
        result["result"] = res;
        return result;
    } else if (check_token_correct(values[0], token) == 2) {
        *res = 101; // token 已过期
        result["result"] = res;
        return result;
    }

    if (db.addOne(TableName::COURSE, values)) {
        *res = 0;
    } else if (db.getErrorMsg() && std::string(db.getErrorMsg()).find("UNIQUE constraint failed") != std::string::npos) {
        *res = 2; // 如果不是课程冲突，返回错误码 2
    } else {
        *res = 1; // 说明课程冲突
    }
    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
choose_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::string account_id = *(std::string *)data.at("account_id");
    std::string token       = *(std::string *)data.at("token");
    std::string course_id = *(std::string *)data.at("course_id");

    delete (std::string *)data.at("account_id");
    delete (std::string *)data.at("token");
    delete (std::string *)data.at("course_id");

    std::unordered_map<std::string, void *> result;
    unsigned char *res = new unsigned char;
    

    if(check_token_correct(account_id, token) == 1) {
        *res = 100; // token 错误
        result["result"] = res;
        return result;
    }
    else if(check_token_correct(account_id, token) == 2) {
        *res = 101; // token 已过期
        result["result"] = res;
        return result;
    }

    // 先查 spare
    std::vector<std::string> course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
    if (!course.empty() && std::stoi(course[3]) > 0) {
        // 更新 spare
        int spare = std::stoi(course[3]) - 1;
        db.updateOneById(TableName::COURSE, "COURSE_SPARE", std::to_string(spare), course_id);
        // 插入选课
        std::vector<std::string> select_values = {account_id + "_" + course_id, account_id, course_id};
        if (db.addOne(TableName::COURSE_SELECT, select_values)) {
            *res = 0;
            ::tokens[account_id].second = get_timestamp();
        } else {
            *res = 2; // 添加选课失败
        }
    } else {
        *res = 1;// 课程已满
    }
    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
query_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::string course_id = *(std::string *)data.at("course_id");
    delete (std::string *)data.at("course_id");


    std::unordered_map<std::string, void *> result;
    std::vector<std::string> course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
    unsigned char *res = new unsigned char;
    if (!course.empty()) {
        *res = 0;  // 查询成功
        result["course_id"] = new std::string(course[0]);
        result["course_name_len"] = new unsigned char(std::stoi(course[1]));
        result["course_name"] = new std::string(course[2]);
        result["course_capacity"] = new unsigned int(std::stoi(course[3]));
        result["course_spare"] = new unsigned int(std::stoi(course[4]));
        result["course_week"] = new std::vector<unsigned char>(course[5].begin(), course[5].end());
        result["course_day"] = new std::vector<unsigned char>(course[6].begin(), course[6].end());
    } else {
        *res = 1;  // 查询失败
    }
    result["result"] = res;//按照原有表格，查询失败没有返回值
    return result;
}

std::unordered_map<std::string, void *>
remove_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::string account_id = *(std::string *)data.at("account_id");
    std::string token = *(std::string *)data.at("token");
    std::string course_id = *(std::string *)data.at("course_id");
    delete (std::string *)data.at("account_id");
    delete (std::string *)data.at("token");
    delete (std::string *)data.at("course_id");

    



    std::unordered_map<std::string, void *> result;
    unsigned char *res = new unsigned char;

    if(check_token_correct(account_id, token) == 1) {
        *res = 100; // token 错误
        result["result"] = res;
        return result;
    }
    else if(check_token_correct(account_id, token) == 2) {
        *res = 101; // token 已过期
        result["result"] = res;
        return result;
    }
    
    // 删除选课
    std::string select_id = account_id + "_" + course_id;
    if (db.deleteOneById(TableName::COURSE_SELECT, select_id)) {
        // spare+1
        std::vector<std::string> course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
        if (!course.empty()) {
            int spare = std::stoi(course[3]) + 1;
            db.updateOneById(TableName::COURSE, "COURSE_SPARE", std::to_string(spare), course_id);
        }
        *res = 0;
        ::tokens[account_id].second = get_timestamp();
    } else {
        *res = 1; // 删除选课失败
    }
    result["result"] = res;
    return result;
}

} // namespace handler
