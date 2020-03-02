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
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//3.连接
	if (connect(server_sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Failed to connect to %s port %d: %s\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), strerror(errno));
		exit(-1);
	}

	struct sockaddr_in local_addr;
	socklen_t local_addrlen = sizeof(struct sockaddr_in);
	getsockname(server_sockfd, (struct sockaddr *)&local_addr, &local_addrlen);
	printf("local address: %s\n", inet_ntoa(local_addr.sin_addr));
	printf("lcoal port: %d\n", (int)ntohs(local_addr.sin_port));
#if 0 // 传统的网络传输数据
	int buflen, retval;
	char buf[BUF_SIZE];

	do {
		fgets(buf, sizeof(buf), stdin);  //fgets从控制台获取输入
		buflen = strlen(buf);
		retval = send(server_sockfd, buf, buflen, 0); // 发送完所有字节才会返回
		if (retval == -1) {
			fprintf(stderr, "Failed to send bytes to socket %d: %s\n", server_sockfd, strerror(errno));
			exit(-1);
		}
		printf("send: %d\n", retval);
	} while (strncasecmp(buf, "quit", 4) != 0);

#elif 0 // 利用prorobuf发送一个数据
	int retval;
	unsigned int len;
	void *buf = NULL;
	
	Student stu = STUDENT__INIT; // 初始化protobuf (student.pb-c.c 和 student.pb-c.h 里有定义)
	stu.name = (char*)malloc(BUF_SIZE);  // stu 里只有一个字段(name)(在.proto文件)
	stu.name = "test";
	len = student__get_packed_size(&stu);  // 算出包大小
	buf = malloc(len);
	student__pack(&stu, buf);  // protobuf 打包
	retval = send(server_sockfd, buf, len, 0);
	printf("retval : %d\n", retval);

#else // 利用prorobuf发送数据  从控制台获取输入
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
#endif
	close(server_sockfd);
	return 0;
}