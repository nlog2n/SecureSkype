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
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <winsock.h>
#include <richedit.h>
#include <psapi.h>
#include <shlobj.h>
#include <shobjidl.h>

#include "keydll.h"
#include "madCHook.h"


#pragma warning (disable: 4995)

#define  MAX_PATH_LEN 1024
#define  BUFSIZE 1024
#define KEY_SIZE 32

#pragma data_seg(".HOOKDATA")//Shared data among all instances.
HHOOK hookhandle = NULL;
HWND hwnd = NULL;
bool hookflag=false;
bool firstrun = true;
char RunDir[MAX_PATH_LEN]= {0};
static unsigned char  token[20] = "password12345678"; // moved inside data segment so you can modify. by fanghui
#pragma data_seg()

#pragma comment(linker, "/SECTION:.HOOKDATA,RWS")//linker directive

#define MAXFILENUM 100
#define MAXDIRNUM 20


typedef struct Struct_OpenFileList
{
	WCHAR OpenFileName[MAX_PATH_LEN];
	HANDLE OpenFileHandle;
} *pOpenFileList;

Struct_OpenFileList OpenFileList[MAXFILENUM];

WCHAR SaveFileList[MAXFILENUM][MAX_PATH_LEN]= {0};

WCHAR SaveDirList[MAXDIRNUM][MAX_PATH_LEN]= {0};

int OpenFileCount = 0;
int OpenFilePos = 0;
int SaveFileCount = 0;
int SaveDirCount = 0;
int SaveDirPos = 0;

bool injectflag=false;
HINSTANCE hinstance = NULL;

bool browsedir = false;

int WordCount=0;
BOOL ActionKey = FALSE;



void encipher(char *key, char *txt, int len);
void decipher(char *key, char *txt, int len);

int base64_encode( unsigned char *dst, size_t *dlen,const unsigned char *src, size_t slen );
int base64_decode( unsigned char *dst, size_t *dlen,const unsigned char *src, size_t slen );

BOOL GetFileNameFromMapView(WCHAR* filename, void* view);
BOOL GetFileNameFromHandle(HANDLE hFile, WCHAR* filename);

void EncryptStream(const unsigned char key[], int keylen,unsigned char* p, int len, unsigned int offset, long blocksize);
size_t EncFile(WCHAR* srcfile, WCHAR* dstfile, const unsigned char *Kc, size_t block_size);


// revised by fanghui
void getEncKey(unsigned char * outkey)
{
	memcpy(outkey, token, sizeof(token));
}

// added by fanghui
KEYDLL3_API void get_password(unsigned char * outkey)
{
	memcpy(outkey, token, sizeof(token));
}

// added by fanghui
KEYDLL3_API void set_password( unsigned char * pwd, int len )
{
	memset(token, 0, sizeof(token));
	int L = min( len, sizeof(token));
	memcpy(token, pwd, L);
}



int (WINAPI *MultiByteToWideCharNextHook)(
	UINT CodePage, 
	DWORD dwFlags,         
	LPCSTR lpMultiByteStr, 
	int cbMultiByte,       
	LPWSTR lpWideCharStr,  
	int cchWideChar        
	)=NULL;

int (WINAPI *WideCharToMultiByteNextHook)(
	UINT CodePage, 
	DWORD dwFlags, 
	LPCWSTR lpWideCharStr,
	int cchWideChar, 
	LPSTR lpMultiByteStr, 
	int cbMultiByte,
	LPCSTR lpDefaultChar,    
	LPBOOL lpUsedDefaultChar
	)=NULL;


int WINAPI MultiByteToWideCharHookProc(
									   UINT CodePage, 
									   DWORD dwFlags,         
									   LPSTR lpMultiByteStr, 
									   int cbMultiByte,       
									   LPWSTR lpWideCharStr,  
									   int cchWideChar        
									   )

