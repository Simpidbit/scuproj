from flask import Flask
from flask import request
from flask_cors import CORS  # 引入 CORS 类
import json
import socket

SERVER_PORT = 8989
KERNEL_PORT = 24354

app = Flask(__name__)
# 允许所有域名跨域，设置允许的请求方法和请求头
CORS(app, methods=['GET', 'POST', 'OPTIONS'], headers=['Content-Type'])

def connect_to_kernel():
    s = socket.socket()
    s.connect(("127.0.0.1", KERNEL_PORT))
    return s

def recvall(sock):
    buffer = bytes()
    blklen = 1024
    while True:
        msg = sock.recv(blklen)
        if len(msg) < blklen:
            buffer += msg
            break
        else:
            buffer += msg
    return buffer

# 登录请求 -> 登录结果
def loginRequest(data) -> str:
    print('=================== 登录请求')
    print(data)
    print('-------------------')
    msg = b'\x00' \
        + bytes([data['account_level']]) \
        + bytes(data['account_id'], encoding = 'utf-8') \
        + bytes(data['account_passwd'], encoding = 'utf-8')

    ks = connect_to_kernel()
    ks.send(msg)
    result = recvall(ks)
    ks.close()

    print("result:", result)

    result_dict = { 
        'type': 1,
        'result': result[1]
    }
    if result[1] == 0:
        # 登录成功
        result_dict['token'] = result[2:].decode('utf-8')
    print("result_dict:", result_dict)
    print("###################")
    return json.dumps(result_dict)

# 修改密码请求 -> 修改密码结果
def modifyPasswordRequest(data) -> str:
    print('=================== 修改密码请求')
    print(data)
    print('-------------------')
    msg = b'\x02' \
        + bytes(data['account_id'], encoding = 'utf-8') \
        + bytes(data['account_passwd'], encoding = 'utf-8') \
        + bytes(data['token'], encoding = 'utf-8')

    ks = connect_to_kernel()
    ks.send(msg)
    result = recvall(ks)
    ks.close()

    print("result:", result)

    result_dict = {
        "type": 3,
        "result": result[1],
    }
    print("result_dict:", result_dict)
    print("###################")
    return json.dumps(result_dict)

# 增加课程请求 -> 增加课程结果
def addCourseRequest(data) -> str:
    print('=================== 增加课程请求')
    print(data)
    print('-------------------')
    msg = b'\x04' \
        + bytes(data['account_id'], encoding = 'utf-8') \
        + bytes(data['token'], encoding = 'utf-8') \
        + bytes(data['course_id'], encoding = 'utf-8') \
        + bytes([data['course_name_len']]) \
        + bytes(data['course_name'], encoding = 'utf-8') \
        + data['course_capacity'].to_bytes(2, byteorder = 'little') \
        + data['course_spare'].to_bytes(2, byteorder = 'little') \
        + bytes(data['course_week']) \
        + bytes(data['course_day'])

    ks = connect_to_kernel()
    ks.send(msg)
    result = recvall(ks)
    ks.close()

    print("result:", result)

    result_dict = {
        "type": 5,
        "result": result[1]
    }
    print("result_dict:", result_dict)
    print("###################")
    return json.dumps(result_dict)

# 选择课程请求 -> 选择课程结果
def chooseCourseRequest(data) -> str:
    print('=================== 选择课程请求')
    print(data)
    print('-------------------')
    msg = b'\x06' \
        + bytes(data['account_id'], encoding = 'utf-8') \
        + bytes(data['token'], encoding = 'utf-8') \
        + bytes(data['course_id'], encoding = 'utf-8')
    
    ks = connect_to_kernel()
    ks.send(msg)
    result = recvall(ks)
    ks.close()

    print("result:", result)

    result_dict = {
        "type": 7,
        "result": result[1]
    }
    print("result_dict:", result_dict)
    print("###################")
    return json.dumps(result_dict)

