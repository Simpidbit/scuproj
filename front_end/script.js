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

document.addEventListener('DOMContentLoaded', function() {
    const loginForm = document.getElementById('loginForm');
    if (loginForm) {
        loginForm.addEventListener('submit', async function(event) {
            // 阻止表单默认提交行为
            event.preventDefault(); 

            const username = document.getElementById('username').value;
            const password = document.getElementById('password').value;
            const encryptedPassword = await sha256Encrypt(password);
            const isAdmin = document.getElementById('adminLogin').checked;

            // 构建包含所需信息的对象
            const loginInfo = {
                type:0,
                account_level: isAdmin ?1:0,
                account_id: username,
                account_passwd: encryptedPassword
            };

            // 将对象转换为 JSON 字符串并打印
            const loginInfoJSON = JSON.stringify(loginInfo, null, 2);
            console.log(loginInfoJSON);

                // 发送POST请求到后端
            const response = await fetch('http://localhost:8989/request', {
                method: 'POST', // 请求方法为POST
                headers: {
                    'Content-Type': 'application/json', // 声明发送JSON格式数据
                    // 可选：添加其他请求头（如token等）
                    // 'Authorization': 'Bearer your_token'
                },
                body: JSON.stringify(loginInfo),// 将数据转为JSON字符串
                mode: 'cors' // 允许跨域请求
            });

            // 解析后端返回的JSON响应
            const result = await response.json();

            // 处理响应结果
            if (response.ok) {
                if(result.hasOwnProperty('token')){
                    if(isAdmin){
                        //设置15分钟有效cookie
                        document.cookie = `token=${result.token};max - age=900; path=/;`;
                        document.cookie = `account_id=${username};max - age=900; path=/;`;
                        window.location.href = 'admin/index.html';
                    }else{
                        document.cookie = `token=${result.token};max - age=900; path=/;`;
                        document.cookie = `account_id=${username};max - age=900; path=/;`;
                        window.location.href = 'student/index.html';
                    }
                    
                }else{
                    errmsg = "登录失败 ";
                    const errcode = result.result;
                    switch(errcode){
                        case 1:
                            errmsg += "账号不存在";
                            break;
                        case 2:
                            errmsg += "密码错误";
                            break;
                        default:
                            errmsg += "没有权限";
                            break;
                    }
                    alert(errmsg);
                }
                // 可以在这里跳转到其他页面
                // window.location.href = '/home';
            } else {
                // 登录失败
                alert(`请求失败`);
            }

        });
    }
});

// 将字符串转换为 SHA-256 哈希值（十六进制）
async function sha256Encrypt(str) {
  // 将字符串转换为 Uint8Array
  const encoder = new TextEncoder();
  const data = encoder.encode(str);
  
  // 计算 SHA-256 哈希
  const hashBuffer = await crypto.subtle.digest('SHA-256', data);
  
  // 将 ArrayBuffer 转换为十六进制字符串
  const hashArray = Array.from(new Uint8Array(hashBuffer));
  const hashHex = hashArray.map(byte => byte.toString(16).padStart(2, '0')).join('');
  
  return hashHex;
}