{
	int result;
	if (hookflag && CodePage == CP_UTF8)
	{

		if(cbMultiByte>3 && lpMultiByteStr[cbMultiByte-3]==0x7F && lpMultiByteStr[cbMultiByte-2]==0x7F && lpMultiByteStr[cbMultiByte-1]==0x7F)  
		{

			int  tmpstrlen=cbMultiByte;
			char* tmpstr= new  char[tmpstrlen];

			base64_decode((unsigned char *)tmpstr,(size_t *)&tmpstrlen,(const unsigned char *)lpMultiByteStr,cbMultiByte-3);

			tmpstr[tmpstrlen]=0x0;
			tmpstr[tmpstrlen+1]=0x0;

			int cchlen=wcslen((const wchar_t *)tmpstr);

			if(cchWideChar ==0 )
			{
				result= cchlen+1;
			}
			else
				if(cchWideChar >= cchlen+1)
				{
					unsigned char key[20];
					getEncKey(key);

					memcpy((void *)lpWideCharStr,(void *)tmpstr,tmpstrlen+2);
					decipher((char *)key, (char *)lpWideCharStr, 2*cchlen);
					result= cchlen+1;
				}
				else 
				{
					//MessageBox(NULL, "Insufficient space for Base64 Decoder transform !", "Error", MB_OK);
					result=0;					
				}

				delete[] tmpstr;
				return result;

		}
	}

	return MultiByteToWideCharNextHook(CodePage,dwFlags,lpMultiByteStr,cbMultiByte, lpWideCharStr,cchWideChar );
}

int WINAPI WideCharToMultiByteHookProc(
									   UINT CodePage, 
									   DWORD dwFlags, 
									   LPWSTR lpWideCharStr,
									   int cchWideChar, 
									   LPSTR lpMultiByteStr, 
									   int cbMultiByte,
									   LPCSTR lpDefaultChar,    
									   LPBOOL lpUsedDefaultChar
									   )

{
	if (hookflag && CodePage == CP_UTF8)
	{

		if(cchWideChar>3 && lpWideCharStr[cchWideChar-3]==0x0001 && lpWideCharStr[cchWideChar-2]==0x0002 && lpWideCharStr[cchWideChar-1]==0x0003)  
		{
			if(cbMultiByte ==0 ) return cchWideChar*3+1+4;  //query on length

			int result;
			unsigned char key[20];
			getEncKey(key);

			encipher((char *)key, (char *)lpWideCharStr, 2*(cchWideChar-3));

			int  tmpstrlen = cchWideChar*3+1;
			char* tmpstr= new  char[tmpstrlen];
			memset(tmpstr,0,tmpstrlen);

			base64_encode((unsigned char *) tmpstr,(size_t *)&tmpstrlen,(const unsigned char *)lpWideCharStr,(cchWideChar-3)*2);

			//Base64_encode length of output:   tmpstrlen= (cchWideChar*2/3+1)*4 

			if(cbMultiByte >= tmpstrlen+4 )  //replace WideCharToMultiByte() with Base64Encoder()			  
			{
				memcpy(lpMultiByteStr,tmpstr,tmpstrlen);
				lpMultiByteStr[tmpstrlen]=0x7F;
				lpMultiByteStr[tmpstrlen+1]=0x7F;
				lpMultiByteStr[tmpstrlen+2]=0x7F;
				lpMultiByteStr[tmpstrlen+3]=0x0;
				result=tmpstrlen+4;
			}
			else
			{
				//
				//MessageBox(NULL, "Insufficient space for Base64 Encoder transform !", "Error", MB_OK);
				result=0;
			}
			delete[] tmpstr;
			return result;
		}
	}

	return WideCharToMultiByteNextHook(CodePage,dwFlags, lpWideCharStr,cchWideChar,lpMultiByteStr,cbMultiByte, lpDefaultChar,lpUsedDefaultChar);

}

void AddDirList(WCHAR * dirname)
{
	int i=0;

	int len=wcslen(dirname);

	if(dirname[len-1] ==  L'\\')
	{
		dirname[len-1] =  L'\0';

	}

	for (i=0;i<SaveDirCount;i++)
	{
		if(wcsicmp(SaveDirList[i],dirname) == 0)
		{
			break;
		}
	}

	if (i==SaveDirCount)
	{

		if (SaveDirCount == MAXDIRNUM)
		{
			wcscpy(SaveDirList[SaveDirPos],dirname);
			SaveDirPos++;
			if(SaveDirPos == MAXDIRNUM)
			{
				SaveDirPos = 0;
			}
		}
		else
		{
			wcscpy(SaveDirList[SaveDirCount],dirname);
			SaveDirCount++;
		}

	}

}

void AddOpenFileList(WCHAR * wfilename)
{
	int i=0;

	for (i=0;i<OpenFileCount;i++)
	{
		if(wcsicmp(OpenFileList[i].OpenFileName,wfilename) == 0)
		{
			break;
		}
	}

	if (i==OpenFileCount)
	{

		if (OpenFileCount == MAXFILENUM)
		{
			while(OpenFileList[OpenFilePos].OpenFileHandle != NULL)
			{
				OpenFilePos++;
				if(OpenFilePos == MAXFILENUM)
				{
					OpenFilePos = 0;
					break;
				}
			}

			wcscpy(OpenFileList[OpenFilePos].OpenFileName,wfilename);
			OpenFileList[OpenFilePos].OpenFileHandle = NULL;

			OpenFilePos++;
			if(OpenFilePos == MAXFILENUM)
			{
				OpenFilePos = 0;
			}
		}
		else
		{
			wcscpy(OpenFileList[OpenFileCount].OpenFileName,wfilename);
			OpenFileList[OpenFileCount].OpenFileHandle = NULL;
			OpenFileCount++;
		}

	}

}


