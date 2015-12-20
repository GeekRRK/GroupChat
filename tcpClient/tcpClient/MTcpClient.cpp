#include "MTcpClient.h"
#include <fstream>
#include <cstdlib>
#include <conio.h>

int MTcpClient::state = CHAT;
pair<int, int> MTcpClient::bulletinCoord;
string MTcpClient::recentMsg;

MTcpClient::MTcpClient(void){
	cout << "ϵͳ����>>>";
	bulletinCoord = getXY();
	cout << "��ӭʹ�ü��װ�������version 1.0" << '\n' << endl;
	cout << "�û��������붼����6-20λ��ĸ(���ִ�Сд)���������,�����������ַ��޷�����" << endl;

	if(!loadWSA())
		exitWithErrorMessage("LoadWSA  failed.");

	if(!createSocket())
		exitWithErrorMessage("Create client socket failed.");

	if(!connectToServer(6000, "127.0.0.1"))
		exitWithErrorMessage("ConnectToServer failed.");

	login();
	communicate();
	
	if(!closeSocket())
		exitWithErrorMessage("CloseSocket failed.");

	closeWSA();
}

MTcpClient::~MTcpClient(void){
}

bool MTcpClient::loadWSA(){
	WSAData wsaData;
	
	if(0 == WSAStartup(MAKEWORD(2, 2), &wsaData)){
		return true;
	}
	
	return false;
}

void MTcpClient::closeWSA(){
	WSACleanup();
}

bool MTcpClient::createSocket(int type/* = SOCK_STREAM */){
	clientSock = socket(AF_INET, type, 0);
	
	if(INVALID_SOCKET == clientSock){
		return false;
	}
	
	return true;
}

bool MTcpClient::connectToServer(short port ,const char* strIp){
	SOCKADDR_IN address;
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(strIp);
	address.sin_port = htons(port);
	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	if(SOCKET_ERROR == connect(clientSock, (const struct sockaddr*)&address, sizeof(struct sockaddr))){
		return false;
	}
	
	return true;
}

void MTcpClient::exitWithErrorMessage(const string& errMsg){
	cout << errMsg << endl;
	system("pause");
	exit(1);
}

bool MTcpClient::closeSocket(){
	if(SOCKET_ERROR == closesocket(clientSock))
		return false;

	return true;
}

string MTcpClient::inputInfo(int mode){
	string info;
	int count;

	do{
		info = "";
		count = 0;

		if(0 == mode){
			cout << "�û���: ";
		}else{
			cout << "����: ";
		}

		int p;
		while((p = getch()) != '\r'){
			if (0 == p || 0xe0 == p){	//������ǹ��ܼ�������Ӧ
				getch();
			}else if((p >= '0' && p <= '9') || (p >= 'a' && p <= 'z') || (p >= 'A' && p <= 'Z')){
				if(count != 20){		//�����Ȳ���20������ַ�
					if(0 == mode){		//��Ϊ�û�������ʱ��ʾ����
						putchar(p);
					}else{				//��Ϊ��������ʱ��ʾ����
						putchar('*');
					}
					info += (char)p;
					++count;
				}
			}else if('\b' == p && info.size() > 0){		//�������˸�����Ҵ�ʱ�Ѿ��������Ч�ַ�����0ʱ����ɾ���ַ�
				putchar('\b');
				putchar(' ');
				putchar('\b');
				info = info.substr(0, info.size() - 1);
				--count;
			}
		}

		putchar('\n');

		if(info.size() < 6){
			if(0 == mode){
				cout << "�û����ɳ���Ϊ6-20����ĸ���������" << endl;
			}else{
				cout << "�����ɳ���Ϊ6-20����ĸ���������" << endl;
			}
		}else{
			return info;
		}
	}while(true);
}

void MTcpClient::waitToLogin(){
	char *buffer = (char *)malloc(RECV_LEN);
	int errCode;

	while(true){
		memset(buffer, 0, RECV_LEN);
		errCode = recv((SOCKET)clientSock, buffer, RECV_LEN, 0);
		if(SOCKET_ERROR != errCode){
			//����������֤ʧ��ʱ�᷵����ϢͷΪLOGINFALURE���ַ���
			if(string(buffer) == string("LOGINFAILURE")){
				free(buffer);
				cout << "���û����Ѵ��������������������,�����µ�½" << endl;
				login();
				return;
			}else{
				cout << buffer << '\n' << endl;
				free(buffer);
				return;
			}
		}else{
			free(buffer);
			exitWithErrorMessage("��½ʧ��,����������ѶϿ�����");
		}
	}
}

void MTcpClient::login(){
	string name, password;

	name = inputInfo(0);
	password = inputInfo(1);
	cout << endl;

	//��NAMEΪ��Ϣͷ���û����������м��#�ַ�������������
	string msg = string("NAME") + name + "#" + password;

	int errCode = send(clientSock, msg.c_str(), msg.size() + 1, 0);
	if(SOCKET_ERROR == errCode){
		exitWithErrorMessage("��½ʧ��,����������ѶϿ�����");
	}

	waitToLogin();
}

