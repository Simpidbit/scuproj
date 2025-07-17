#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ctime>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include "picosha2.h"

// 前向声明
class User;
class Student;
class AcademicOffice;
class Course;
class CourseSelectionSystem;
class TimeSlot;

// 权限类型枚举（0=学生，1=管理员）
enum class PermissionType {
    STUDENT = 0,    // 学生权限（仅选课）
    ADMIN = 1       // 管理员权限（修改课程）
};

// 字符串管理类：为不含\0的字符串创建新空间并补\0
class NullTerminatedString {
private:
    char* data;      // 存储带\0的字符串
    size_t length;   // 字符串长度（不含\0）

public:
    // 从无\0的字符流和长度构造（核心功能）
    NullTerminatedString(const char* str, size_t len) : length(len) {
        if (len == 0) {
            throw std::invalid_argument("字符串长度不能为0");
        }
        data = new char[length + 1];  // 新空间 = 原长度 + 1（用于\0）
        std::memcpy(data, str, length);  // 拷贝原始数据
        data[length] = '\0';  // 补终止符
    }

    // 从std::string构造（兼容已有代码）
    NullTerminatedString(const std::string& str) : NullTerminatedString(str.c_str(), str.size()) {}

    // 析构函数：释放动态内存
    ~NullTerminatedString() {
        delete[] data;
    }

    // 拷贝构造：避免浅拷贝
    NullTerminatedString(const NullTerminatedString& other) : length(other.length) {
        data = new char[length + 1];
        std::memcpy(data, other.data, length + 1);  // 拷贝包括\0
    }

    // 移动构造：资源转移
    NullTerminatedString(NullTerminatedString&& other) noexcept : data(other.data), length(other.length) {
        other.data = nullptr;  // 原对象放弃所有权
        other.length = 0;
    }

    // 拷贝赋值：避免内存泄漏
    NullTerminatedString& operator=(const NullTerminatedString& other) {
        if (this != &other) {
            delete[] data;  // 释放原有资源
            length = other.length;
            data = new char[length + 1];
            std::memcpy(data, other.data, length + 1);
        }
        return *this;
    }

    // 移动赋值：资源转移
    NullTerminatedString& operator=(NullTerminatedString&& other) noexcept {
        if (this != &other) {
            delete[] data;  // 释放原有资源
            data = other.data;
            length = other.length;
            other.data = nullptr;  // 原对象放弃所有权
            other.length = 0;
        }
        return *this;
    }

    // 获取带\0的C风格字符串
    const char* c_str() const {
        return data;
    }

    // 获取字符串长度（不含\0）
    size_t size() const {
        return length;
    }

    // 转换为std::string（便于输出和比较）
    std::string str() const {
        return std::string(data, length);  // 仅取有效长度
    }
};

// 课程时间槽类
class TimeSlot {
public:
    // 课程周信息：[长度, 开始周, 结束周, ...]（如[2,1,10,11,16]表示1-10周、11-16周）
    std::vector<unsigned char> courseWeek;
    // 课程节次信息：线性表，格式为 [总段数, 周几1, 开始节1, 结束节1, 周几2, 开始节2, 结束节2, ...]
    std::vector<unsigned char> courseDay;

    TimeSlot(const std::vector<unsigned char>& week, const std::vector<unsigned char>& day)
        : courseWeek(week), courseDay(day) {
        // 验证周数范围（1-18周）
        for (size_t i = 1; i < courseWeek.size(); i += 2) {
            if (courseWeek[i] < 1 || courseWeek[i] > 18 || courseWeek[i + 1] < 1 || courseWeek[i + 1] > 18) {
                throw std::invalid_argument("周数必须在1-18之间");
            }
        }
        
        // 验证节次信息
        if (courseDay.empty()) {
            throw std::invalid_argument("节次信息不能为空");
        }
        
        unsigned char segmentCount = courseDay[0];  // 第一位为总段数
        if (courseDay.size() != 1 + segmentCount * 3) {  // 总长度 = 1 + 3*段数
            throw std::invalid_argument("节次信息长度与总段数不匹配");
        }
        
        // 验证节次范围（周几1-7，节次1-12）
        for (size_t i = 0; i < segmentCount; ++i) {
            size_t offset = 1 + i * 3;
            unsigned char weekday = courseDay[offset];
            unsigned char start = courseDay[offset + 1];
            unsigned char end = courseDay[offset + 2];
            
            if (weekday < 1 || weekday > 7 || start < 1 || start > 12 || end < 1 || end > 12) {
                throw std::invalid_argument("周几必须在1-7之间，节数必须在1-12之间");
            }
        }
    }

