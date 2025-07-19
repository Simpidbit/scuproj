#include "database.h"
#include "handler.h"
#include "picosha2.h"
#include "parser.h"
#include "pit.h"

#include <cstring>

#include <algorithm>
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
static int
check_token_correct(std::string account_id, std::string token)
{
  if (!check_token(account_id)) return 0;// token 不存在
  else if (::tokens[account_id].first != token) return 1;// token 不匹配
  else if (check_token_timeout(account_id)) {
    ::tokens.erase(account_id);
    return 2;// token 已过期
  }
  else return 3; // token 有效  
}


// 删除token
static void
remove_token(std::string account_id)
{
  if (!check_token(account_id)) return;
  ::tokens.erase(account_id);
}


static unsigned char
get_account_level(Database &db, const std::string &account_id)
{
  std::vector<std::string> user = db.searchOne(TableName::ACCOUNT, "ACCOUNT_ID", account_id);
  if (user.empty()) return 255;

  return std::stoi(user[3]);
}

namespace handler {

std::unordered_map<std::string, void *>
login_request_handle(const std::unordered_map<std::string, void *> &data)
{
  Database db;
  std::unordered_map<std::string, void *> result;
  result["type"] = new unsigned char(1);


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
  result["type"] = new unsigned char(3);

  std::string account_id  = *(std::string *)data.at("account_id");
  std::string new_passwd  = *(std::string *)data.at("account_passwd");
  std::string token       = *(std::string *)data.at("token");

  delete (std::string *)data.at("account_id");
  delete (std::string *)data.at("account_passwd");
  delete (std::string *)data.at("token");

  std::vector<std::string> user = db.searchOne(TableName::ACCOUNT, "ACCOUNT_ID", account_id);
  unsigned char *res = new unsigned char;

  if (check_token_correct(account_id, token) == 3)
    update_token(account_id);

  if (!user.empty() && check_token_correct(account_id, token)==3) {
    ::tokens[account_id].second = get_timestamp();
    if (db.updateOneById(TableName::ACCOUNT, "ACCOUNT_PASSWD", new_passwd, account_id)) {
      *res = 0; // 修改成功
    }
  } else if(check_token_correct(account_id, token)==1) {
    *res = 100; // token 不匹配
  } else if(check_token_correct(account_id, token)==2) {
    *res = 101; // token 已过期
  } else {
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
    result["type"] = new unsigned char(5);

    std::vector<std::string> values = {
        *(std::string *)data.at("course_id"),
        *(std::string *)data.at("course_name"),
        std::to_string(*(unsigned int *)data.at("course_capacity")),
        std::to_string(*(unsigned int *)data.at("course_spare")),
        parser::vec_to_dbstr(*(std::vector<unsigned char> *)data.at("course_week")),
        parser::vec_to_dbstr(*(std::vector<unsigned char> *)data.at("course_day"))
    };

    std::string account_id = *(std::string *)data.at("account_id");
    std::string token = *(std::string *)data.at("token");

    delete (std::string *)data.at("course_id");
    delete (std::string *)data.at("token");
    delete (std::string *)data.at("course_name");
    delete (unsigned int *)data.at("course_capacity");
    delete (unsigned int *)data.at("course_spare");
    delete (std::vector<unsigned char> *)data.at("course_week");
    delete (std::vector<unsigned char> *)data.at("course_day");


    unsigned char *res = new unsigned char;
      result["result"] = res;

    unsigned char account_level = get_account_level(db, account_id);
    if (account_level != 1) {
      *res = 2;
      return result;
    }

    if (check_token_correct(account_id, token) == 3)
      update_token(account_id);

    if (check_token_correct(values[0], token) == 1) {
        *res = 100; // token 错误
        return result;
    } else if (check_token_correct(values[0], token) == 2) {
        *res = 101; // token 已过期
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

static bool
check_segment_overlap(
  std::vector<std::pair<unsigned char, unsigned char> > &seg1,
  std::vector<std::pair<unsigned char, unsigned char> > &seg2)
{
  auto cmp_func = 
    [] (const std::pair<unsigned char, unsigned char> &e1,
        const std::pair<unsigned char, unsigned char> &e2) -> bool {
      return e1.first < e2.first;
    };

  auto min_num = 
    [] (const unsigned char &e1,
        const unsigned char &e2) -> unsigned char {
      return e1 < e2 ? e1 : e2;
    };

  auto max_num = 
    [] (const unsigned char &e1,
        const unsigned char &e2) -> unsigned char {
      return e1 < e2 ? e2 : e1;
    };

  std::sort(seg1.begin(), seg1.end(), cmp_func);
  std::sort(seg2.begin(), seg2.end(), cmp_func);

  unsigned char min_val = min_num(seg1.front().first, seg2.front().first);
  unsigned char max_val = max_num(seg1.back().second, seg2.back().second);

  std::vector<unsigned char> ground(max_val - min_val + 1, 0);
  for (auto &segpair : seg1) {
    for (unsigned char i = segpair.first; i != segpair.second + 1; i++)
      ground[i - min_val] = 1;
  }

  bool has_overlap = false;
  for (auto &segpair : seg2) {
    for (unsigned char i = segpair.first; i != segpair.second + 1; i++) {
      if (ground[i - min_val]) {
        has_overlap = true;
        break;
      }
    }
  }

  return has_overlap;
}

static bool
check_course_conflict(
  Database &          db,
  const std::string & account_id,
  const std::string & course_id, 
  const std::string & course_id_list)
{
  unsigned char course_id_list_len = static_cast<unsigned char>(course_id_list.size() / 10);
  std::string cur_course_id;

  std::vector<unsigned char> *cur_course_week;
  std::vector<unsigned char> *cur_course_day;
  std::vector<std::pair<unsigned char, unsigned char> > cur_week_seg;
  std::vector<std::pair<unsigned char, unsigned char> > cur_someday_seg;

  std::vector<unsigned char> *target_course_week;
  std::vector<unsigned char> *target_course_day;
  unsigned char target_course_weekday_stat[1 + 7];
  memset(target_course_weekday_stat, 0, 1 + 7);

  std::vector<std::pair<unsigned char, unsigned char> > target_week_seg;
  std::vector<std::pair<unsigned char, unsigned char> > target_someday_seg;

  auto week_vec_to_week_seg = 
    [] (std::vector<unsigned char> &week_vec,
        std::vector<std::pair<unsigned char, unsigned char> > &week_seg) -> void {
      week_seg.clear();

      unsigned char segnum = (week_vec[0] - 1) / 2;
      for (unsigned char i = 0; i != segnum; i++)
        week_seg.push_back(std::make_pair(week_vec[1 + i * 2], week_vec[1 + i * 2 + 1]));
    };

  auto someday_to_seg = 
    [] (unsigned char whichday,
        std::vector<unsigned char> &someday_vec,
        std::vector<std::pair<unsigned char, unsigned char> > &someday_seg) -> void {
      someday_seg.clear();

      unsigned char segnum = someday_vec[0];
      for (unsigned char i = 0; i != segnum; i++) {
        unsigned char this_weekday = someday_vec[3 * i + 1];
        if (this_weekday == whichday)   // 拾进来
          someday_seg.push_back(std::make_pair(someday_vec[3 * i + 2], someday_vec[3 * i + 3]));
      }
    };


  std::vector<std::string> target_course_info = 
    db.searchOne(TableName::COURSE, "COURSE_ID", course_id);


  target_course_week = parser::dbstr_to_vec(target_course_info[4]);
  week_vec_to_week_seg(*target_course_week, target_week_seg);

  target_course_day = parser::dbstr_to_vec(target_course_info[5]);
  unsigned char target_seg_num = (*target_course_day)[0];
  
  for (unsigned char i = 0; i != target_seg_num; i++) {
    unsigned char this_weekday = (*target_course_day)[i * 3 + 1];
    target_course_weekday_stat[this_weekday] = 1;
  }

  for (unsigned char i = 0; i != course_id_list_len; i++) {
    cur_course_id = course_id_list.substr(i * 10, 10);

    std::vector<std::string> course_info =
      db.searchOne(TableName::COURSE, "COURSE_ID", cur_course_id);

    cur_course_week = parser::dbstr_to_vec(course_info[4]);
    week_vec_to_week_seg(*cur_course_week, cur_week_seg);

    cur_course_day = parser::dbstr_to_vec(course_info[5]);

    if (check_segment_overlap(target_week_seg, cur_week_seg)) {
      // 周有重叠
      // 检查是否有同一日重叠
      std::vector<unsigned char> same_weekday;
      unsigned char cur_seg_num = (*cur_course_day)[0];

      for (unsigned char i = 0; i != cur_seg_num; i++) {
        unsigned char this_weekday = (*cur_course_day)[i * 3 + 1];
        if (target_course_weekday_stat[this_weekday])
          same_weekday.push_back(this_weekday);   // 有同一日重叠
      }

      // 检查每个重叠的日子有没有课程重叠
      bool day_conflict = false;
      for (auto &each_same_weekday : same_weekday) {
        someday_to_seg(each_same_weekday, *cur_course_day, cur_someday_seg);
        someday_to_seg(each_same_weekday, *target_course_day, target_someday_seg);
        day_conflict = check_segment_overlap(cur_someday_seg, target_someday_seg);
        if (day_conflict) return true;
      }
    }
  }

  return false;
}

std::unordered_map<std::string, void *>
choose_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::string account_id  = *(std::string *)data.at("account_id");
    std::string token       = *(std::string *)data.at("token");
    std::string course_id   = *(std::string *)data.at("course_id");

    delete (std::string *)data.at("account_id");
    delete (std::string *)data.at("token");
    delete (std::string *)data.at("course_id");


    std::unordered_map<std::string, void *> result;
    result["type"] = new unsigned char(7);
    unsigned char *res = new unsigned char;
    result["result"] = res;

    if (get_account_level(db, account_id) != 0) {
      *res = 1;
      return result;
    }

    if (check_token_correct(account_id, token) == 3)
      update_token(account_id);

    int token_stat = check_token_correct(account_id, token);
    if (token_stat == 0 || token_stat == 1) {
        *res = 100; // token 错误
        return result;
    }
    else if (token_stat == 2) {
        *res = 101; // token 已过期
        return result;
    }

    // 先查 spare
    std::vector<std::string> course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
    int spare = std::stoi(course[3]);

    if (!course.empty() && std::stoi(course[3]) > 0) {


      std::vector<std::string> student_selection = db.searchOne(TableName::COURSE_SELECT, "ACCOUNT_ID", account_id);

      if (student_selection.empty()) {
        // 不存在这个学生选课记录，说明以前从未选课
        if (db.addOne(TableName::COURSE_SELECT, {account_id, course_id})) {
          // 更新 spare
          db.updateOneById(TableName::COURSE, "COURSE_SPARE", std::to_string(spare - 1), course_id);
          *res = 0;
        }
        else *res = 1;
        return result;
      }

      std::string course_id_list = student_selection[1];
      unsigned char course_id_list_len = static_cast<unsigned char>(course_id_list.size() / 10);

      // 检查学生是否已经选择此课程
      bool has_chosen = false;
      for (unsigned char i = 0; i != course_id_list_len; i++) {
        if (course_id == course_id_list.substr(i * 10, 10)) {
          has_chosen = true;  // 已选择
          break;
        }
      }

      if (has_chosen) {
        // 已选择
        *res = 1;
      } else {
        // 未选择，更新上去

        // 检查是否与已选课程冲突
        if (check_course_conflict(db, account_id, course_id, course_id_list)) {
          *res = 1;     // 冲突
        }
        else {          // 未冲突
          *res = 0;
          std::string new_course_id_list = course_id_list + course_id;
          db.updateOneById(TableName::COURSE_SELECT, "COURSE_ID_LIST", new_course_id_list, account_id);
          // 更新 spare
          db.updateOneById(TableName::COURSE, "COURSE_SPARE", std::to_string(spare - 1), course_id);
        }
      }
    } else *res = 1;// 其他错误

    return result;
}

std::unordered_map<std::string, void *>
query_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::string course_id = *(std::string *)data.at("course_id");
    delete (std::string *)data.at("course_id");

    std::unordered_map<std::string, void *> result;
    result["type"] = new unsigned char(9);
    std::vector<std::string> course = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
    unsigned char *res = new unsigned char;
    if (!course.empty()) {
      *res = 0;
      result["course_id"] = new std::string(course[0]);
      result["course_name"] = new std::string(course[1]);
      result["course_name_len"] = new unsigned char((unsigned char)course[1].size());

      result["course_capacity"] = new unsigned int(std::stoi(course[2]));
      result["course_spare"] = new unsigned int(std::stoi(course[3]));
      //result["course_week"] = new std::vector<unsigned char>(course[5].begin(), course[5].end());
      //result["course_day"] = new std::vector<unsigned char>(course[6].begin(), course[6].end());
      result["course_week"] = (void *)parser::dbstr_to_vec(course[4]);
      result["course_day"] = (void *)parser::dbstr_to_vec(course[5]);
    } else *res = 1;

    result["result"] = res;
    return result;
}

std::unordered_map<std::string, void *>
remove_course_request_handle(const std::unordered_map<std::string, void *> &data)
{
    Database db;
    std::string account_id  = *(std::string *)data.at("account_id");
    std::string token       = *(std::string *)data.at("token");
    std::string course_id   = *(std::string *)data.at("course_id");

    delete (std::string *)data.at("account_id");
    delete (std::string *)data.at("token");
    delete (std::string *)data.at("course_id");

    std::unordered_map<std::string, void *> result;
    result["type"] = new unsigned char(11);
    unsigned char *res = new unsigned char;
    result["result"] = res;

    if (get_account_level(db, account_id) != 0) {
      *res = 1;
      return result;
    }


    if (check_token_correct(account_id, token) == 3)
      update_token(account_id);

    int token_stat = check_token_correct(account_id, token);
    if(token_stat == 0 || token_stat == 1) {
        *res = 100; // token 错误
        return result;
    }
    else if(check_token_correct(account_id, token) == 2) {
        *res = 101; // token 已过期
        return result;
    }
    
    // 删除选课
    std::vector<std::string> student_selection = db.searchOne(TableName::COURSE_SELECT, "ACCOUNT_ID", account_id);

    if (student_selection.empty()) {
      *res = 1;   // 没有这条记录
      return result;
    }

    std::string course_id_list = student_selection[1];
    unsigned char course_id_list_len = static_cast<unsigned char>(course_id_list.size() / 10);

    for (unsigned char i = 0; i != course_id_list_len; i++) {
      if (course_id == course_id_list.substr(i * 10, 10)) {
        // 就是删除这个课
        std::string backlist;

        if (i == course_id_list_len - 1)
          backlist = "";    // 删除的是最后一个课
        else
          backlist = course_id_list.substr((i + 1) * 10, ((unsigned int)(course_id_list_len - i - 1)) * 10);

        std::string new_course_id_list = course_id_list.substr(0, i * 10) + backlist;

        if (db.updateOneById(TableName::COURSE_SELECT, "COURSE_ID_LIST", new_course_id_list, account_id)) {

          std::vector<std::string> courseinfo = db.searchOne(TableName::COURSE, "COURSE_ID", course_id);
          int spare = std::stoi(courseinfo[3]);

          db.updateOneById(TableName::COURSE, "COURSE_SPARE", std::to_string(spare + 1), course_id);

          *res = 0;
          return result;
        } else {
          *res = 1;
          return result;
        }
      }
    }

    // 能到这里说明学生没选这个课
    *res = 1;
    return result;
}


std::unordered_map<std::string, void *>
query_student_selection_request_handle(
  const std::unordered_map<std::string, void *> &data)
{
  Database db;
  std::unordered_map<std::string, void *> result;
  result["type"] = new unsigned char(13);

  std::string account_id  = *(std::string *)data.at("account_id");
  std::string token       = *(std::string *)data.at("token");

  std::cout << "token = " << token << std::endl;
  std::cout << "account_id" << account_id << std::endl;
  std::cout << "correct token = " << ::tokens[account_id].first << std::endl;

  delete (std::string *)data.at("account_id");
  delete (std::string *)data.at("token");
  
  std::vector<std::string> user = db.searchOne(TableName::COURSE_SELECT, "ACCOUNT_ID", account_id);
  unsigned char *res = new unsigned char;

  if (check_token_correct(account_id, token) == 3)
    update_token(account_id);

  if (user.empty())
    *res = 1;
  else if (check_token_correct(account_id, token) == 0)
    *res = 100;
  else if (check_token_correct(account_id, token) == 1)
    *res = 100;
  else if (check_token_correct(account_id, token) == 2)
    *res = 101;
  else {
    *res = 0;
    result["course_id_list_len"] = new unsigned char(static_cast<unsigned char>(user[1].size() / 10));
    result["course_id_list"] = new std::string(user[1]);
  }

  result["result"] = res;
  return result;
}

} // namespace handler
