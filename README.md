# InnoFileCheck [![Build status](https://img.shields.io/github/actions/workflow/status/past-due/innofilecheck/CI.yml?branch=main&logo=GitHub&label=Build)](https://github.com/past-due/innofilecheck/actions/workflows/CI.yml?query=workflow%3ACI+branch%3Amain) [![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT) [![Inno Setup: 6.0+](https://img.shields.io/badge/Inno%20Setup-6.0%2B-orange.svg)](https://jrsoftware.org/isinfo.php)
Library for [Inno Setup](https://jrsoftware.org/isinfo.php) that enables:
- [Verifying a file's Authenticode code signature (including details)](#verifyfilecodesignature)
- [Obtaining a file's string version info](#getfileversionstring)

> [!IMPORTANT] 
> Please read the **[Security Tips](#security-tips)** section before using.

### Supports:
- **Windows**: Windows XP -> Windows 11+

### Examples Support:
- **Inno Setup**: 6.0+
  - Should also work on earlier versions of Unicode Inno Setup, but you may have to tweak the example code.

### General Compatibility Notes:
The resulting `innofilecheck.dll`:
- Does **not** have a dependency on the CRT, and should run on systems that do not yet have the VCRedist / CRT installed.
- Dynamically loads all libraries except `kernel32.dll` and `user32.dll`, and handles differing OS / patch-level support of the underlying Windows APIs used automatically.

# Setup

```iss
#ifndef INNOFILECHECK_PATH
// Default to assuming that innofilecheck.dll has been placed in the same directory as the script
#define INNOFILECHECK_PATH SourcePath
#endif

// [...]

[Files]
Source: "{#INNOFILECHECK_PATH}\innofilecheck.dll"; Flags: dontcopy solidbreak

// [...]

[Code]
function VerifyFileCodeSignature(const filePath: String; const certName: String; const certIssuerName: String; const microsoftRootCheck: BOOL): Integer;
external 'VerifyFileCodeSignature@files:innofilecheck.dll cdecl delayload';

function GetFileVersionString(const filePath: String; const stringName: String; wLanguage: Word; wCodePage: Word; out_str: String; out_len: UINT): Integer;
external 'GetFileVersionString@files:innofilecheck.dll cdecl delayload';
```

# VerifyFileCodeSignature

`VerifyFileCodeSignature(const filePath: String; const certName: String; const certIssuerName: String; const microsoftRootCheck: BOOL): Integer`

This call returns `0` if the file's Authenticode signature is valid (and passes any additional checks); otherwise, it returns an error code (see: `CODESIGNCHECK_STATUS_CODES` in [src/codesigncheck.h](src/codesigncheck.h)).

- **filePath**
  - Must specify the full path to the file to be verified
- **certName**
  - Check that the first valid signature is associated with a certificate with name NAME. (May be an empty string - `''` - to skip the check.)
- **certIssuerName**
  - Check that the first valid signature is associated with a certificate with _issuer_ name ISSUERNAME. (May be an empty string - `''` - to skip the check.)
- **microsoftRootCheck**
  - If `True`, checks for a Microsoft root certificate. If `False`, skips the check.

### Examples:

Check for a valid Microsoft code-signature, specifying a `certName` and `certIssuerName`:
```iss
function Example_Microsoft_CodeSignature_Check(FilePath: String): Boolean;
var
  FileCheckResult: Integer;
begin
  FileCheckResult := VerifyFileCodeSignature(FilePath, 'Microsoft Corporation', 'Microsoft Code Signing PCA', True);
  if FileCheckResult <> 0 then begin
    Log('"' + FilePath + '" failed to verify code signature with failure code: ' + IntToStr(FileCheckResult));
    Result := False;
    Exit;
  end;
  Result := True;
end;
```

# GetFileVersionString

`GetFileVersionString(const filePath: String; const stringName: String; wLanguage: Word; wCodePage: Word; out_str: String; out_len: UINT): Integer`

Retrieves a string value from the file version info.

- If successful, returns the number of characters that would have been written to `out_str` if `out_len` had been sufficiently large, _not_ counting the terminating null character.
- If an error occurs, a negative error code is returned.

> NOTE: Only when the return value is non-negative and less than `out_len` has the string been completely written.

Example Wrapper:
```iss
function GetFileVersionStringWrapper(filePath: String; const stringName: String; wLanguage: Word; wCodePage: Word; var out_str: String): Boolean;
var
  FileCheckResult: Integer;
  BufferLength: Integer;
begin
  // Call once to determine required length of string
  FileCheckResult := GetFileVersionString(filePath, stringName, wLanguage, wCodePage, '', 0);
  if FileCheckResult < 0 then
  begin
    Log('GetFileVersionString of "' + filePath + '" failed with code: ' + IntToStr(FileCheckResult));
    Result := False;
    Exit;
  end;
  // Allocate a buffer of desired length (including space for null terminator)
  BufferLength := FileCheckResult + 1;
  SetLength(out_str, BufferLength);
  // Request the string value
  FileCheckResult := GetFileVersionString(filePath, stringName, wLanguage, wCodePage, out_str, BufferLength);
  if FileCheckResult < 0 then
  begin
    Log('GetFileVersionString of "' + filePath + '" failed with code: ' + IntToStr(FileCheckResult));
    Result := False;
    Exit;
  end;
  // Check for truncation (should only happen if the file changed between the initial and subsequent calls)
  if FileCheckResult >= BufferLength then
  begin
    Log('GetFileVersionString of "' + filePath + '" unexpectedly truncated string');
    Result := False;
    Exit;
  end;
  // Set the buffer length to the returned length
  SetLength(out_str, FileCheckResult);
  Result := True;
end;
```

# Security Tips

### Consider the Appropriate Solution

Inno Setup now has built-in functions to calculate the checksum of a file (see: [GetSHA256OfFile](https://jrsoftware.org/ishelp/topic_isxfunc_getsha256offile.htm)). When you expect a specific file, consider using those functions.

Inno Setup 6.5.0+ has a new [signature-verification capability](https://jrsoftware.org/ishelp/index.php?topic=issig), separate and apart from Authenticode code-signing signatures. Consider whether this is a better solution for your use-case.

However, if you want to verify / validate an Authenticode signature, or check other file version strings, this library may be useful. Just please be sure that you consider whether that is a sufficient or appropriate check, and whether it should be combined (or replaced) with other checks.

### Avoid TOCTOU

[Time of check to time of use (TOCTOU / TOCTTOU)](https://en.wikipedia.org/wiki/Time_of_check_to_time_of_use) bugs can lead to security vulnerabilities.

Do not assume that a file that has been checked has not been modified between the time of the check and the time of the use. Use proper security permissions on any containing / temporary folders to ensure that nothing unprivileged can modify a file between a check and any use.
