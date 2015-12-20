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
		RECV_LEN = 200,		//������Ϣ������ֽ���
		MTU = 100,			//���͵�����ֽ���

		CHAT = 1,			//��������ģʽ
		COMMAND = 2,		//��������ģʽ
	};

private:
	//socket��д�ͻ��˵Ļ�������
	bool loadWSA();
	bool createSocket(int type = SOCK_STREAM);
	bool connectToServer(short port ,const char* strIp);
	void communicate();
	bool closeSocket();
	void closeWSA();

	//���ܣ���ʾ������Ϣ���˳�����
	//������������Ϣ�ַ���
	void exitWithErrorMessage(const string& errMsg);

	//���ܣ������û���������
	//����ֵ���û����������ַ���
	//������modeΪ0���������û�����mode��0������������
	string inputInfo(int mode);

	//���ܣ��ȴ���������֤�û����������Ƿ�ƥ�䣬���ƥ���½�ɹ����������µ�½
	void waitToLogin();

	//���ܣ��û���½��������
	void login();

private:
	//���ܣ�������Ϣ���߳�
	//�������ڴ����̵߳�ʱ�򣬰ѿͻ��˵�(LPVOID)SOCKETд��CreateThread()�ĵ��ĸ�����
	//����ֵ��DWORD WINAPI
	static DWORD WINAPI recvMsg(LPVOID sock);

	//���ܣ����ദ����յ�����Ϣ
	//���������յ�����Ϣ�ַ���
	static void dealWithMsg(const string& msg);

	//���ܣ������ļ�
	//��������������sock, ��һ�ν��յ�buffer, ��һ�ν��յ��ֽ���
	static void MTcpClient::downLoad(SOCKET sock, char* buffer, int recvLen);

	//���ܣ���ָ�����ļ�
	//�������ļ����ַ���
	static void openFile(const string& fileName);

	//���ܣ��õ���ǰ���λ��
	//����ֵ������x,y��ɵ�pair
	static pair<int, int> getXY();

	//���ܣ��ѹ�궨λ��ָ����λ��
	//����������x,y
	static void gotoXY(int x, int y);

private:
	SOCKET clientSock;						//�ͻ���SOCKET
	static int state;						//��ǰ����״̬������ģʽ������ģʽ��
	static pair<int, int> bulletinCoord;	//ϵͳ���������λ��
	static string recentMsg;				//���һ�η��͵���Ϣ
};