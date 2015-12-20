#pragma once
#pragma comment(lib, "ws2_32.lib ")
#include <WINSOCK2.H>
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

class MTcpClient{
public:
	MTcpClient(void);
	~MTcpClient(void);

	enum{
		RECV_LEN = 200,		//接收消息的最大字节数
		MTU = 100,			//发送的最大字节数

		CHAT = 1,			//聊天输入模式
		COMMAND = 2,		//命令输入模式
	};

private:
	//socket编写客户端的基本步骤
	bool loadWSA();
	bool createSocket(int type = SOCK_STREAM);
	bool connectToServer(short port ,const char* strIp);
	void communicate();
	bool closeSocket();
	void closeWSA();

	//功能：提示错误信息并退出程序
	//参数：错误信息字符串
	void exitWithErrorMessage(const string& errMsg);

	//功能：输入用户名或密码
	//返回值：用户名或密码字符串
	//参数：mode为0代表输入用户名，mode非0代表输入密码
	string inputInfo(int mode);

	//功能：等待服务器验证用户名和密码是否匹配，如果匹配登陆成功，否则重新登陆
	void waitToLogin();

	//功能：用户登陆到聊天室
	void login();

private:
	//功能：接收消息的线程
	//参数：在创建线程的时候，把客户端的(LPVOID)SOCKET写进CreateThread()的第四个参数
	//返回值：DWORD WINAPI
	static DWORD WINAPI recvMsg(LPVOID sock);

	//功能：分类处理接收到的消息
	//参数：接收到的消息字符串
	static void dealWithMsg(const string& msg);

	//功能：下载文件
	//参数：服务器的sock, 第一次接收的buffer, 第一次接收的字节数
	static void MTcpClient::downLoad(SOCKET sock, char* buffer, int recvLen);

	//功能：打开指定的文件
	//参数：文件名字符串
	static void openFile(const string& fileName);

	//功能：得到当前光标位置
	//返回值：坐标x,y组成的pair
	static pair<int, int> getXY();

	//功能：把光标定位到指定的位置
	//参数：坐标x,y
	static void gotoXY(int x, int y);

private:
	SOCKET clientSock;						//客户端SOCKET
	static int state;						//当前输入状态（聊天模式和命令模式）
	static pair<int, int> bulletinCoord;	//系统公告的坐标位置
	static string recentMsg;				//最近一次发送的消息
};