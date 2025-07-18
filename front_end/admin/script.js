// 定义学生数组
let students = [];
// 定义课程数组
let courses = [];

// 分页相关变量
let studentCurrentPage = 1;
let courseCurrentPage = 1;
const itemsPerPage = 8; // 每页显示 8 条信息

// 从 JSON 文件加载学生信息
async function loadStudents() {
    try {
        const response = await fetch('stu_info.json');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        students = await response.json();
        renderStudents();
    } catch (error) {
        console.error('加载学生信息时出错:', error);
    }
}

// 从 JSON 文件加载课程信息
async function loadCourses() {
    try {
        const response = await fetch('course_info.json');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        courses = await response.json();
        renderCourses();
    } catch (error) {
        console.error('加载课程信息时出错:', error);
    }
}

// 渲染学生列表
function renderStudents() {
    const studentList = document.getElementById('studentList');
    studentList.innerHTML = '';

    // 计算当前页要显示的学生范围
    const startIndex = (studentCurrentPage - 1) * itemsPerPage;
    const endIndex = startIndex + itemsPerPage;
    const paginatedStudents = students.slice(startIndex, endIndex);

    paginatedStudents.forEach(student => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${student.account_id}</td>
            <td>${student.account_name}</td>
            <td>${student.account_level === 0 ? '学生' : '管理员'}</td>
            <td>
                <button class="action-btn edit-btn" onclick="showPasswordModal('${student.account_id}')">修改密码</button>
                <button class="action-btn edit-btn" onclick="showModal('${student.account_id}')">编辑</button>
                <button class="action-btn delete-btn" onclick="deleteStudent('${student.account_id}')">删除</button>
            </td>
        `;
        studentList.appendChild(row);
    });

    // 渲染学生分页控件
    renderStudentPagination();
}

// 渲染学生分页控件
function renderStudentPagination() {
    const pagination = document.getElementById('studentPagination');
    pagination.innerHTML = '';

    const totalPages = Math.ceil(students.length / itemsPerPage);

    // 上一页按钮
    const prevButton = document.createElement('button');
    prevButton.textContent = '上一页';
    prevButton.disabled = studentCurrentPage === 1;
    prevButton.onclick = () => {
        studentCurrentPage--;
        renderStudents();
    };
    pagination.appendChild(prevButton);

    // 页码显示
    for (let i = 1; i <= totalPages; i++) {
        const pageButton = document.createElement('button');
        pageButton.textContent = i;
        pageButton.classList.toggle('active', i === studentCurrentPage);
        pageButton.onclick = () => {
            studentCurrentPage = i;
            renderStudents();
        };
        pagination.appendChild(pageButton);
    }

    // 下一页按钮
    const nextButton = document.createElement('button');
    nextButton.textContent = '下一页';
    nextButton.disabled = studentCurrentPage === totalPages;
    nextButton.onclick = () => {
        studentCurrentPage++;
        renderStudents();
    };
    pagination.appendChild(nextButton);
}

// 渲染课程列表
function renderCourses() {
    const courseList = document.getElementById('courseList');
    courseList.innerHTML = '';

    // 计算当前页要显示的课程范围
    const startIndex = (courseCurrentPage - 1) * itemsPerPage;
    const endIndex = startIndex + itemsPerPage;
    const paginatedCourses = courses.slice(startIndex, endIndex);

    paginatedCourses.forEach(course => {
        // 处理课程周显示，不同区间用 <br> 分隔
        const courseWeekChunks = [];
        for (let i = 1; i < course.course_week.length; i += 2) {
            courseWeekChunks.push(`${course.course_week[i]}-${course.course_week[i + 1]}周`);
        }
        const courseWeekStr = courseWeekChunks.join('<br>');

        // 处理课程节次显示，不同时间段用 <br> 分隔
        const courseDayChunks = [];
        const weekDays = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
        for (let i = 1; i < course.course_day.length; i += 3) {
            const dayIndex = course.course_day[i] - 1;
            courseDayChunks.push(`${weekDays[dayIndex]}${course.course_day[i + 1]}-${course.course_day[i + 2]}节`);
        }
        const courseDayStr = courseDayChunks.join('<br>');

        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${course.course_id}</td>
            <td>${course.course_name}</td>
            <td>${course.course_capacity}</td>
            <td>${course.course_spare}</td>
            <td>${courseWeekStr}</td>
            <td>${courseDayStr}</td>
            <td>
                <button class="action-btn edit-btn" onclick="showCourseModal('${course.course_id}')">编辑</button>
                <button class="action-btn delete-btn" onclick="deleteCourse('${course.course_id}')">删除</button>
            </td>
        `;
        courseList.appendChild(row);
    });

    // 渲染课程分页控件
    renderCoursePagination();
}

