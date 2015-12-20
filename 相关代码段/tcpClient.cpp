/*
	0:�����׽��ֿ⣨WSAStartup()��
	1:�����׽��֣�socket()��
	2:�������������������connect()��
	3:�ͷ������˽���ͨ�ţ�send()/recv()��
	4:�ر��׽��֣�closesocket()��
	5:�رռ��ص��׽��ֿ⣨WSAStartup()��
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
bool ConnectToServer(SOCKET& sock, short port, const char* strIp);
bool Communicate(SOCKET& sock);
bool CloseSocket(SOCKET& sock);
void CloseWSA();
void ExitWithErrorMessage(const string& errMsg);

int main()
{
	if(!LoadWSA())
		ExitWithErrorMessage("LoadWSA failed.");
	SOCKET clientSock;
	if(!CreateSocket(clientSock))
		ExitWithErrorMessage("Create Server Socket failed.");
	if(ConnectToServer(clientSock, 6000, "127.0.0.1"))
		ExitWithErrorMessage("ConnectToServer failed.");
	if(!Communicate(clientSock))
		ExitWithErrorMessage("Communicate failed.");
	if(!CloseSocket(clientSock))
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

bool ConnectToServer(SOCKET& sock, short port, const char* strIp){
	SOCKADDR_IN address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(strIp);
	address.sin_port = htons(port);
	memset(address.sin_zero, 0, sizeof(address.sin_zero));
	if(SOCKET_ERROR == connect(sock, (const struct sockaddr*)&address, sizeof(struct sockaddr)))
		return false;
	return true;
}

bool Communicate(SOCKET& sock){
	bool flag = true;
	int errCode;
	char buffer[100] = {0};
	while(true){
		memset(buffer, 0, 100);
		cin.getline(buffer, 100);
		errCode = send(sock, buffer, strlen(buffer), 0);
		if(SOCKET_ERROR == errCode){
			cout << "Send message error.\n";
			flag = false;
			break;
		}
		memset(buffer, 0, 100);
		recv(sock, buffer, 100, 0);
		cout << buffer << endl;
	}
	return flag;
}

void ExitWithErrorMessage(const string& errMsg){
	cout << errMsg << endl;
	system("pause");
	exit(1);
}

bool CloseSocket(SOCKET& sock){
	if(SOCKET_ERROR == closesocket(sock))
		return false;
	return true;
}