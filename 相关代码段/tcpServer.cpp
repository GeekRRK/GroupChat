/*
	0:加载套接字库（WSAStartup()）
	1:创建套接字（socket()）
	2:绑定套接字到一个IP地址和一个端口上（bind()）
	3:将套接字设置为监听模式等待连接请求（listen()）
	4:请求到来后，接受连接请求，返回一个新的对应于此次连接的套接字（accept()）
	5:用返回的套接字和客户端进行通信（send()/recv）
	6:返回，等待另一连接请求
	7：关闭套接字（closesocket()）
	8:关闭加载的套接字库（WSACleanup()）
*/

#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <WINSOCK2.H>
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

bool LoadWSA();
bool CreateSocket(SOCKET& sock, int type = SOCK_STREAM);
bool BindSocket(SOCKET& sock, short port, const char* strIp);
bool SetListen(SOCKET& sock, int number);
bool Communicate(SOCKET& serverSock);
bool CloseSocket(SOCKET& sock);
void CloseWSA();
void ExitWithErrorMessage(const string& errMsg);
DWORD WINAPI ClientThread(LPVOID sockClient);

int main()
{
	if(!LoadWSA())
		ExitWithErrorMessage("LoadWSA failed.");
	SOCKET serverSock;
	if(!CreateSocket(serverSock))
		ExitWithErrorMessage("Create server socket failed.");
	if(!BindSocket(serverSock, 6000, "127.0.0.1"))
		ExitWithErrorMessage("BindSocket failed.");
	if(!SetListen(serverSock, 10))
		ExitWithErrorMessage("SetListen failed.");
	if(!Communicate(serverSock))
		ExitWithErrorMessage("Communicate failed.");
	if(!CloseSocket(serverSock))
		ExitWithErrorMessage("CloseSocket failed.");
	CloseWSA();
}

bool LoadWSA(){
	WSAData wsaData;
	if(0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
		return true;
	return false;
}

void CloseWSA(){
	WSACleanup();
}

bool CreateSocket(SOCKET& sock, int type){
	sock = socket(AF_INET, type, 0);
	if(INVALID_SOCKET == sock)
		return false;
	return true;
}

bool BindSocket(SOCKET& sock, short port, const char* strIp){
	SOCKADDR_IN address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(strIp);
	address.sin_port = htons(port);
	memset(address.sin_zero, 0, sizeof(address.sin_zero));
	if(SOCKET_ERROR == bind(sock, (const struct sockaddr*)&address, sizeof(struct sockaddr)))
		return false;
	return true;
}

bool SetListen(SOCKET& sock, int number){
	if(SOCKET_ERROR == listen(sock, number))
		return false;
	return true;
}

bool Communicate(SOCKET& serverSock){
	SOCKADDR_IN addrClient;
	SOCKET newSock;
	HANDLE hThread = NULL;
	int n = sizeof(struct sockaddr);
	bool flag = true;
	while(true){
		newSock = accept(serverSock, (struct sockaddr*)&addrClient, &n);
		if(INVALID_SOCKET == newSock){
			cout << "Accept failed.\n";
			flag = false;
			break;
		}
		cout << "Client from " << inet_ntoa(addrClient.sin_addr) << ":" << ntohs(addrClient.sin_port) << endl;
		hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)newSock, 0, NULL);
		if(NULL == hThread){
			cout << "CreateThread failed.\n";
				flag = false;
			break;
		}
		CloseHandle(hThread);
	}
	closesocket(newSock);
	return flag;
}

bool CloseSocket(SOCKET& sock){
	if(SOCKET_ERROR == closesocket(sock))
		return false;
	return true;
}

void ExitWithErrorMessage(const string& errMsg){
	cout << errMsg << endl;
	system("pause");
	exit(1);
}

DWORD WINAPI ClientThread(LPVOID sockClient){
	SOCKET sock = (SOCKET)sockClient;
	char buffer[100] = {0};
	int errCode;
	while(true){
		memset(buffer, 0, 100);
		errCode = recv(sock, buffer, 100, 0);
		if(0 == errCode || SOCKET_ERROR == errCode){
			cout << "Client exit.\n";
			break;
		}
		cout << "Message from client：" << buffer << endl;
		send(sock, "Welcome.", strlen("Welcome."), 0);
	}
	return 0;
}