import requests
import json

SERVER_IP = '127.0.0.1'
SERVER_PORT = 22334

url = f'http://{SERVER_IP}:{SERVER_PORT}/request'
headers = { 'Content-Type': 'application/json' }

def any_post(dt):
    print("请求开始")
    res = requests.post(url, headers = headers, data = json.dumps(dt))
    print(res.content)
    print(res.json())
    print("请求结束")

dt1 = {
    "type": 12,
    "account_id": "00000003",
    "token": "daa83cb2658ad2d137814f80511af423e2e88020eb0904bbfb0e4c7bd508c2fb"
}

dt2 = {
    "type": 0,
    "account_id": "00000003",
    "account_passwd": "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824",
    "account_level": 0
}

if __name__ == '__main__':
    #test_login_request()
    any_post(dt1)
