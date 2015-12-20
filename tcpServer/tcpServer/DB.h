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
	//功能：报错并退出程序
	//参数：错误信息字符串
	void exitWithErrorMessage(const string& errMsg);

	//功能：连接数据库
	void connectMysql();

	//功能：根据用户名判断用户是否存在于数据库中
	//参数：用户名字符串
	//返回值:如果存在返回true，不存在返回false
	bool existByName(const string& name);

	//功能：根据用户名查找数据库中的用户
	//参数：用户名字符串
	//返回值:User类型的对象
	User queryByName(const string& name);

	//功能：插入新用户到数据库中
	//参数：User类型的新用户
	void insertUser(User user);

	//功能：根据User对象去更新数据库中和User对象名字一致的记录的相应属性
	//参数：User类型和数据库中对应记录名字(主键)一致但有最新的属性的用户
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