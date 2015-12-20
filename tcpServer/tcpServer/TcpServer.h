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
		RECV_LEN = 200,			//������Ϣ������ֽ���
		MTU = 100,				//���͵�����ֽ���

		NEWUSER = 1,			//���û���־
		OLDUSER = 2				//���û���־
	};

private:
	//socket��д�������Ļ�������
	bool loadWSA();
	bool createSocket(int type = SOCK_STREAM);
	bool bindSocket(short port, const char* ip);
	bool setListen(int number);
	void communicate();
	bool closeSocket();
	void closeWSA();
	void exitWithErrorMessage(const string& errMsg);

private:
	//���ܣ��ȴ��ͻ��˵����ӵ��߳�
	//�������ڴ����̵߳�ʱ�򣬰ѷ�������(LPVOID)SOCKETд��CreateThread()�ĵ��ĸ�����
	//����ֵ��DWORD WINAPI
	static DWORD WINAPI acceptClient(LPVOID sock);

	//���ܣ�������Ϣ���߳�
	//�������ڴ����̵߳�ʱ�򣬰ѷ�������(LPVOID)SOCKETд��CreateThread()�ĵ��ĸ�����
	//����ֵ��DWORD WINAPI
	static DWORD WINAPI recvMsg(LPVOID sock);

	//���ܣ���֤��ǰ�û������û��������û�����������û��Ͳ��뵽���ݿ���
	//�������û������ֺ������ַ���
	//����ֵ�����û�����NEWUSER�����û�����OLDUSER
	static int checkNamePwd(const string& name, const string& pwd);

	//���ܣ�ʹ�û���½�������ң���¼�û����ֲ����������û��б�
	//�������ͻ��˵�SOCKET���û����ַ���
	static void clientLogin(SOCKET sock, const string& name);

	//���ܣ�ʹ�û����˳������ң����û��������û���ɾ�������������û��б�
	//�������ͻ��˵�SOCKET
	static void clientLogout(SOCKET sock);

	//���ܣ����������û��б�
	static void listClients();

	//���ܣ����û��������û���ɾ��
	//�������ͻ��˵�SOCKET
	static void removeClient(SOCKET sock);

	//���ܣ��õ��û���Ip��Port����ַ���
	//�������ͻ��˵�SOCKET
	//����ֵ��Ip��Port����ַ���
	static string getClientIpPort(SOCKET sock);

	//���ܣ�֪ͨ�����û�(����ĳ���û�)��Ϣ
	//����������֪ͨ��ĳ���û���SOCKET����Ϣ�ַ���(��sockΪ-1ʱ֪ͨ�����û�)
	static void notifyOtherClients(SOCKET sock, const string& msg);

	//���ܣ����ļ����ݷ��͸��û�
	//�������û���SOCKET��Ҫ���͵��ļ����ļ����ַ���
	static void sendFile(SOCKET sock, const string& fileName);

	//���ܣ����ദ����յ�����Ϣ
	//�������û���SOCKET�����յ�����Ϣ�ַ���
	static void dealWithMsg(SOCKET sock, const string& msg);

private:
	static SOCKET serverSock;					//��������SOCKET
	static DB db;								//���ݿ����
	static vector<SOCKET> clients;				//�洢���߿ͻ���SOCKET������
	static map<SOCKET, string> client_name;		//�ͻ�SOCKET���ǳƵ�ӳ��
	static map<SOCKET, string> client_time;		//�ͻ�SOCKET�����һ�ε�½ʱ���ӳ��
};