    bool weeksOverlap() const {
        // 提取所有周范围
        std::vector<std::pair<int, int>> weeklyRanges;
        for (size_t i = 1; i < courseWeek.size(); i += 2) {
            weeklyRanges.emplace_back(courseWeek[i], courseWeek[i + 1]);
        }

        // 按开始周排序
        std::sort(weeklyRanges.begin(), weeklyRanges.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        // 检查相邻周范围是否重叠
        for (size_t i = 1; i < weeklyRanges.size(); ++i) {
            if (weeklyRanges[i].first <= weeklyRanges[i - 1].second) {
                return true;  // 存在重叠
            }
        }

        return false;  // 无重叠
    }

    // 检查两个时间槽是否冲突
    bool conflictsWith(const TimeSlot& other) const {
        // 1. 检查周范围是否重叠
        if (!weeksOverlap()) return false;
        
        // 2. 检查同一天的节次是否重叠
        unsigned char mySegmentCount = courseDay[0];
        unsigned char otherSegmentCount = other.courseDay[0];
        
        for (size_t i = 0; i < mySegmentCount; ++i) {
            size_t myOffset = 1 + i * 3;
            int myWeekday = courseDay[myOffset];
            int myStart = courseDay[myOffset + 1];
            int myEnd = courseDay[myOffset + 2];
            
            for (size_t j = 0; j < otherSegmentCount; ++j) {
                size_t otherOffset = 1 + j * 3;
                int otherWeekday = other.courseDay[otherOffset];
                int otherStart = other.courseDay[otherOffset + 1];
                int otherEnd = other.courseDay[otherOffset + 2];
                
                if (myWeekday == otherWeekday && !(myEnd <= otherStart || otherEnd <= myStart)) {
                    return true;  // 节次冲突
                }
            }
        }
        return false;
    }

    // 转换为字符串表示
    std::string toString() const {
        std::stringstream ss;
        // 周信息
        ss << "Weeks: ";
        for (size_t i = 1; i < courseWeek.size(); i += 2) {
            ss << static_cast<int>(courseWeek[i]) << "-" << static_cast<int>(courseWeek[i + 1]) << "周, ";
        }
        // 节次信息
        ss << "Days: ";
        static const std::vector<std::string> dayNames = {"", "周一", "周二", "周三", "周四", "周五", "周六", "周日"};
        
        unsigned char segmentCount = courseDay[0];
        for (size_t i = 0; i < segmentCount; ++i) {
            size_t offset = 1 + i * 3;
            int weekday = courseDay[offset];
            int start = courseDay[offset + 1];
            int end = courseDay[offset + 2];
            ss << dayNames[weekday] << start << "-" << end << "节, ";
        }
        
        std::string res = ss.str();
        return res.substr(0, res.size() - 2);  // 移除末尾逗号
    }

private:
    std::vector<std::pair<int, int>> weeklyRanges;  // 存储多个不连续周区间
};

// 课程类
class Course {
public:
    // 构造函数：使用NullTerminatedString存储courseId和courseName
    Course(const std::string& id, const std::string& name, int capacity,
           PermissionType requiredPermission, const TimeSlot& timeSlot)
        : courseId(id), courseName(name), courseCapacity(capacity),
          courseSpare(capacity), requiredPermission(requiredPermission), timeSlot(timeSlot) {}

    // Getters（返回std::string便于使用）
    std::string getCourseId() const { return courseId.str(); }
    std::string getCourseName() const { return courseName.str(); }
    int getCourseCapacity() const { return courseCapacity; }
    int getCourseSpare() const { return courseSpare; }
    PermissionType getRequiredPermission() const { return requiredPermission; }
    const TimeSlot& getTimeSlot() const { return timeSlot; }

