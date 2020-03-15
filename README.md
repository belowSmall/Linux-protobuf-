## 定义

> **socket 是计算机网络中用于在节点内发送或接收数据的内部端点。具体来说，它是网络软件(协议栈)中这个端点的一种表示，包含通信协议、目标地址、状态等，是系统资源的一种形式。**(转自维基百科)

socket即套接字，能够唯一确定通信双方。（一般是客户端和服务端）
每一个套接字都有唯一的一个编号（对于操作系统来说），称为 **文件描述符** 。通信过程中的任何操作都需要这个 **文件描述符** 。

> * **socket API是位于应用层和传输层之间**

## 创建套接字
```c
int socket(int domain, int type, int protocol);
```
第一个参数：```domain``` 在英文里是“域”的意思。这里约定的是通信方式。
> AF 即 Address Family （地址族）

|domain取值                     |含义|
|:-----                        |:-----|
|```AF_UNIX``` 或 ```AF_LOCAL```|本地通信|
|```AF_INET```                  |IPv4|
|```AF_INET6```                 |IPv6|

第二个参数：```type```，套接字类型。
|type取值         |含义|
|:-----          |:-----|
|```SOCK_STREAM```|面向字节流，用于 TCP 协议|
|```SOCK_DGRAM``` |面向数据报，用于 UDP 协议|
|```SOCK_RAW```   |原始套接字，用于 IP、ICMP 等底层协议|

第三个参数：```protocol```，协议（tcp 或 udp）
|protocol取值         |含义|
|:-----           |:-----|
|```IPPROTO_TCP```|TCP|
|```IPPROTO_UDP```|UDP|

* **创建一个套接字**
创建失败返回 -1
```c
int server_sockfd;

server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//基于IPv4寻址、面向字节流的TCP套接字
```

## 确定ip地址、端口号等信息
到这里，我们只是创建了fd，并没有告知ip地址和端口号等信息
这里用到了一个结构体 ```sockaddr_in``` （对于 TCP/IPv4）
```c
// /usr/include/netinet/in.h
struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;                 /* Port number.  */
    struct in_addr sin_addr;            /* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr) -
                           __SOCKADDR_COMMON_SIZE -
                           sizeof (in_port_t) -
                           sizeof (struct in_addr)];
  };
```

第一个成员是 ```sin_family ``` 无符号短整型，和 ```socket()``` 函数的第一个参数一样
```c
// /usr/include/x86_64-linux-gnu/bits/sockaddr.h
/* POSIX.1g specifies this type name for the `sa_family' member.  */
typedef unsigned short int sa_family_t;

/* This macro is used to declare the initial common members
   of the data types used for socket addresses, `struct sockaddr',
   `struct sockaddr_in', `struct sockaddr_un', etc.  */

#define __SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family

