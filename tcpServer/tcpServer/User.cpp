#include "User.h"

User::User(void){
}

User::~User(void){
}

string User::getName(){
	return name;
}

void User::setName(const string& name){
	this->name = name;
}

string User::getPassord(){
	return password;
}

void User::setPassword(const string& password){
	this->password = password;
}

string User::getTime(){
	return time;
}

void User::setTime(const string& time){
	this->time = time;
}