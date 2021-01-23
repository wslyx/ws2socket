#define _WINSOCK_DEPRECATED_NO_WARNINGS
 
#include <iostream>
#include <string>
 
#include <winsock2.h>  // 包含网络通信头文件
 
#pragma comment(lib, "ws2_32.lib")  // 程序在链接的时候将该库链接进去。隐式加载动态库。加载 ws2_32.dll。
 
// 动态链接库有两种加载方式：隐式加载和显示加载。
// 1.隐式加载又叫载入时加载，指在主程序载入内存时搜索DLL，并将DLL载入内存。隐式加载也会有静态链接库的问题，如果程序稍大，加载时间就会过长，用户不能接受。
// .lib 文件包含DLL导出的函数和变量的符号名，只是用来为链接程序提供必要的信息，以便在链接时找到函数或变量的入口地址；
// .dll 文件才包含实际的函数和数据。所以首先需要将 dllDemo.lib 引入到当前项目.
// 2.显式加载又叫运行时加载，指主程序在运行过程中需要DLL中的函数时再加载。显式加载是将较大的程序分开加载的，程序运行时只需要将主程序载入内存，软件打开速度快，用户体验好。
 
// 动态链接库的好处：在需要的时候加载动态链接库某个函数。
// 隐式链接的缺点：使用比较简单，在程序的其他部分可以任意使用函数，但是当程序访问十来个dll动态链接库的时候，此时如果都使用隐式链接的时候，
// 启动此程序的时候，这十来个动态链接库都需要加载到内存，映射到内存的地址空间，这就会加大进程的启动时间，而且程序运行过程中，只是在某个条件下使用某个函数，
// 如果使用隐式链接会造成资源的浪费。这样需要采用动态加载的方式。
 
#define PORT 8888
#define  BUFFER_SIZE 256
 
static const std::string kExitFlag = "-1";
 
// 参考：http://c.biancheng.net/cpp/html/3030.html
 
int main() {
  // 初始化socket dll。
  // WinSock 规范的最新版本号为 2.2。
  // //主版本号为2，副版本号为2，返回 0x0202。
  // WSADATA保存Dll的信息。
  WORD winsock_version = MAKEWORD(2,2);
  WSADATA wsa_data;
  if (WSAStartup(winsock_version, &wsa_data) != 0) {
    std::cout << "Failed to init socket dll!" << std::endl;
    return 1;
  }
 
  // 参数 AF_INET 表示使用 IPv4 地址，SOCK_STREAM 表示使用面向连接的数据传输方式，IPPROTO_TCP 表示使用 TCP 协议。
  // AF 是“Address Family”的简写，INET是“Inetnet”的简写。AF_INET 表示 IPv4 地址，例如 127.0.0.1；AF_INET6 表示 IPv6 地址，例如 1030::C9B4:FF12:48AA:1A2B。
  // 据传输方式，常用的有两种：SOCK_STREAM 和 SOCK_DGRAM。 SOCK_STREAM 表示面向连接的数据传输方式。 SOCK_DGRAM 表示无连接的数据传输方式。
  //  protocol 表示传输协议，常用的有 IPPROTO_TCP 和 IPPTOTO_UDP，分别表示 TCP 传输协议和 UDP 传输协议。
  SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket == INVALID_SOCKET) {
    std::cout << "Failed to create server socket!" << std::endl;
    return 2;
  }
 
  // 绑定IP和端口。
  // IP地址和端口都保存在 sockaddr_in 结构体中。
  //struct sockaddr_in {
  //  sa_family_t     sin_family;   //地址族（Address Family），也就是地址类型
  //  uint16_t        sin_port;     //16位的端口号
  //  struct in_addr  sin_addr;     //32位IP地址
  //  char            sin_zero[8];  //不使用，一般用0填充
  //};
 
  //struct in_addr {
  //  in_addr_t  s_addr;  //32位的IP地址
  //};
 
  //struct sockaddr {
  //  sa_family_t  sin_family;   //地址族（Address Family），也就是地址类型
  //  char         sa_data[14];  //IP地址和端口号
  //};
 
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
 
  // 为什么使用 sockaddr_in 而不使用 sockaddr.
  // sockaddr 是一种通用的结构体，可以用来保存多种类型的IP地址和端口号，而 sockaddr_in 是专门用来保存 IPv4 地址的结构体。
  // 正是由于通用结构体 sockaddr 使用不便，才针对不同的地址类型定义了不同的结构体。
  if (bind(server_socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
    std::cout << "Failed to bind port!" << std::endl;
    return 3;
  }
 
  // 监听。
  // 第二个参数是 请求队列的长度。
  // listen() 只是让套接字进入监听状态，并没有真正接收客户端请求，listen() 后面的代码会继续执行，直到遇到 accept()。
  // accept() 会阻塞程序执行（后面代码不能被执行），直到有新的请求到来。
  if (listen(server_socket, 10)) {
    std::cout << "Failed to listen!" << std::endl;
    return 4;
  }
 
  // 循环接收数据。
  sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  std::cout << "Wait for connecting..." << std::endl;
 
  // 程序一旦执行到 accept() 就会被阻塞（暂停运行），直到客户端发起请求。
  SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_len);
  if (client_socket == INVALID_SOCKET) {
    std::cout << "Failed to accept!" << std::endl;
    return 5;
  }
 
  std::cout << "Succeed to receive a connection: " << inet_ntoa(client_addr.sin_addr) << std::endl;
 
  char recv_buf[BUFFER_SIZE] = {};
  while (true) {
    // 接收数据。
    // 返回值是读取的字节数。没有内容时，等待请求。
    int ret = recv(client_socket, recv_buf, BUFFER_SIZE, 0);
    if (ret < 0) {
      std::cout << "Failed to receive data!" << std::endl;
      break;
    }
 
    std::cout << "Receive from Client: " << recv_buf << std::endl;
    if (kExitFlag == recv_buf) {
      std::cout << "Exit!" << std::endl;
      break;
    }
 
    // 发送数据给客户端。
    char* send_data = "Hello, Tcp Client!\n";
    send(client_socket, send_data, strlen(send_data), 0);
  }
 
  // 关闭套接字。
  closesocket(client_socket);
  closesocket(server_socket);
 
  // 释放dll。
  WSACleanup();
 
  return 0;
}