// 渲染课程分页控件
function renderCoursePagination() {
    const pagination = document.getElementById('coursePagination');
    pagination.innerHTML = '';

    const totalPages = Math.ceil(courses.length / itemsPerPage);

    // 上一页按钮
    const prevButton = document.createElement('button');
    prevButton.textContent = '上一页';
    prevButton.disabled = courseCurrentPage === 1;
    prevButton.onclick = () => {
        courseCurrentPage--;
        renderCourses();
    };
    pagination.appendChild(prevButton);

    // 页码显示
    for (let i = 1; i <= totalPages; i++) {
        const pageButton = document.createElement('button');
        pageButton.textContent = i;
        pageButton.classList.toggle('active', i === courseCurrentPage);
        pageButton.onclick = () => {
            courseCurrentPage = i;
            renderCourses();
        };
        pagination.appendChild(pageButton);
    }

    // 下一页按钮
    const nextButton = document.createElement('button');
    nextButton.textContent = '下一页';
    nextButton.disabled = courseCurrentPage === totalPages;
    nextButton.onclick = () => {
        courseCurrentPage++;
        renderCourses();
    };
    pagination.appendChild(nextButton);
}

// 初始化时渲染课程列表和分页控件
renderCourses();

function showStudentManagement() {
    //奇妙bug，函数调用无效
    displayStudentManagement();
    studentPagination.style.display = 'block';
    coursePagination.style.display = 'none';
}

// 显示课程管理界面
function showCourseManagement() {
    const studentTable = document.getElementById('studentTable');
    const courseTable = document.getElementById('courseTable');
    const pageTitle = document.getElementById('pageTitle');
    const addStudentBtn = document.getElementById('addStudentBtn');
    const addCourseBtn = document.getElementById('addCourseBtn');
    const studentPagination = document.getElementById('studentPagination');
    const coursePagination = document.getElementById('coursePagination');

    if (studentTable && courseTable && pageTitle && addStudentBtn && addCourseBtn && studentPagination && coursePagination) {
        studentTable.style.display = 'none';
        courseTable.style.display = 'table';
        pageTitle.textContent = '课程管理';
        addStudentBtn.style.display = 'none';
        addCourseBtn.style.display = 'block';
        studentPagination.style.display = 'none';
        coursePagination.style.display = 'block';
        renderCourses(); // 渲染课程列表
    } else {
        console.error('部分元素未找到，请检查 HTML 文件。');
    }
}

// 显示学生信息弹窗
function showModal(id) {
    const modal = document.getElementById('myModal');
    const modalTitle = document.getElementById('modalTitle');
    const modalStudentId = document.getElementById('modalStudentId');
    const modalStudentName = document.getElementById('modalStudentName');
    const modalStudentRole = document.getElementById('modalStudentRole');

    if (id) {
        const student = students.find(student => student.account_id === id);
        if (student) {
            modalTitle.textContent = '编辑学生';
            modalStudentId.value = student.account_id;
            modalStudentId.disabled = true; 
            modalStudentName.value = student.account_name;
            modalStudentRole.value = student.account_level === 0 ? '学生' : '管理员';
        }
    } else {
        modalTitle.textContent = '添加学生';
        modalStudentId.value = '';
        modalStudentId.disabled = false; 
        modalStudentName.value = '';
        modalStudentRole.value = '学生';
    }
    modal.style.display = 'block';
}

// 关闭学生信息弹窗
function closeModal() {
    const modal = document.getElementById('myModal');
    modal.style.display = 'none';
}