BOOL CheckOpenFileList(WCHAR * chkfile,HANDLE hObject)
{
	BOOL result=FALSE;
	int i=0;


	for (i=0;i<OpenFileCount;i++)
	{
		if(wcsicmp(OpenFileList[i].OpenFileName,chkfile) == 0)
		{

			OpenFileList[i].OpenFileHandle = hObject;
			result=TRUE;
			break;
		}
	}

	return result;
}


BOOL CheckFileList(WCHAR * chkfile,BOOL isOpen)
{
	BOOL result=FALSE;
	int i=0;

	if(isOpen)
	{

		for (i=0;i<OpenFileCount;i++)
		{
			if(wcsicmp(OpenFileList[i].OpenFileName,chkfile) == 0)
			{
				result=TRUE;
				break;
			}
		}
	}
	else
	{
		for (i=0;i<SaveFileCount;i++)
		{
			if(wcsicmp(SaveFileList[i],chkfile) == 0)
			{
				result=TRUE;
				break;
			}
		}

		if (!result)
		{

			WCHAR tmpPath[MAX_PATH_LEN];

			wcscpy(tmpPath,chkfile);

			WCHAR *	presult = wcsrchr( chkfile, L'\\' );
			if ( presult != NULL )
			{
				int	len = presult - chkfile;
				tmpPath[len] = L'\0';
			}

			for (i=0;i<SaveDirCount;i++)
			{
				if(wcsnicmp(SaveDirList[i],tmpPath,wcslen(tmpPath)) == 0)
				{
					result=TRUE;
					break;
				}
			}


		}

	}
	return result;
}


BOOL RemoveFileList(WCHAR * chkfile,BOOL isOpen)
{
	BOOL result=FALSE;
	WCHAR tmpPath[MAX_PATH_LEN];
	int i=0;

	wcscpy(tmpPath,chkfile);

	WCHAR *	presult = wcsrchr( chkfile, L'\\' );
	if ( presult != NULL )
	{
		int	len = presult - chkfile;
		tmpPath[len] = L'\0';
	}

	if(isOpen)
	{

		for (i=0;i<OpenFileCount;i++)
		{
			if(wcsnicmp(OpenFileList[i].OpenFileName,chkfile,wcslen(chkfile)) == 0)
			{
				result=TRUE;
				break;
			}
		}
	}
	else
	{
		for (i=0;i<SaveFileCount;i++)
		{
			if(wcsnicmp(SaveFileList[i],chkfile,wcslen(chkfile)) == 0)
			{
				result=TRUE;
				break;
			}
		}
	}

	if(result)
	{
		if(isOpen)
		{
			for (;i<OpenFileCount-1;i++)
			{
				wcscpy(OpenFileList[i].OpenFileName,OpenFileList[i++].OpenFileName);
				OpenFileList[i].OpenFileHandle = OpenFileList[i+1].OpenFileHandle; 
			}
			OpenFileCount--;
			if (OpenFileCount< 0) OpenFileCount=0;
		}
		else
		{
			for (;i<SaveFileCount-1;i++)
			{
				wcscpy(SaveFileList[i],SaveFileList[i++]);

			}
			SaveFileCount--;

			if (SaveFileCount< 0) SaveFileCount=0;

		}

	}


	return result;
}

BOOL RemoveFileListByHandle(HANDLE hObject)
{
	BOOL result=FALSE;
	int i=0;

	if(hObject != NULL)
	{
		for (i=0;i<OpenFileCount;i++)
		{
			if(OpenFileList[i].OpenFileHandle == hObject)
			{
				OpenFileList[i].OpenFileHandle = NULL;
				result=TRUE;
				break;
			}
		}
	}

	return result;
}

BOOL (WINAPI* GetOpenFileNameWNextHook) (LPOPENFILENAMEW lpofn)=NULL;
BOOL (WINAPI* GetSaveFileNameWNextHook) (LPOPENFILENAMEW lpofn)=NULL;

