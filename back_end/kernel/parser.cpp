#include "parser.h"

/**
 * @brief 从原始字节流信息(raw)中截取一段字节串，
 *        由此在堆区构造std::string对象.
 *
 * @param raw 原始字节流信息.
 * @param first 截取的开始位置.
 * @param last 截取的最后一个位置.
 *
 * @return 返回在堆区构造的std::string对象指针.
 */
static std::string *
cut_to_string(
  const std::vector<unsigned char> &raw,
  size_t first,
  size_t last)
{
  std::vector<char> buffer(last - first + 2, 0);
  memcpy(&buffer[0], &raw[first], last - first + 1);
  std::string *s = new std::string(&buffer[0]);
  return s;
}


/**
 * @brief 将原始字节流信息(raw)中的某两个连续字节
 *        解释为小端序的2字节整型数，转换为unsigned int，
 *        并将得到的unsigned int存放在堆区.
 *
 * @param first 2字节整型数的低位地址.
 *
 * @return 得到的存放在堆区的unsigned int.
 */
static unsigned int *
int_2bytes_little(
  const std::vector<unsigned char> &raw,
  size_t first) 
{
  unsigned int *n = new unsigned int(0);
  unsigned char *p = 
    static_cast<unsigned char *>(static_cast<void *>(n));

  *p = raw[first];
  *(p + 1) = raw[first + 1];
  return n;
}

/**
 * @brief 复制原始字节流信息(raw)中的一段，
 *        并将复制结果存于堆内存中的std::vector<unsigned char>对象.
 *
 * @return 复制得到的在堆内存中的std::vector<unsigned char>对象指针.
 */
static std::vector<unsigned char> *
bytes_to_vec(
  const std::vector<unsigned char> &raw,
  size_t first, 
  size_t last) 
{
  std::vector<unsigned char> *vec = new std::vector<unsigned char>(last - first + 1, 0);
  for (size_t i = first; i != last + 1; i++)
    (*vec)[i - first] = raw[i];
  return vec;
}


/**
 * @brief 解析登录请求.
 */
static void
login_request_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["account_level"] = new unsigned char(raw[1]);
  data["account_id"] = cut_to_string(raw, 2, 9);
  data["account_passwd"] = cut_to_string(raw, 10, 73);
}

/**
 * @brief 解析登录结果.
 */
static void
login_result_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["result"] = new unsigned char(raw[1]);
  data["token"] = cut_to_string(raw, 2, 65);
}

/**
 * @brief 解析修改密码请求.
 */
static void
modify_password_request_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["account_id"] = cut_to_string(raw, 1, 8);
  data["account_passwd"] = cut_to_string(raw, 9, 72);
  data["token"] = cut_to_string(raw, 73, 136);
}

/**
 * @brief 解析修改密码结果.
 */
static void
modify_password_result_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["result"] = new unsigned char(raw[1]);
}

/**
 * @brief 解析增加课程请求.
 */
static void
add_course_request_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["account_id"] = cut_to_string(raw, 1, 8);
  data["token"] = cut_to_string(raw, 9, 72);
  data["course_id"] = cut_to_string(raw, 73, 82);
  data["course_name_len"] = new unsigned char(raw[83]);
  unsigned char N = *static_cast<unsigned char *>(data["course_name_len"]);
  data["course_name"] = cut_to_string(raw, 84, 84 + N - 1);
  data["course_capacity"] = int_2bytes_little(raw, 84 + N);
  data["course_spare"] = int_2bytes_little(raw, 84 + N + 2);
  unsigned char w = raw[84 + N + 4];
  data["course_week"] = bytes_to_vec(raw, 84 + N + 4, 84 + N + w + 3);
  data["course_day"] = bytes_to_vec(raw, 84 + N + w + 4, 84 + N + w + 3 + 1 + (3 * raw[84 + N + w + 4]));
}


/**
 * @brief 解析增加课程结果.
 */
static void
add_course_result_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["result"] = new unsigned char(raw[1]);
}

/**
 * @brief 解析选择课程请求.
 */
static void
choose_course_request_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["account_id"] = cut_to_string(raw, 1, 8);
  data["token"] = cut_to_string(raw, 9, 72);
  data["course_id"] = cut_to_string(raw, 73, 82);
}

/**
 * @brief 解析选择课程结果.
 */
static void
choose_course_result_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["result"] = new unsigned char(raw[1]);
}


/**
 * @brief 解析查询单个课程信息请求.
 */
static void
query_course_request_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["course_id"] = cut_to_string(raw, 1, 10);
}

