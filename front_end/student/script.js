function toggleSidebar() {
    const sidebar = document.getElementById('sidebar');
    sidebar.classList.toggle('collapsed');
}


// 显示已选课程列表
function displayEnrolledCourses() {
    const enrolledCourseList = document.getElementById('enrolledCourseList');
    enrolledCourseList.innerHTML = enrolledCourses.length > 0 ? `
        <table class="course-table">
            <thead>
                <tr>
                    <th>课程 ID</th>
                    <th>课程名称</th>
                    <th>课程周</th>
                    <th>课程节次</th>
                    <th>操作</th>
                </tr>
            </thead>
            <tbody>
                ${enrolledCourses.map(course => {
                    // 处理课程周显示
                    const courseWeekChunks = [];
                    const segmentCount = course.course_week[0]; // 上课时间段数量
                    for (let i = 0; i < segmentCount; i++) {
                        const startIndex = 1 + i * 2;
                        const endIndex = 2 + i * 2;
                        // 检查索引是否在数组范围内
                        if (startIndex < course.course_week.length && endIndex < course.course_week.length) {
                            const startWeek = course.course_week[startIndex];
                            const endWeek = course.course_week[endIndex];
                            courseWeekChunks.push(`${startWeek}-${endWeek}周`);
                        }
                    }
                    const courseWeekStr = courseWeekChunks.join('<br>');

                    // 处理课程节次显示
                    const courseDayChunks = [];
                    const weekDays = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
                    const periodsCount = course.course_day[0];
                    for (let i = 0; i < periodsCount; i++) {
                        const dayIndex = course.course_day[1 + i * 3] - 1;
                        const startSection = course.course_day[2 + i * 3];
                        const endSection = course.course_day[3 + i * 3];
                        courseDayChunks.push(`${weekDays[dayIndex]} 第${startSection}-${endSection}节`);
                    }
                    const courseDayStr = courseDayChunks.join('<br>');

                    return `
                        <tr>
                            <td>${course.course_id}</td>
                            <td>${course.course_name}</td>
                            <td>${courseWeekStr}</td>
                            <td>${courseDayStr}</td>
                            <td><button class="drop-course-btn" onclick="dropCourse('${course.course_id}')">退课</button></td>
                        </tr>
                    `;
                }).join('')}
            </tbody>
        </table>
    ` : '<div class="result-item">暂无已选课程</div>';
}

// 显示查询结果
async function displayResults(results) {
    const courseList = document.getElementById('courseList');
    courseList.innerHTML = results.length > 0 ? `
        <table class="course-table">
            <thead>
                <tr>
                    <th>课程 ID</th>
                    <th>课程名称</th>
                    <th>课容量</th>
                    <th>课余量</th>
                    <th>课程周</th>
                    <th>课程节次</th>
                    <th>操作</th>
                </tr>
            </thead>
            <tbody>
                ${results.map(course => {
                    // 处理课程周显示
                    const courseWeekChunks = [];
                    const segmentCount = course.course_week[0]; 
                    for (let i = 0; i < segmentCount; i++) {
                        const startIndex = 1 + i * 2;
                        const endIndex = 2 + i * 2;
                        if (startIndex < course.course_week.length && endIndex < course.course_week.length) {
                            const startWeek = course.course_week[startIndex];
                            const endWeek = course.course_week[endIndex];
                            courseWeekChunks.push(`${startWeek}-${endWeek}周`);
                        }
                    }
                    const courseWeekStr = courseWeekChunks.join('<br>');

                    // 处理课程节次显示
                    const courseDayChunks = [];
                    const weekDays = ['周一', '周二', '周三', '周四', '周五', '周六', '周日'];
                    const periodsCount = course.course_day[0];
                    for (let i = 0; i < periodsCount; i++) {
                        const dayIndex = course.course_day[1 + i * 3] - 1;
                        const startSection = course.course_day[2 + i * 3];
                        const endSection = course.course_day[3 + i * 3];
                        courseDayChunks.push(`${weekDays[dayIndex]} 第${startSection}-${endSection}节`);
                    }
                    const courseDayStr = courseDayChunks.join('<br>');

                    // 判断课余量是否为 0，设置按钮状态和样式
                    const isDisabled = course.course_spare === 0;
                    const disabledAttr = isDisabled ? 'disabled' : '';
                    const disabledClass = isDisabled ? 'disabled-btn' : '';

                    return `
                        <tr>
                            <td>${course.course_id}</td>
                            <td>${course.course_name}</td>
                            <td>${course.course_capacity}</td>
                            <td>${course.course_spare}</td>
                            <td>${courseWeekStr}</td>
                            <td>${courseDayStr}</td>
                            <td>
                                <button 
                                    class="select-course-btn ${disabledClass}" 
                                    onclick="selectCourse('${course.course_id}')"
                                    ${disabledAttr}
                                >
                                    ${isDisabled ? '已满' : '选课'}
                                </button>
                            </td>
                        </tr>
                    `;
                }).join('')}
            </tbody>
        </table>
    ` : '<div class="result-item">未找到相关课程</div>';
}

