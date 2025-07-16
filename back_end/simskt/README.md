这是一个支持`Linux`、`macOS`、`Windows`的跨平台socket接口封装

## 使用方法

下载`simskt.cpp`和`simskt.h`

在你需要使用时添加`#include "simskt.h"`即可

请注意：
- 编译时要带上`simskt.cpp`，同时`simskt.cpp`仅支持C++11及之后的标准，不支持C++11之前的标准
- `Windows`用户在编译时请链接`ws2_32.lib`，以MinGW为例，需要添加`-lws2_32`编译选项


Example：main.cpp

```cpp
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
```

## 编译方法

#### Windows
若用`MinGW`编译，则编译命令:
```
g++ -std=c++11 .\simskt.cpp .\test.cpp -lws2_32 -o test
```

#### Linux/macOS
```
g++ -std=c++11 ./simskt.cpp ./test.cpp -o test
```

