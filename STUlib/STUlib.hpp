#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS


// incompatible with C99's stdbool.h
#ifndef __cplusplus
typedef unsigned char bool;
static const bool FALSE = 0;
static const bool TRUE = 1;
#endif


// MoX_TYPE.H
typedef unsigned char* byte_ptr;
typedef unsigned char* SAMB_ptr;

typedef unsigned char       byte;
typedef unsigned short int  word;
typedef unsigned int        dword;

// #include <stdint.h>
#define _STDINT
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;


#include "LoadGameFiles.hpp"

#include "GameData/Export-UnitTypeTable.hpp"
#include "GameData/OFS_LEN.H"
#include "GameData/NUM.H"
#include "GameData/s_UNITTYPE.H"
#include "GameData/UnitType.hpp"
#include "GameData/WZD_OFS.H"

#include <iostream>
#include <malloc.h>     /* malloc() */
#include <stdint.h>
#include <stdio.h>      /* FILE; fclose(), fopen(), fread(), fseek(), printf(); */
#include <stdlib.h>     /* itoa() */
#include <string.h>     /* strcat(), strcpy() */

#include <Windows.h>
// winbase.h  GetCurrentDirectory()



// MoX_BITS.H

#define GET_1B_OFS(_ptr_,_ofs_) (                              \
  ( (uint8_t)  ( (uint8_t) *( (_ptr_) + (_ofs_) + 0) )       ) \
)
#define GET_2B_OFS(_ptr_,_ofs_) (                              \
  ( (uint16_t) ( (uint8_t) *( (_ptr_) + (_ofs_) + 0) )       ) \
| ( (uint16_t) ( (uint8_t) *( (_ptr_) + (_ofs_) + 1) ) <<  8 ) \
)
#define GET_4B_OFS(_ptr_,_ofs_) (                              \
  ( (uint32_t) ( (uint8_t) *( (_ptr_) + (_ofs_) + 0) )       ) \
| ( (uint32_t) ( (uint8_t) *( (_ptr_) + (_ofs_) + 1) ) <<  8 ) \
| ( (uint32_t) ( (uint8_t) *( (_ptr_) + (_ofs_) + 2) ) << 16 ) \
| ( (uint32_t) ( (uint8_t) *( (_ptr_) + (_ofs_) + 3) ) << 24 ) \
)

#define SET_1B_OFS(_ptr_,_ofs_,_val_) (        \
*( (_ptr_) + (_ofs_) + 0 ) = ( (_val_)      )  \
)
#define SET_2B_OFS(_ptr_,_ofs_,_val_) (        \
*( (_ptr_) + (_ofs_) + 0 ) = ( (_val_)      ), \
*( (_ptr_) + (_ofs_) + 1 ) = ( (_val_) >> 8 )  \
)
#define SET_4B_OFS(_ptr_,_ofs_,_val_) (        \
*( (_ptr_) + (_ofs_) + 0 ) = ( (_val_)      ), \
*( (_ptr_) + (_ofs_) + 1 ) = ( (_val_) >>  8), \
*( (_ptr_) + (_ofs_) + 2 ) = ( (_val_) >> 16), \
*( (_ptr_) + (_ofs_) + 3 ) = ( (_val_) >> 24)  \
)



long Length_of_File(const char* file_path_name);

unsigned char* Load_Entire_File(const char* file_path_name);
