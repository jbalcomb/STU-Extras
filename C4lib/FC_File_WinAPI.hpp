#pragma once

#include <windows.h>

HRESULT WinAPI_Load_File_W
(
    __in LPCWSTR lpFileName,
    // wchar_t * FileName ... Why not LPCWSTR?
    __deref_out_bcount(*pcbFileData) LPBYTE* ppaFileData,
    // Will contain the pointer to the file data buffer
    __out DWORD* pcbFileData
    // Size of file data buffer returned
);
