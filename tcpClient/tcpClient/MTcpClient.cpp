#include "MTcpClient.h"
#include <fstream>
#include <cstdlib>
#include <conio.h>

int MTcpClient::state = CHAT;
pair<int, int> MTcpClient::bulletinCoord;
string MTcpClient::recentMsg;

MTcpClient::MTcpClient(void){
	cout << "系统公告>>>";
	bulletinCoord = getXY();
	cout << "欢迎使用简易版聊天室version 1.0" << '\n' << endl;
	cout << "用户名和密码都是由6-20位字母(区分大小写)或数字组成,超出和其他字符无法输入" << endl;

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
			cout << "用户名: ";
		}else{
			cout << "密码: ";
		}

		int p;
		while((p = getch()) != '\r'){
			if (0 == p || 0xe0 == p){	//键入的是功能键则不做回应
				getch();
			}else if((p >= '0' && p <= '9') || (p >= 'a' && p <= 'z') || (p >= 'A' && p <= 'Z')){
				if(count != 20){		//当长度不足20就输出字符
					if(0 == mode){		//当为用户名输入时显示明文
						putchar(p);
					}else{				//当为密码输入时显示密文
						putchar('*');
					}
					info += (char)p;
					++count;
				}
			}else if('\b' == p && info.size() > 0){		//当输入退格键并且此时已经输入的有效字符大于0时进行删除字符
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
				cout << "用户名由长度为6-20的字母或数字组成" << endl;
			}else{
				cout << "密码由长度为6-20的字母或数字组成" << endl;
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
			//当服务器验证失败时会返回消息头为LOGINFALURE的字符串
			if(string(buffer) == string("LOGINFAILURE")){
				free(buffer);
				cout << "此用户名已存在且您输入的密码有误,请重新登陆" << endl;
				login();
				return;
			}else{
				cout << buffer << '\n' << endl;
				free(buffer);
				return;
			}
		}else{
			free(buffer);
			exitWithErrorMessage("登陆失败,您与服务器已断开连接");
		}
	}
}

void MTcpClient::login(){
	string name, password;

	name = inputInfo(0);
	password = inputInfo(1);
	cout << endl;

	//以NAME为消息头把用户名和密码中间加#字符串发给服务器
	string msg = string("NAME") + name + "#" + password;

	int errCode = send(clientSock, msg.c_str(), msg.size() + 1, 0);
	if(SOCKET_ERROR == errCode){
		exitWithErrorMessage("登陆失败,您与服务器已断开连接");
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
			cout << "接收信息失败,您与服务器断开连接" << endl;
			break;
		}else if(string(buffer, buffer + FILE.size()) == FILE){		//当收到的消息头为FILE时则进行文件下载
			downLoad((SOCKET)sock, buffer, errCode);
			state = COMMAND;			//由聊天模式变为命令模式
			cout << "文件下载成功,是否立即打开(Y/N): ";
		}else{
			dealWithMsg(buffer);		//普通消息则进行分类处理
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
				cout << "发送失败,每次只能发送100以内个字节(一个汉字占两个字节)" << endl;
			}else if(0 == msg.size()){
				cout << "不能发送空消息" << endl;
			}else{
				errCode = send(clientSock, msg.c_str(), msg.size() + 1, 0);
				if(SOCKET_ERROR == errCode){
					cout << "发送信息失败,您与服务器与断开连接" << endl;
					break;
				}
				recentMsg = msg;
			}
		}else if(COMMAND == state){		//当前为命令模式则进行相关的响应后转换回聊天模式
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
	//160为标准cmd窗口两行字符个数,FILL_LEN表示填充的空格个数
	const int FILL_LEN = 160 - strlen("系统公告>>>") - msg.size();

	//当为公告消息时则光标移到公告位置并进行输出公告再返回原来位置
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

	//网络过差时可能连消息头都接不全,所以这里可以处理这种情况
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

	//只要接收的字节数不足要求的字节数就不断累加
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

	//把接收到的所有字节写入文件
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
		cout << "暂不支持此格式文件的自动查看,请手动打开" << endl;
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