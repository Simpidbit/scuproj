/**
 * 根据名称获取 cookie 值
 * @param {string} name - cookie 名称
 * @returns {string} - cookie 值
 */
function getCookie(name) {
    const value =  `${document.cookie}`;
    let result = '';
    for (let i = 0; i < value.length; i++) {
        if (value.substr(i, name.length) === name) {
            for (let j = i + name.length + 1; j < value.length; j++) {
                result += value[j];
            }
        }
    }
    console.log(`getCookie: cookie = ${document.cookie}. result = ${result}.`);
    return result;
}

/**
 * 获取 token 值
 * @returns {string} - token 值
 */
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

/**
 * 获取 account_id 值
 * @returns {string} - account_id 值
 */
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