# 查询单个课程信息 -> 查询单个课程结果
def queryCourseRequest(data) -> str:
    print('=================== 查询单个课程信息')
    print(data)
    print('-------------------')
    msg = b'\x08' + bytes(data['course_id'], encoding = 'utf-8')

    ks = connect_to_kernel()
    ks.send(msg)
    result = recvall(ks)
    ks.close()

    print("result:", result)

    if result[1] != 0:
        result_dict = {
            "type": result[0],
            "result": result[1]
        }
        print("result_dict:", result_dict)
        print("###################")
        return json.dumps(result_dict)


    N = result[12]
    w = result[13 + N + 4]
    result_dict = {
        "type": result[0],
        "result": result[1],
        "course_id": result[2:12].decode('utf-8'),
        "course_name_len": N,
        "course_name": result[13 : 13 + N].decode('utf-8'),
        "course_capacity": int.from_bytes(result[13 + N : 13 + N + 2], byteorder = 'little'),
        "course_spare": int.from_bytes(result[13 + N + 2 : 13 + N + 4], byteorder = 'little'),
        "course_week": [i for i in result[(13 + N + 4) : (13 + N + 4) + w]],
        "course_day": [i for i in result[(13 + N + 4) + w:]]
    }
    print("result_dict:", result_dict)
    print("###################")
    return json.dumps(result_dict)

# 退课请求 -> 退课结果
def removeCourseRequest(data) -> str:
    print('=================== 退课请求')
    print(data)
    print('-------------------')
    msg = b'\x0a' \
        + bytes(data['account_id'], encoding = 'utf-8') \
        + bytes(data['token'], encoding = 'utf-8') \
        + bytes(data['course_id'], encoding = 'utf-8')

    ks = connect_to_kernel()
    ks.send(msg)
    result = recvall(ks)
    ks.close()

    print("result:", result)

    result_dict = {
        "type": 11,
        "result": result[1]
    }
    print("result_dict:", result_dict)
    print("###################")
    return json.dumps(result_dict)

# 查询学生已选课程的请求 -> 查询学生已选课程的结果
def queryStudentSelectionRequest(data) -> str:
    print('=================== 查询学生已选课程的请求')
    print(data)
    print('-------------------')
    msg = b'\x0c' \
        + bytes(data['account_id'], encoding = 'utf-8') \
        + bytes(data['token'], encoding = 'utf-8')

    ks = connect_to_kernel()
    ks.send(msg)
    result = recvall(ks)
    ks.close()

    result_dict = {
        "type": 13,
        "result": result[1]
    }

    print("result:", result)

    if result[1] == 0:
        result_dict["course_id_list_len"] = result[2]
        result_dict["course_id_list"] = result[3:].decode('utf-8')

    print("result_dict:", result_dict)
    print("###################")
    return json.dumps(result_dict)


handlerMap = [ \
    loginRequest,                   # 登录请求(0) -> 登录结果(1)
    modifyPasswordRequest,          # 修改密码请求(2) -> 修改密码结果(3)
    addCourseRequest,               # 增加课程请求(4) -> 增加课程结果(5)
    chooseCourseRequest,            # 选择课程请求(6) -> 选择课程结果(7)
    queryCourseRequest,             # 查询单个课程信息请求(8) -> 查询单个课程信息结果(9)
    removeCourseRequest,            # 退课请求(10) -> 退课结果(11)
    queryStudentSelectionRequest    # 查询学生已选课程的请求(12) -> 查询学生已选课程的结果(13)
]

@app.route('/request', methods = ['POST'])
def requestHandler():
    data = json.loads(request.get_data())
    return handlerMap[int(data['type'] / 2)](data)

"""
@app.route("/")
def index():
    with open(f"./root/index.md", "rt") as f:
        passage_markdown = f.read()
    return render_template("article_template.html")
"""

if __name__ == '__main__':
    app.run("0.0.0.0", SERVER_PORT, debug = True)
