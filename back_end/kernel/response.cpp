#include "response.h"
#include "pit.h"    // TODO: delete

namespace response {


std::vector<unsigned char>
login_result(std::unordered_map<std::string, void *> &result)
{

  std::vector<unsigned char> resmsg;
  unsigned char result_stat = *(unsigned char *)result["result"];
  if (result_stat == 0)
    resmsg.resize(66);
  else
    resmsg.resize(2);


  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = result_stat;

  if (result_stat == 0)
    memcpy(&resmsg[2], static_cast<std::string *>(result["token"])->c_str(), 64);


  delete (unsigned char *)result["type"];
  delete (unsigned char *)result["result"];
  
  if (result_stat == 0)
    delete (std::string *)result["token"];


  return resmsg;
}

std::vector<unsigned char>
modify_password_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);
  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  delete (unsigned char *)result["type"];
  delete (unsigned char *)result["result"];

  return resmsg;
}

std::vector<unsigned char>
add_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);

  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  delete (unsigned char *)result["type"];
  delete (unsigned char *)result["result"];

  return resmsg;
}


std::vector<unsigned char>
choose_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);

  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  delete (unsigned char *)result["type"];
  delete (unsigned char *)result["result"];

  return resmsg;
}

std::vector<unsigned char>
query_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  unsigned char result_stat = *(unsigned char *)result["result"];
  if (result_stat == 1) {
    resmsg.resize(2);
    resmsg[0] = *(unsigned char *)result["type"];
    resmsg[1] = result_stat;
    delete (unsigned char *)result["type"];
    delete(unsigned char *)result["result"];
    return resmsg;
  }

  size_t N = static_cast<size_t>(*(unsigned char *)result["course_name_len"]);
  size_t w = ((std::vector<unsigned char> *)result["course_week"])->size();
  size_t d = ((std::vector<unsigned char> *)result["course_day"])->size();

  resmsg.resize(13 + N + w + d + 4);

  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = result_stat;
  memcpy(&resmsg[2], ((std::string *)result["course_id"])->c_str(), 10);
  resmsg[12] = *(unsigned char *)result["course_name_len"];
  memcpy(&resmsg[13], ((std::string *)result["course_name"])->c_str(), N);
  memcpy(&resmsg[13 + N], result["course_capacity"], 2);
  memcpy(&resmsg[13 + N + 2], result["course_spare"], 2);
  memcpy(&resmsg[13 + N + 4], &(*(std::vector<unsigned char> *)result["course_week"])[0], w);
  memcpy(&resmsg[13 + N + 4 + w], &(*(std::vector<unsigned char> *)result["course_day"])[0], d);

  delete (unsigned char *)result["type"];
  delete (unsigned char *)result["result"];
  delete (std::string *)result["course_id"];
  delete (unsigned char *)result["course_name_len"];
  delete (std::string *)result["course_name"];
  delete (unsigned int *)result["course_capacity"];
  delete (unsigned int *)result["course_spare"];
  delete (std::vector<unsigned char> *)result["course_week"];
  delete (std::vector<unsigned char> *)result["course_day"];

  return resmsg;
}

std::vector<unsigned char>
remove_course_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  resmsg.resize(2);
  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];

  delete (unsigned char *)result["type"];
  delete (unsigned char *)result["result"];

  return resmsg;
}


std::vector<unsigned char>
query_student_selection_result(std::unordered_map<std::string, void *> &result)
{
  std::vector<unsigned char> resmsg;

  unsigned char result_stat = *(unsigned char *)result["result"];

  if (result_stat != 0) {
    resmsg.resize(2);
    resmsg[0] = *(unsigned char *)result["type"];
    resmsg[1] = result_stat;
    delete (unsigned char *)result["type"];
    delete (unsigned char *)result["result"];

    return resmsg;
  }

  unsigned char course_id_list_len = *(unsigned char *)result["course_id_list_len"];
  resmsg.resize(3 + 10 * course_id_list_len);

  resmsg[0] = *(unsigned char *)result["type"];
  resmsg[1] = *(unsigned char *)result["result"];
  resmsg[2] = *(unsigned char *)result["course_id_list_len"];

  memcpy(&resmsg[3], ((std::string *)result["course_id_list"])->c_str(), course_id_list_len * 10);

  delete (unsigned char *)result["type"];
  delete (unsigned char *)result["result"];
  delete (unsigned char *)result["course_id_list_len"];
  delete (std::string *)result["course_id_list"];

  return resmsg;
}


} // namespace response
