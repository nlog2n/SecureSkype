#include "stdafx.h"
#include <strsafe.h>
#include <psapi.h>


#define  MAX_PATH_LEN 1024
#define  BUFSIZE 1024

//===========================================
// Utility functions
//===========================================

BOOL GetFileNameFromMapView(WCHAR* filename, void* view) 
{
	if (GetMappedFileNameW(GetCurrentProcess(), view, filename, MAX_PATH_LEN)) {
		// Translate path with device name to drive letters.
		WCHAR szTemp[BUFSIZE];
		szTemp[0] = L'\0';

		if (GetLogicalDriveStringsW(BUFSIZE-1, szTemp)) {
			WCHAR szName[MAX_PATH_LEN];
			WCHAR szDrive[3] = L" :";
			BOOL bFound = FALSE;
			WCHAR* p = szTemp;

			do {
				// Copy the drive letter to the template string
				*szDrive = *p;

				// Look up each device name
				if (QueryDosDeviceW(szDrive, szName, MAX_PATH_LEN)) {
					size_t uNameLen = wcslen(szName);

					if (uNameLen < MAX_PATH_LEN) {
						bFound = wcsnicmp(filename, szName, uNameLen) == 0;
						if (bFound) {
							// Reconstruct filename using szTempFile
							// Replace device path with DOS path
							WCHAR szTempFile[MAX_PATH_LEN];
							StringCchPrintfW(szTempFile, MAX_PATH_LEN, L"%s%s", szDrive, filename+uNameLen);
							StringCchCopyNW(filename, MAX_PATH_LEN+1, szTempFile, wcslen(szTempFile));
						}
					}
				}

				// Go to the next NULL character.
				while (*p++);
			} while (!bFound && *p); // end of string
		}
		return TRUE;
	}
	return FALSE;
}



BOOL GetFileNameFromHandle(HANDLE hFile, WCHAR* filename) 
{ 
	BOOL b = FALSE;
	HANDLE hFileMap;

	if (filename != NULL) {
		// Create a file mapping object.
		hFileMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 1, NULL);
		if (hFileMap) {
			// Create a file mapping to get the file name.
			void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

			if (pMem) 
			{
				//b = GetMappedFileNameW(GetCurrentProcess(), pMem, filename, MAX_PATH_LEN);
				b = GetFileNameFromMapView(filename, pMem);
				UnmapViewOfFile(pMem);
			}
			CloseHandle(hFileMap);
		}
	}
	return b;
}

