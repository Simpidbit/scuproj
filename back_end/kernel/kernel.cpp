#include "simskt.h"
#include "kernel_config.h"
#include "parser.h"
#include "handler.h"

#include <iostream>

#include <unordered_map>
#include <string>
#include <functional>


/**
  * 入口：接收来自 server 的字节流数据（客户端请求数据）
  * 出口：向 server 发送处理结果
  */

/*
| 字段名          | 数据类型                   |
| --------------- | -------------------------- |
| account_id      | std::string                |
| account_level   | unsigned char              |
| account_passwd  | std::string                |
| course_capacity | unsigned int               |
| course_day      | std::vector<unsigned char> |
| course_id       | std::string                |
| course_name     | std::string                |
| course_name_len | unsigned char              |
| course_spare    | unsigned int               |
| course_week     | std::vector<unsigned char> |
| result          | unsigned char              |
| token           | std::string                |
| type            | unsigned char              |
*/

static void
print_data(const std::unordered_map<std::string, void *> &data)
{
  for (const auto &kv : data) {
    std::cout << kv.first << ": ";
    if (kv.first == "type") {
      std::cout << (int)*((unsigned char *)kv.second);
    } else if (kv.first == "account_id") {
      std::cout << *(std::string *)kv.second;
    } else if (kv.first == "account_passwd") {
      std::cout << *(std::string *)kv.second;
    } else if (kv.first == "account_level") {
      std::cout << (int)*((unsigned char*)kv.second);
    }
    std::cout << std::endl;
  }
}

void
handle(SP::Client *cli) 
{
  std::vector<unsigned char> raw = cli->brecvall();
  std::unordered_map<std::string, void *> data = parser::parse(raw);
  std::unordered_map<std::string, void *> result;
  // data 由各逻辑完成判断后释放!

  print_data(data); // TODO

  switch(*static_cast<unsigned char *>(data["type"])) {
    case 0:   // 登录请求
      result = handler::login_request_handle(data);
      break;
    case 2:   // 修改密码请求
      result = handler::modify_password_request_handle(data);
      break;
    case 4:   // 增加课程请求
      result = handler::add_course_request_handle(data);
      break;
    case 6:   // 选择课程请求
      result = handler::choose_course_request_handle(data);
      break;
    case 8:   // 查询单个课程信息请求
      result = handler::query_course_request_handle(data);
      break;
    case 10:  // 退课请求
      result = handler::remove_course_request_handle(data);
      break;
  }


  /* TODO */


  return;
}

int main()
{
  SP::Server serv;
  SP::Client *cli;

  serv.bind("0.0.0.0", LISTEN_PORT);
  serv.listen(32);

  for (;;) {
    cli = serv.accept();
    handle(cli);
    delete cli;
  }

  return 0;
}
