#include "TcpServer.h"
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <malloc.h>

SOCKET TcpServer::serverSock;
DB TcpServer::db;
vector<SOCKET> TcpServer::clients;
map<SOCKET, string> TcpServer::client_name;
map<SOCKET, string> TcpServer::client_time;

TcpServer::TcpServer(void){
	cout << "��ӭʹ�������ҹ���ϵͳ,��ȴ��û�������>>>" << endl;

	if(!loadWSA())
		exitWithErrorMessage("LoadWSA failed.");

	if(!createSocket())
		exitWithErrorMessage("Create server socket failed.");

	if(!bindSocket(6000, "127.0.0.1"))
		exitWithErrorMessage("BindSocket failed.");

	if(!setListen(10))
		exitWithErrorMessage("SetListen failed.");

	communicate();

	if(!closeSocket())
		exitWithErrorMessage("CloseSocket failed.");

	closeWSA();
}

TcpServer::~TcpServer(void){
}

bool TcpServer::loadWSA(){
	WSAData wsaData;

	if(0 == WSAStartup(MAKEWORD(2, 2), &wsaData)){
		return true;
	}

	return false;
}

void TcpServer::closeWSA(){
	WSACleanup();
}

bool TcpServer::createSocket(int type/* = SOCK_STREAM */){
	serverSock = socket(AF_INET, type, 0);

	if(INVALID_SOCKET == serverSock){
		return false;
	}

	return true;
}

bool TcpServer::bindSocket(short port, const char* ip){
	SOCKADDR_IN address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port = htons(port);
	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	if(SOCKET_ERROR == bind(serverSock, (const struct sockaddr*)&address, sizeof(struct sockaddr))){
		return false;
	}

	return true;
}

bool TcpServer::setListen(int number){
	if(SOCKET_ERROR == listen(serverSock, number)){
		return false;
	}

	return true;
}

bool TcpServer::closeSocket(){
	if(SOCKET_ERROR == closesocket(serverSock)){
		return false;
	}

	return true;
}

void TcpServer::exitWithErrorMessage(const string& errMsg){
	cout << errMsg << endl;
	system("pause");
	exit(1);
}

void TcpServer::communicate(){
	HANDLE hThread = NULL;

	hThread = CreateThread(NULL, 0, acceptClient, (LPVOID)0, 0, NULL);
	if(NULL == hThread){
		cout << "CreateThread recvMsg failed.\n";
	}
	CloseHandle(hThread);

	string msg;
	while(true){
		msg = "";
		getline(cin, msg);
		cin.clear();

		if(msg.size() > MTU){
			cout << "����ʧ��,ÿ��ֻ�ܷ���100���ڸ��ֽ�(һ������ռ�����ֽ�)" << endl;
		}else if(0 == msg.size()){
			cout << "���ܷ��Ϳ���Ϣ" << endl;
		}else{
			if(clients.size() > 0){
				//����ϢͷΪBULLETIN����ϢΪ������Ϣ
				notifyOtherClients(-1, string("BULLETIN") + msg);
				notifyOtherClients(-1, "ϵͳ����>>>" + msg);
				cout << "���淢�ͳɹ�" << endl;
			}else{
				cout << "��ǰû�������û�" << endl;
			}
		}
	}
}

string TcpServer::getClientIpPort(SOCKET sock){
	struct sockaddr_in sa;
	int len = sizeof(sa);
	string ip("δ֪�û�");
	string port("δ֪�˿�");
	
	if(!getpeername((SOCKET)sock, (struct sockaddr*)&sa, &len)){
		ip = inet_ntoa(sa.sin_addr);
		char strPort[10] = {0};
		_itoa_s(ntohs(sa.sin_port), strPort, sizeof(strPort), 10);
		port = strPort;
	}

	return ip + "|" + port;
}

