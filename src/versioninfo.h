#pragma once
#ifndef ___VERSION_INFO__H___
#define ___VERSION_INFO__H___

#include <Windows.h>

#ifdef __cplusplus 
extern "C" {
#endif

// Retrieves a string value from the file version info of lptstrFilename.
// If successful, returns the number of characters that would have been written to out_str if out_len had been sufficiently large, not counting the terminating null character.
// If an error occurs, a negative error code is returned.
// NOTE: Only when the return value is non-negative and less than out_len has the string been completely written.
int __declspec(dllexport) __cdecl GetFileVersionString(LPCWSTR lptstrFilename, LPCWSTR stringName, WORD wLanguage /*= 1033*/, WORD wCodePage /*= 1252*/, LPWSTR out_str, UINT out_len);

#ifdef __cplusplus
}
#endif

#endif//!___VERSION_INFO__H___
