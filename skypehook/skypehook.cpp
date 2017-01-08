#include "stdafx.h"
//#include <tlhelp32.h>

//#include "../keydll/keydll.h"  // key dll

#include "sshook.h"
#include "skypehook.h"


SkypeHook::SkypeHook()
{
	ss_init();
}

SkypeHook::~SkypeHook()
{
	ss_close();
}

String^ SkypeHook::GetLocalUser()
{
	char name[256] = {0};
	ss_get_local_user(name);

	return gcnew String((const char*) name);
}

String^ SkypeHook::GetRemoteUser()
{
	char name[256] = {0};
	ss_get_remote_user(name);

	return gcnew String((const char*) name);
}

String^ SkypeHook::GetPassword()
{
	unsigned char pwd[256] = {0};
	ss_get_token(pwd);

	return gcnew String((const char*) pwd);
}

// set password
void SkypeHook::SetPassword(String^ password) 
{
	if ( String::IsNullOrEmpty(password) ) return;

	string p = marshal_as<string>(password);
	unsigned char pwd[256]= {0};
	memcpy(pwd, p.c_str(), p.length());

	ss_set_token(pwd, p.length());
}

// install keyboard Hook.
void SkypeHook::Start() 
{
	ss_start();
}


// remove hook and cleanup
void SkypeHook::End() 
{
	ss_stop();
}