/**
 * 通用的请求函数
 * @param {Object} data - 请求数据
 * @returns {Promise<Object>} - 包含响应结果的 Promise 对象
 */
async function makeRequest(data) {
    const requestJson = JSON.stringify(data, null, 2);
    console.log(requestJson);

    const response = await fetch('http://118.89.112.170:8989/request', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: requestJson,
        mode: 'cors'
    });

    // 解析后端返回的 JSON 响应
    const result = await response.json();

    // 处理响应结果
    if (response.ok) {
        const statusCode = result.result;
        switch (statusCode) {
            case 0:
                break;
            case 1:
                alert(`其他错误`);
                break;
            case 100:
                alert(`token 错误`);
                window.location.href = '../index.html';
                break;
            case 101:
                alert(`token 已超时失效`);
                window.location.href = '../index.html';
                break;
        }
    } else {
        // 请求失败
        const errorMessage = result.message || '未知错误';
        alert(`请求失败: ${errorMessage}`);
    }

    return result;
}

// 退课函数
async function dropCourse(courseId) {
    // 使用 confirm 函数弹出确认对话框
    const isConfirmed = confirm('确定要退选该课程吗？');
    if (isConfirmed) {
        const index = enrolledCourses.findIndex(course => course.course_id === courseId);
        if (index !== -1) {
            // 从已选课程列表中移除该课程
            enrolledCourses.splice(index, 1);

            const selectinfo = {
                type: 10,
                accountId: getID(),
                token: getToken(),
                course_id: courseId
            };

            const result = await makeRequest(selectinfo);

            if (result.result === 0) {
                alert(`退课成功`);
            }

            // 重新渲染已选课程列表
            displayEnrolledCourses();
        }
    }
}

// 显示页面函数
function showPage(pageId) {
    const pages = document.querySelectorAll('.page');
    pages.forEach(page => {
        page.style.display = 'none';
    });
    const targetPage = document.getElementById(pageId);
    if (targetPage) {
        targetPage.style.display = 'block';
    }
    if (pageId === 'calendar') {
        // 重新渲染日历
        initCalendar();
    } else if (pageId === 'dropCourse') {
        // 显示退课页面时渲染已选课程列表
        displayEnrolledCourses();
    }
}

async function selectCourse(courseId) {
    console.log('选课成功');
    const selectinfo = {
        type: 6,
        accountId: getID(),
        token: getToken(),
        course_id: courseId
    };

    const result = await makeRequest(selectinfo);

    if (result.result === 0) {
        alert(`添加成功`);
    }
    // 这里可以添加实际的选课逻辑，比如调用后端 API
}

async function searchCourses() {
    const courseId = document.getElementById('courseIdInput').value.trim();

    const courseData = {
        type: 8,
        accountId: getID(),
        token: getToken(),
        course_id: courseId
    };

    const result = await makeRequest(courseData);

    console.log(result);
    if (result.result != 0) {
           displayResults([]);  
    }else{
         displayResults([result]);
    }
        

   
}