BOOL WINAPI GetOpenFileNameWHookProc(LPOPENFILENAMEW lpofn)
{

	BOOL result=GetOpenFileNameWNextHook(lpofn);
	BOOL multiflag=false;
	if (result)
	{
		WCHAR* p = lpofn->lpstrFile;
		/*
		if(lpofn->Flags & OFN_ALLOWMULTISELECT)
		{
		multiflag=true;
		}
		*/

		WCHAR dir_buf[MAX_PATH_LEN];
		if(p && p[0])
		{
			wcscpy(dir_buf,p);
			p = p + wcslen(dir_buf)+1;
			if(!p[0])
			{
				AddOpenFileList(dir_buf);

			}
			else
			{
				wcscat(dir_buf,L"\\");
			}

		}
		int i=1;
		WCHAR tmpname[MAX_PATH_LEN];
		WCHAR fullfilename[MAX_PATH_LEN];

		while(p && p[0])
		{
			wcscpy(tmpname, p);
			wcscpy(fullfilename,dir_buf);
			wcscat(fullfilename,tmpname);
			p = p + wcslen(tmpname)+1;


			AddOpenFileList(fullfilename);

		}



	}
	return result;
}

BOOL WINAPI GetSaveFileNameWHookProc(LPOPENFILENAMEW lpofn)
{

	BOOL result=GetSaveFileNameWNextHook(lpofn);
	if (result)
	{
		wcscpy(SaveFileList[SaveFileCount],lpofn->lpstrFile);
		SaveFileCount++;

	}
	return result;
}

BOOL (WINAPI* ReadFileNextHook)(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)=NULL;



BOOL WINAPI ReadFileHookProc(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	BOOL ret;
	DWORD offset = 0;
	WCHAR wfilename[MAX_PATH_LEN];

	// number of bytes to encrypt
	DWORD enc_len = 0;
	DWORD file_size = 0;


	ret = ReadFileNextHook(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

	if (! ret) {
		// read error or asynchronous I/O 
		if (GetLastError() != ERROR_IO_PENDING) {

		}
		return ret;  
	}

	memset(wfilename, 0, MAX_PATH_LEN*sizeof(WCHAR));
	if (GetFileNameFromHandle(hFile, wfilename)) {


		if(CheckOpenFileList(wfilename,hFile))
		{

			offset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
			if (offset == INVALID_SET_FILE_POINTER) {
				offset = 0;

			}

			enc_len = *lpNumberOfBytesRead;
			offset -= *lpNumberOfBytesRead;
			if (offset == 0 && enc_len>0)
			{

				unsigned char* tmp_buf = new unsigned char[nNumberOfBytesToRead+3];

				if (tmp_buf == NULL) { 
					return FALSE; 
				}

				memcpy(tmp_buf+3, (unsigned char*)lpBuffer, enc_len);
				tmp_buf[0]='\0';
				tmp_buf[1]='\1';
				tmp_buf[2]='\2';

				size_t block_size = 1024;

				unsigned char key[20];
				getEncKey(key);

				EncryptStream((const unsigned char *)&key[0], KEY_SIZE, 
					static_cast<unsigned char*>(tmp_buf+3),
					static_cast<unsigned int>(enc_len),
					static_cast<unsigned int>(offset), 
					static_cast<long>(block_size));
				memcpy(lpBuffer,tmp_buf,nNumberOfBytesToRead);

				if(enc_len <= nNumberOfBytesToRead-3)
				{
					*lpNumberOfBytesRead=enc_len+3;
				}
				else
				{
					*lpNumberOfBytesRead=nNumberOfBytesToRead;
				}

				delete [] tmp_buf;


				if(enc_len > nNumberOfBytesToRead-3)
				{
					SetFilePointer(hFile, enc_len-3, NULL, FILE_BEGIN);
				}

			}
			else
			{

				size_t block_size = 1024;

				unsigned char key[20];
				getEncKey(key);

				EncryptStream((const unsigned char *)&key[0], KEY_SIZE, 
					static_cast<unsigned char*>(lpBuffer),
					static_cast<unsigned int>(enc_len),
					static_cast<unsigned int>(offset), 
					static_cast<long>(block_size));
			}

		}

	}

	return ret;

}


BOOL  (WINAPI* GetFileInformationByHandleNextHook)(HANDLE hFile,LPBY_HANDLE_FILE_INFORMATION lpFileInformation)=NULL;

BOOL  WINAPI GetFileInformationByHandleHookProc(HANDLE hFile,LPBY_HANDLE_FILE_INFORMATION lpFileInformation)
{
	WCHAR wfilename[MAX_PATH_LEN];
	BOOL ret = GetFileInformationByHandleNextHook(hFile, lpFileInformation);
	if( ret) 
	{
		memset(wfilename, 0, MAX_PATH_LEN*sizeof(WCHAR));
		if (GetFileNameFromHandle(hFile, wfilename)) {
			if(CheckOpenFileList(wfilename,hFile))
			{
				lpFileInformation->nFileSizeLow = lpFileInformation->nFileSizeLow+3;
			}
		}
	}
	return ret;
}


BOOL (WINAPI* CloseHandleNextHook)(HANDLE hObject)=NULL;
BOOL WINAPI CloseHandleHookProc(HANDLE hObject)
{
	BOOL result=CloseHandleNextHook(hObject);
	RemoveFileListByHandle(hObject);

	return result;

}

BOOL (WINAPI* MoveFileWNextHook)(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName)=NULL;
BOOL WINAPI MoveFileWHookProc(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName)
{
	if(CheckFileList((WCHAR *)lpNewFileName,FALSE))
	{
		if(wcslen(lpExistingFileName)>0)
		{
			size_t block_size = 1024;

			unsigned char key[20];
			getEncKey(key);

			DWORD filesize=(DWORD)EncFile((WCHAR *)lpExistingFileName,(WCHAR *)lpNewFileName,(const unsigned char *)key,block_size);
			DeleteFileW(lpExistingFileName);
			RemoveFileList((WCHAR *)lpNewFileName,FALSE);

			return TRUE;
		}
	}

	return MoveFileWNextHook(lpExistingFileName,lpNewFileName);

}

PVOID GetInterfaceMethod(PVOID intf, DWORD methodIndex)
{
	return *(PVOID*)(*(DWORD*)intf + methodIndex * 4);
}

HRESULT (WINAPI* FileOpenDialogGetResultsNextHook)(IFileOpenDialog * This,IShellItemArray **ppenum)=NULL;


HRESULT WINAPI FileOpenDialogGetResultsHookProc(IFileOpenDialog * This,IShellItemArray **ppenum)
{

	HRESULT hr = FileOpenDialogGetResultsNextHook(This,ppenum);


	if (SUCCEEDED(hr))
	{
		PWSTR pszFilePath = NULL;
		int i=0;

		DWORD count =0;
		hr = (*ppenum)->GetCount(&count);

		if (SUCCEEDED(hr))
		{

			IShellItem *psiResult;

			while(i<count)
			{

				(*ppenum)->GetItemAt(i,&psiResult);
				if (SUCCEEDED(hr))
				{

					hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						AddOpenFileList(pszFilePath);

						CoTaskMemFree(pszFilePath);
					}
				}
				i++;
			}
		}
	}

	return hr;
}

