#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "student.pb-c.h"

#define BUF_SIZE 128
#define PORT     8888

int main() {
	int server_sockfd; // fd: file descriptor 文件描述符
	//1.创建套接字
	server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 基于IPv4寻址、面向字节流的TCP套接字
	if (server_sockfd == -1) {
		perror("Failed to create socket.");
		exit(-1);
	}

	//2.配置寻址方式、端口号、IP地址
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);  // host to network short 从主机字节序到网络字节序
	server_addr.sin_addr.s_addr = INADDR_ANY; // 0x00000000

	//3.将配置好的sockaddr_in绑定到套接字
	if (bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) == -1) {
		perror("Failed to bind socket.");
		exit(-1);
	}

	if (server_addr.sin_port == 0) { // 如果没有配置端口或者设置为0 内核会分配一个空闲的端口号 getsockname将套接字绑定的信息写入server_addr
		socklen_t server_addrlen = sizeof(struct sockaddr_in);
		getsockname(server_sockfd, (struct sockaddr*)&server_addr, &server_addrlen);
		printf("real port : %d\n", (int)ntohs(server_addr.sin_port)); // network to host short
	}

	//4.监听(面向字节流的套接字才需要 面向数据包的套接字不需要)
	if (listen(server_sockfd, SOMAXCONN) == -1) { // 第二个参数是最大连接数 比如设置为3 SOMAXCONN表示由系统决定最大队列长度
		perror("Failed to listen socket.");
		exit(-1);
	}

	int client_sockfd;
	client_sockfd = accept(server_sockfd, NULL, NULL);
#if 0  // 传统的网络传输数据
	int buflen;
	char buf[BUF_SIZE];

	do {buflen = recv(client_sockfd, buf, BUF_SIZE, 0); // 收到就返回 哪怕一个字节
		if (buflen > 0) {
			buf[buflen] = '\0';
			printf("recv: %d: %s", buflen, buf);
		} else if (buflen == -1) {
			fprintf(stderr, "Failed to receive bytes from socket %d: %s\n", client_sockfd, strerror(errno));
			exit(-1);
		}
	} while (strncasecmp(buf, "quit", 4) != 0);

#elif 0 // 利用prorobuf接收一个数据
	Student *msg = NULL;
	int buflen;
	char buf[BUF_SIZE];

	buflen = recv(client_sockfd, buf, BUF_SIZE, 0);
	msg = student__unpack(NULL, buflen, buf);  // protobuf 解包
	printf("msg name : %s\n", msg->name);

#else // 利用prorobuf接收数据
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

	student__free_unpacked(msg, NULL); // 释放空间

#endif
	close(client_sockfd);
	close(server_sockfd);

	return 0;
}