void TcpServer::notifyOtherClients(SOCKET sock, const string& msg){
	for(vector<SOCKET>::const_iterator it = clients.begin(); it != clients.end(); ++it){
		if(*it != sock){
			send(*it, msg.c_str(), msg.size() + 1, 0);
		}
	}
}

DWORD WINAPI TcpServer::recvMsg(LPVOID sockClient){
	SOCKET sock = (SOCKET)sockClient;
	char *buffer = (char *)malloc(RECV_LEN);
	int errCode;

	while(true){
		memset(buffer, 0, RECV_LEN);
		errCode = recv(sock, buffer, RECV_LEN, 0);

		if(0 == errCode || SOCKET_ERROR == errCode){
			clientLogout(sock);
			break;
		}

		dealWithMsg(sock, buffer);
	}

	free(buffer);

	return 0;
}

DWORD WINAPI TcpServer::acceptClient(LPVOID sock){
	SOCKADDR_IN addrClient;
	SOCKET newSock;
	HANDLE hThread = NULL;
	int n = sizeof(struct sockaddr);

	while(true){
		newSock = accept(serverSock, (struct sockaddr*)&addrClient, &n);
		if(INVALID_SOCKET == newSock){
			cout << "Accept failed.\n";
			break;
		}

		hThread = CreateThread(NULL, 0, recvMsg, (LPVOID)newSock, 0, NULL);
		if(NULL == hThread){
			cout << "CreateThread recvMsg failed.\n";
		}

		CloseHandle(hThread);
	}

	closesocket(newSock);
	return 0;
}

void TcpServer::listClients(){
	system("cls");

	cout << "��ǰ��������Ϊ: " << clients.size() << '\n' << endl;

	//����ΪһЩ��ʽ������
	cout.setf(ios::left);
	cout.width(20);
	cout << "�û��ǳ�";
	cout.setf(ios::left);
	cout.width(20);
	cout << "��ַ�˿�";
	cout.setf(ios::left);
	cout.width(20);
	cout << "����ʱ��" << endl;
	cout << endl;

	for(vector<SOCKET>::const_iterator it = clients.begin(); it != clients.end(); ++it){
		cout.setf(ios::left);
		cout.width(20);
		cout << client_name.find(*it)->second;
		cout.setf(ios::left);
		cout.width(20);
		cout << getClientIpPort(*it);
		cout.setf(ios::left);
		cout.width(20);
		cout << client_time.find(*it)->second << endl;
	}
}

void TcpServer::removeClient(SOCKET sock){
	for(vector<SOCKET>::const_iterator it = clients.begin(); it != clients.end(); ++it){
		if(*it == sock){
			clients.erase(it);
			break;
		}
	}
}

int TcpServer::checkNamePwd(const string& name, const string& pwd){

	if(!db.existByName(name)){
		User user;
		user.setName(name);
		user.setPassword(pwd);
		db.insertUser(user);

		return NEWUSER;
	}else{
		//�������ݿ��в����ִ�Сд,�������һ������
		if(db.queryByName(name).getName() != name){
			User user;
			user.setName(name);
			user.setPassword(pwd);
			db.insertUser(user);

			return NEWUSER;
		}else{
			if(db.queryByName(name).getPassord() == pwd){
				return OLDUSER;
			}
		}
	}
}

void TcpServer::clientLogin(SOCKET sock, const string& name){
	clients.push_back(sock);
	client_name.insert(pair<SOCKET, string>(sock, name));

	//��õ�ǰ����ʱ��
	time_t t = time(0);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%m/%d  %A  %X", localtime(&t));
	client_time.insert(pair<SOCKET, string>(sock, tmp));

	User user;
	user.setName(name);
	user.setPassword("��Ч");
	user.setTime(tmp);
	db.updateUser(user);
	
	notifyOtherClients(-1, client_name.find(sock)->second + "����������");
	listClients();
}

void TcpServer::clientLogout(SOCKET sock){
	map<SOCKET, string>::const_iterator it = client_name.find(sock);
	if(it == client_name.end()){
		removeClient(sock);
	}else{
		notifyOtherClients(-1, client_name.find(sock)->second + "�˳�������");
		removeClient(sock);
		listClients();
	}
}