    // 选课相关方法
    bool hasRemainingCapacity() const { return courseSpare > 0; }
    bool enrollStudent() {
        if (courseSpare <= 0) return false;
        courseSpare--;
        return true;
    }
    void dropStudent() {
        if (courseSpare < courseCapacity) courseSpare++;
    }

private:
    NullTerminatedString courseId;         // 课程ID（10位纯数字，带\0）
    NullTerminatedString courseName;       // 课程名称（带\0）
    int courseCapacity;                    // 总容量
    int courseSpare;                       // 课余量
    PermissionType requiredPermission;     // 选课所需权限
    TimeSlot timeSlot;                     // 课程时间
};

// 用户基类
class User {
public:
    // 构造函数：使用NullTerminatedString存储accountId和accountName
    User(const std::string& id, const std::string& name, const std::string& passwd, PermissionType permission)
        : accountId(id), accountName(name), accountPasswd(picosha2::sha256(passwd)), permissionLevel(permission) {}

    virtual ~User() = default;

    // Getters（返回std::string便于使用）
    std::string getAccountId() const { return accountId.str(); }
    std::string getAccountName() const { return accountName.str(); }
    std::string getAccountPasswd() const { return accountPasswd; }
    PermissionType getPermissionLevel() const { return permissionLevel; }

    // 虚函数，由子类实现
    virtual void displayInfo() const = 0;

protected:
    NullTerminatedString accountId;        // 账户ID（8位纯数字，带\0）
    NullTerminatedString accountName;      // 账户名称（带\0）
    std::string accountPasswd;             // 加密后的密码
    PermissionType permissionLevel;        // 权限等级
};

// 学生类
class Student : public User {
public:
    Student(const std::string& id, const std::string& name, const std::string& passwd)
        : User(id, name, passwd, PermissionType::STUDENT) {}

    void displayInfo() const override {
        std::cout << "Student ID: " << accountId.str() << ", Name: " << accountName.str() << std::endl;
    }

    const std::vector<std::shared_ptr<Course>>& getCourseSchedule() const {
        std::lock_guard<std::mutex> lock(scheduleMutex);
                return courseSchedule;
    }

    bool addCourse(const std::shared_ptr<Course>& course) {
        std::lock_guard<std::mutex> lock(scheduleMutex);
        
        // 检查课程时间是否冲突
        if (hasTimeConflict(course)) {
            std::cout << "Time conflict with existing courses!" << std::endl;
            return false;
        }
        // 检查权限
        if (permissionLevel < course->getRequiredPermission()) {
            std::cout << "Insufficient permission to take this course!" << std::endl;
            return false;
        }
        courseSchedule.push_back(course);
        return true;
    }

    bool removeCourse(const std::shared_ptr<Course>& course) {
        auto it = std::find(courseSchedule.begin(), courseSchedule.end(), course);
        if (it != courseSchedule.end()) {
            courseSchedule.erase(it);
            return true;
        }
        return false;
    }

private:
    std::vector<std::shared_ptr<Course>> courseSchedule;
    mutable std::mutex scheduleMutex;  // 添加互斥锁

    bool hasTimeConflict(const std::shared_ptr<Course>& newCourse) const {
        const TimeSlot& newTimeSlot = newCourse->getTimeSlot();
        for (const auto& course : courseSchedule) {
            if (course->getTimeSlot().conflictsWith(newTimeSlot)) {
                return true;
            }
        }
        return false;
    }
};

// 教务处类（管理员）
class AcademicOffice : public User {
public:
    AcademicOffice(const std::string& id, const std::string& name, const std::string& passwd)
        : User(id, name, passwd, PermissionType::ADMIN) {}

    void displayInfo() const override {
        std::cout << "Admin ID: " << accountId.str() << ", Name: " << accountName.str() << std::endl;
    }

    std::shared_ptr<Course> createCourse(const std::string& id, const std::string& name, int capacity,PermissionType requiredPermission, const TimeSlot& timeSlot) {
        return std::make_shared<Course>(id, name, capacity, requiredPermission, timeSlot);
    }
};

// 选课系统类
class CourseSelectionSystem {
public:
    void addUser(const std::shared_ptr<User>& user) {
        if (isValidAccountId(user->getAccountId())) {
            users[user->getAccountId()] = user;
        } else {
            std::cout << "Invalid account ID (must be 8 digits)!" << std::endl;
        }
    }