// 保存学生信息
function saveStudent() {
    const modalStudentId = document.getElementById('modalStudentId');
    const accountId = modalStudentId.value;
    const accountName = document.getElementById('modalStudentName').value;
    const accountLevel = document.getElementById('modalStudentRole').value === '学生' ? 0 : 1;

    if (!modalStudentId.disabled) {
        if (!/^\d+$/.test(accountId)) {
            alert('学号必须为纯数字，请重新输入！');
            return;
        }
        if (!accountId) {
            alert('学号不能为空，请填写！');
            return;
        }
    }

    if (!accountName) {
        alert('姓名不能为空，请填写！');
        return;
    }

    const existingStudent = students.find(student => student.account_id === accountId);
    const studentData = {
        account_id: accountId,
        account_name: accountName,
        account_level: accountLevel,
        // 保留原密码，若为新增学生，需添加修改密码逻辑
        account_passwd: existingStudent ? existingStudent.account_passwd : '' 
    };

    if (existingStudent) {
        const index = students.findIndex(student => student.account_id === accountId);
        students[index] = studentData;
    } else {
        students.push(studentData);
    }
    renderStudents();
    // 调用关闭弹窗函数
    closeModal(); 
}

// 显示修改密码弹窗
function showPasswordModal(id) {
    const student = students.find(student => student.account_id === id);
    if (student) {
        const newPassword = prompt('请输入新密码', '');
        if (newPassword!== null) {
            student.password = newPassword;
            renderStudents();
        }
    }
}

// 删除学生信息
function deleteStudent(id) {
    showDeleteModal(id, 'student');
}

// 显示课程信息弹窗
function showCourseModal(id) {
    const modal = document.getElementById('courseModal');
    const modalTitle = document.getElementById('courseModalTitle');
    const modalCourseId = document.getElementById('modalCourseId');
    const modalCourseName = document.getElementById('modalCourseName');
    const modalCourseCapacity = document.getElementById('modalCourseCapacity');
    const modalCourseSpare = document.getElementById('modalCourseSpare');
    const modalCourseWeek = document.getElementById('modalCourseWeek');
    const modalCourseDay = document.getElementById('modalCourseDay');

    if (id) {
        const course = courses.find(course => course.course_id === id);
        if (course) {
            modalTitle.textContent = '编辑课程';
            modalCourseId.value = course.course_id;
            modalCourseId.disabled = true;
            modalCourseName.value = course.course_name;
            modalCourseCapacity.value = course.course_capacity;
            modalCourseSpare.value = course.course_spare;
            modalCourseWeek.value = course.course_week.slice(1).join(' ');
            modalCourseDay.value = course.course_day.slice(1).join(' ');
        }
    } else {
        modalTitle.textContent = '添加课程';
        modalCourseId.value = '';
        modalCourseId.disabled = false;
        modalCourseName.value = '';
        modalCourseCapacity.value = '';
        modalCourseSpare.value = '';
        modalCourseWeek.value = '';
        modalCourseDay.value = '';
    }
    modal.style.display = 'block';
}

