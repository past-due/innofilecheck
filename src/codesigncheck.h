#pragma once
#ifndef ___CODE_SIGN_CHECK__H___
#define ___CODE_SIGN_CHECK__H___

#include <Windows.h>

enum CODESIGNCHECK_STATUS_CODES {
	STATUS_OK = 0,
	ERROR_VERIFYTRUSTFAILURE,
	ERROR_NOTMICROSOFTROOT,
	ERROR_CERTDETAILFETCHFAILED,
	ERROR_CERTNAMENOTEQUAL,
	ERROR_CERTISSUERNAMENOTEQUAL,
	ERROR_LOADLIBRARYFAILURE,
	ERROR_GETPROCADDRESSFAILURE,
	ERROR_WTHELPERFAILED,
	ERROR_NONSPECIFIC
};

#ifdef __cplusplus 
extern "C" {
#endif

int __declspec(dllexport) __cdecl VerifyFileCodeSignature(const WCHAR * filePath,
	const WCHAR * certName,
	const WCHAR * certIssuerName,
	BOOL microsoftRootCheck);

#ifdef __cplusplus
}
#endif

#endif//!___CODE_SIGN_CHECK__H___
