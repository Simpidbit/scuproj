#ifndef SCUPROJ_KERNEL_HANDLER_H
#define SCUPROJ_KERNEL_HANDLER_H

#include <unordered_map>
#include <string>

namespace handler {

std::unordered_map<std::string, void *>
login_request_handle(
  const std::unordered_map<std::string, void *> &data);

std::unordered_map<std::string, void *>
modify_password_request_handle(
  const std::unordered_map<std::string, void *> &data);

std::unordered_map<std::string, void *>
add_course_request_handle(
  const std::unordered_map<std::string, void *> &data);

std::unordered_map<std::string, void *>
choose_course_request_handle(
  const std::unordered_map<std::string, void *> &data);

std::unordered_map<std::string, void *>
query_course_request_handle(
  const std::unordered_map<std::string, void *> &data);

std::unordered_map<std::string, void *>
remove_course_request_handle(
  const std::unordered_map<std::string, void *> &data);

std::unordered_map<std::string, void *>
query_student_selection_request_handle(
  const std::unordered_map<std::string, void *> &data);

} // namespace handler

#endif // SCUPROJ_KERNEL_HANDLER_H
