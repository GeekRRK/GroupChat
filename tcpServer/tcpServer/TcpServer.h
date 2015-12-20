#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <WINSOCK2.H>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "DB.h"
using namespace std;

class TcpServer{
public:
	TcpServer(void);
	~TcpServer(void);

	enum{
		RECV_LEN = 200,			//接收消息的最大字节数
		MTU = 100,				//发送的最大字节数

		NEWUSER = 1,			//新用户标志
		OLDUSER = 2				//老用户标志
	};

private:
	//socket编写服务器的基本步骤
	bool loadWSA();
	bool createSocket(int type = SOCK_STREAM);
	bool bindSocket(short port, const char* ip);
	bool setListen(int number);
	void communicate();
	bool closeSocket();
	void closeWSA();
	void exitWithErrorMessage(const string& errMsg);

private:
	//功能：等待客户端的连接的线程
	//参数：在创建线程的时候，把服务器的(LPVOID)SOCKET写进CreateThread()的第四个参数
	//返回值：DWORD WINAPI
	static DWORD WINAPI acceptClient(LPVOID sock);

	//功能：接收消息的线程
	//参数：在创建线程的时候，把服务器的(LPVOID)SOCKET写进CreateThread()的第四个参数
	//返回值：DWORD WINAPI
	static DWORD WINAPI recvMsg(LPVOID sock);

	//功能：验证当前用户是新用户还是老用户，如果是新用户就插入到数据库中
	//参数：用户的名字和密码字符串
	//返回值：新用户返回NEWUSER，老用户返回OLDUSER
	static int checkNamePwd(const string& name, const string& pwd);

	//功能：使用户登陆到聊天室，记录用户名字并更新在线用户列表
	//参数：客户端的SOCKET，用户名字符串
	static void clientLogin(SOCKET sock, const string& name);

	//功能：使用户登退出聊天室，把用户从在线用户中删除并更新在线用户列表
	//参数：客户端的SOCKET
	static void clientLogout(SOCKET sock);

	//功能：更新在线用户列表
	static void listClients();

	//功能：把用户从在线用户中删除
	//参数：客户端的SOCKET
	static void removeClient(SOCKET sock);

	//功能：得到用户的Ip和Port组合字符串
	//参数：客户端的SOCKET
	//返回值：Ip和Port组合字符串
	static string getClientIpPort(SOCKET sock);

	//功能：通知所有用户(除了某个用户)消息
	//参数：不想通知的某个用户的SOCKET，消息字符串(当sock为-1时通知所有用户)
	static void notifyOtherClients(SOCKET sock, const string& msg);

	//功能：把文件内容发送给用户
	//参数：用户的SOCKET，要发送的文件的文件名字符串
	static void sendFile(SOCKET sock, const string& fileName);

	//功能：分类处理接收到的消息
	//参数：用户的SOCKET，接收到的消息字符串
	static void dealWithMsg(SOCKET sock, const string& msg);

private:
	static SOCKET serverSock;					//服务器的SOCKET
	static DB db;								//数据库对象
	static vector<SOCKET> clients;				//存储在线客户端SOCKET的容器
	static map<SOCKET, string> client_name;		//客户SOCKET和昵称的映射
	static map<SOCKET, string> client_time;		//客户SOCKET和最近一次登陆时间的映射
};