DWORD WINAPI MTcpClient::recvMsg(LPVOID sock){
	int errCode;
	const string FILE("FILE");
	const int RECV_LEN = 200;

	while(true){
		char *buffer = (char *)malloc(RECV_LEN);
		memset(buffer, 0, RECV_LEN);
		errCode = recv((SOCKET)sock, buffer, RECV_LEN, 0);

		if(0 == errCode || SOCKET_ERROR == errCode){
			cout << "������Ϣʧ��,����������Ͽ�����" << endl;
			break;
		}else if(string(buffer, buffer + FILE.size()) == FILE){		//���յ�����ϢͷΪFILEʱ������ļ�����
			downLoad((SOCKET)sock, buffer, errCode);
			state = COMMAND;			//������ģʽ��Ϊ����ģʽ
			cout << "�ļ����سɹ�,�Ƿ�������(Y/N): ";
		}else{
			dealWithMsg(buffer);		//��ͨ��Ϣ����з��ദ��
			free(buffer);
		}
	}

	return 0;
}

void MTcpClient::communicate(){
	HANDLE hThread = NULL;
	hThread = CreateThread(NULL, 0, recvMsg, (LPVOID)clientSock, 0, NULL);
	CloseHandle(hThread);

	int errCode;
	string msg;

	while(true){
		msg = "";
		getline(cin, msg);
		cin.clear();

		if(CHAT == state){
			if(msg.size() > MTU){
				cout << "����ʧ��,ÿ��ֻ�ܷ���100���ڸ��ֽ�(һ������ռ�����ֽ�)" << endl;
			}else if(0 == msg.size()){
				cout << "���ܷ��Ϳ���Ϣ" << endl;
			}else{
				errCode = send(clientSock, msg.c_str(), msg.size() + 1, 0);
				if(SOCKET_ERROR == errCode){
					cout << "������Ϣʧ��,�����������Ͽ�����" << endl;
					break;
				}
				recentMsg = msg;
			}
		}else if(COMMAND == state){		//��ǰΪ����ģʽ�������ص���Ӧ��ת��������ģʽ
			if(msg.size() > 0){
				if(msg[0] == 'Y' || msg[0] == 'y'){
					openFile(recentMsg.substr(recentMsg.find_last_of("\\") + 1));
				}
			}

			state = CHAT;
		}
	}
}

void MTcpClient::dealWithMsg(const string& msg){
	const string BULLETIN("BULLETIN");
	//160Ϊ��׼cmd���������ַ�����,FILL_LEN��ʾ���Ŀո����
	const int FILL_LEN = 160 - strlen("ϵͳ����>>>") - msg.size();

	//��Ϊ������Ϣʱ�����Ƶ�����λ�ò�������������ٷ���ԭ��λ��
	if(msg.substr(0, BULLETIN.size()) == BULLETIN){
		pair<int, int> currentCoord = getXY();
		gotoXY(bulletinCoord.first, bulletinCoord.second);
		cout << msg.substr(BULLETIN.size()) + string(FILL_LEN, ' ');
		gotoXY(currentCoord.first, currentCoord.second);
	}else{
		cout << msg << endl;
	}
}

void MTcpClient::downLoad(SOCKET sock, char* buffer, int recvLen){
	const int RECV_MAX = 400000;
	const int HEAD_LEN = 14;
	int reqLen;
	int currLen;

	currLen = recvLen;

	//�������ʱ��������Ϣͷ���Ӳ�ȫ,����������Դ����������
	while(currLen < HEAD_LEN){
		char *tmp = (char *)malloc(HEAD_LEN);
		recvLen = recv(sock, tmp, RECV_MAX, 0);
		for(int i = 0; i < recvLen; ++i){
			buffer[i] = tmp[i];
		}
		free(tmp);
		currLen += recvLen;
	}

	ofstream fout;
	fout.open(recentMsg.substr(recentMsg.find_last_of("\\") + 1), ios::binary);

	reqLen = atoi(string(buffer).substr(4, 10).c_str());
	buffer = (char *)realloc(buffer, reqLen);

	//ֻҪ���յ��ֽ�������Ҫ����ֽ����Ͳ����ۼ�
	while(currLen < reqLen){
		char *tmp = (char *)malloc(RECV_MAX);
		recvLen = recv(sock, tmp, RECV_MAX, 0);

		if(currLen + recvLen > reqLen){
			for(long i = 0; i < reqLen - currLen; ++i){
				buffer[currLen + i] = tmp[i];
			}
			break;
		}

		for(long i = 0; i < recvLen; ++i){
			buffer[currLen + i] = tmp[i];
		}
		currLen += recvLen;
		
		free(tmp);
	}

	//�ѽ��յ��������ֽ�д���ļ�
	for(int i = 14; i < reqLen; ++i){
		fout.put(buffer[i]);
	}

	fout.close();
	free(buffer);
}

void MTcpClient::openFile(const string& fileName){
	string exe;
	string postfix = fileName.substr(fileName.find_last_of(".") + 1, fileName.find_last_not_of(" ") - fileName.find_last_of("."));
	if((postfix == string("txt")) || (postfix == string("mp3"))){
		system(fileName.c_str());
	}else{
		cout << "�ݲ�֧�ִ˸�ʽ�ļ����Զ��鿴,���ֶ���" << endl;
	}
}

pair<int, int> MTcpClient::getXY(){
	HANDLE hStdout;
    CONSOLE_SCREEN_BUFFER_INFO pBuffer;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &pBuffer); 

	return pair<int, int>(pBuffer.dwCursorPosition.X, pBuffer.dwCursorPosition.Y);
}

void MTcpClient::gotoXY(int x, int y){
	CONSOLE_SCREEN_BUFFER_INFO    csbiInfo;
    HANDLE    hConsoleOut;
    hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsoleOut,&csbiInfo);
    csbiInfo.dwCursorPosition.X = x;
    csbiInfo.dwCursorPosition.Y = y;
    SetConsoleCursorPosition(hConsoleOut,csbiInfo.dwCursorPosition);
}