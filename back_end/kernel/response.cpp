#include "response.h"

namespace response {


std::vector<unsigned char>
login_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;
  resmsg.resize(66);
  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];
  memcpy(&resmsg[2], static_cast<std::string *>(result["token"])->c_str(), 64);

  return resmsg;
}

std::vector<unsigned char>
modify_password_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);
  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  return resmsg;
}

std::vector<unsigned char>
add_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);
  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  return resmsg;
}


std::vector<unsigned char>
choose_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);
  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  return resmsg;
}

std::vector<unsigned char>
query_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  size_t N = static_cast<size_t>(*(unsigned char *)result["course_name_len"]);
  size_t w = ((std::vector<unsigned char> *)result["course_week"])->size();
  size_t d = ((std::vector<unsigned char> *)result["course_day"])->size();

  resmsg.resize(12 + N + w + d + 4);

  resmsg[0] = *(unsigned char *)result["type"];
  memcpy(&resmsg[1], ((std::string *)result["course_id"])->c_str(), 10);
  resmsg[11] = *(unsigned char *)result["course_name_len"];
  memcpy(&resmsg[12], ((std::string *)result["course_name"])->c_str(), N);
  memcpy(&resmsg[12 + N], result["course_capacity"], 2);
  memcpy(&resmsg[12 + N + 2], result["course_spare"], 2);
  memcpy(&resmsg[12 + N + 4], &(*(std::vector<unsigned char> *)result["course_week"])[0], w);
  memcpy(&resmsg[12 + N + 4 + w], &(*(std::vector<unsigned char> *)result["course_day"])[0], d);

  return resmsg;
}

std::vector<unsigned char>
remove_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);
  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  return resmsg;
}


std::vector<unsigned char>
query_student_selection_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  unsigned char course_id_list_len = *(unsigned char *)result["course_id_list_len"];
  resmsg.resize(3 + 10 * course_id_list_len);

  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];
  resmsg[2] = *(unsigned char *)result["course_id_list_len"];

  memcpy(&resmsg[3], ((std::string *)result["course_id_list"])->c_str(), course_id_list_len * 10);

  return resmsg;
}


} // namespace response
