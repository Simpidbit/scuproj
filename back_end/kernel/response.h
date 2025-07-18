#ifndef SCUPROJ_KERNEL_RESPONSE_H
#define SCUPROJ_KERNEL_RESPONSE_H

#include <cstring>

#include <string>
#include <unordered_map>
#include <vector>

namespace response {

std::vector<unsigned char>
login_result(std::unordered_map<std::string, void *> &result);

std::vector<unsigned char>
modify_password_result(std::unordered_map<std::string, void *> &result);

std::vector<unsigned char>
add_course_result(std::unordered_map<std::string, void *> &result);


std::vector<unsigned char>
choose_course_result(std::unordered_map<std::string, void *> &result);

std::vector<unsigned char>
query_course_result(std::unordered_map<std::string, void *> &result);

std::vector<unsigned char>
remove_course_result(std::unordered_map<std::string, void *> &result);

std::vector<unsigned char>
query_student_selection_result(std::unordered_map<std::string, void *> &result);


} // namespace response

#endif  // SCUPROJ_KERNEL_RESPONSE_H
