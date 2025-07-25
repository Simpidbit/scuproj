# 编程实战：选课系统

| 目录      | 内容                 |
| --------- | -------------------- |
| docs      | 各类文档             |
| front_end | 前端代码（含README） |
| back_end  | 后端代码（含README） |



---

## 文档索引

- [数据格式](docs/数据格式.md)
- [后端数据交互格式](docs/后端数据交互格式.md)



## 项目架构

此项目为学生选课管理系统，主要功能是管理员添加课程、学生选择课程。项目整体采用B/S架构，即前端为Web网页，用户直接在Web网页中交互，后端由两个模块组成：server模块负责处理前端发来的POST请求（携带用户的登录请求、选课请求等信息）、将POST请求携带的信息转发给kernel模块，kernel模块负责接收server模块发来的请求信息，并结合数据库中的数据进行业务处理（如根据数据库中的数据判断密码是否正确、将学生的选课结果写入数据库等），再将处理结果发回server模块，server模块将处理结果发回前端作为反馈。另有一个独立的nginx服务端用于向用户提供静态资源（网页的HTML、JS、CSS等）。



## 项目具体细节

前端与后端server模块交互，包括前端向后端发送的POST请求和后端给前端反馈的结果，都是HTTP协议附带JSON格式的数据，具体的JSON键值结构是[后端数据交互格式](docs/后端数据交互格式.md)中每个量的英文名称作为Key，其值作为Value。

后端server模块使用Python Flask框架。

后端server模块和kernel模块之间的交互使用socket实现，文档[后端数据交互格式](docs/后端数据交互格式.md)中描述了报文的具体格式。

所有数据库操作都由kernel模块执行，使用SQLite3数据库。