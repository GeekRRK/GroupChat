#pragma once
#include <string>
#include <windows.h>
#include "mysql.h"
#include "User.h"
using namespace std;

class DB{
public:
	DB(void);
	~DB(void);

public:
	//���ܣ������˳�����
	//������������Ϣ�ַ���
	void exitWithErrorMessage(const string& errMsg);

	//���ܣ��������ݿ�
	void connectMysql();

	//���ܣ������û����ж��û��Ƿ���������ݿ���
	//�������û����ַ���
	//����ֵ:������ڷ���true�������ڷ���false
	bool existByName(const string& name);

	//���ܣ������û����������ݿ��е��û�
	//�������û����ַ���
	//����ֵ:User���͵Ķ���
	User queryByName(const string& name);

	//���ܣ��������û������ݿ���
	//������User���͵����û�
	void insertUser(User user);

	//���ܣ�����User����ȥ�������ݿ��к�User��������һ�µļ�¼����Ӧ����
	//������User���ͺ����ݿ��ж�Ӧ��¼����(����)һ�µ������µ����Ե��û�
	void updateUser(User user);

private:
	MYSQL *con;
	MYSQL_RES *res;
	MYSQL_ROW row;

	string dbuser;
	string dbpassword;
	string dbip;
	string dbname;
	string tablename;
};