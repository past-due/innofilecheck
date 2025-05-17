//
//	versioninfo.cpp
//
//	Obtain details from the file version info
//
//	The MIT License
//
//	Copyright (c) 2018-2025 pastdue  https://github.com/past-due/
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
//

#include "versioninfo.h"
#include <Strsafe.h>
#include "utils.h"

typedef DWORD (WINAPI *PGetFileVersionInfoSize)(
	LPCWSTR lpwstrFilename,
	LPDWORD lpdwHandle
);

typedef BOOL (WINAPI *PGetFileVersionInfo)(
	LPCWSTR lpwstrFilename,
	DWORD dwHandle,
	DWORD dwLen,
	LPVOID lpData
);

typedef BOOL (WINAPI *PVerQueryValue)(
	LPCVOID pBlock,
	LPCWSTR lpSubBlock,
	LPVOID * lplpBuffer,
	PUINT puLen
);

#define STRINGFILEINFO_PREFIX L"\\StringFileInfo\\"
#define STRINGFILEINFO_PREFIX_LEN 16

// Retrieves a string value from the file version info of lpwstrFilename.
// If successful, copies the result to out_str and returns the number of character copied
extern "C" int __cdecl GetFileVersionString(LPCWSTR lpwstrFilename, LPCWSTR stringName, WORD wLanguage /*= 1033*/, WORD wCodePage /*= 1252*/, LPWSTR out_str, UINT out_len)
{
	int retValue = -1;
	void *pVersionInfoData = NULL;
	LPWSTR wLanguageStr = NULL;
	LPWSTR wCodePageStr = NULL;

	if (lpwstrFilename == NULL) return NULL;
	if (stringName == NULL) return NULL;

	// Attempt to dynamically load the Version APIs
	HMODULE hVersionModule = SafeLoadSystemLibrary(L"version.dll");
	if (hVersionModule == NULL)
	{
		// Can't load version.dll - bail!
		return NULL;
	}
	PGetFileVersionInfoSize Func_GetFileVersionInfoSize = (PGetFileVersionInfoSize)GetProcAddress(hVersionModule, "GetFileVersionInfoSizeW");
	if (NULL == Func_GetFileVersionInfoSize) goto Cleanup;
	PGetFileVersionInfo Func_GetFileVersionInfo = (PGetFileVersionInfo)GetProcAddress(hVersionModule, "GetFileVersionInfoW");
	if (NULL == Func_GetFileVersionInfo) goto Cleanup;
	PVerQueryValue Func_VerQueryValue = (PVerQueryValue)GetProcAddress(hVersionModule, "VerQueryValueW");
	if (NULL == Func_VerQueryValue) goto Cleanup;

	DWORD lpThrowawayHandle = 0;
	DWORD versionInfoSize = Func_GetFileVersionInfoSize(lpwstrFilename, &lpThrowawayHandle);
	if (0 == versionInfoSize)
	{
		goto Cleanup;
	}
	pVersionInfoData = (void*)LocalAlloc(LPTR, versionInfoSize);
	if (NULL == pVersionInfoData)
	{
		// Memory allocation failure
		goto Cleanup;
	}

	if (Func_GetFileVersionInfo(lpwstrFilename, 0, versionInfoSize, pVersionInfoData) != 0)
	{
		// GetFileVersionInfo succeeded

		// The following is a verbose equivalent of:
		// StringCchPrintf(SubBlock, SubBlockLen, TEXT("\\StringFileInfo\\%04x%04x\\%s"), wLanguage, wCodePage, stringName)
		// because StringCchPrintf won't work without linking to the CRT

		wLanguageStr = WordToHexString(wLanguage);
		wCodePageStr = WordToHexString(wCodePage);
		const size_t SubBlockLen = MAX_PATH;
		TCHAR SubBlock[SubBlockLen];
		if (FAILED(StringCchCopyW(SubBlock, SubBlockLen, STRINGFILEINFO_PREFIX)))
		{
			retValue = -1;
			goto Cleanup;
		}
		if (FAILED(StringCchCatW(SubBlock, SubBlockLen, wLanguageStr)))
		{
			retValue = -1;
			goto Cleanup;
		}
		if (FAILED(StringCchCatW(SubBlock, SubBlockLen, wCodePageStr)))
		{
			retValue = -1;
			goto Cleanup;
		}
		if (FAILED(StringCchCatW(SubBlock, SubBlockLen, L"\\")))
		{
			retValue = -1;
			goto Cleanup;
		}
		if (FAILED(StringCchCatW(SubBlock, SubBlockLen, stringName)))
		{
			retValue = -1;
			goto Cleanup;
		}

		// Retrieve file description for language and code page. 
		LPCWSTR lpBuffer = NULL;
		UINT strLen = 0;
		if (Func_VerQueryValue(pVersionInfoData, SubBlock, (LPVOID *)&lpBuffer, &strLen) == 0)
		{
			retValue = -1;
			goto Cleanup;
		}

		// Copy the string
		if (out_str != NULL && out_len > 0)
		{
			HRESULT copyResult = StringCchCopyNW(out_str, out_len, lpBuffer, strLen);
			switch (copyResult)
			{
			case S_OK:
			case STRSAFE_E_INSUFFICIENT_BUFFER:
				retValue = (int)(strLen);
				break;
			case STRSAFE_E_INVALID_PARAMETER:
				retValue = -2;
				goto Cleanup;
				break;
			}
		}
		else
		{
			// Just return the length of the string (not including null terminator)
			retValue = (int)(strLen);
		}
	}

Cleanup:
	if (wCodePageStr) HeapFree(GetProcessHeap(), 0, wCodePageStr);
	if (wLanguageStr) HeapFree(GetProcessHeap(), 0, wLanguageStr);
	if (pVersionInfoData) LocalFree(pVersionInfoData);

	FreeLibrary(hVersionModule);

	return retValue;
}
