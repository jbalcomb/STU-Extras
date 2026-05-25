
; Segment type:	Pure code
segment	seg012 byte public 'CODE' use16
assume cs:seg012
;org 0Ah
assume es:nothing, ss:nothing, ds:dseg,	fs:nothing, gs:nothing
CS_EMM_Logical_Page dw 0
proc EMM_DetectDevice far
push	si
push	di
loc_16A9E:
push	es
push	ds
loc_16AA0:
mov	dx, offset EMM_DevName
mov	ax, 3D00h
loc_16AA6:
int	21h
jb	short @@Done__Return_Failure
mov	bx, ax
loc_16AAC:
mov	ax, 4400h
loc_16AAF:
int	21h
jb	short @@Done__Return_Failure
loc_16AB3:
and	dx, 80h
jz	short @@Done__Return_Failure
loc_16AB9:
mov	ax, 4407h
int	21h
jb	short @@Done__Return_Failure
or	al, al
jz	short @@Done__Return_Failure
mov	ah, 3Eh
int	21h
jb	short @@Done__Return_Failure
mov	ah, 46h
int	67h
cmp	ah, 0
loc_16AD1:
jnz	short @@Done__Return_Failure
cmp	al, 40h
js	short @@Done__Return_Failure
mov	ah, 41h
int	67h
cmp	ah, 0
jnz	short @@Done__Return_Failure
mov	[EMS_PFBA], bx
mov	ax, e_ST_SUCCESS
pop	ds
pop	es
pop	di
pop	si
retf
@@Done__Return_Failure:
mov	ax, e_ST_FAILURE
pop	ds
pop	es
pop	di
pop	si
retf
endp EMM_DetectDevice
proc EMM_GetActiveHandleCount far
push	es
push	ds
push	si
push	di
mov	ah, 4Bh
int	67h
cmp	ah, 0
jnz	short loc_16B08
mov	ax, bx
pop	di
pop	si
pop	ds
pop	es
retf
loc_16B08:
mov	ax, 0
pop	di
pop	si
pop	ds
pop	es
retf
endp EMM_GetActiveHandleCount
proc EMM_GetFreePageCnt	far
push	es
push	ds
push	si
push	di
mov	ah, 42h
int	67h
cmp	ah, 0
jnz	short loc_16B24
mov	ax, bx
pop	di
pop	si
pop	ds
pop	es
retf
loc_16B24:
mov	ax, 0
pop	di
pop	si
pop	ds
loc_16B2A:
pop	es
retf
endp EMM_GetFreePageCnt
; Attributes: bp-based frame
proc EMM_GetPageCount far
EMM_Handle= word ptr  6
push	bp
mov	bp, sp
push	es
push	ds
push	si
push	di
mov	ah, 4Ch
mov	dx, [bp+EMM_Handle]
int	67h
loc_16B3A:
cmp	ah, 0
jnz	short loc_16B47
mov	ax, bx
pop	di
pop	si
pop	ds
pop	es
pop	bp
retf
loc_16B47:
mov	ax, 0
pop	di
pop	si
pop	ds
pop	es
pop	bp
retf
endp EMM_GetPageCount
; Attributes: bp-based frame
proc EMM_MakeNamedHandle far
handle=	word ptr -2
pages= word ptr	 6
name= word ptr	8
push	bp
mov	bp, sp
sub	sp, 2
push	es
push	ds
push	si
push	di
mov	ah, 43h
mov	bx, [bp+pages]
int	67h
cmp	ah, 0
jnz	short loc_16B81
mov	[bp+handle], dx
mov	ax, 5301h
mov	si, [bp+name]
int	67h
cmp	ah, 0
jnz	short loc_16B81
loc_16B76:
mov	ax, [bp+handle]
pop	di
pop	si
pop	ds
pop	es
mov	sp, bp
pop	bp
retf
loc_16B81:
mov	ax, e_ST_FAILURE
pop	di
pop	si
pop	ds
pop	es
mov	sp, bp
pop	bp
retf
endp EMM_MakeNamedHandle
; Attributes: bp-based frame
proc EMM_GetHandleName far
Name_Buffer= word ptr  6
EMM_Handle= word ptr  8
push	bp
mov	bp, sp
push	es
push	ds
push	si
push	di
mov	ax, 5300h
mov	dx, [bp+EMM_Handle]
mov	di, [bp+Name_Buffer]
mov	bx, ds
mov	es, bx
assume es:dseg
int	67h
cmp	ah, 0
jnz	short loc_16BB9
mov	di, [bp+Name_Buffer]
add	di, 8
mov	al, 0
stosb
mov	ax, -1
pop	di
pop	si
pop	ds
pop	es
assume es:nothing
pop	bp
retf
loc_16BB9:
mov	ax, 0
pop	di
pop	si
pop	ds
pop	es
pop	bp
retf
endp EMM_GetHandleName
; Attributes: bp-based frame
proc EMM_ReleaseHandle__STUB far
handle=	word ptr  6
push	bp
mov	bp, sp
push	es
push	ds
push	si
push	di
cmp	[bp+handle], 0
jz	short loc_16BD6
mov	ah, 45h
mov	dx, [bp+handle]
int	67h
loc_16BD6:
pop	di
pop	si
pop	ds
pop	es
pop	bp
retf
endp EMM_ReleaseHandle__STUB
; Attributes: bp-based frame
proc EMM_MapnRead far
Target_Off= word ptr  6
src_sgmt= word ptr  8
Data_Offset= dword ptr	0Ah
Amt_Bytes= word	ptr  0Eh
EMM_Handle= word ptr  10h
push	bp
mov	bp, sp
sub	sp, 2
push	es
push	ds
push	si
push	di
loc_16BE6:
cmp	[bp+src_sgmt], 0
jnz	short loc_16BF1
mov	ax, ds
mov	[bp+src_sgmt], ax
loc_16BF1:
mov	ax, [word ptr bp+Data_Offset]
mov	dx, [word ptr bp+Data_Offset+2]
shl	ax, 1
rcl	dx, 1
shl	ax, 1
rcl	dx, 1
mov	bx, dx
loc_16C01:
mov	[cs:CS_EMM_Logical_Page], bx
shr	ax, 1
shr	ax, 1
mov	si, ax
mov	ax, 4400h
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4401h
mov	bx, [cs:CS_EMM_Logical_Page]
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4402h
mov	bx, [cs:CS_EMM_Logical_Page]
inc	bx
inc	bx
loc_16C2C:
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4403h
mov	bx, [cs:CS_EMM_Logical_Page]
loc_16C39:
inc	bx
inc	bx
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, [bp+src_sgmt]
mov	es, ax
loc_16C46:
mov	di, [bp+Target_Off]
loc_16C49:
mov	cx, [bp+Amt_Bytes]
mov	ax, [EMS_PFBA]
mov	ds, ax
rep movsb
pop	di
pop	si
pop	ds
pop	es
mov	sp, bp
pop	bp
retf
endp EMM_MapnRead
; Attributes: bp-based frame
proc EMM_MapnWrite far
Source@= dword ptr  6
EMM_Offset= dword ptr  0Ah
Amt_Bytes= word	ptr  0Eh
EMM_Handle= word ptr  10h
push	bp
mov	bp, sp
sub	sp, 2
push	es
push	ds
push	si
push	di
cmp	[word ptr bp+Source@+2], 0
jnz	short loc_16C70
mov	ax, ds
mov	[word ptr bp+Source@+2], ax
loc_16C70:
mov	ax, [word ptr bp+EMM_Offset]
mov	dx, [word ptr bp+EMM_Offset+2]
shl	ax, 1
rcl	dx, 1
shl	ax, 1
rcl	dx, 1
mov	bx, dx
mov	[cs:CS_EMM_Logical_Page], bx
shr	ax, 1
shr	ax, 1
mov	di, ax
mov	ax, 4400h
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4401h
mov	bx, [cs:CS_EMM_Logical_Page]
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4402h
mov	bx, [cs:CS_EMM_Logical_Page]
inc	bx
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4403h
mov	bx, [cs:CS_EMM_Logical_Page]
inc	bx
inc	bx
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, [EMS_PFBA]
mov	es, ax
mov	cx, [bp+Amt_Bytes]
mov	si, [word ptr bp+Source@]
mov	ax, [word ptr bp+Source@+2]
mov	ds, ax
rep movsb
pop	di
pop	si
pop	ds
pop	es
mov	sp, bp
pop	bp
retf
endp EMM_MapnWrite
proc EMM_GetPageFrame far
push	ds
loc_16CDB:
mov	ax, seg	dseg
mov	ds, ax
mov	ax, [EMS_PFBA]
pop	ds
retf
endp EMM_GetPageFrame
; Attributes: bp-based frame
proc EMM_MapFourPages far
EMM_Handle= word ptr  6
First_Log_Page=	word ptr  8
push	bp
mov	bp, sp
push	es
push	ds
mov	ax, 4400h
mov	dx, [bp+EMM_Handle]
mov	bx, [bp+First_Log_Page]
int	67h
mov	ax, 4401h
mov	bx, [bp+First_Log_Page]
inc	bx
loc_16CFC:
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4402h
mov	bx, [bp+First_Log_Page]
inc	bx
inc	bx
mov	dx, [bp+EMM_Handle]
loc_16D0C:
int	67h
mov	ax, 4403h
mov	bx, [bp+First_Log_Page]
loc_16D14:
inc	bx
inc	bx
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
pop	ds
pop	es
pop	bp
retf
endp EMM_MapFourPages
; Attributes: bp-based frame
proc EMM_MapMulti4 far
emsFirst= word ptr  6
emsHandle= word	ptr  8
push	bp
mov	bp, sp
push	ax
push	bx
push	cx
push	dx
push	si
push	ds
mov	bx, [bp+emsFirst]
mov	dx, [bp+emsHandle]
mov	ax, seg	dseg
mov	ds, ax
mov	[ems_mapping_array.Logical_Page], bx
inc	bx
mov	[ems_mapping_array.Logical_Page+4], bx
inc	bx
loc_16D3E:
mov	[ems_mapping_array.Logical_Page+8], bx
loc_16D42:
inc	bx
loc_16D43:
mov	[ems_mapping_array.Logical_Page+0Ch], bx
inc	bx
loc_16D48:
mov	cx, 4
loc_16D4B:
mov	si, offset ems_mapping_array
loc_16D4E:
mov	ax, 5000h
int	67h
pop	ds
pop	si
pop	dx
loc_16D56:
pop	cx
pop	bx
loc_16D58:
pop	ax
pop	bp
locret_16D5A:
retf
endp EMM_MapMulti4
ends seg012
; Segment type:	Pure code
segment	seg013 byte public 'CODE' use16
assume cs:seg013
;org 0Bh
assume es:nothing, ss:nothing, ds:dseg,	fs:nothing, gs:nothing
; Attributes: bp-based frame
proc EMS_Startup far
buffer=	byte ptr -66h
EMM_Open_Handle_Count= word ptr	-2
push	bp
loc_16D5C:
mov	bp, sp
loc_16D5E:
sub	sp, 66h
push	si
loc_16D62:
call	EMM_DetectDevice
or	ax, ax
jnz	short loc_16D6E
loc_16D6B:
jmp	loc_16EF1
loc_16D6E:
call	EMM_GetActiveHandleCount
loc_16D73:
mov	[bp+EMM_Open_Handle_Count], ax
loc_16D76:
mov	[EmmHndlNbr_YOMOMA], 0
mov	si, 1
jmp	short loc_16DA9
loc_16D81:
push	si
lea	ax, [bp+buffer]
push	ax
call	EMM_GetHandleName
pop	cx
pop	cx
or	ax, ax
jz	short loc_16DA8
loc_16D91:
lea	ax, [bp+buffer]
push	ax
mov	ax, offset cnst_EMM_Hnd1
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short loc_16DA8
mov	[EmmHndlNbr_YOMOMA], si
loc_16DA8:
inc	si
loc_16DA9:
cmp	si, [bp+EMM_Open_Handle_Count]
jge	short loc_16DB5
cmp	[EmmHndlNbr_YOMOMA], 0
jz	short loc_16D81
loc_16DB5:
mov	[EMM_Open_Handles], 0
cmp	[EmmHndlNbr_YOMOMA], 0
jnz	short loc_16DDA
xor	ax, ax
push	ax
mov	ax, offset cnst_EMM_Hnd12
push	ax
mov	ax, 1
push	ax
nop
push	cs
call	near ptr EMM_GetHandle
add	sp, 6
mov	[EmmHndlNbr_YOMOMA], ax
jmp	short loc_16E18
loc_16DDA:
mov	ax, offset cnst_EMM_Hnd1
push	ax
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
add	ax, offset EMM_Table.name
push	ax
loc_16DEA:
call	strcpy
pop	cx
pop	cx
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
mov	bx, ax
mov	[EMM_Table.reserved+bx], 0
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
mov	dx, [EmmHndlNbr_YOMOMA]
mov	bx, ax
mov	[EMM_Table.handle+bx], dx
mov	[EMM_Open_Handles], 1
loc_16E18:
cmp	[EmmHndlNbr_YOMOMA], 0
jg	short loc_16E33
lea	ax, [bp+buffer]
push	ax
nop
push	cs
call	near ptr EMM_Error_Message
pop	cx
lea	ax, [bp+buffer]
push	ax
call	Exit_With_Message
pop	cx
loc_16E33:
mov	[EMM_OK], e_ST_TRUE
mov	ax, [EmmHndlNbr_YOMOMA]
inc	ax
mov	si, ax
jmp	short loc_16E90
loc_16E41:
push	si
lea	ax, [bp+buffer]
push	ax
call	EMM_GetHandleName
pop	cx
pop	cx
or	ax, ax
jz	short loc_16E8F
cmp	[bp+buffer], 0
jz	short loc_16E8F
mov	ax, [EMM_Open_Handles]
mov	dx, size s_EMM_RECORD
imul	dx
mov	bx, ax
mov	[EMM_Table.reserved+bx], 0
lea	ax, [bp+buffer]
push	ax
mov	ax, [EMM_Open_Handles]
mov	dx, size s_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, [EMM_Open_Handles]
mov	dx, size s_EMM_RECORD
imul	dx
mov	bx, ax
mov	[EMM_Table.handle+bx], si
inc	[EMM_Open_Handles]
loc_16E8F:
inc	si
loc_16E90:
cmp	si, 50
jge	short loc_16E9C
cmp	[EMM_Open_Handles], 40
jl	short loc_16E41
loc_16E9C:
call	EMM_GetFreePageCnt
cmp	ax, [EMM_Pages_Reserved]
loc_16EA5:
jge	short loc_16EBB
lea	ax, [bp+buffer]
push	ax
nop
push	cs
call	near ptr EMM_Error_Message
pop	cx
lea	ax, [bp+buffer]
push	ax
call	Exit_With_Message
pop	cx
loc_16EBB:
mov	ax, 1
push	ax
mov	ax, offset str_VGAFILEH__seg013
push	ax
loc_16EC3:
mov	ax, 5
push	ax
nop
push	cs
call	near ptr EMM_GetHandle
add	sp, 6
mov	[_VGAFILEH_seg], ax
mov	ax, 1
push	ax
mov	ax, offset str_EMMDATAH__seg013
push	ax
mov	ax, 4
push	ax
nop
push	cs
call	near ptr EMM_GetHandle
add	sp, 6
mov	[_EMMDATAH_seg], ax
loc_16EE9:
mov	[EMMDATAH_offset], 0
jmp	short @@Done
loc_16EF1:
lea	ax, [bp+buffer]
push	ax
nop
push	cs
loc_16EF7:
call	near ptr EMM_Error_Message
pop	cx
lea	ax, [bp+buffer]
push	ax
call	Exit_With_Message
pop	cx
@@Done:
pop	si
mov	sp, bp
pop	bp
retf
endp EMS_Startup
; Attributes: bp-based frame
proc EMM_Load_LBX_File_0 far
FileName@= word	ptr  6
push	bp
loc_16F0B:
mov	bp, sp
xor	ax, ax
push	ax
push	[bp+FileName@]
nop
push	cs
call	near ptr EMM_Load_LBX_File
pop	cx
pop	cx
pop	bp
retf
endp EMM_Load_LBX_File_0
; Attributes: bp-based frame
proc EMM_Load_LBX_File_1 far
lbx_name= word ptr  6
push	bp
mov	bp, sp
mov	ax, 1
push	ax
push	[bp+lbx_name]
nop
push	cs
call	near ptr EMM_Load_LBX_File
pop	cx
pop	cx
pop	bp
retf
endp EMM_Load_LBX_File_1
; Attributes: bp-based frame
proc EMM_Load_LBX_File far
Handle_Name= byte ptr -3Eh
LBX_FileName= byte ptr -2Ah
Bytes_Copied= dword ptr	-16h
Bytes_To_Copy= dword ptr -12h
Target_Logical_Page= word ptr -0Eh
Amt_Bytes= word	ptr -0Ch
Load_Segment= word ptr -0Ah
Page_Count= word ptr -8
Handle_Exists= word ptr	-6
EMM_Handle= word ptr -4
File_Handle= word ptr -2
FileName@= word	ptr  6
Reserved= word ptr  8
push	bp
mov	bp, sp
sub	sp, 3Eh
push	si
push	di
push	[bp+FileName@]
call	File_Name_Base
pop	cx
loc_16F40:
push	[bp+FileName@]
lea	ax, [bp+Handle_Name]
push	ax
call	strcpy
pop	cx
pop	cx
xor	si, si
jmp	short loc_16F53
loc_16F52:
inc	si
loc_16F53:
cmp	[bp+si+Handle_Name], 0
jnz	short loc_16F52
mov	di, si
jmp	short loc_16F62
loc_16F5D:
mov	[bp+di+Handle_Name], 0
inc	di
loc_16F62:
cmp	di, 9
jl	short loc_16F5D
mov	[bp+Handle_Exists], 0
xor	si, si
jmp	short loc_16F90
loc_16F70:
lea	ax, [bp+Handle_Name]
push	ax
mov	ax, si
mov	dx, 12
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short loc_16F8F
mov	[bp+Handle_Exists], 1
loc_16F8F:
inc	si
loc_16F90:
cmp	si, [EMM_Open_Handles]
jl	short loc_16F70
cmp	[bp+Handle_Exists], 0
jz	short loc_16FAF
xor	ax, ax
push	ax
mov	ax, le_load_reg_first
push	ax
lea	ax, [bp+Handle_Name]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_16FAF:
cmp	[EMM_OK], 0
jnz	short loc_16FB9
jmp	loc_17137
loc_16FB9:
cmp	[EMM_Open_Handles], 40
jl	short loc_16FC3
jmp	loc_17137
loc_16FC3:
lea	ax, [bp+Handle_Name]
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	strcpy
pop	cx
pop	cx
loc_16FD2:
mov	ax, offset str_LBX_extension__seg013
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	strcat
pop	cx
pop	cx
lea	ax, [bp+LBX_FileName]
push	ax
call	LOF
pop	cx
mov	[word ptr bp+Bytes_To_Copy+2], dx
mov	[word ptr bp+Bytes_To_Copy], ax
mov	ax, [word ptr bp+Bytes_To_Copy]
or	ax, [word ptr bp+Bytes_To_Copy+2]
jnz	short loc_16FFC
jmp	loc_17137
loc_16FFC:
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+Bytes_To_Copy+2]
push	[word ptr bp+Bytes_To_Copy]
call	LDIV@
mov	[bp+Page_Count], ax
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+Bytes_To_Copy+2]
push	[word ptr bp+Bytes_To_Copy]
call	LMOD@
or	ax, dx
jz	short loc_1702A
inc	[bp+Page_Count]
loc_1702A:
cmp	[bp+Reserved], 0
jnz	short loc_17045
call	EMM_GetFreePageCnt
mov	dx, [EMM_Pages_Reserved]
add	dx, [bp+Page_Count]
cmp	ax, dx
jnb	short loc_17045
xor	ax, ax
loc_17042:
jmp	loc_1713D
loc_17045:
push	[bp+Reserved]
lea	ax, [bp+Handle_Name]
push	ax
push	[bp+Page_Count]
nop
push	cs
call	near ptr EMM_GetHandle
add	sp, 6
mov	[bp+EMM_Handle], ax
cmp	[bp+EMM_Handle], 0
jnz	short loc_17063
jmp	loc_1711F
loc_17063:
call	EMM_GetPageFrame
mov	[bp+Load_Segment], ax
mov	[bp+Target_Logical_Page], 0
cmp	[bp+Load_Segment], 0
jnz	short loc_17079
jmp	loc_1711D
loc_17079:
lea	ax, [bp+LBX_FileName]
push	ax
call	lbx_open
pop	cx
mov	[bp+File_Handle], ax
cmp	[bp+File_Handle], 0
jnz	short loc_1708F
jmp	loc_1711D
loc_1708F:
mov	[word ptr bp+Bytes_Copied+2], 0
mov	[word ptr bp+Bytes_Copied], 0
mov	[bp+Amt_Bytes],	16384
jmp	short loc_170D3
loc_170A0:
sub	[word ptr bp+Bytes_To_Copy], 16384
sbb	[word ptr bp+Bytes_To_Copy+2], 0
push	[bp+Target_Logical_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	[bp+File_Handle]
push	[bp+Amt_Bytes]
push	[bp+Load_Segment]
call	lbx_read_sgmt
add	sp, 6
inc	[bp+Target_Logical_Page]
add	[word ptr bp+Bytes_Copied], 16384
adc	[word ptr bp+Bytes_Copied+2], 0
loc_170D3:
cmp	[word ptr bp+Bytes_To_Copy+2], 0
jg	short loc_170A0
jl	short loc_170E2
cmp	[word ptr bp+Bytes_To_Copy], 16384
jnb	short loc_170A0
loc_170E2:
cmp	[word ptr bp+Bytes_To_Copy+2], 0
jl	short loc_17114
jg	short loc_170F0
cmp	[word ptr bp+Bytes_To_Copy], 0
jbe	short loc_17114
loc_170F0:
mov	ax, [word ptr bp+Bytes_To_Copy]
mov	[bp+Amt_Bytes],	ax
push	[bp+Target_Logical_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	[bp+File_Handle]
push	[bp+Amt_Bytes]
push	[bp+Load_Segment]
call	lbx_read_sgmt
add	sp, 6
loc_17114:
push	[bp+File_Handle]
call	lbx_close
pop	cx
loc_1711D:
jmp	short loc_17137
loc_1711F:
cmp	[bp+Reserved], 1
jnz	short loc_17137
xor	ax, ax
push	ax
mov	ax, le_EM_swap_fail
push	ax
push	[bp+FileName@]
call	Error_Handler
db  83h
db 0C4h
db    6
loc_17137:
mov	ax, [bp+EMM_Handle]
jmp	loc_17042
loc_1713D:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_Load_LBX_File
; Attributes: bp-based frame
proc RP_EMM_LoadLBX2Hnd	far
FileName@= word	ptr  6
Entry_Index= word ptr  8
EHnd_Name@= word ptr  0Ah
push	bp
mov	bp, sp
push	[bp+EHnd_Name@]
xor	ax, ax
push	ax
push	[bp+Entry_Index]
push	[bp+FileName@]
nop
push	cs
call	near ptr UU_EMM_LoadLBX_Fail
add	sp, 8
pop	bp
retf
endp RP_EMM_LoadLBX2Hnd
; Attributes: bp-based frame
proc RP_EMM_LoadLBX2Hnd2 far
FileName@= word	ptr  6
Entry_Index= word ptr  8
EHnd_Name@= word ptr  0Ah
push	bp
mov	bp, sp
push	[bp+EHnd_Name@]
mov	ax, 1
push	ax
push	[bp+Entry_Index]
push	[bp+FileName@]
nop
push	cs
call	near ptr UU_EMM_LoadLBX_Fail
add	sp, 8
pop	bp
retf
endp RP_EMM_LoadLBX2Hnd2
; Attributes: bp-based frame
proc UU_EMM_LoadLBX_Fail far
Handle_Name= byte ptr -42h
LBX_FileName= byte ptr -2Eh
File_Offset= dword ptr -1Ah
Entry_Size= dword ptr -16h
Total_Bytes_Left= dword	ptr -12h
Bytes_To_Copy= word ptr	-0Eh
Page_Frame_Seg=	word ptr -0Ch
Page_Count= word ptr -0Ah
Logical_Page= word ptr -8
Handle_Found= word ptr -6
EMM_Handle= word ptr -4
LBX_File_Handle= word ptr -2
FileName@= word	ptr  6
Entry_Index= word ptr  8
Fmt= word ptr  0Ah
EHnd_Name@= word ptr  0Ch
push	bp
mov	bp, sp
sub	sp, 42h
push	si
push	di
push	[bp+FileName@]
call	File_Name_Base
pop	cx
push	[bp+FileName@]
lea	ax, [bp+LBX_FileName]
push	ax
call	strcpy
pop	cx
pop	cx
push	[bp+EHnd_Name@]
lea	ax, [bp+Handle_Name]
push	ax
call	strcpy
pop	cx
pop	cx
xor	si, si
jmp	short loc_171A8
loc_171A7:
inc	si
loc_171A8:
cmp	[bp+si+Handle_Name], 0
jnz	short loc_171A7
mov	di, si
jmp	short loc_171B7
loc_171B2:
mov	[bp+di+Handle_Name], 0
inc	di
loc_171B7:
cmp	di, 9
jl	short loc_171B2
mov	[bp+Handle_Found], 0
xor	si, si
jmp	short loc_171E5
loc_171C5:
lea	ax, [bp+Handle_Name]
push	ax
mov	ax, si
mov	dx, 0Ch
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short loc_171E4
mov	[bp+Handle_Found], 1
loc_171E4:
inc	si
loc_171E5:
cmp	si, [EMM_Open_Handles]
jl	short loc_171C5
cmp	[bp+Handle_Found], 0
jz	short loc_17204
xor	ax, ax
push	ax
mov	ax, le_load_reg_first
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
loc_171FC:
call	Error_Handler
db  83h
unk_17202 db 0C4h
db    6
loc_17204:
cmp	[EMM_OK], 0
jnz	short loc_1720E
jmp	loc_17353
loc_1720E:
cmp	[EMM_Open_Handles], 28h
jl	short loc_17218
jmp	loc_17353
loc_17218:
push	[bp+Fmt]
lea	ax, [bp+Entry_Size]
push	ax
lea	ax, [bp+File_Offset]
push	ax
push	[bp+Entry_Index]
lea	ax, [bp+LBX_FileName]
push	ax
call	LBX_Get_Entry_Data
add	sp, 0Ah
mov	[bp+LBX_File_Handle], ax
push	[bp+LBX_File_Handle]
push	[word ptr bp+File_Offset+2]
push	[word ptr bp+File_Offset]
call	lbx_seek
add	sp, 6
or	ax, ax
jnz	short loc_1725D
push	[bp+Entry_Index]
mov	ax, le_corrupted
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_1725D:
xor	ax, ax
mov	dx, 4000h
push	ax
push	dx
push	[word ptr bp+Entry_Size+2]
push	[word ptr bp+Entry_Size]
call	LDIV@
mov	[bp+Page_Count], ax
xor	ax, ax
mov	dx, 4000h
push	ax
push	dx
push	[word ptr bp+Entry_Size+2]
push	[word ptr bp+Entry_Size]
call	LMOD@
or	ax, dx
jz	short loc_1728B
inc	[bp+Page_Count]
loc_1728B:
xor	ax, ax
push	ax
lea	ax, [bp+Handle_Name]
push	ax
push	[bp+Page_Count]
nop
push	cs
call	near ptr EMM_GetHandle
add	sp, 6
mov	[bp+EMM_Handle], ax
loc_172A0:
cmp	[bp+EMM_Handle], 0
jnz	short loc_172B9
xor	ax, ax
push	ax
mov	ax, le_low_EMS
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_172B9:
call	EMM_GetPageFrame
mov	[bp+Page_Frame_Seg], ax
mov	[bp+Logical_Page], 0
mov	ax, [word ptr bp+Entry_Size+2]
mov	dx, [word ptr bp+Entry_Size]
mov	[word ptr bp+Total_Bytes_Left+2], ax
mov	[word ptr bp+Total_Bytes_Left],	dx
mov	[bp+Bytes_To_Copy], 4000h
jmp	short loc_17303
loc_172D9:
sub	[word ptr bp+Total_Bytes_Left],	4000h
sbb	[word ptr bp+Total_Bytes_Left+2], 0
push	[bp+Logical_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
loc_172EF:
push	[bp+LBX_File_Handle]
push	[bp+Bytes_To_Copy]
push	[bp+Page_Frame_Seg]
call	lbx_read_sgmt
add	sp, 6
inc	[bp+Logical_Page]
loc_17303:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jg	short loc_172D9
jl	short loc_17312
cmp	[word ptr bp+Total_Bytes_Left],	4000h
jnb	short loc_172D9
loc_17312:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jl	short loc_17353
jg	short loc_17320
cmp	[word ptr bp+Total_Bytes_Left],	0
jbe	short loc_17353
loc_17320:
xor	ax, ax
mov	dx, 10h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_Left+2]
push	[word ptr bp+Total_Bytes_Left]
call	LMOD@
mov	[bp+Bytes_To_Copy], ax
push	[bp+Logical_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	[bp+LBX_File_Handle]
push	[bp+Bytes_To_Copy]
push	[bp+Page_Frame_Seg]
call	lbx_read_sgmt
add	sp, 6
loc_17353:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp UU_EMM_LoadLBX_Fail
; Attributes: bp-based frame
proc UU_EMM_CustLoadLBX	far
FName@=	word ptr  6
Entry= word ptr	 8
HndName@= word ptr  0Ah
EOff= dword ptr	 0Ch
Bytes= dword ptr  10h
push	bp
mov	bp, sp
push	[word ptr bp+Bytes+2]
push	[word ptr bp+Bytes]
push	[word ptr bp+EOff+2]
push	[word ptr bp+EOff]
push	[bp+HndName@]
xor	ax, ax
push	ax
push	[bp+Entry]
push	[bp+FName@]
nop
push	cs
call	near ptr UU_EMM_CustomLBXLoad
add	sp, 10h
loc_1737C:
pop	bp
retf
endp UU_EMM_CustLoadLBX
; Attributes: bp-based frame
proc UU_EMM_CustLoadLBX2 far
FName@=	word ptr  6
Entry= word ptr	 8
HndName@= word ptr  0Ah
EOff= dword ptr	 0Ch
Bytes= dword ptr  10h
push	bp
mov	bp, sp
push	[word ptr bp+Bytes+2]
push	[word ptr bp+Bytes]
push	[word ptr bp+EOff+2]
push	[word ptr bp+EOff]
push	[bp+HndName@]
mov	ax, 1
push	ax
push	[bp+Entry]
push	[bp+FName@]
nop
push	cs
call	near ptr UU_EMM_CustomLBXLoad
add	sp, 10h
pop	bp
retf
endp UU_EMM_CustLoadLBX2
; Attributes: bp-based frame
proc UU_EMM_CustomLBXLoad far
EMM_Handle_Name= byte ptr -42h
LBX_FileName= byte ptr -2Eh
File_Offset= dword ptr -1Ah
LBX_Entry_Size=	dword ptr -16h
Total_Bytes_Left= dword	ptr -12h
Bytes_To_Copy= word ptr	-0Eh
Buffer_Seg= word ptr -0Ch
Page_Count= word ptr -0Ah
First_Log_Page=	word ptr -8
Handle_Found= word ptr -6
EMM_Handle= word ptr -4
File_Handle= word ptr -2
FName@=	word ptr  6
Entry= word ptr	 8
Fmt= word ptr  0Ah
HndName@= word ptr  0Ch
EOff= dword ptr	 0Eh
Bytes= dword ptr  12h
push	bp
mov	bp, sp
sub	sp, 42h
push	si
push	di
push	[bp+FName@]
call	File_Name_Base
pop	cx
push	[bp+FName@]
lea	ax, [bp+LBX_FileName]
loc_173BB:
push	ax
loc_173BC:
call	strcpy
pop	cx
pop	cx
loc_173C3:
push	[bp+HndName@]
lea	ax, [bp+EMM_Handle_Name]
push	ax
call	strcpy
pop	cx
pop	cx
xor	si, si
jmp	short loc_173D6
loc_173D5:
inc	si
loc_173D6:
cmp	[bp+si+EMM_Handle_Name], 0
jnz	short loc_173D5
mov	di, si
jmp	short loc_173E5
loc_173E0:
mov	[bp+di+EMM_Handle_Name], 0
inc	di
loc_173E5:
cmp	di, 9
jl	short loc_173E0
mov	[bp+Handle_Found], 0
xor	si, si
jmp	short loc_17413
loc_173F3:
lea	ax, [bp+EMM_Handle_Name]
push	ax
mov	ax, si
mov	dx, 0Ch
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short loc_17412
mov	[bp+Handle_Found], 1
loc_17412:
inc	si
loc_17413:
cmp	si, [EMM_Open_Handles]
jl	short loc_173F3
cmp	[bp+Handle_Found], 0
jz	short loc_17432
xor	ax, ax
push	ax
mov	ax, le_load_reg_first
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_17432:
cmp	[EMM_OK], 0
jnz	short loc_1743C
jmp	loc_175C5
loc_1743C:
cmp	[EMM_Open_Handles], 28h
jl	short loc_17446
jmp	loc_175C5
loc_17446:
push	[bp+Fmt]
lea	ax, [bp+LBX_Entry_Size]
push	ax
lea	ax, [bp+File_Offset]
push	ax
push	[bp+Entry]
lea	ax, [bp+LBX_FileName]
push	ax
call	LBX_Get_Entry_Data
add	sp, 0Ah
mov	[bp+File_Handle], ax
mov	ax, [word ptr bp+EOff+2]
mov	dx, [word ptr bp+EOff]
add	[word ptr bp+File_Offset], dx
adc	[word ptr bp+File_Offset+2], ax
mov	ax, [word ptr bp+EOff+2]
mov	dx, [word ptr bp+EOff]
sub	[word ptr bp+LBX_Entry_Size], dx
sbb	[word ptr bp+LBX_Entry_Size+2],	ax
cmp	[word ptr bp+LBX_Entry_Size+2],	0
jg	short loc_1749C
jnz	short loc_17489
cmp	[word ptr bp+LBX_Entry_Size], 0
ja	short loc_1749C
loc_17489:
push	[bp+Entry]
mov	ax, le_corrupted
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_1749C:
mov	ax, [word ptr bp+Bytes+2]
mov	dx, [word ptr bp+Bytes]
cmp	ax, [word ptr bp+LBX_Entry_Size+2]
jg	short loc_174BA
jl	short loc_174AE
cmp	dx, [word ptr bp+LBX_Entry_Size]
jnb	short loc_174BA
loc_174AE:
mov	ax, [word ptr bp+Bytes+2]
mov	dx, [word ptr bp+Bytes]
mov	[word ptr bp+LBX_Entry_Size+2],	ax
mov	[word ptr bp+LBX_Entry_Size], dx
loc_174BA:
push	[bp+File_Handle]
push	[word ptr bp+File_Offset+2]
push	[word ptr bp+File_Offset]
call	lbx_seek
add	sp, 6
or	ax, ax
jnz	short loc_174E2
push	[bp+Entry]
mov	ax, le_corrupted
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_174E2:
xor	ax, ax
mov	dx, 4000h
push	ax
push	dx
push	[word ptr bp+LBX_Entry_Size+2]
push	[word ptr bp+LBX_Entry_Size]
call	LDIV@
mov	[bp+Page_Count], ax
xor	ax, ax
mov	dx, 4000h
push	ax
push	dx
push	[word ptr bp+LBX_Entry_Size+2]
push	[word ptr bp+LBX_Entry_Size]
call	LMOD@
or	ax, dx
jz	short loc_17510
inc	[bp+Page_Count]
loc_17510:
lea	ax, [bp+EMM_Handle_Name]
push	ax
push	[bp+Page_Count]
nop
push	cs
call	near ptr EMM_GetHandle
pop	cx
pop	cx
mov	[bp+EMM_Handle], ax
cmp	[bp+EMM_Handle], 0
jnz	short loc_1753A
xor	ax, ax
push	ax
mov	ax, le_low_EMS
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_1753A:
call	EMM_GetPageFrame
mov	[bp+Buffer_Seg], ax
mov	[bp+First_Log_Page], 0
mov	ax, [word ptr bp+LBX_Entry_Size+2]
mov	dx, [word ptr bp+LBX_Entry_Size]
mov	[word ptr bp+Total_Bytes_Left+2], ax
mov	[word ptr bp+Total_Bytes_Left],	dx
mov	[bp+Bytes_To_Copy], 4000h
jmp	short loc_17584
loc_1755A:
sub	[word ptr bp+Total_Bytes_Left],	4000h
sbb	[word ptr bp+Total_Bytes_Left+2], 0
push	[bp+First_Log_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	[bp+File_Handle]
push	[bp+Bytes_To_Copy]
push	[bp+Buffer_Seg]
call	lbx_read_sgmt
add	sp, 6
inc	[bp+First_Log_Page]
loc_17584:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jg	short loc_1755A
jl	short loc_17593
cmp	[word ptr bp+Total_Bytes_Left],	4000h
jnb	short loc_1755A
loc_17593:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jl	short loc_175C5
jg	short loc_175A1
cmp	[word ptr bp+Total_Bytes_Left],	0
jbe	short loc_175C5
loc_175A1:
mov	ax, [word ptr bp+Total_Bytes_Left]
mov	[bp+Bytes_To_Copy], ax
push	[bp+First_Log_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	[bp+File_Handle]
push	[bp+Bytes_To_Copy]
push	[bp+Buffer_Seg]
call	lbx_read_sgmt
add	sp, 6
loc_175C5:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp UU_EMM_CustomLBXLoad
; Attributes: bp-based frame
proc EMM_LBX_EntryLoader far
Header_Offset= dword ptr -18h
Total_Bytes_Left= dword	ptr -14h
Entry_Offset= dword ptr	-10h
Amt_Paragraphs=	word ptr -0Ch
Load_Segment= word ptr -0Ah
Bytes_To_Copy= word ptr	-8
LBX_File_Type= word ptr	-6
Entry_Count= word ptr -4
EMM_Handle= word ptr -2
Hnd_Name@= word	ptr  6
Entry_Index= word ptr  8
Alloc_Seg= word	ptr  0Ah
Load_Type= word	ptr  0Ch
argLbxHdrFmt_Post= word	ptr  0Eh
push	bp
mov	bp, sp
sub	sp, 18h
push	si
push	di
mov	si, [bp+Alloc_Seg]
xor	di, di
push	[bp+Hnd_Name@]
nop
push	cs
call	near ptr EMM_CheckHandleOpen
loc_175E0:
pop	cx
mov	[bp+EMM_Handle], ax
cmp	[bp+EMM_Handle], 0
jnz	short @@EmmHndlNmExists
jmp	loc_1785E
@@EmmHndlNmExists:
cmp	[g_EmmRsvd], 1
jnz	short @@NotEmmRsvd
; __linkproc__ EmmRsvd
@@EmmRsvd:
push	[bp+EMM_Handle]
mov	ax, 2
push	ax
xor	ax, ax
mov	dx, 6
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+LBX_File_Type]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
cmp	[bp+LBX_File_Type], 0
jz	short @@LoadPictureHeader
push	[bp+Entry_Index]
mov	ax, le_cantload_reserved
push	ax
push	[bp+Hnd_Name@]
call	Error_Handler
db  83h
db 0C4h
db    6
db 0EBh
db  1Ah
@@LoadPictureHeader:
push	[bp+argLbxHdrFmt_Post]
push	[bp+Load_Type]
push	si
push	[bp+Entry_Index]
push	[bp+Hnd_Name@]
push	[bp+EMM_Handle]
nop
push	cs
call	near ptr EMM_LBX_Load_Picture_Header
add	sp, 0Ch
mov	di, ax
jmp	loc_1785E
@@NotEmmRsvd:
push	[bp+EMM_Handle]
mov	ax, 2
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
lea	ax, [bp+Entry_Count]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [bp+Entry_Index]
cmp	ax, [bp+Entry_Count]
jl	short loc_17671
loc_1766C:
xor	ax, ax
loc_1766E:
jmp	loc_17863
loc_17671:
cmp	[bp+argLbxHdrFmt_Post],	0
jnz	short loc_17687
mov	ax, [bp+Entry_Index]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
jmp	short loc_17695
loc_17687:
mov	ax, [bp+Entry_Index]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 520
loc_17695:
adc	dx, 0
mov	[word ptr bp+Header_Offset+2], dx
mov	[word ptr bp+Header_Offset], ax
push	[bp+EMM_Handle]
mov	ax, 4
push	ax
push	[word ptr bp+Header_Offset+2]
push	[word ptr bp+Header_Offset]
xor	ax, ax
push	ax
loc_176AE:
lea	ax, [bp+Entry_Offset]
push	ax
call	EMM_MapnRead
loc_176B7:
add	sp, 0Ch
push	[bp+EMM_Handle]
mov	ax, 4
push	ax
mov	ax, [word ptr bp+Header_Offset+2]
mov	dx, [word ptr bp+Header_Offset]
add	dx, 4
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+Total_Bytes_Left]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [word ptr bp+Entry_Offset+2]
mov	dx, [word ptr bp+Entry_Offset]
sub	[word ptr bp+Total_Bytes_Left],	dx
sbb	[word ptr bp+Total_Bytes_Left+2], ax
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jge	short loc_176F3
jmp	loc_1785E
loc_176F3:
jg	short loc_176FE
loc_176F5:
cmp	[word ptr bp+Total_Bytes_Left],	0
loc_176F9:
ja	short loc_176FE
jmp	loc_1785E
loc_176FE:
xor	ax, ax
mov	dx, 10h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_Left+2]
push	[word ptr bp+Total_Bytes_Left]
call	LDIV@
mov	[bp+Amt_Paragraphs], ax
test	[bp+Amt_Paragraphs], 0Fh
jz	short loc_1771D
inc	[bp+Amt_Paragraphs]
loc_1771D:
mov	ax, [bp+Load_Type]
or	ax, ax
jz	short loc_17731
cmp	ax, 1
jz	short loc_17746
cmp	ax, 2
jz	short loc_17782
jmp	loc_177D0
loc_17731:
push	[bp+Amt_Paragraphs]
call	Allocate_Space_No_Header__EntrySize
pop	cx
mov	di, ax
or	di, di
jnz	short loc_17743
jmp	loc_1766C
loc_17743:
jmp	loc_177D0
loc_17746:
push	si
call	Check_Allocation
pop	cx
or	ax, ax
jnz	short loc_17754
jmp	loc_1766C
loc_17754:
mov	ax, SAMB.size
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
dec	ax
cmp	ax, [bp+Amt_Paragraphs]
jnb	short loc_17769
jmp	loc_1766C
loc_17769:
mov	ax, si
inc	ax
mov	di, ax
mov	ax, [bp+Amt_Paragraphs]
inc	ax
push	ax
loc_17773:
mov	ax, SAMB.used
push	ax
push	si
call	farpokew
add	sp, 6
jmp	short loc_177D0
loc_17782:
or	si, si
jnz	short loc_1778B
mov	ax, 1
jmp	short loc_1778D
loc_1778B:
xor	ax, ax
loc_1778D:
push	ax
call	Check_Allocation
pop	cx
or	ax, ax
jz	short loc_1779B
jmp	loc_1766C
loc_1779B:
push	si
call	Get_Free_Blocks
pop	cx
cmp	ax, [bp+Amt_Paragraphs]
jnb	short loc_177AA
jmp	loc_1766C
loc_177AA:
mov	ax, SAMB.used
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
mov	dx, si
add	dx, ax
mov	di, dx
mov	ax, SAMB.used
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
mov	dx, [bp+Amt_Paragraphs]
add	dx, ax
push	dx
jmp	short loc_17773
loc_177D0:
or	di, di
jnz	short loc_177D7
jmp	loc_1785E
loc_177D7:
mov	[bp+Load_Segment], di
mov	[bp+Bytes_To_Copy], 8000h
jmp	short loc_17812
loc_177E1:
sub	[word ptr bp+Total_Bytes_Left],	8000h
sbb	[word ptr bp+Total_Bytes_Left+2], 0
push	[bp+EMM_Handle]
push	[bp+Bytes_To_Copy]
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
push	[bp+Load_Segment]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
add	[word ptr bp+Entry_Offset], 8000h
adc	[word ptr bp+Entry_Offset+2], 0
add	[bp+Load_Segment], 800h
loc_17812:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jg	short loc_177E1
jnz	short loc_17821
cmp	[word ptr bp+Total_Bytes_Left],	8000h
ja	short loc_177E1
loc_17821:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jl	short loc_1785E
jg	short loc_1782F
cmp	[word ptr bp+Total_Bytes_Left],	0
jbe	short loc_1785E
loc_1782F:
xor	ax, ax
mov	dx, 8000h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_Left+2]
push	[word ptr bp+Total_Bytes_Left]
call	LMOD@
mov	[bp+Bytes_To_Copy], ax
push	[bp+EMM_Handle]
push	[bp+Bytes_To_Copy]
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
push	[bp+Load_Segment]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
loc_1785E:
mov	ax, di
jmp	loc_1766E
loc_17863:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_LBX_EntryLoader
; Attributes: bp-based frame
proc EMM_LBX_Load_Picture_Header far
Header_Offset= dword ptr -10h
Entry_Offset= dword ptr	-0Ch
First_Logical_Page= word ptr -8
Data_Offset= word ptr -6
Header_Size= word ptr -4
Entry_Count= word ptr -2
EMM_Handle= word ptr  6
EMM_Handle_Name= word ptr  8
LBX_Entry_Index= word ptr  0Ah
Alloc_Seg= word	ptr  0Ch
Load_Type= word	ptr  0Eh
push	bp
mov	bp, sp
sub	sp, 10h
push	si
push	di
mov	si, [bp+Alloc_Seg]
xor	di, di
push	[bp+EMM_Handle]
mov	ax, 2
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
lea	ax, [bp+Entry_Count]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [bp+LBX_Entry_Index]
cmp	ax, [bp+Entry_Count]
jl	short loc_178AC
push	[bp+LBX_Entry_Index]
mov	ax, le_entries_exceeded
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
loc_178AC:
mov	ax, [bp+LBX_Entry_Index]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
adc	dx, 0
mov	[word ptr bp+Header_Offset+2], dx
mov	[word ptr bp+Header_Offset], ax
push	[bp+EMM_Handle]
mov	ax, 4
push	ax
push	[word ptr bp+Header_Offset+2]
push	[word ptr bp+Header_Offset]
xor	ax, ax
push	ax
lea	ax, [bp+Entry_Offset]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	[bp+Header_Size], 1
mov	ax, [bp+Load_Type]
or	ax, ax
jz	short @@LoadTypeZero
@@NotLoadTypeZero:
cmp	ax, 1
jz	short @@LoadTypeOne
@@NotLoadTypeOne:
cmp	ax, 2
jnz	short @@NotLoadTypeTwo
jmp	loc_1797D
@@NotLoadTypeTwo:
jmp	loc_179ED
@@LoadTypeZero:
push	[bp+Header_Size]
call	Allocate_Space_No_Header__EntrySize
pop	cx
mov	di, ax
or	di, di
jnz	short loc_1791C
push	[bp+LBX_Entry_Index]
mov	ax, le_low_RAM
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
loc_1791C:
jmp	loc_179ED
@@LoadTypeOne:
push	si
call	Check_Allocation
pop	cx
or	ax, ax
jnz	short loc_1793C
push	[bp+LBX_Entry_Index]
mov	ax, le_alloc_fail
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
loc_1793C:
mov	ax, SAMB.size
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
dec	ax
cmp	ax, [bp+Header_Size]
jnb	short loc_17964
mov	ax, 1
push	ax
push	[bp+LBX_Entry_Index]
mov	ax, le_reload_fail
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    8
loc_17964:
mov	ax, si
inc	ax
mov	di, ax
mov	ax, [bp+Header_Size]
inc	ax
push	ax
loc_1796E:
mov	ax, SAMB.used
push	ax
push	si
call	farpokew
add	sp, 6
jmp	short loc_179ED
loc_1797D:
or	si, si
jnz	short loc_17986
mov	ax, 1
jmp	short loc_17988
loc_17986:
xor	ax, ax
loc_17988:
push	ax
call	Check_Allocation
pop	cx
or	ax, ax
jz	short loc_179A5
push	[bp+LBX_Entry_Index]
mov	ax, le_alloc_fail
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
loc_179A5:
push	si
call	Get_Free_Blocks
pop	cx
cmp	ax, [bp+Header_Size]
jnb	short loc_179C7
mov	ax, 1
push	ax
push	[bp+LBX_Entry_Index]
mov	ax, le_reload_fail
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    8
loc_179C7:
mov	ax, SAMB.used
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
mov	dx, si
add	dx, ax
mov	di, dx
mov	ax, SAMB.used
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
mov	dx, [bp+Header_Size]
add	dx, ax
push	dx
jmp	short loc_1796E
loc_179ED:
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
call	LDIV@
mov	[bp+First_Logical_Page], ax
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
call	LMOD@
mov	[bp+Data_Offset], ax
loc_17A17:
push	[bp+EMM_Handle]
mov	ax, 16
push	ax
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
push	di
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	[bp+EMM_Handle]
mov	ax, s_FLIC_HDR.emm_handle_number
push	ax
push	di
call	farpokeb
add	sp, 6
push	[bp+First_Logical_Page]
mov	ax, s_FLIC_HDR.EMM_Logical_Page_Number
push	ax
push	di
call	farpokeb
add	sp, 6
push	[bp+Data_Offset]
mov	ax, s_FLIC_HDR.EMM_Logical_Page_Offset
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, di
jmp	short $+2
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_LBX_Load_Picture_Header
; Attributes: bp-based frame
proc EMM_LBX_RecLoader far
Header_Offset= dword ptr -1Ah
Total_Bytes_Left= dword	ptr -16h
Entry_Offset= dword ptr	-12h
File_Record_Count= word	ptr -0Eh
File_Record_Size= word ptr -0Ch
Amt_Paragraphs=	word ptr -0Ah
Target_Seg= word ptr -8
Bytes_To_Copy= word ptr	-6
Entry_Count= word ptr -4
Result_Data_Segment= word ptr -2
Hnd_Name@= word	ptr  6
Entry_Index= word ptr  8
LBX_Alloc_Seg= word ptr	 0Ah
Load_Type= word	ptr  0Ch
FirstRec= word ptr  0Eh
RecCount= word ptr  10h
RecSize= word ptr  12h
push	bp
mov	bp, sp
sub	sp, 1Ah
push	si
push	di
mov	si, [bp+LBX_Alloc_Seg]
mov	[bp+Result_Data_Segment], 0
push	[bp+Hnd_Name@]
nop
push	cs
call	near ptr EMM_CheckHandleOpen
pop	cx
mov	di, ax
or	di, di
jnz	short loc_17A8C
jmp	loc_17D20
loc_17A8C:
push	di
mov	ax, 2
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
lea	ax, [bp+Entry_Count]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [bp+Entry_Index]
cmp	ax, [bp+Entry_Count]
jl	short loc_17AB3
loc_17AAE:
xor	ax, ax
loc_17AB0:
jmp	loc_17D26
loc_17AB3:
mov	ax, [bp+Entry_Index]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
adc	dx, 0
mov	[word ptr bp+Header_Offset+2], dx
mov	[word ptr bp+Header_Offset], ax
push	di
mov	ax, 4
push	ax
push	[word ptr bp+Header_Offset+2]
push	[word ptr bp+Header_Offset]
xor	ax, ax
push	ax
lea	ax, [bp+Entry_Offset]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	di
mov	ax, 4
push	ax
mov	ax, [word ptr bp+Header_Offset+2]
mov	dx, [word ptr bp+Header_Offset]
add	dx, 4
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+Total_Bytes_Left]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [word ptr bp+Entry_Offset+2]
mov	dx, [word ptr bp+Entry_Offset]
sub	[word ptr bp+Total_Bytes_Left],	dx
sbb	[word ptr bp+Total_Bytes_Left+2], ax
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jge	short loc_17B1B
jmp	loc_17D20
loc_17B1B:
jg	short loc_17B26
cmp	[word ptr bp+Total_Bytes_Left],	0
ja	short loc_17B26
jmp	loc_17D20
loc_17B26:
mov	ax, [word ptr bp+Entry_Offset+2]
mov	dx, [word ptr bp+Entry_Offset]
mov	[word ptr bp+Header_Offset+2], ax
mov	[word ptr bp+Header_Offset], dx
push	di
mov	ax, 2
push	ax
push	[word ptr bp+Header_Offset+2]
push	[word ptr bp+Header_Offset]
xor	ax, ax
push	ax
lea	ax, [bp+File_Record_Count]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	di
mov	ax, 2
push	ax
mov	ax, [word ptr bp+Header_Offset+2]
mov	dx, [word ptr bp+Header_Offset]
add	dx, 2
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+File_Record_Size]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
cmp	[bp+File_Record_Size], 0
jz	short loc_17B7C
mov	ax, [bp+RecSize]
cmp	ax, [bp+File_Record_Size]
jz	short loc_17B7F
loc_17B7C:
jmp	loc_17AAE
loc_17B7F:
mov	ax, [bp+FirstRec]
add	ax, [bp+RecCount]
cmp	ax, [bp+File_Record_Count]
jbe	short loc_17B8D
jmp	loc_17AAE
loc_17B8D:
mov	ax, [bp+FirstRec]
cwd
push	ax
mov	ax, [bp+File_Record_Size]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
add	ax, 4
adc	dx, 0
add	[word ptr bp+Entry_Offset], ax
adc	[word ptr bp+Entry_Offset+2], dx
mov	ax, [bp+RecCount]
cwd
push	ax
mov	ax, [bp+File_Record_Size]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
mov	[word ptr bp+Total_Bytes_Left+2], dx
mov	[word ptr bp+Total_Bytes_Left],	ax
xor	ax, ax
mov	dx, 10h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_Left+2]
push	[word ptr bp+Total_Bytes_Left]
call	LDIV@
inc	ax
mov	[bp+Amt_Paragraphs], ax
mov	ax, [bp+Load_Type]
or	ax, ax
jz	short loc_17BED
cmp	ax, 1
jz	short loc_17C05
cmp	ax, 2
jz	short loc_17C42
jmp	loc_17C91
loc_17BED:
push	[bp+Amt_Paragraphs]
call	Allocate_Space_No_Header__EntrySize
pop	cx
mov	[bp+Result_Data_Segment], ax
cmp	[bp+Result_Data_Segment], 0
jnz	short loc_17C02
jmp	loc_17AAE
loc_17C02:
jmp	loc_17C91
loc_17C05:
push	si
call	Check_Allocation
pop	cx
or	ax, ax
jnz	short loc_17C13
jmp	loc_17AAE
loc_17C13:
mov	ax, SAMB.size
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
dec	ax
cmp	ax, [bp+Amt_Paragraphs]
jnb	short loc_17C28
jmp	loc_17AAE
loc_17C28:
mov	ax, si
inc	ax
mov	[bp+Result_Data_Segment], ax
mov	ax, [bp+Amt_Paragraphs]
inc	ax
push	ax
loc_17C33:
mov	ax, SAMB.used
push	ax
push	si
call	farpokew
add	sp, 6
jmp	short loc_17C91
loc_17C42:
or	si, si
jnz	short loc_17C4B
mov	ax, 1
jmp	short loc_17C4D
loc_17C4B:
xor	ax, ax
loc_17C4D:
push	ax
call	Check_Allocation
pop	cx
or	ax, ax
jz	short loc_17C5B
jmp	loc_17AAE
loc_17C5B:
push	si
call	Get_Free_Blocks
pop	cx
cmp	ax, [bp+Amt_Paragraphs]
jnb	short loc_17C6A
jmp	loc_17AAE
loc_17C6A:
mov	ax, SAMB.used
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
mov	dx, si
add	dx, ax
mov	[bp+Result_Data_Segment], dx
mov	ax, SAMB.used
push	ax
push	si
call	farpeekw
pop	cx
pop	cx
mov	dx, [bp+Amt_Paragraphs]
add	dx, ax
push	dx
jmp	short loc_17C33
loc_17C91:
cmp	[bp+Result_Data_Segment], 0
jnz	short loc_17C9A
jmp	loc_17D20
loc_17C9A:
mov	ax, [bp+Result_Data_Segment]
mov	[bp+Target_Seg], ax
mov	[bp+Bytes_To_Copy], 8000h
jmp	short loc_17CD6
loc_17CA7:
sub	[word ptr bp+Total_Bytes_Left],	8000h
sbb	[word ptr bp+Total_Bytes_Left+2], 0
push	di
push	[bp+Bytes_To_Copy]
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
push	[bp+Target_Seg]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
add	[word ptr bp+Entry_Offset], 8000h
adc	[word ptr bp+Entry_Offset+2], 0
add	[bp+Target_Seg], 800h
loc_17CD6:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jg	short loc_17CA7
jnz	short loc_17CE5
cmp	[word ptr bp+Total_Bytes_Left],	8000h
ja	short loc_17CA7
loc_17CE5:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jl	short loc_17D20
jg	short loc_17CF3
cmp	[word ptr bp+Total_Bytes_Left],	0
jbe	short loc_17D20
loc_17CF3:
xor	ax, ax
mov	dx, 8000h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_Left+2]
push	[word ptr bp+Total_Bytes_Left]
call	LMOD@
mov	[bp+Bytes_To_Copy], ax
push	di
push	[bp+Bytes_To_Copy]
loc_17D0C:
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
push	[bp+Target_Seg]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
loc_17D20:
mov	ax, [bp+Result_Data_Segment]
jmp	loc_17AB0
loc_17D26:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_LBX_RecLoader
; Attributes: bp-based frame
proc EMM_LBXR_DirectLoad far
Header_Offset= dword ptr -18h
Total_Bytes_Left= dword	ptr -14h
Entry_Offset= dword ptr	-10h
File_Record_Count= word	ptr -0Ch
File_Record_Size= word ptr -0Ah
Unused_Para_Cnt= word ptr -8
Amt_Bytes= word	ptr -6
File_Entry_Count= word ptr -4
Unused_Local= word ptr -2
Hnd_Name@= word	ptr  6
Entry_Index= word ptr  8
LoadTo@= word ptr  0Ah
FirstRec= word ptr  0Ch
RecCount= word ptr  0Eh
RecSize= word ptr  10h
push	bp
mov	bp, sp
sub	sp, 18h
push	si
push	di
mov	di, [bp+Entry_Index]
mov	[bp+Unused_Local], 0
push	[bp+Hnd_Name@]
nop
push	cs
call	near ptr EMM_CheckHandleOpen
pop	cx
mov	si, ax
or	si, si
jnz	short loc_17D4E
jmp	loc_17EC6
loc_17D4E:
push	si
mov	ax, 2
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
lea	ax, [bp+File_Entry_Count]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
cmp	di, [bp+File_Entry_Count]
jl	short loc_17D72
loc_17D6D:
xor	ax, ax
loc_17D6F:
jmp	loc_17ECF
loc_17D72:
mov	ax, di
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
adc	dx, 0
mov	[word ptr bp+Header_Offset+2], dx
mov	[word ptr bp+Header_Offset], ax
push	si
mov	ax, 4
push	ax
push	[word ptr bp+Header_Offset+2]
push	[word ptr bp+Header_Offset]
xor	ax, ax
push	ax
lea	ax, [bp+Entry_Offset]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	si
mov	ax, 4
push	ax
mov	ax, [word ptr bp+Header_Offset+2]
mov	dx, [word ptr bp+Header_Offset]
add	dx, 4
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+Total_Bytes_Left]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [word ptr bp+Entry_Offset+2]
mov	dx, [word ptr bp+Entry_Offset]
sub	[word ptr bp+Total_Bytes_Left],	dx
sbb	[word ptr bp+Total_Bytes_Left+2], ax
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jge	short loc_17DD9
jmp	loc_17EC4
loc_17DD9:
jg	short loc_17DE4
cmp	[word ptr bp+Total_Bytes_Left],	0
ja	short loc_17DE4
jmp	loc_17EC4
loc_17DE4:
mov	ax, [word ptr bp+Entry_Offset+2]
mov	dx, [word ptr bp+Entry_Offset]
mov	[word ptr bp+Header_Offset+2], ax
mov	[word ptr bp+Header_Offset], dx
push	si
mov	ax, 2
push	ax
push	[word ptr bp+Header_Offset+2]
push	[word ptr bp+Header_Offset]
xor	ax, ax
push	ax
lea	ax, [bp+File_Record_Count]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	si
mov	ax, 2
push	ax
mov	ax, [word ptr bp+Header_Offset+2]
mov	dx, [word ptr bp+Header_Offset]
add	dx, 2
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+File_Record_Size]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
cmp	[bp+File_Record_Size], 0
jz	short loc_17E3A
mov	ax, [bp+RecSize]
cmp	ax, [bp+File_Record_Size]
jz	short loc_17E3D
loc_17E3A:
jmp	loc_17D6D
loc_17E3D:
mov	ax, [bp+FirstRec]
add	ax, [bp+RecCount]
cmp	ax, [bp+File_Record_Count]
jbe	short loc_17E4B
jmp	loc_17D6D
loc_17E4B:
mov	ax, [bp+FirstRec]
cwd
push	ax
mov	ax, [bp+File_Record_Size]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
add	ax, 4
adc	dx, 0
add	[word ptr bp+Entry_Offset], ax
adc	[word ptr bp+Entry_Offset+2], dx
mov	ax, [bp+RecCount]
cwd
push	ax
mov	ax, [bp+File_Record_Size]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
mov	[word ptr bp+Total_Bytes_Left+2], dx
mov	[word ptr bp+Total_Bytes_Left],	ax
xor	ax, ax
mov	dx, 10h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_Left+2]
push	[word ptr bp+Total_Bytes_Left]
call	LDIV@
inc	ax
mov	[bp+Unused_Para_Cnt], ax
xor	ax, ax
mov	dx, 8000h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_Left+2]
push	[word ptr bp+Total_Bytes_Left]
call	LMOD@
mov	[bp+Amt_Bytes],	ax
push	si
push	[bp+Amt_Bytes]
push	[word ptr bp+Entry_Offset+2]
push	[word ptr bp+Entry_Offset]
xor	ax, ax
push	ax
push	[bp+LoadTo@]
call	EMM_MapnRead
add	sp, 0Ch
loc_17EC4:
jmp	short loc_17EC9
loc_17EC6:
jmp	loc_17D6D
loc_17EC9:
mov	ax, -1
jmp	loc_17D6F
loc_17ECF:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_LBXR_DirectLoad
; Attributes: bp-based frame
proc EMM_CheckHandleOpen far
EMM_HandleName@= word ptr  6
push	bp
mov	bp, sp
push	si
push	di
xor	di, di
xor	si, si
jmp	short loc_17F18
loc_17EE0:
push	[bp+EMM_HandleName@]
mov	ax, si
mov	dx, 0Ch
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short loc_17F17
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
mov	di, [EMM_Table.handle+bx]
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
mov	al, [EMM_Table.reserved+bx]
cbw
mov	[g_EmmRsvd], ax
loc_17F17:
inc	si
loc_17F18:
cmp	si, [EMM_Open_Handles]
jge	short loc_17F22
or	di, di
jz	short loc_17EE0
loc_17F22:
mov	ax, di
jmp	short $+2
pop	di
pop	si
pop	bp
retf
endp EMM_CheckHandleOpen
; Attributes: bp-based frame
proc EMM_Release_All far
_SI_itr	= si
push	bp
mov	bp, sp
push	_SI_itr
xor	_SI_itr, _SI_itr
jmp	short loc_17F46
loc_17F32:
mov	ax, _SI_itr
mov	dx, size s_EMM_RECORD
imul	dx
mov	bx, ax
push	[EMM_Table.handle+bx]
call	EMM_ReleaseHandle__STUB
pop	cx
inc	_SI_itr
loc_17F46:
cmp	_SI_itr, [EMM_Open_Handles]
jl	short loc_17F32
cmp	[g_EmmHndl_OVERXYZ], 0
jz	short loc_17F5D
push	[g_EmmHndl_OVERXYZ]
call	EMM_ReleaseHandle__STUB
pop	cx
loc_17F5D:
pop	_SI_itr
pop	bp
retf
endp EMM_Release_All
; Attributes: bp-based frame
proc EMM_Release_Resd far
push	bp
mov	bp, sp
push	si
xor	si, si
jmp	short loc_17F8C
loc_17F68:
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
cmp	[EMM_Table.reserved+bx], 1
jnz	short loc_17F8B
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
push	[EMM_Table.handle+bx]
call	EMM_ReleaseHandle__STUB
pop	cx
loc_17F8B:
inc	si
loc_17F8C:
cmp	si, [EMM_Open_Handles]
jl	short loc_17F68
pop	si
pop	bp
locret_17F94:
retf
endp EMM_Release_Resd
; Attributes: bp-based frame
proc UU_EMM_ReleaseHnd far
Handle_Name= byte ptr -32h
File_Name= byte	ptr -14h
Name@= word ptr	 6
push	bp
mov	bp, sp
sub	sp, 32h
push	si
push	di
push	[bp+Name@]
lea	ax, [bp+Handle_Name]
push	ax
call	strcpy
pop	cx
pop	cx
lea	ax, [bp+Handle_Name]
push	ax
call	File_Name_Base
pop	cx
lea	ax, [bp+Handle_Name]
push	ax
push	cs
call	near ptr EMM_CheckHandleOpen
pop	cx
mov	di, ax
lea	ax, [bp+Handle_Name]
push	ax
lea	ax, [bp+File_Name]
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, offset str_LBX_extension__seg013
push	ax
lea	ax, [bp+File_Name]
push	ax
call	strcat
pop	cx
pop	cx
or	di, di
jnz	short loc_17FE5
jmp	loc_1807A
loc_17FE5:
lea	ax, [bp+Handle_Name]
push	ax
lea	ax, [bp+File_Name]
push	ax
call	DIR
pop	cx
pop	cx
or	ax, ax
jnz	short loc_17FFB
jmp	loc_1807A
loc_17FFB:
cmp	[g_EmmRsvd], 1
jnz	short loc_1800D
push	di
call	EMM_GetPageCount
pop	cx
add	[EMM_Pages_Reserved], ax
loc_1800D:
push	di
call	EMM_ReleaseHandle__STUB
pop	cx
mov	si, di
jmp	short loc_1806E
loc_18018:
mov	ax, si
mov	dx, 0Ch
imul	dx
add	ax, (offset EMM_Table.name+0Ch)
push	ax
mov	ax, si
mov	dx, 0Ch
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
mov	ax, [(EMM_Table.handle+0Ch)+bx]
push	ax
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
pop	ax
mov	[EMM_Table.handle+bx], ax
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
mov	al, [(EMM_Table.reserved+0Ch)+bx]
push	ax
mov	ax, si
mov	dx, 0Ch
imul	dx
mov	bx, ax
pop	ax
mov	[EMM_Table.reserved+bx], al
inc	si
loc_1806E:
mov	ax, [EMM_Open_Handles]
dec	ax
cmp	ax, si
jg	short loc_18018
dec	[EMM_Open_Handles]
loc_1807A:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp UU_EMM_ReleaseHnd
; Attributes: bp-based frame
proc UU_EMM_Available far
push	bp
mov	bp, sp
mov	ax, [EMM_OK]
jmp	short $+2
pop	bp
retf
endp UU_EMM_Available
; Attributes: bp-based frame
proc EMM_GetHandle far
buffer=	byte ptr -0Ah
handle=	word ptr -4
itr= word ptr -2
page_count= word ptr  6
handle_name= word ptr  8
reserved_flag= word ptr	 0Ah
_DI_EmmHndlNbr = di
_SI_handle_name	= si
push	bp
mov	bp, sp
sub	sp, 0Ah
push	_SI_handle_name
push	_DI_EmmHndlNbr
mov	_SI_handle_name, [bp+handle_name]
cmp	[byte ptr _SI_handle_name], 0
jnz	short loc_1809F
@@Done__Return_ZERO:
xor	ax, ax
@@JmpDone:
jmp	@@Done
loc_1809F:
mov	[bp+handle], e_ST_UNDEFINED
mov	[bp+itr], 0
jmp	short loc_180CC
loc_180AB:
mov	ax, [bp+itr]
mov	dx, 12
imul	dx
add	ax, offset EMM_Table.name
push	ax
push	_SI_handle_name
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short loc_180C9
mov	ax, [bp+itr]
mov	[bp+handle], ax
loc_180C9:
inc	[bp+itr]
loc_180CC:
mov	ax, [bp+itr]
cmp	ax, [EMM_Open_Handles]
jl	short loc_180AB
cmp	[bp+handle], e_ST_UNDEFINED
jz	short loc_180DE
jmp	loc_181ED
loc_180DE:
push	_SI_handle_name
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
cmp	[bp+reserved_flag], e_ST_TRUE
jnz	short loc_18118
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
mov	bx, ax
mov	al, [byte ptr EMM_Table.name+bx]
neg	al
push	ax
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
mov	bx, ax
pop	ax
mov	[byte ptr EMM_Table.name+bx], al
loc_18118:
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
add	ax, offset EMM_Table
push	ax
push	[bp+page_count]
call	EMM_MakeNamedHandle
pop	cx
pop	cx
mov	_DI_EmmHndlNbr,	ax
or	_DI_EmmHndlNbr,	_DI_EmmHndlNbr
jnz	short loc_18137
jmp	loc_181EA
loc_18137:
push	_SI_handle_name
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
mov	dl, [byte ptr bp+reserved_flag]
mov	bx, ax
mov	[EMM_Table.reserved+bx], dl
mov	ax, [EMM_Open_Handles]
mov	dx, 12
imul	dx
mov	bx, ax
mov	[EMM_Table.handle+bx], _DI_EmmHndlNbr
inc	[EMM_Open_Handles]
cmp	[bp+reserved_flag], e_ST_TRUE
jnz	short loc_1817B
mov	ax, [bp+page_count]
sub	[EMM_Pages_Reserved], ax
loc_1817B:
cmp	[EMM_Pages_Reserved], 0
jge	short loc_181EA
mov	ax, 10
push	ax
lea	ax, [bp+buffer]
push	ax
mov	ax, [EMM_Pages_Reserved]
neg	ax
push	ax
call	itoa
add	sp, 6
mov	ax, offset cnst_EMMErr_ResOut1
push	ax
mov	ax, offset near_buffer
push	ax
call	strcpy
pop	cx
pop	cx
lea	ax, [bp+buffer]
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset cnst_EMMErr_ResOut2
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
push	_SI_handle_name
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset cnst_EMMErr_ResOut3
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset near_buffer
push	ax
call	Exit_With_Message
pop	cx
loc_181EA:
jmp	@@Done__Return_EmmHndlNbr
loc_181ED:
mov	ax, [bp+handle]
mov	dx, 12
imul	dx
mov	bx, ax
mov	_DI_EmmHndlNbr,	[EMM_Table.handle+bx]
push	_DI_EmmHndlNbr
call	EMM_GetPageCount
pop	cx
cmp	ax, [bp+page_count]
jl	short loc_1820A
jmp	@@Done__Return_EmmHndlNbr
loc_1820A:
mov	ax, 10
push	ax
mov	ax, offset Tmp_Conv_Str_1
push	ax
mov	ax, [bp+handle]
mov	dx, 12
imul	dx
mov	bx, ax
push	[EMM_Table.handle+bx]
call	itoa
add	sp, 6
mov	ax, 10
push	ax
mov	ax, offset Tmp_Conv_Str_2
push	ax
push	_DI_EmmHndlNbr
call	itoa
add	sp, 6
mov	ax, 10
push	ax
mov	ax, offset Tmp_Conv_Str_3
push	ax
push	[bp+page_count]
call	itoa
add	sp, 6
push	_SI_handle_name
mov	ax, offset near_buffer
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, offset cnst_EMMErr_Reload1
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset Tmp_Conv_Str_1
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset cnst_EMMErr_Space
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset Tmp_Conv_Str_2
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset cnst_EMMErr_Space
push	ax
mov	ax, offset near_buffer
push	ax
call	strcat
pop	cx
pop	cx
mov	ax, offset Tmp_Conv_Str_3
push	ax
loc_182A7:
mov	ax, offset near_buffer
push	ax
loc_182AB:
call	strcat
pop	cx
pop	cx
mov	ax, offset near_buffer
push	ax
call	Exit_With_Message
pop	cx
@@Done__Return_EmmHndlNbr:
mov	ax, _DI_EmmHndlNbr
jmp	@@JmpDone
@@Done:
pop	_DI_EmmHndlNbr
pop	_SI_handle_name
mov	sp, bp
pop	bp
retf
endp EMM_GetHandle
; Attributes: bp-based frame
proc RP_EMM_VGAWrite_fP5 far
Source_Seg= word ptr  6
push	bp
mov	bp, sp
push	[_VGAFILEH_seg]
mov	ax, 16384
push	ax
mov	ax, 1
xor	dx, dx
push	ax
push	dx
push	[bp+Source_Seg]
xor	ax, ax
push	ax
call	EMM_MapnWrite
add	sp, 0Ch
pop	bp
retf
endp RP_EMM_VGAWrite_fP5
; Attributes: bp-based frame
proc RP_EMM_VGARead_fP5	far
Target_Seg= word ptr  6
push	bp
mov	bp, sp
push	[_VGAFILEH_seg]
mov	ax, 4000h
push	ax
mov	ax, 1
xor	dx, dx
push	ax
push	dx
push	[bp+Target_Seg]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
pop	bp
retf
endp RP_EMM_VGARead_fP5
; Attributes: bp-based frame
proc RP_EMM_VGAWrite_P5	far
Source_Seg= word ptr -2
Source_Offset= word ptr	 6
Amt_Bytes= word	ptr  8
push	bp
mov	bp, sp
sub	sp, 2
mov	ax, ds
mov	[bp+Source_Seg], ax
push	[_VGAFILEH_seg]
push	[bp+Amt_Bytes]
mov	ax, 1
xor	dx, dx
push	ax
push	dx
push	[bp+Source_Seg]
push	[bp+Source_Offset]
call	EMM_MapnWrite
add	sp, 0Ch
mov	sp, bp
pop	bp
retf
endp RP_EMM_VGAWrite_P5
; Attributes: bp-based frame
proc RP_EMM_VGARead_P5 far
Target_Seg= word ptr -2
Target_Off= word ptr  6
Amt_Bytes= word	ptr  8
push	bp
mov	bp, sp
sub	sp, 2
mov	ax, ds
mov	[bp+Target_Seg], ax
push	[_VGAFILEH_seg]
push	[bp+Amt_Bytes]
mov	ax, 1
xor	dx, dx
push	ax
push	dx
push	[bp+Target_Seg]
push	[bp+Target_Off]
call	EMM_MapnRead
add	sp, 0Ch
mov	sp, bp
pop	bp
retf
endp RP_EMM_VGARead_P5
; Attributes: bp-based frame
proc EMMDATAH_Map far
push	bp
mov	bp, sp
xor	ax, ax
push	ax
push	[_EMMDATAH_seg]
call	EMM_MapFourPages
pop	cx
pop	cx
pop	bp
retf
endp EMMDATAH_Map
; Attributes: bp-based frame
proc EMM_EMMDATAH_AllocFirst far
Free_EMM_Data_Paras= word ptr -2
Amt_Paragraphs=	word ptr  6
push	bp
mov	bp, sp
sub	sp, 2
push	si
push	di
mov	si, [bp+Amt_Paragraphs]
mov	[EMMDATAH_offset], 0
mov	ax, [EMS_PFBA]
add	ax, [EMMDATAH_offset]
mov	di, ax
push	cs
call	near ptr EMMDATAH_Map
mov	ax, si
inc	ax
mov	si, ax
mov	ax, 4090
sub	ax, [EMMDATAH_offset]
mov	[bp+Free_EMM_Data_Paras], ax
cmp	[bp+Free_EMM_Data_Paras], si
jge	short loc_183B7
mov	ax, si
sub	ax, [bp+Free_EMM_Data_Paras]
push	ax
mov	ax, Alloc_EMM_too_small
push	ax
call	Allocation_Error
db  59h
db  59h
loc_183B7:
mov	ax, SA_MEMSIG1
push	ax
mov	ax, SAMB.MemSig1
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, SA_MEMSIG2
push	ax
mov	ax, SAMB.MemSig2
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
dec	ax
push	ax
mov	ax, SAMB.size
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, SAMB.used
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, SAMB.mark
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
inc	ax
add	[EMMDATAH_offset], ax
mov	ax, di
inc	ax
jmp	short $+2
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_EMMDATAH_AllocFirst
; Attributes: bp-based frame
proc EMM_EMMDATAH_AllocNext far
Free_EMM_Data_Paras= word ptr -2
Amt_Paragraphs=	word ptr  6
push	bp
mov	bp, sp
sub	sp, 2
push	si
push	di
mov	si, [bp+Amt_Paragraphs]
push	cs
call	near ptr EMMDATAH_Map
mov	ax, [EMS_PFBA]
add	ax, [EMMDATAH_offset]
mov	di, ax
push	cs
call	near ptr EMMDATAH_Map
mov	ax, si
inc	ax
mov	si, ax
mov	ax, 0FFAh
sub	ax, [EMMDATAH_offset]
mov	[bp+Free_EMM_Data_Paras], ax
cmp	[bp+Free_EMM_Data_Paras], si
jge	short loc_1845F
mov	ax, si
sub	ax, [bp+Free_EMM_Data_Paras]
push	ax
mov	ax, Alloc_EMM_too_small
push	ax
call	Allocation_Error
db  59h
db  59h
loc_1845F:
mov	ax, SA_MEMSIG1
push	ax
mov	ax, SAMB.MemSig1
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, SA_MEMSIG2
push	ax
mov	ax, SAMB.MemSig2
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
dec	ax
push	ax
mov	ax, SAMB.size
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, SAMB.used
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, SAMB.mark
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
inc	ax
add	[EMMDATAH_offset], ax
mov	ax, di
inc	ax
jmp	short $+2
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_EMMDATAH_AllocNext
; Attributes: bp-based frame
proc EMMDATAH_Save_Last_Offset far
push	bp
mov	bp, sp
mov	ax, [EMMDATAH_offset]
mov	[EMMDATAH_offset], ax
pop	bp
retf
endp EMMDATAH_Save_Last_Offset
; Attributes: bp-based frame
proc EMMDATAH_Restore_Last_Offset far
push	bp
mov	bp, sp
mov	ax, [EMMDATAH_offset]
mov	[EMMDATAH_offset], ax
pop	bp
retf
endp EMMDATAH_Restore_Last_Offset
; Attributes: bp-based frame
proc UU_EMM_LBX_Load2Hnd far
File_Name= byte	ptr -20h
Handle_Page_Count= word	ptr -12h
EMM_Target_Offset= dword ptr -10h
Total_Bytes_left= dword	ptr -0Ch
First_Log_Page=	word ptr -8
Bytes_To_Copy= word ptr	-6
Buffer_Seg= word ptr -4
Needed_Pages= word ptr -2
FileName@= word	ptr  6
EMM_Handle= word ptr  8
push	bp
mov	bp, sp
sub	sp, 20h
push	si
push	di
push	[bp+FileName@]
lea	ax, [bp+File_Name]
push	ax
call	strcpy
pop	cx
pop	cx
xor	si, si
jmp	short loc_18501
loc_184F6:
cmp	[bp+si+File_Name], 2Eh
jnz	short loc_18500
mov	[bp+si+File_Name], 0
loc_18500:
inc	si
loc_18501:
cmp	si, 0Dh
jl	short loc_184F6
mov	ax, offset str_LBX_extension__seg013
push	ax
lea	ax, [bp+File_Name]
push	ax
call	strcat
pop	cx
pop	cx
lea	ax, [bp+File_Name]
push	ax
call	LOF
pop	cx
mov	[word ptr bp+Total_Bytes_left+2], dx
mov	[word ptr bp+Total_Bytes_left],	ax
mov	ax, [word ptr bp+Total_Bytes_left]
or	ax, [word ptr bp+Total_Bytes_left+2]
jnz	short loc_1853F
xor	ax, ax
push	ax
mov	ax, le_EM_swap_fail
push	ax
push	[bp+FileName@]
call	Error_Handler
db  83h
db 0C4h
db    6
loc_1853F:
xor	ax, ax
mov	dx, 4000h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_left+2]
push	[word ptr bp+Total_Bytes_left]
call	LDIV@
mov	[bp+Needed_Pages], ax
xor	ax, ax
mov	dx, 4000h
push	ax
push	dx
push	[word ptr bp+Total_Bytes_left+2]
push	[word ptr bp+Total_Bytes_left]
call	LMOD@
or	ax, dx
jz	short loc_1856D
inc	[bp+Needed_Pages]
loc_1856D:
mov	ah, 4Ch
mov	dx, [bp+EMM_Handle]
loc_18572:
int	67h
mov	[bp+Handle_Page_Count],	bx
mov	ax, [bp+Needed_Pages]
cmp	ax, [bp+Handle_Page_Count]
jbe	short loc_18589
mov	ax, offset str_LBX_File_Too_Long
push	ax
call	Exit_With_Message
pop	cx
loc_18589:
call	EMM_GetPageFrame
mov	[bp+Buffer_Seg], ax
mov	[bp+First_Log_Page], 0
cmp	[bp+Buffer_Seg], 0
jnz	short loc_1859F
jmp	loc_18639
loc_1859F:
push	[bp+FileName@]
loc_185A2:
call	lbx_open
pop	cx
mov	di, ax
or	di, di
jnz	short loc_185B1
jmp	loc_18639
loc_185B1:
mov	[word ptr bp+EMM_Target_Offset+2], 0
mov	[word ptr bp+EMM_Target_Offset], 0
mov	[bp+Bytes_To_Copy], 4000h
loc_185C0:
jmp	short loc_185F3
loc_185C2:
sub	[word ptr bp+Total_Bytes_left],	4000h
sbb	[word ptr bp+Total_Bytes_left+2], 0
push	[bp+First_Log_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	di
loc_185D9:
push	[bp+Bytes_To_Copy]
push	[bp+Buffer_Seg]
call	lbx_read_sgmt
add	sp, 6
inc	[bp+First_Log_Page]
add	[word ptr bp+EMM_Target_Offset], 4000h
adc	[word ptr bp+EMM_Target_Offset+2], 0
loc_185F3:
cmp	[word ptr bp+Total_Bytes_left+2], 0
jg	short loc_185C2
jl	short loc_18602
cmp	[word ptr bp+Total_Bytes_left],	4000h
jnb	short loc_185C2
loc_18602:
cmp	[word ptr bp+Total_Bytes_left+2], 0
jl	short loc_18632
jg	short loc_18610
cmp	[word ptr bp+Total_Bytes_left],	0
loc_1860E:
jbe	short loc_18632
loc_18610:
mov	ax, [word ptr bp+Total_Bytes_left]
mov	[bp+Bytes_To_Copy], ax
push	[bp+First_Log_Page]
push	[bp+EMM_Handle]
loc_1861C:
call	EMM_MapFourPages
pop	cx
pop	cx
push	di
loc_18624:
push	[bp+Bytes_To_Copy]
push	[bp+Buffer_Seg]
call	lbx_read_sgmt
add	sp, 6
loc_18632:
push	di
loc_18633:
call	lbx_close
pop	cx
loc_18639:
pop	di
pop	si
loc_1863B:
mov	sp, bp
pop	bp
retf
endp UU_EMM_LBX_Load2Hnd
; Attributes: bp-based frame
proc EMM_Set_Minimum far
amount=	word ptr  6
push	bp
mov	bp, sp
mov	ax, [bp+amount]
mov	[m_min_expanded_memory], ax
pop	bp
retf
endp EMM_Set_Minimum
; Attributes: bp-based frame
proc EMM_Error_Message far
temp= byte ptr -14h
string=	word ptr  6
_SI_string = si
push	bp
loc_1864B:
mov	bp, sp
sub	sp, 14h
push	_SI_string
mov	_SI_string, [bp+string]
mov	ax, 10
push	ax
lea	ax, [bp+temp]
push	ax
push	[m_min_expanded_memory]
call	itoa
add	sp, 6
mov	ax, offset cnst_EMMErr_TooLow1
push	ax
push	_SI_string
call	strcpy
pop	cx
pop	cx
lea	ax, [bp+temp]
push	ax
push	_SI_string
call	strcat
loc_1867E:
pop	cx
pop	cx
loc_18680:
mov	ax, offset cnst_EMMErr_TooLow2
push	ax
loc_18684:
push	_SI_string
loc_18685:
call	strcat
loc_1868A:
pop	cx
pop	cx
loc_1868C:
pop	_SI_string
loc_1868D:
mov	sp, bp
pop	bp
locret_18690:
retf
endp EMM_Error_Message
ends seg013
