#pragma once
#include <string>
using namespace std;

class User{
public:
	User(void);
	~User(void);

public:
	string getName();
	void setName(const string& name);

	string getPassord();
	void setPassword(const string& password);

	string getTime();
	void setTime(const string& time);

private:
	string name;
	string password;
	string time;
};