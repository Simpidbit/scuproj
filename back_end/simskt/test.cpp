// 一个简单的复读程序：先发送"Hello"，之后收到的消息全部发送回去，并显示在屏幕上
#include <iostream>
#include <string>
#include "simskt.h"

using namespace std;
using namespace simpid;


int main()
{
    Server s;
    s.bind("0.0.0.0", 12348);
    s.listen(32);
    
    Client *cli = s.accept();
    cli->send("Hello");
    while(1) {
        string msg = cli->recvall();
        if (msg.size() == 0) break;
        cout << msg << endl;
        cli->send(msg);
    }
    
    return 0;
}