HRESULT (WINAPI* FileSaveDialogGetResultsNextHook)(IFileSaveDialog * This,IShellItem **ppsi)=NULL;


HRESULT WINAPI FileSaveDialogGetResultsHookProc(IFileSaveDialog * This,IShellItem **ppsi)
{

	HRESULT hr = FileSaveDialogGetResultsNextHook(This,ppsi);


	if (SUCCEEDED(hr))
	{
		PWSTR pszFilePath;
		hr = (*ppsi)->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

		// Display the file name to the user.
		if (SUCCEEDED(hr))
		{
			wcscpy(SaveFileList[SaveFileCount],pszFilePath);
			SaveFileCount++;
			CoTaskMemFree(pszFilePath);
		}

	}


	return hr;
}



HRESULT (WINAPI* CoCreateInstanceNextHook) (REFCLSID  rclsid,
											LPUNKNOWN pUnkOuter,
											DWORD dwClsContext,
											REFIID riid,
											LPVOID *ppv)=NULL;


HRESULT WINAPI CoCreateInstanceHookProc(REFCLSID  rclsid,
										LPUNKNOWN pUnkOuter,
										DWORD dwClsContext,
										REFIID riid,
										LPVOID *ppv)
{
	HRESULT ret;

	ret = CoCreateInstanceNextHook(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (ret == S_OK && rclsid == CLSID_FileOpenDialog) {

		if ( !FileOpenDialogGetResultsNextHook != NULL)
		{
			HookCode(GetInterfaceMethod(*ppv, 27), FileOpenDialogGetResultsHookProc, (PVOID*) &FileOpenDialogGetResultsNextHook);
		}
		else
		{
			RenewHook((PVOID*) &FileOpenDialogGetResultsNextHook);
		}
	}
	if (ret == S_OK && rclsid == CLSID_FileSaveDialog) {

		if ( !FileSaveDialogGetResultsNextHook != NULL)
		{
			HookCode(GetInterfaceMethod(*ppv, 20), FileSaveDialogGetResultsHookProc, (PVOID*) &FileSaveDialogGetResultsNextHook);
		}
		else
		{
			RenewHook((PVOID*) &FileSaveDialogGetResultsNextHook);
		}
	}

	return (ret);
}

PIDLIST_ABSOLUTE (WINAPI* SHBrowseForFolderWNextHook)(LPBROWSEINFO lpbi)=NULL;
PIDLIST_ABSOLUTE WINAPI  SHBrowseForFolderWHookProc(LPBROWSEINFO lpbi)
{
	PIDLIST_ABSOLUTE  pidl= SHBrowseForFolderWNextHook(lpbi);
	if(pidl != NULL)
	{
		browsedir =true;
	}
	return pidl;

}



BOOL (WINAPI*  SHGetPathFromIDListWNextHook)(PCIDLIST_ABSOLUTE pidl,LPWSTR pszPath)=NULL;

BOOL WINAPI  SHGetPathFromIDListWHookProc(PCIDLIST_ABSOLUTE pidl,LPWSTR pszPath)
{

	BOOL ret = SHGetPathFromIDListWNextHook(pidl,pszPath);

	if(browsedir && ret)
	{
		browsedir = false;
		AddDirList(pszPath);
	}



	return ret;

}


void MyStartHook()
{
	int i=0;

	for (i=0;i<MAXFILENUM;i++)
	{
		memset(OpenFileList[i].OpenFileName,0,sizeof(WCHAR)*MAX_PATH_LEN);
		OpenFileList[i].OpenFileHandle = NULL;
		memset(SaveFileList[i],0,sizeof(WCHAR)*MAX_PATH_LEN);
	}

	for (i=0;i<MAXDIRNUM;i++)
	{
		memset(SaveDirList[i],0,sizeof(WCHAR)*MAX_PATH_LEN);
	}

	InitializeMadCHook();
	CollectHooks();

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	BOOL succ = GetVersionEx (&osvi);
	BOOL bIsVistaOrLater = ( osvi.dwMajorVersion > 5);

	HookAPI("kernel32.dll", "MultiByteToWideChar", MultiByteToWideCharHookProc, ( void** )&MultiByteToWideCharNextHook);
	HookAPI("kernel32.dll", "WideCharToMultiByte", WideCharToMultiByteHookProc, ( void** )&WideCharToMultiByteNextHook);

	if(bIsVistaOrLater)
	{
		HookAPI("Ole32.dll", "CoCreateInstance",CoCreateInstanceHookProc, ( void** )&CoCreateInstanceNextHook);
	}
	else
	{
		HookAPI("Comdlg32.dll", "GetOpenFileNameW", GetOpenFileNameWHookProc, ( void** )&GetOpenFileNameWNextHook);
		HookAPI("Comdlg32.dll", "GetSaveFileNameW", GetSaveFileNameWHookProc, ( void** )&GetSaveFileNameWNextHook);
	}

	HookAPI("kernel32.dll", "ReadFile", ReadFileHookProc, ( void** )&ReadFileNextHook);
	HookAPI("kernel32.dll", "MoveFileW", MoveFileWHookProc, ( void** )&MoveFileWNextHook);
	HookAPI("kernel32.dll", "GetFileInformationByHandle", GetFileInformationByHandleHookProc, ( void** )&GetFileInformationByHandleNextHook);

	HookAPI("kernel32.dll", "CloseHandle", CloseHandleHookProc, ( void** )&CloseHandleNextHook);

	HookAPI("Shell32.dll", "SHBrowseForFolderW", SHBrowseForFolderWHookProc, ( void** )&SHBrowseForFolderWNextHook);
	HookAPI("Shell32.dll", "SHGetPathFromIDListW", SHGetPathFromIDListWHookProc, ( void** )&SHGetPathFromIDListWNextHook);

	FlushHooks();
	injectflag=true;

}

void MyStopHook()
{

	injectflag = false;

	if(MultiByteToWideCharNextHook != NULL)
		UnhookCode( ( void** ) &MultiByteToWideCharNextHook );

	if(WideCharToMultiByteNextHook != NULL)
		UnhookCode( ( void** ) &WideCharToMultiByteNextHook );

	if(GetOpenFileNameWNextHook != NULL)
		UnhookCode( ( void** ) &GetOpenFileNameWNextHook );

	if(GetSaveFileNameWNextHook != NULL)
		UnhookCode( ( void** ) &GetSaveFileNameWNextHook );


	if(ReadFileNextHook != NULL)
		UnhookCode( ( void** ) &ReadFileNextHook );

	if(MoveFileWNextHook != NULL)
		UnhookCode( ( void** ) &MoveFileWNextHook );

	if(GetFileInformationByHandleNextHook != NULL)
		UnhookCode( ( void** ) &GetFileInformationByHandleNextHook );

	if(CoCreateInstanceNextHook != NULL)
		UnhookCode( ( void** ) &CoCreateInstanceNextHook );

	if ( FileOpenDialogGetResultsNextHook != NULL)
		UnhookCode( ( void** ) &FileOpenDialogGetResultsNextHook );

	if ( FileSaveDialogGetResultsNextHook != NULL)
		UnhookCode( ( void** ) &FileSaveDialogGetResultsNextHook );

	if ( SHBrowseForFolderWNextHook != NULL)
		UnhookCode( ( void** ) &SHBrowseForFolderWNextHook );

	if ( SHGetPathFromIDListWNextHook != NULL)
		UnhookCode( ( void** ) &SHGetPathFromIDListWNextHook );

	if ( CloseHandleNextHook != NULL)
		UnhookCode( ( void** ) &CloseHandleNextHook );

	FinalizeMadCHook();

}


BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	char lpFileName[1024];
	memset(lpFileName,0,1024);
	char * copy1=NULL;

	int len;
	char DLLFileName[MAX_PATH+1]= {0};
	char *presult;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		if(firstrun)
		{
			firstrun = false;
			GetModuleFileNameA((HMODULE)hModule, DLLFileName, MAX_PATH);
			presult = strrchr( DLLFileName, '\\' );
			if ( presult != NULL )
			{
				len = presult - DLLFileName;
				strncpy(RunDir,DLLFileName, len);
				RunDir[len] = '\0';
			}

			hinstance = (HINSTANCE)hModule;
			hookhandle = NULL;

		}			



		GetModuleFileNameA(NULL,lpFileName,1024);
		copy1 = _strdup(lpFileName);

		_strlwr(copy1);


		if (strstr(copy1, "skype.exe") != NULL)
		{
			MyStartHook();
		}

		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:

		if(injectflag)
		{
			GetModuleFileNameA(NULL,lpFileName,1024);
			copy1 = _strdup( lpFileName); 

			_strlwr( copy1 );

			if (strstr(copy1, "skype.exe") != NULL)
			{
				MyStopHook();
			}
		}
		break;
	}


	return TRUE;
}
void ErrorExit(LPTSTR lpszFunction) 
{ 
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
	sprintf((char *)lpDisplayBuf,
		"%s failed with error %d: %s", 
		lpszFunction, dw, lpMsgBuf); 
	MessageBoxA(NULL, (LPCSTR)lpDisplayBuf, "Error", MB_OK); 

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}