/**
 * @brief 解析查询单个课程信息结果.
 */
static void
query_course_result_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["course_id"] = cut_to_string(raw, 1, 10);
  data["course_name_len"] = new unsigned char(raw[11]);
  unsigned char N = *static_cast<unsigned char *>(data["course_name_len"]);
  data["course_name"] = cut_to_string(raw, 12, 12 + N - 1);
  data["course_capacity"] = int_2bytes_little(raw, 12 + N);
  data["course_spare"] = int_2bytes_little(raw, 12 + N + 2);
  unsigned char w = raw[12 + N + 4];
  data["course_week"] = bytes_to_vec(raw, 12 + N + 4, 12 + N + w + 3);
  data["course_day"] = bytes_to_vec(raw, 12 + N + w + 4, 12 + N + w + 3 + 1 + (3 * raw[12 + N + w + 4]));
}


/**
 * @brief 解析退课请求.
 */
static void
remove_course_request_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["account_id"] = cut_to_string(raw, 1, 8);
  data["token"] = cut_to_string(raw, 9, 72);
  data["course_id"] = cut_to_string(raw, 73, 82);
}

/**
 * @brief 解析退课结果.
 */
static void
remove_course_result_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["result"] = new unsigned char(raw[1]);
}


/**
 * @brief 查询学生已选课程的请求.
 */
static void
query_student_selection_request_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["account_id"] = cut_to_string(raw, 1, 8);
  data["token"] = cut_to_string(raw, 9, 72);
}


/**
 * @brief 查询学生已选课程的结果.
 */
static void
query_student_selection_result_parse(
  std::unordered_map<std::string, void *> &data,
  const std::vector<unsigned char> &raw)
{
  data["result"] = new unsigned char(raw[1]);
  data["course_id_list_len"] = new unsigned char(raw[2]);
  data["course_id_list"] = cut_to_string(raw, 3, raw.size() - 1);
}

namespace parser {

// 主解析函数
std::unordered_map<std::string, void *>
parse(const std::vector<unsigned char> &raw)
{
  assert(raw.size() >= 1);

  std::unordered_map<std::string, void *> data;
  data["type"] = static_cast<void *>(new unsigned char);
  *static_cast<unsigned char *>(data["type"]) = raw[0];

  switch (raw[0]) {
    case 0:   // 登录请求
      login_request_parse(data, raw);
      break;
    case 1:   // 登录结果
      login_result_parse(data, raw);
      break;
    case 2:   // 修改密码请求
      modify_password_request_parse(data, raw);
      break;
    case 3:   // 修改密码结果
      modify_password_result_parse(data, raw);
      break;
    case 4:   // 增加课程请求
      add_course_request_parse(data, raw);
      break;
    case 5:   // 增加课程结果
      add_course_result_parse(data, raw);
      break;
    case 6:   // 选择课程请求
      choose_course_request_parse(data, raw);
      break;
    case 7:   // 选择课程结果
      choose_course_result_parse(data, raw);
      break;
    case 8:   // 查询单个课程信息请求
      query_course_request_parse(data, raw);
      break;
    case 9:   // 查询单个课程信息结果
      query_course_result_parse(data, raw);
      break;
    case 10:  // 退课请求
      remove_course_request_parse(data, raw);
      break;
    case 11:  // 退课结果
      remove_course_result_parse(data, raw);
      break;
    case 12:  // 查询学生已选课程的请求
      query_student_selection_request_parse(data, raw);
      break;
    case 13:  // 查询学生已选课程的结果
      query_student_selection_result_parse(data, raw);
      break;
  }
  return data;
}


std::string
vec_to_dbstr(std::vector<unsigned char> &vec)
{
  std::string dbstr = "";

  unsigned int tmp;

  for (auto iter = vec.begin(); iter != vec.end(); iter++) {
    tmp = (unsigned int) (*iter);
    dbstr += std::to_string(tmp);
    if (iter == vec.end() - 1) break;
    dbstr += ',';
  }

  return dbstr;
}

std::vector<unsigned char> *
dbstr_to_vec(std::string &str)
{
  std::cout << "dbstr_to_vec: " << str << "#" << std::endl;
  std::vector<unsigned char> *vecp = new std::vector<unsigned char>;
  std::string tmp;

  for (auto iter = str.begin(); iter != str.end(); iter++) {
    tmp += *iter;
    if (*iter == ',') {
      vecp->push_back(std::stoi(tmp));
      tmp = "";
    }
  }
  vecp->push_back(std::stoi(tmp));

  return vecp;
}

} // namespace parser
