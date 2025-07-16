import socket

def recvall(sock):
    buffer = bytes()
    blklen = 3
    while True:
        msg = sock.recv(blklen)
        if len(msg) < blklen:
            buffer += msg
            break
        else:
            buffer += msg
    return buffer

s = socket.socket()
s.bind(("0.0.0.0", 9999))
s.listen(32)

while True:
    conn,addr = s.accept()
    print(recvall(conn))
    conn.close()
