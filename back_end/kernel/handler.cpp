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
// 有效返回 true
static bool
check_token_correct(std::string account_id, std::string token)
{
  if (!check_token(account_id)) return false;
  return (::tokens[account_id].first == token
       && !check_token_timeout(account_id));
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

  std::string   account_id      = *(std::string *)data["account_id"];
  std::string   account_passwd  = *(std::string *)data["account_passwd"];
  unsigned char account_level   = *(unsigned char *)data["account_level"];

  delete (unsigned char *)data["type"];
  delete (std::string *)data["account_id"];
  delete (std::string *)data["account_passwd"];
  delete (unsigned char *)data["account_level"];

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

    ::tokens[account_id] = std::make_pair<std::string, long long>(token, timestamp);
  }

  result["result"] = res;
  return result;
}

std::unordered_map<std::string, void *>
modify_password_request_handle(const std::unordered_map<std::string, void *> &data)
{
  Database db;
  std::unordered_map<std::string, void *> result;

  std::string account_id  = *(std::string *)data["account_id"];
  std::string new_passwd  = *(std::string *)data["account_passwd"];
  std::string token       = *(std::string *)data["token"];

  delete (std::string *)data["account_id"];
  delete (std::string *)data["account_passwd"];
  delete (std::string *)data["token"];

  std::vector<std::string> user = db.searchOne(TableName::ACCOUNT, "ACCOUNT_ID", account_id);
  unsigned char *res = new unsigned char;
  if (!user.empty() && check_token_correct(account_id, token)) {
    if (db.updateOneById(TableName::ACCOUNT, "ACCOUNT_PASSWD", new_passwd, account_id))
      *res = 1; // 修改成功
    else
      *res = 0; // 数据库更新失败
  } else *res = 0; // token 错误

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

} // namespace handler
