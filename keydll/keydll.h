
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KEYDLL3_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KEYDLL3_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//----------------------------------------------------------------------------------
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

// include this macro to avoid re-def, added by fanghui
#ifndef __KEY_DLL_H__
#define __KEY_DLL_H__


#ifdef KEYDLL3_EXPORTS
#define KEYDLL3_API __declspec(dllexport)
#else
#define KEYDLL3_API __declspec(dllimport)
#endif

KEYDLL3_API bool installhook(HWND h, DWORD pid);//This function installs the Keyboard hook.
KEYDLL3_API bool removehook();//This function removes the previously installed hook.
KEYDLL3_API LRESULT CALLBACK hookproc(int ncode,WPARAM wparam,LPARAM lparam);//This function is called when the keyboard is operated.
KEYDLL3_API LRESULT CALLBACK hookmsgproc(int ncode,WPARAM wparam,LPARAM lparam);//This function is called when the keyboard is operated.

KEYDLL3_API void get_password(unsigned char * outkey); // added by fanghui
KEYDLL3_API void set_password(unsigned char * pwd, int len ); // set password for this hook, added by fanghui

#endif