KEYDLL3_API bool installhook(HWND h, DWORD pid)
{
	if (!hookflag)
	{
		hwnd = h;

		//hookhandle = SetWindowsHookEx(WH_KEYBOARD,hookproc,hinstance,pid);//NULL);
		hookhandle = SetWindowsHookEx(WH_CALLWNDPROC,hookmsgproc,hinstance,pid);
		if(hookhandle==NULL)
			MessageBoxA(NULL,"Unable to install hook","Error!",MB_OK);
		else
		{
			// commented by fanghui
			//char  welcomeMsg[100]="Enter secure communication mode !";
			//int len=strlen(welcomeMsg);
			//SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)(LPCWSTR)welcomeMsg); // WM_SETTEXT	

			hookflag = true;
		}
	}
	return hookflag;
}

KEYDLL3_API bool removehook()
{
	if (hookflag)
	{
		if(UnhookWindowsHookEx(hookhandle))
		{
			// commented by fanghui
			//char  welcomeMsg[100]="Exit secure communication mode !";
			//int len=strlen(welcomeMsg);
			//SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)(LPCWSTR)welcomeMsg); // WM_SETTEXT	

			hookflag = false;
		}
		else
		{
			ErrorExit((LPTSTR)"Unhook");
		}
	}
	return !hookflag;

}