// 动态添加 CSS 样式
function addTableStyles() {
    const style = document.createElement('style');
    style.textContent = `
        .course-table {
            width: 100%;
            border-collapse: separate;
            border-spacing: 0 10px; /* 增大行间距 */
            font-family: Arial, sans-serif;
        }

        .course-table th,
        .course-table td {
            padding: 12px; /* 增大单元格内边距 */
            text-align: left;
            border-bottom: 1px solid #ddd;
        }

        .course-table th {
            background-color: #a80011; /* 修改表头背景色 */
            color: white; /* 设置表头文字颜色为白色，提高对比度 */
        }

        .course-table tbody tr:nth-child(even) {
            background-color: #f9f9f9; /* 偶数行背景色 */
        }

        .course-table tbody tr:nth-child(odd) {
            background-color: #ffffff; /* 奇数行背景色 */
        }

        .course-table tbody tr:hover {
            background-color: #e9e9e9; /* 鼠标悬停行背景色 */
        }

        .select-course-btn,
        .drop-course-btn {
            padding: 8px 16px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
            transition: background-color 0.3s ease;
        }

        .select-course-btn {
            background-color: #4CAF50;
            color: white;
        }

        .select-course-btn:hover {
            background-color: #45a049;
        }

        .drop-course-btn {
            background-color: #f44336;
            color: white;
        }

        .drop-course-btn:hover {
            background-color: #da190b;
        }
    `;
    document.head.appendChild(style);
}

// 在 DOM 加载完成时调用添加样式函数
document.addEventListener('DOMContentLoaded', function() {
    addTableStyles();
    // 初始化显示日历
    showPage('calendar');
    initCalendar();

    // 为课程 ID 输入框添加键盘事件监听
    const courseIdInput = document.getElementById('courseIdInput');
    if (courseIdInput) {
        courseIdInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                searchCourses();
            }
        });
    }
});

function initCalendar() {
    const calendarEl = document.getElementById('calendar');
    const calendar = new FullCalendar.Calendar(calendarEl, {
        locale: 'zh-cn',
        initialView: 'timeGridWeek',
        events: [
            {
                title: '高等数学',
                daysOfWeek: [1, 3, 5],
                startTime: '08:00',
                endTime: '09:30',
                backgroundColor: '#ff9f89'
            },
            {
                title: '大学英语',
                daysOfWeek: [2, 4],
                startTime: '14:00',
                endTime: '15:30',
                backgroundColor: '#73a6ff'
            },
            {
                title: '大学物理',
                daysOfWeek: [1, 5],
                startTime: '10:00',
                endTime: '12:00',
                backgroundColor: '#45c890'
            }
        ]
    });
    calendar.render();
}


// 定义 enrolledCourses 为普通变量，后续赋值为实际课程数据
let enrolledCourses = [];

// 异步获取并处理已选课程数据
async function fetchEnrolledCourses() {
    try {
        // 读取本地JSON文件，假设这里用 selected_info.json 获取已选课程数据
        const response = await fetch('./selected_info.json');
        
        if (!response.ok) throw new Error(`请求失败: ${response.status}`);
        
        const mockCourses = await response.json();
        console.log("本地JSON数据:", mockCourses);
        
        // 假设不需要过滤，直接将获取的数据赋值给 enrolledCourses
        enrolledCourses = mockCourses;
        
        // 渲染已选课程列表
        displayEnrolledCourses();
    } catch (error) {
        console.error("读取失败:", error.message);
        // 清空已选课程列表
        enrolledCourses = [];
        displayEnrolledCourses();
    }
}

// 在 DOM 加载完成时调用获取已选课程数据的函数
document.addEventListener('DOMContentLoaded', async function() {
    addTableStyles();
    // 初始化显示日历
    showPage('calendar');
    initCalendar();
    // 获取已选课程数据
    await fetchEnrolledCourses();

    // 为课程 ID 输入框添加键盘事件监听
    const courseIdInput = document.getElementById('courseIdInput');
    if (courseIdInput) {
        courseIdInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                searchCourses();
            }
        });
    }
});

function getCookie(name) {
            const value =  `${document.cookie}`;
            //const parts = value.split(`; ${name}=`);
            //if (parts.length === 2) return parts.pop().split(';').shift();

            let result = '';
            for (let i = 0; i < value.length; i++) {
                if (value.substr(i, name.length) == name) {
                    for (let j = i + name.length + 1; j < value.length; j++) {
                        result += value[j];
                    }
                }
            }

            console.log(`getCookie: cookie = ${document.cookie}. result = ${result}.`)

            return result;
        }

function getToken(){
    const token = getCookie('token');
    if (token) {
        console.log('获取到的 token:', token);
        return token;
    } else {
        console.log('未获取到 token');
        alert("登录信息已过期，请重新登录");
        window.location.href = '../index.html';
    }
} 

function getID(){
    const token = getCookie('account_id');
    if (token) {
        console.log('获取到的 account_id:', token);
        return token;
    } else {
        console.log('未获取到 id');
        alert("登录信息已过期，请重新登录");
        //window.location.href = '../index.html';
    }
} 