void TcpServer::sendFile(SOCKET sock, const string& fileName){
	const int MAXDOWN = 1024 * 1024 * 50;	//�����û����ص�����ļ����ֽ���Ϊ50M
	int count = 14;		//�ۼ�Ҫ���͵��ֽ���,��ʼΪ��Ϣͷ����14�ֽ�(4�ֽ�����,10�ֽڳ���)

	ifstream infile;
	infile.open(fileName.c_str(), ios::binary);
	if(!infile){
		send(sock, "û�д��ļ�", strlen("û�д��ļ�") + 1, 0);
	}else{
		//�õ��ļ���С
		infile.seekg(0, ios::end);
		streampos ps = infile.tellg();
		infile.close();
		if(ps > MAXDOWN){
			send(sock, "�ļ������޷�����,֧������50M���ڵ��ļ�", strlen("�ļ������޷�����,֧������50M���ڵ��ļ�") + 1, 0);
		}else{
			char *p = (char *)malloc(MAXDOWN);
			//��ϢͷΪFILEΪ�ļ�����
			p[0] = 'F'; p[1] = 'I'; p[2] = 'L'; p[3] = 'E';

			infile.open(fileName.c_str(), ios::binary);
			char ch;
			while((ch = infile.get()) != EOF){
				p[count++] = ch;
			}
			infile.close();

			//���㳤���ֶ�
			char len[10] = {0};
			itoa(count, len, 10);
			int n = 0;
			int tmp = count;
			while(tmp){
				++n;
				tmp /= 10;
			}

			//�ѳ����ֶμ�����Ϣͷ��
			for(int i = 0; i < n; ++i){
				p[4 + i] = len[i];
			}

			//����10λ�����ֶ���#���
			for(int i = n; i < 10; ++i){
				p[4 + i] = '#';
			}
			send(sock, p, count, 0);
			
			free(p);
		}
	}
}

void TcpServer::dealWithMsg(SOCKET sock, const string& msg){
	const string NAME = "NAME";
	const string LOGINFAILURE = "LOGINFAILURE";

	//��ε�ǰΪ��NAMEΪ��Ϣͷ����Ϣ������֤��ǰ�û��Ƿ������ٶ������û��ֱ���,�ڶ���������Ϊ�˴��������û�����NAME��ͷ����Ϣ
	if(msg.substr(0, NAME.size()) == NAME && client_name.find(sock) == client_name.end()){

		//����Ϣ�л�ȡ�û���������
		string name = msg.substr(NAME.size(), msg.find_first_of("#") - NAME.size());
		string password = msg.substr(msg.find_first_of("#") + 1);

		if(NEWUSER == checkNamePwd(name, password)){
			send(sock, "��������һ�ε�½,���μ��û���������", strlen("��������һ�ε�½,���μ��û���������") + 1, 0);
			clientLogin(sock, name);
		}else if(OLDUSER == checkNamePwd(name, password)){
			send(sock, ("�����һ�ε�½ʱ��Ϊ: " + db.queryByName(name).getTime() + ",�粻�Ǳ��˵�½����ϵ����Ա").c_str(), 
				("�����һ�ε�½ʱ��Ϊ: " + db.queryByName(name).getTime() + ",�粻�Ǳ��˵�½����ϵ����Ա").size() + 1, 0);
			clientLogin(sock, name);
		}else{
			send(sock, LOGINFAILURE.c_str(), LOGINFAILURE.size() + 1, 0);
		}
	}else if(string::npos != msg.find(":\\") || string::npos != msg.find(":/")){	//��Ϣ�а���:\��:/��Ϊ�ļ�����������Ϣ
		sendFile(sock, msg);
	}else{
		notifyOtherClients(-1, client_name.find(sock)->second +  ": " + msg);
	}
}