    void addCourse(const std::shared_ptr<Course>& course) {
        if (isValidCourseId(course->getCourseId())) {
            courses[course->getCourseId()] = course;
        } else {
            std::cout << "Invalid course ID (must be 10 digits)!" << std::endl;
        }
    }

    bool studentSelectCourse(const std::string& studentId, const std::string& courseId) {
        auto studentIt = users.find(studentId);
        auto courseIt = courses.find(courseId);
        if (studentIt == users.end() || courseIt == courses.end()) {
            std::cout << "Student or course not found!" << std::endl;
            return false;
        }

        Student* student = dynamic_cast<Student*>(studentIt->second.get());
        if (!student) {
            std::cout << "User is not a student!" << std::endl;
            return false;
        }

        if (!courseIt->second->hasRemainingCapacity()) {
            std::cout << "Course is already full!" << std::endl;
            return false;
        }

        if (student->addCourse(courseIt->second)) {
            courseIt->second->enrollStudent();
            return true;
        }
        return false;
    }

    
    bool studentDropCourse(const std::string& studentId, const std::string& courseId) {
            auto studentIt = users.find(studentId);
            auto courseIt = courses.find(courseId);
            if (studentIt == users.end() || courseIt == courses.end()) {
                std::cout << "Student or course not found!" << std::endl;
                return false;
            }

            Student* student = dynamic_cast<Student*>(studentIt->second.get());
            if (!student) {
                std::cout << "User is not a student!" << std::endl;
                return false;
            }

            if (student->removeCourse(courseIt->second)) {
                courseIt->second->dropStudent();
                return true;
            }
            return false;
        }


    void displayAllCourses() const {
        std::cout << "All available courses:" << std::endl;
        for (const auto& pair : courses) {
            const auto& course = pair.second;
            std::cout << "Course ID: " << course->getCourseId()
                      << ", Name: " << course->getCourseName()
                      << ", Capacity: " << (course->getCourseCapacity() - course->getCourseSpare())
                      << "/" << course->getCourseCapacity()
                      << ", Time: " << course->getTimeSlot().toString()
                      << std::endl;
        }
    }

    void displayStudentSchedule(const std::string& studentId) const {
        auto studentIt = users.find(studentId);
        if (studentIt == users.end()) {
            std::cout << "Student not found!" << std::endl;
            return;
        }

        Student* student = dynamic_cast<Student*>(studentIt->second.get());
        if (!student) {
            std::cout << "User is not a student!" << std::endl;
            return;
        }

        std::cout << "Schedule for student " << student->getAccountName() << " (" << studentId << "):" << std::endl;
        const auto& schedule = student->getCourseSchedule();
        if (schedule.empty()) {
            std::cout << "No courses selected." << std::endl;
            return;
        }
        for (const auto& course : schedule) {
            std::cout << "Course ID: " << course->getCourseId()
                      << ", Name: " << course->getCourseName()
                      << ", Time: " << course->getTimeSlot().toString()
                      << std::endl;
        }
    }

    // 登录验证
    std::pair<bool, std::string> login(const std::string& accountId, const std::string& accountPasswd, PermissionType permission) {
        auto userIt = users.find(accountId);
        if (userIt == users.end()) {
            return {false, "账号不存在"};
        }
        auto user = userIt->second;
        if (user->getAccountPasswd() != accountPasswd) {
            return {false, "账号存在但密码错误"};
        }
        if (user->getPermissionLevel() != permission) {
            return {false, "账号、密码都正确，但权限错误"};
        }
        // 生成token
        auto now = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        std::string tokenStr = now + accountId + accountPasswd;
        std::string token = picosha2::sha256(tokenStr);
        // 存储token和过期时间
        tokens[accountId] = {token, std::chrono::system_clock::now() + std::chrono::minutes(15)};
        return {true, token};
    }

    // 验证token
    bool verifyToken(const std::string& accountId, const std::string& token) {
        auto it = tokens.find(accountId);
        if (it == tokens.end()) {
            return false;
        }
        auto& tokenInfo = it->second;
        if (tokenInfo.first != token || std::chrono::system_clock::now() > tokenInfo.second) {
            tokens.erase(it);
            return false;
        }
        return true;
    }

private:
    std::map<std::string, std::shared_ptr<User>> users;
    std::map<std::string, std::shared_ptr<Course>> courses;
    std::map<std::string, std::pair<std::string, std::chrono::time_point<std::chrono::system_clock>>> tokens;