// 保存课程信息
function saveCourse() {
    const modalCourseId = document.getElementById('modalCourseId');
    const courseId = modalCourseId.value;
    const courseName = document.getElementById('modalCourseName').value;
    const courseCapacity = parseInt(document.getElementById('modalCourseCapacity').value);
    const courseSpare = parseInt(document.getElementById('modalCourseSpare').value);
    const courseWeekInput = document.getElementById('modalCourseWeek').value.trim().split(' ').map(Number);
    const courseDayInput = document.getElementById('modalCourseDay').value.trim().split(' ').map(Number);

    // 课程号验证
    if (!modalCourseId.disabled) {
        if (!/^\d{10}$/.test(courseId)) {
            alert('课程号必须为 10 位纯数字，请重新输入！');
            return;
        }
    }

    // 课程名验证
    if (!courseName || new TextEncoder().encode(courseName).length > 255) {
        alert('课程名不能为空且长度需在 255 字节内，请重新输入！');
        return;
    }

    // 课容量验证
    if (isNaN(courseCapacity) || courseCapacity <= 0) {
        alert('课容量必须为正整数，请重新输入！');
        return;
    }

    // 课余量验证
    if (isNaN(courseSpare) || courseSpare < 0 || courseSpare > courseCapacity) {
        alert('课余量必须为非负整数且不超过课容量，请重新输入！');
        return;
    }

    // 课程周验证
    if (courseWeekInput.length % 2!== 0 || courseWeekInput.some(isNaN)) {
        alert('课程周输入格式错误，请重新输入！');
        return;
    }
    const courseWeek = [courseWeekInput.length / 2, ...courseWeekInput];

    // 课程节次验证
    if (courseDayInput.length % 3!== 0 || courseDayInput.some(isNaN)) {
        alert('课程节次输入格式错误，请重新输入！');
        return;
    }
    const courseDay = [courseDayInput.length / 3, ...courseDayInput];

    const courseData = {
        course_id: courseId,
        course_name: courseName,
        course_capacity: courseCapacity,
        course_spare: courseSpare,
        course_week: courseWeek,
        course_day: courseDay
    };

    if (courses.some(course => course.course_id === courseId)) {
        const index = courses.findIndex(course => course.course_id === courseId);
        courses[index] = courseData;
    } else {
        courses.push(courseData);
    }
    renderCourses();
    closeCourseModal();
}

// 关闭课程信息弹窗
function closeCourseModal() {
    const modal = document.getElementById('courseModal');
    modal.style.display = 'none';
}

// 删除课程
function deleteCourse(id) {
    if (confirm('确定要删除该课程吗？')) {
        courses = courses.filter(course => course.course_id!== id);
        renderCourses();
    }
}

let deletingItemId = null;
let deletingItemType = null;

// 显示删除确认弹窗
function showDeleteModal(id, type) {
    deletingItemId = id;
    deletingItemType = type;
    const deleteModal = document.getElementById('deleteModal');
    deleteModal.style.display = 'block';
}

// 关闭删除确认弹窗
function closeDeleteModal() {
    const deleteModal = document.getElementById('deleteModal');
    deleteModal.style.display = 'none';
    deletingItemId = null;
    deletingItemType = null;
}

// 确认删除操作
function confirmDelete() {
    if (deletingItemId && deletingItemType) {
        if (deletingItemType === 'student') {
            students = students.filter(student => student.id!== deletingItemId);
            renderStudents();
        } else if (deletingItemType === 'course') {
            const [courseId, courseNumber] = deletingItemId.split('-');
            courses = courses.filter(course =>!(course.id === courseId && course.number === courseNumber));
            renderCourses();
        }
        closeDeleteModal();
    }
}

// Display the student management interface
function displayStudentManagement() {
    const studentTable = document.getElementById('studentTable');
    const courseTable = document.getElementById('courseTable');
    const pageTitle = document.getElementById('pageTitle');
    const addStudentBtn = document.getElementById('addStudentBtn');
    const addCourseBtn = document.getElementById('addCourseBtn');

    if (studentTable && courseTable && pageTitle && addStudentBtn && addCourseBtn) {
        studentTable.style.display = 'table';
        courseTable.style.display = 'none';
        pageTitle.textContent = '学生管理';
        addStudentBtn.style.display = 'block';
        addCourseBtn.style.display = 'none';
        renderStudents(); // 渲染学生列表
    } else {
        console.error('部分元素未找到，请检查 HTML 文件。');
    }

    studentPagination.style.display = 'block';
    coursePagination.style.display = 'none';
}

// Page loads to display the student management interface
window.addEventListener('load', () => {
    loadStudents();
    loadCourses();
    displayStudentManagement();
    const studentPagination = document.getElementById('studentPagination');
    const coursePagination = document.getElementById('coursePagination');
    if (studentPagination && coursePagination) {
        studentPagination.style.display = 'block';
        coursePagination.style.display = 'none';
    }
});


// Toggle the sidebar expansion and contraction state
function toggleSidebar() {
    const sidebar = document.getElementById('sidebar');
    if (sidebar) {
        sidebar.classList.toggle('collapsed');
    } else {
        console.error('Sidebar element with id "sidebar" not found.');
    }
}