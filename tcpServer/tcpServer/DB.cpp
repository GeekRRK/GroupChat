#include "DB.h"
#include <iostream>
using namespace std;

DB::DB(void): dbuser("root"), dbpassword(""), dbip("localhost"), dbname("chat"), tablename("user"){
	connectMysql();
}

DB::~DB(void){
	mysql_close(con);
}

void DB::exitWithErrorMessage(const string& errMsg){
	cout << errMsg << endl;
	system("pause");
	exit(1);
}

void DB::connectMysql(){
	con = mysql_init((MYSQL*)0);
	if(con != NULL && mysql_real_connect(con, dbip.c_str(), dbuser.c_str(), dbpassword.c_str(), dbname.c_str(), 3306, NULL, 0)){
		con->reconnect = 1;
		char *query = "set names \'GBK\'";
		int rt = mysql_real_query(con, query, strlen(query));
		if(rt){
			exitWithErrorMessage(mysql_error(con));
		}

		char *q = "select * from user";
		rt = mysql_real_query(con, q, strlen(q));
		if(rt){
			exitWithErrorMessage(mysql_error(con));
		}else{
			res = mysql_store_result(con);
			int num = mysql_num_fields(res);
			if(num != 4){
				exitWithErrorMessage("连接失败,请更新数据表user(在Navicat中双击打开名为chat的数据库,再右键chat数据库,选择'运行SQL文件...',然后选择与程序同目录下的chat.sql,按开始)");
			}
		}
	}else{
		exitWithErrorMessage("Connect to database failed.");
	}
}

bool DB::existByName(const string& name){
	string query = "select * from " + tablename + " where name='" + name + "'";
	int rt = mysql_real_query(con, query.c_str(), query.size());
	if(rt){
		exitWithErrorMessage(mysql_error(con));
	}

	res = mysql_store_result(con);
	if(row = mysql_fetch_row(res)){
		mysql_free_result(res);

		return true;
	}

	mysql_free_result(res);

	return false;
}

User DB::queryByName(const string& name){
	string query = "select * from " + tablename + " where name='" + name + "'";
	int rt = mysql_real_query(con, query.c_str(), query.size());
	if(rt){
		exitWithErrorMessage(mysql_error(con));
	}
	res = mysql_store_result(con);
	row = mysql_fetch_row(res);

	User user;
	user.setName(row[1]);
	user.setPassword(row[2]);
	user.setTime(row[3]);

	mysql_free_result(res);

	return user;
}

void DB::insertUser(User user){
	string query = "insert into " + tablename + "(name, password, time) values('" + user.getName() + "','" + user.getPassord() + "','" + user.getTime() + "')";
	int rt = mysql_real_query(con, query.c_str(), query.size());
	if(rt){
		exitWithErrorMessage(mysql_error(con));
	}
}

void DB::updateUser(User user){
	string query = "update " + tablename + " set time='" + user.getTime() + "' where name='" + user.getName() + "'";
	int rt = mysql_real_query(con, query.c_str(), query.size());
	if(rt){
		exitWithErrorMessage(mysql_error(con));
	}
}