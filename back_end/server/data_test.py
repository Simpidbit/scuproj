import requests
import json

SERVER_IP = '127.0.0.1'
SERVER_PORT = 22334

url = f'http://{SERVER_IP}:{SERVER_PORT}/request'
headers = { 'Content-Type': 'application/json' }

# 测试登录请求
def test_login_request():
    print("============================ 登录请求")
    dict1 = {
        "type": 0,
        "account_level": 0,
        "account_id": "12345678",
        "account_passwd": "6ca13d52ca70c883e0f0bb101e425a89e8624de51db2d2392593af6a84118090" # abc123
    }

    dict2 = {
        "type": 0,
        "account_level": 1,
        "account_id": "87654321",
        "account_passwd": "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824" # hello
    }

    res1 = requests.post(url, headers = headers, data = json.dumps(dict1))
    print('--------------------- res1')
    print(res1.content)
    res2 = requests.post(url, headers = headers, data = json.dumps(dict2))
    print('--------------------- res2')
    print(res2.content)

if __name__ == '__main__':
    test_login_request()