#define __SOCKADDR_COMMON_SIZE  (sizeof (unsigned short int))
```
第二个成员是 ```sin_port``` 16位无符号短整型，保存端口号。
第三个成员是 ```sin_addr``` 类型是 ```struct  in_addr ```
结构体 ```struct  in_addr ``` 只有一个成员，32位无符号整形， 保存ip地址。
```c
/* Internet address.  */
typedef uint32_t in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };
```
第四个成员是 ```sin_zero``` 没有实际意义，填 ```0``` 。为了和 ```struct sockaddr``` 的长度保持一致。（```struct sockaddr``` 在后面会用到，例如 ```bind()```）

> * 确定 ```struct sockaddr_in``` 信息

网络字节序是大端格式，而主机字节序不一定（有的是小端，有的是大端）。```htons()``` 从主机字节序到网络字节序
```c
struct sockaddr_in server_addr;
bzero(&server_addr, sizeof(struct sockaddr_in)); // 置 0
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);  // host to network short 从主机字节序到网络字节序
server_addr.sin_addr.s_addr = INADDR_ANY; // 0x00000000
// 这里也可以改为 server_addr.sin_addr.s_addr = inet_addr("0.0.0.0") 和上面的是一个意思
```
```inet_addr()```将点分十进制的ip地址转变为8位16进制的ip地址
```INADDR_ANY```定义：
```c
// /usr/include/netinet/in.h
#define INADDR_ANY              ((in_addr_t) 0x00000000)
```

## 绑定信息到描述符
* 客户端不需要此操作
```c
int bind(int fd, const struct sockaddr * addr, socklen_t addr_len)
```
> * ```fd```描述符
> * ```addr```的类型是```struct sockaddr *``` ，而前面定义的结构的类型是```struct sockaddr_in```，这里需要进行强制类型转换
> * ```addr_len```地址结构大小，也就是```sizeof(struct sockaddr_in)```
绑定成功返回```0```，绑定失败返回```-1```


## 服务器监听
```c
int listen(int fd, int n)
```
> * ```fd```套接字
> * ```n```最大队列长度，大于这个值的请求将被拒绝。可以用常量```SOMAXCONN```，由系统决定队列的最大长度
监听成功返回```0```，监听失败返回```-1```

## 等待客户端
```c
int accept(int fd, struct sockaddr * addr, socklen_t * addr_len)
```
```c
int client_sockfd;
client_sockfd = accept(server_sockfd, NULL, NULL);
```
> * 有客户端连接时才有返回，返回值一个新的fd。

## 客户端请求建立连接
服务器没有请求连接一说
```c
int connect(int fd, const struct sockaddr * addr, socklen_t addr_len);
```
```c
struct sockaddr_in server_addr;
bzero(&server_addr, sizeof(struct sockaddr_in));
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);  // host to network short 从主机字节序到网络字节序
server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
connect(server_sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in))
```
> * ```fd```需要请求连接的fd

## 传输数据
```recv()``` 读取数据
```send()``` 发送数据
```c
// 读取数据  对于服务器来说
int buflen;
char buf[BUF_SIZE];
buflen = recv(client_sockfd, buf, BUF_SIZE, 0)
```
```c
// 发送数据  对于客户端来说
int buflen, retval;
char buf[BUF_SIZE];
buflen = strlen(buf);
retval = send(server_sockfd, buf, buflen, 0)
```
***到这里，网络传输就可以实现了，下面是用protobuf来做网络传输的媒介***

---
---
---

## 利用protobuf传输数据
prorobuf的安装和使用参考我另一篇文章 [安装和使用protobuf](https://github.com/belowSmall/protobuf-c-use)
这里也是使用我另一篇文章里的 **student** 的例子
需要在服务器和客户端引入头文件 ```student.pb-c.h```

关于protobuf在服务端的处理
```c
Student *msg = NULL;
int buflen;
char buf[BUF_SIZE];

do {buflen = recv(client_sockfd, buf, BUF_SIZE, 0); // 收到就返回 哪怕一个字节
	if (buflen > 0) {
		msg = student__unpack(NULL, buflen, buf);  // protobuf 解包
		printf("msg name : %s\n", msg->name);
	} else if (buflen == -1) {
		fprintf(stderr, "Failed to receive bytes from socket %d: %s\n", client_sockfd, strerror(errno));
		exit(-1);
	}
} while (strncasecmp(msg->name, "quit", 4) != 0);
```
关于protobuf在客户端的处理
```c
int retval;
unsigned int len;
void *buf = NULL;
char tempbuf[BUF_SIZE];
	
Student stu = STUDENT__INIT; // 初始化protobuf (student.pb-c.c 和 student.pb-c.h 里有定义)
stu.name = (char*)malloc(BUF_SIZE);  // stu 里只有一个字段(name)(在.proto文件)
do {
	bzero(stu.name, sizeof(stu.name));
	fgets(tempbuf, sizeof(tempbuf), stdin); // 控制台获取输入
	memcpy(stu.name, tempbuf, strlen(tempbuf)-1);
	len = student__get_packed_size(&stu);  // 算出包大小
	buf = malloc(len);
	student__pack(&stu, buf);  // protobuf 打包
	retval = send(server_sockfd, buf, len, 0);
	if (retval == -1) {
		fprintf(stderr, "Failed to send bytes to socket %d: %s\n", server_sockfd, strerror(errno));
		exit(-1);
	}
	printf("send: %d\n", retval);
} while (strncasecmp(stu.name, "quit", 4) != 0);
```

编译：
```gcc server.c student.pb-c.c -o server -lprotobuf-c```
```gcc client.c student.pb-c.c -o client -lprotobuf-c```

先启动服务端
后启动客户端
```c
local address: 127.0.0.1
lcoal port: 50338
测试  # 这里是输入
send: 8
```

服务端收到
```c
msg name : 测试
```
---
2020.3.2 17:03 广州
