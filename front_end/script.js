// 假设 /pic/landscape 文件夹下有这些图片，你需要根据实际情况修改
const backgroundImages = [
    './pic/landscape/pic1.jpg',
    './pic/landscape/pic2.jpg',
    './pic/landscape/pic3.jpg',
    './pic/landscape/pic4.jpg',
    './pic/landscape/pic5.jpg',
    './pic/landscape/pic6.jpg',
    // 可以继续添加更多图片路径
];

// 随机选择一张图片
const randomIndex = Math.floor(Math.random() * backgroundImages.length);
const randomImage = backgroundImages[randomIndex];

// 设置背景图片
document.body.style.backgroundImage = `url('${randomImage}')`;

const loginForm = document.getElementById('loginForm');
loginForm.addEventListener('submit', function (e) {
    e.preventDefault();
    const adminLogin = document.getElementById('adminLogin').checked;
    if (adminLogin) {
        // 跳转到管理员界面
        window.location.href = 'admin/index.html';
    } else {
        // 跳转到学生界面
        window.location.href = 'student/index.html';
    }
});