    // 验证账户ID（8位数字）
    bool isValidAccountId(const std::string& id) {
        return id.size() == 8 && std::all_of(id.begin(), id.end(), ::isdigit);
    }

    // 验证课程ID（10位数字）
    bool isValidCourseId(const std::string& id) {
        return id.size() == 10 && std::all_of(id.begin(), id.end(), ::isdigit);
    }
};

// 主函数示例
int main() {
    CourseSelectionSystem system;

    // 创建管理员（账户ID：8位数字）
    auto admin = std::make_shared<AcademicOffice>("10000001", "教务处王老师", "admin123");
    system.addUser(admin);

    // 创建学生（账户ID：8位数字）
   auto student1 = std::make_shared<Student>("00000001", "张三", "student1");
   auto student2 = std::make_shared<Student>("00000002", "李四", "student2");
    system.addUser(student1);
    system.addUser(student2);

    // 创建课程时间（符合线性表格式，第一位为总段数）
    // 高等数学：1-16周，周一1-2节
    TimeSlot mathTime({2, 1, 16}, {1, 1, 1, 2});  // 总段数1，周几1，开始节1，结束节2
    // 大学物理：1-16周，周二3-4节
    TimeSlot physicsTime({2, 1, 16}, {1, 2, 3, 4});  // 总段数1，周几2，开始节3，结束节4
    // 计算机科学：1-16周，周三1-2节和周五3-4节
    TimeSlot csTime({2, 1, 16}, {2, 3, 1, 2, 5, 3, 4});  // 总段数2

    // 创建课程（课程ID：10位数字）
    auto mathCourse = admin->createCourse("0000000001", "高等数学", 30,PermissionType::STUDENT,mathTime);
    auto physicsCourse = admin->createCourse("0000000002", "大学物理", 25, PermissionType::STUDENT, physicsTime);
    auto csCourse = admin->createCourse("0000000003", "计算机科学", 20, PermissionType::ADMIN, csTime);

    system.addCourse(mathCourse);
    system.addCourse(physicsCourse);
    system.addCourse(csCourse);

    // 学生1选课（高等数学）
    std::cout << "Student 1 selecting Math (should succeed): "
              << (system.studentSelectCourse("00000001", "0000000001") ? "Success" : "Failed") << std::endl;

    // 学生1选计算机科学（权限不足）
    std::cout << "Student 1 selecting Computer Science (permission denied): "
              << (system.studentSelectCourse("00000001", "0000000003") ? "Success" : "Failed") << std::endl;

    // 学生2选计算机科学（权限不足）
    std::cout << "Student 2 selecting Computer Science (permission denied): "
              << (system.studentSelectCourse("00000002", "0000000003") ? "Success" : "Failed") << std::endl;
    // 学生1退课（高等数学）
    std::cout << "Student 1 dropping Math (should succeed): "
                  << (system.studentDropCourse("00000001", "0000000001") ? "Success" : "Failed") << std::endl;

        // 显示学生1课表
        std::cout << "\n--- Student 1 Schedule after dropping Math ---" << std::endl;
        system.displayStudentSchedule("00000001");

    // 显示所有课程
    std::cout << "\n--- All Courses ---" << std::endl;
    system.displayAllCourses();

    // 显示学生课表
    std::cout << "\n--- Student 1 Schedule ---" << std::endl;
    system.displayStudentSchedule("00000001");

    std::cout << "\n--- Student 2 Schedule ---" << std::endl;
    system.displayStudentSchedule("00000002");

    // 登录测试
    auto [loginSuccess, token] = system.login("00000001", picosha2::sha256("student1"), PermissionType::STUDENT);
    if (loginSuccess) {
        std::cout << "Login success! Token: " << token << std::endl;
        // 验证token
        if (system.verifyToken("00000001", token)) {
            std::cout << "Token verification success!" << std::endl;
        } else {
            std::cout << "Token verification failed!" << std::endl;
        }
    } else {
        std::cout << "Login failed: " << token << std::endl;
    }

    return 0;
}