KEYDLL3_API LRESULT CALLBACK hookproc(int ncode,WPARAM wparam,LPARAM lparam)
{
	WCHAR txt[1024]={0};
	BOOL funcKey = FALSE; 
	BOOL helpkey=FALSE;



	if (ncode == HC_ACTION )
	{
		if((lparam & 0x80000000) == 0x00000000)//Check whether key was pressed(not released).  
		{
			if(wparam==VK_RETURN) 
			{

				if(GetAsyncKeyState(VK_LSHIFT)& 0x8000 || GetAsyncKeyState(VK_RSHIFT)& 0x8000 ||
					GetAsyncKeyState(VK_LCONTROL)& 0x8000||GetAsyncKeyState(VK_RCONTROL)& 0x8000 ||
					GetAsyncKeyState(VK_LMENU)& 0x8000||GetAsyncKeyState(VK_RMENU)& 0x8000 ||
					GetAsyncKeyState(VK_BACK)& 0x8000||GetAsyncKeyState(VK_TAB)& 0x8000)  
				{  
					funcKey = TRUE;  
				}  
				else  
				{  
					funcKey = FALSE;  
				}  
				if(!funcKey) 
				{

					SendMessageW(hwnd,WM_GETTEXT,512,(LPARAM)(LPCWSTR)txt); // WM_SETTEXT
					int len=wcslen(txt);

					if(len>0 && txt[0] == '/')
					{
						helpkey=TRUE;

						for(int i=1; i<len; i++) 
						{	

							if(txt[i] == ' ')
							{
								helpkey=TRUE;
								break;
							}
							if(txt[i] == '/')
							{
								helpkey=FALSE;
								break;
							}

						}
					}
					CANDIDATEFORM imelist;
					if(len>0 && !helpkey && !ImmGetCandidateWindow(ImmGetContext(hwnd),0,&imelist))
					{

						WCHAR  addstr[]={0x0001,0x0002,0x0003,0x0000};

						int ndx = SendMessageW ( hwnd, WM_GETTEXTLENGTH, 0, 0 ); 
						SendMessageW (hwnd, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
						SendMessageW (hwnd, EM_REPLACESEL, 0, (LPARAM) ((LPWSTR) addstr));

					}

				}
			}
		}
	}
	return ( CallNextHookEx(hookhandle,ncode,wparam,lparam) );//pass control to next hook in the hook chain.
}

LRESULT CALLBACK hookmsgproc(int nCode,WPARAM wParam, LPARAM lParam)
{
	WCHAR txt[1024]={0};
	BOOL funcKey = FALSE; 
	BOOL helpkey=FALSE;


	if (nCode == HC_ACTION)
	{
		CWPSTRUCT *msg = (CWPSTRUCT *)lParam;

		if(msg->hwnd == hwnd)
		{

			if(msg->message == 0xBD00  || msg->message == 0xBD01  || msg->message == 0xBD02) 
			{
				ActionKey =FALSE;

			}
			else
				if(msg->message == EM_STREAMOUT) 
				{

					if( (GetAsyncKeyState(VK_RETURN) & 0x8000)  && 
						(GetAsyncKeyState(VK_LSHIFT)& 0x8000 || GetAsyncKeyState(VK_RSHIFT)& 0x8000 ||
						GetAsyncKeyState(VK_LCONTROL)& 0x8000||GetAsyncKeyState(VK_RCONTROL)& 0x8000 ||
						GetAsyncKeyState(VK_LMENU)& 0x8000||GetAsyncKeyState(VK_RMENU)& 0x8000 ) )  
					{  
						funcKey = TRUE;  
					}  
					else  
					{  
						funcKey = FALSE;  
					} 

					if (!funcKey)
					{

						SendMessageW(hwnd,WM_GETTEXT,1000,(LPARAM)(LPCWSTR)txt); // WM_SETTEXT
						int len=wcslen(txt);

						if(len>0 && txt[0] == '/')
						{
							helpkey=TRUE;

							for(int i=1; i<len; i++) 
							{	

								if(txt[i] == ' ')
								{
									helpkey=TRUE;
									break;
								}
								if(txt[i] == '/')
								{
									helpkey=FALSE;
									break;
								}

							}
						}


						if(len>0 && WordCount==len &&ActionKey && !helpkey && !(txt[len-3]==0x0001 && txt[len-2]==0x0002 && txt[len-1]==0x0003) )
						{

							WCHAR  addstr[]={0x0001,0x0002,0x0003,0x0000};

							ActionKey = FALSE;

							int ndx = SendMessageW ( hwnd, WM_GETTEXTLENGTH, 0, 0 ); 
							SendMessageW (hwnd, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
							SendMessageW (hwnd, EM_REPLACESEL, 0, (LPARAM) ((LPWSTR) addstr));

						}

						if (WordCount != len)
						{
							WordCount=len;
						}


						if(len>0 && !ActionKey)
						{
							ActionKey = TRUE;
						}

					}
				}
		}
	}

	return CallNextHookEx(hookhandle,nCode,wParam,lParam);
}


