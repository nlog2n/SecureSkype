#include "stdafx.h"
#include <tlhelp32.h>

#include <stdlib.h>
#include <string.h>
using namespace std;

#include "../keydll/keydll.h"  // key dll

#define SKYPE_HOOK_EXPORTS
#include "sshook.h"

bool _hooked = false;
HWND _txtinWND = NULL;
char _loginuser[256] = {0};
LPTSTR  _remoteuser[256] = {0};

// Find the HWND of the text input window
BOOL CALLBACK WindowEnumProc(HWND hWnd, LPARAM lParam)
{
	char className[256];
	GetClassName(hWnd, className,256);
	if((!_stricmp("TChatRichEdit", className)))
	{
		_txtinWND=hWnd;
	}
	return TRUE;
}




//add a function like   BOOL CALLBACK WindowEnumProc(HWND hWnd, LPARAM lParam)
BOOL CALLBACK WindowEnumProcUsername(HWND hWnd, LPARAM lParam)
{
	char className[256];
	GetClassName(hWnd, className,256);
	if((!_stricmp("TConversationForm", className)))
	{
		::GetWindowText(hWnd,(LPSTR)_remoteuser,sizeof(_remoteuser)); 
	}
	return TRUE;
}




void parse_user(const char *txt, int len)
{
	memset(_loginuser, 0, sizeof(_loginuser));
	for(int offset = 0; offset < len; offset++ )
	{
		if ( txt[offset] == '-' )
		{
			offset ++;
			offset ++; // remove the space, by fanghui
			if ( offset < len )
			{
				// copy
				memcpy(_loginuser, txt + offset, len - offset);
			}
			break;
		}
	}
}


HWND FindTargetTopWindow(char * targetname, DWORD *pid )
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_stricmp(entry.szExeFile, targetname) == 0)
			{  
				HWND hTopWnd = ::GetTopWindow(0 );
				while ( hTopWnd )
				{
					DWORD dwTheardId = ::GetWindowThreadProcessId(hTopWnd,pid);
					if ( *pid == entry.th32ProcessID )
					{
						char txt[256];
						int len=GetWindowText(hTopWnd,  txt, 256);

						//if(!_strnicmp(txt, targetname, strlen(targetname)-4))
						if(!_strnicmp(txt, targetname, 5))
						{
							// get login user name from txt, like "Skype? - username"
							parse_user((const char*) txt, len);

							*pid=dwTheardId;
							CloseHandle(snapshot);
							return hTopWnd;
						}
					}
					hTopWnd = ::GetNextWindow( hTopWnd , GW_HWNDNEXT);
				}
			}
		}
	}

	CloseHandle(snapshot);
	return 0;
}






SKYPE_HOOK_API bool ss_init()
{
	_hooked = false;
	//_txtinWND = NULL;

	return true;
}


SKYPE_HOOK_API void ss_close()
{
	ss_stop();
}

SKYPE_HOOK_API void ss_set_token( unsigned char * pwd, int len )
{
	set_password(pwd, len);
}

SKYPE_HOOK_API void ss_get_token(unsigned char * outkey)
{
	get_password(outkey);
}


SKYPE_HOOK_API void ss_get_local_user(char *name)
{
	strcpy(name, _loginuser);
}


SKYPE_HOOK_API void ss_get_remote_user(char *name)
{
	strcpy(name, (const char*)_remoteuser);
}



// install keyboard Hook.
SKYPE_HOOK_API bool ss_start() 
{
	ss_stop();

	DWORD pid;
	HWND hTopWnd = FindTargetTopWindow("Skype.exe", (DWORD *)&pid); 
	if(hTopWnd)
	{
		// find user name by enuming all child windows
		memset(_remoteuser,0,sizeof(_remoteuser));
		EnumChildWindows(hTopWnd,WindowEnumProcUsername,NULL);
		if (strlen((const char*)_remoteuser) >0)
		{
			//get Remote Username
		}


		// find text-in window by enuming all child windows
		_txtinWND=NULL;  // reset
		EnumChildWindows(hTopWnd,WindowEnumProc,NULL); 
		if (_txtinWND != NULL)
		{
			if(installhook(_txtinWND,pid))
			{
				_hooked = true;

				CWnd *pWnd=CWnd::FromHandle(_txtinWND); 
				if (pWnd->GetSafeHwnd()) 
				{ 
					try {
						pWnd->ShowWindow(SW_NORMAL); 
						pWnd->SetForegroundWindow(); 
					}
					catch( ... )
					{
						// added by fanghui
					}
					keybd_event(VK_RETURN, 0, 0, 0); 
					keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); 
				}

				return true;
			}
		}
	}

	return false;
}


// remove hook and cleanup
SKYPE_HOOK_API void ss_stop() 
{
	if(_hooked==true)
	{
		if(removehook())
		{
			_hooked = false;
			if (_txtinWND != NULL)
			{
				CWnd *pWnd=CWnd::FromHandle(_txtinWND); 
				if (pWnd->GetSafeHwnd()) 
				{ 
					try 
					{
						pWnd->ShowWindow(SW_NORMAL); 
						pWnd->SetForegroundWindow(); 
					}
					catch( ... ) // Win32Exception &e )
					{
						// fanghui: somehow when you start the hook, switch the conversation,
						// and then stop the hook (not for previous conversation),
						// you will get a win32 exception here!
					}
					keybd_event(VK_RETURN, 0, 0, 0); 
					keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); 
				}
			}
		}
	}
}