
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#include <Windows.h>
// winbase.h  GetCurrentDirectory()

#include <malloc.h>     /* malloc() */
#include <stdio.h>      /* FILE; fclose(), fopen(), fread(), fseek(); */
#include <stdlib.h>     /* itoa() */
#include <string.h>     /* strcat(), strcpy() */



/*

    Get_Current_Working_Directory();
    // C:\STU\devel\STU - GameData\STU - GameData

*/



#ifdef _WIN32
/*
    DWORD GetCurrentDirectory([in]  DWORD  nBufferLength, [out] LPTSTR lpBuffer);
    GetCurrentDirectoryA()
    GetCurrentDirectoryW()

*/
char Debug_Message_Buffer[4096];
void Get_Current_Working_Directory()
{
    TCHAR tszBuffer[MAX_PATH];
    CHAR szBuffer[MAX_PATH];
    DWORD dwRet;
    char Current_Working_Directory[MAX_PATH];  // actual type: wchar_t [260]

    GetCurrentDirectoryA(MAX_PATH, Current_Working_Directory);

    dwRet = GetCurrentDirectory(MAX_PATH, tszBuffer);

    // Meh?      // if(dwRet == 0) { OutputDebugStringA("FAILURE: GetCurrentDirectory(): function failed\n"); }
    // Meh?      // if(dwRet > MAX_PATH) { OutputDebugStringA("FAILURE: GetCurrentDirectory(): buffer too short\n"); }

    // sprintf(Debug_Message_Buffer, "%s\n", tszBuffer);
    sprintf(Debug_Message_Buffer, "%s\n", szBuffer);
    // // OutputDebugStringA(Current_Working_Directory);
    // // OutputDebugStringA(tszBuffer);  // TCHAR * vs. LPCSTR
    OutputDebugString(tszBuffer);

    sprintf(Debug_Message_Buffer, "%s\n", Current_Working_Directory);
    OutputDebugStringA(Debug_Message_Buffer);
}
#endif
