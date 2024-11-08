

#include <iostream>     /* wprintf() */
#include <Windows.h>
// #include <stdio.h>		/* printf(); */
// #include <stdlib.h>		/* malloc(); */
// #include <malloc.h>
// #include <memory.h>
/* ? new ? */





// https://stackoverflow.com/questions/8991192/check-the-file-size-without-opening-file-in-c

__int64 FileSize(const wchar_t* name)
{
    HANDLE hFile = CreateFile(name, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1; // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size))
    {
        CloseHandle(hFile);
        return -1; // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return size.QuadPart;
}

/*
    Microsoft Windows API
    CreateFile
        :: CreateFileA
        :: CreateFileW
    fileapi.h
        #ifdef UNICODE
        #define CreateFile  CreateFileW
        #else
        #define CreateFile  CreateFileA
        #endif // !UNICODE

WINBASEAPI
HANDLE
WINAPI
CreateFileW(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwShareMode,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    _In_ DWORD dwCreationDisposition,
    _In_ DWORD dwFlagsAndAttributes,
    _In_opt_ HANDLE hTemplateFile
    );

#ifdef UNICODE
#define CreateFile  CreateFileW
#else
#define CreateFile  CreateFileA
#endif // !UNICODE



convert `const char *` to `LPCWSTR` (AKA `const wchar_t *`)

https://gist.github.com/jsxinvivo/11f383ac61a56c1c0c25
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString=new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}


*/
HRESULT WinAPI_Load_File_W
(
    __in LPCWSTR lpFileName,
    // wchar_t * FileName ... Why not LPCWSTR?
    __deref_out_bcount(*pcbFileData) LPBYTE* ppaFileData,
    // Will contain the pointer to the file data buffer
    __out DWORD* pcbFileData
    // Size of file data buffer returned
)
{

    HANDLE hFile = NULL;
    HRESULT hResult;
    DWORD dwBytesRead;

    hResult = S_OK;

    if (!ppaFileData || !pcbFileData)
    {
        return E_INVALIDARG;
    }

    hFile = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        //DisplayError(TEXT("CreateFile"));
        wprintf(L"Terminal failure: unable to open file \"%s\" for read.\n", lpFileName);
        exit(1);
        //return EXIT_FAILURE;
    }
    *pcbFileData = GetFileSize(hFile, NULL);
    *ppaFileData = new BYTE[*pcbFileData];
    dwBytesRead = 0;
    if (FALSE == ReadFile(hFile, *ppaFileData, *pcbFileData, &dwBytesRead, NULL)) {
        // // DisplayError(TEXT("ReadFile"));
        // _tprintf(TEXT("Terminal failure: Unable to read from file.\n GetLastError=%08x\n"), GetLastError());
        wprintf(L"Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
        CloseHandle(hFile);
        //return EXIT_FAILURE;

    }
    if (dwBytesRead != *pcbFileData) {
        // // DisplayError(TEXT("ReadFile"));
        // _tprintf(TEXT("Terminal failure: Unable to read from file.\n GetLastError=%08x\n"), GetLastError());
        wprintf(L"Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
        CloseHandle(hFile);
        //return EXIT_FAILURE;
    }

//    //if (_msize(hFile) == 0) { CloseHandle(hFile); }
////#pragma warning disable
//#pragma warning disable C6001 // Warning suppressed from here
//    CloseHandle(hFile);
//#pragma warning restore C6001 // Warning recognized from here
//    //#pragma warning restore

    //if (NULL != hFile)
    //{
    //    CloseHandle(hFile);
    //}

    return hResult;
}
