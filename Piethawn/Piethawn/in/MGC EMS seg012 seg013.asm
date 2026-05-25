
; Segment type:	Pure code
segment	seg012 byte public 'CODE' use16
assume cs:seg012
;org 6
assume es:nothing, ss:nothing, ds:dseg,	fs:nothing, gs:nothing
EMM_Logical_Page dw 0
proc EMM_DetectDevice far
push	si
loc_16179:
push	di
loc_1617A:
push	es
loc_1617B:
push	ds
loc_1617C:
mov	dx, offset g_EMM_device_name
mov	ax, 3D00h
int	21h
jb	short @@Failure
mov	bx, ax
loc_16188:
mov	ax, 4400h
int	21h
jb	short @@Failure
loc_1618F:
and	dx, 10000000b
jz	short @@Failure
mov	ax, 4407h
int	21h
jb	short @@Failure
or	al, al
jz	short @@Failure
mov	ah, 3Eh
int	21h
jb	short @@Failure
mov	ah, 46h
int	67h
cmp	ah, 0
jnz	short @@Failure
cmp	al, 40h
js	short @@Failure
mov	ah, 41h
int	67h
cmp	ah, 0
jnz	short @@Failure
@@Success:
mov	[sa_EMM_PFBA], bx
mov	ax, 0FFFFh
pop	ds
pop	es
pop	di
pop	si
retf
@@Failure:
mov	ax, 0
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
jnz	short @@GotErrorCode
mov	ax, bx
pop	di
pop	si
pop	ds
pop	es
retf
@@GotErrorCode:
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
jnz	short @@Failure
@@Success:
mov	ax, bx
pop	di
pop	si
pop	ds
pop	es
retf
@@Failure:
mov	ax, 0
pop	di
pop	si
pop	ds
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
cmp	ah, 0
loc_16219:
jnz	short loc_16223
mov	ax, bx
pop	di
pop	si
pop	ds
pop	es
pop	bp
retf
loc_16223:
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
jnz	short @@error
mov	[bp+handle], dx
mov	ax, 5301h
mov	si, [bp+name]
int	67h
cmp	ah, 0
jnz	short @@error
mov	ax, [bp+handle]
pop	di
pop	si
pop	ds
pop	es
mov	sp, bp
pop	bp
retf
@@error:
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
argEmsHandleName= word ptr  6
argEmsHandleNumber= word ptr  8
push	bp
mov	bp, sp
push	es
push	ds
push	si
push	di
mov	ax, 5300h
mov	dx, [bp+argEmsHandleNumber]
mov	di, [bp+argEmsHandleName]
mov	bx, ds
mov	es, bx
assume es:dseg
int	67h
cmp	ah, 0
jnz	short @@error
mov	di, [bp+argEmsHandleName]
add	di, 8
mov	al, 0
stosb
mov	ax, 0FFFFh
pop	di
pop	si
pop	ds
pop	es
assume es:nothing
pop	bp
retf
@@error:
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
argEmmHandle= word ptr	6
push	bp
mov	bp, sp
push	es
push	ds
push	si
push	di
cmp	[bp+argEmmHandle], 0
jz	short loc_162B2
mov	ah, 45h
mov	dx, [bp+argEmmHandle]
int	67h
loc_162B2:
pop	di
pop	si
pop	ds
pop	es
pop	bp
retf
endp EMM_ReleaseHandle__STUB
; Attributes: bp-based frame
proc EMM_MapnRead far
Dst_Ofst= word ptr  6
Dst_Sgmt= word ptr  8
Src_Ofst= word ptr  0Ah
Src_Sgmt= word ptr  0Ch
nbytes=	word ptr  0Eh
EMM_Handle= word ptr  10h
push	bp
mov	bp, sp
sub	sp, 2
push	es
push	ds
push	si
push	di
cmp	[bp+Dst_Sgmt], 0
jnz	short loc_162CD
mov	ax, ds
mov	[bp+Dst_Sgmt], ax
loc_162CD:
mov	ax, [bp+Src_Ofst]
mov	dx, [bp+Src_Sgmt]
shl	ax, 1
rcl	dx, 1
shl	ax, 1
rcl	dx, 1
mov	bx, dx
mov	[cs:EMM_Logical_Page], bx
shr	ax, 1
shr	ax, 1
mov	si, ax
mov	ax, 4400h
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4401h
mov	bx, [cs:EMM_Logical_Page]
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4402h
mov	bx, [cs:EMM_Logical_Page]
inc	bx
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, 4403h
mov	bx, [cs:EMM_Logical_Page]
inc	bx
inc	bx
inc	bx
mov	dx, [bp+EMM_Handle]
int	67h
mov	ax, [bp+Dst_Sgmt]
mov	es, ax
mov	di, [bp+Dst_Ofst]
mov	cx, [bp+nbytes]
mov	ax, [sa_EMM_PFBA]
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
Src_Ofst= word ptr  6
Src_Sgmt= word ptr  8
Dst_Ofst= word ptr  0Ah
Dst_Sgmt= word ptr  0Ch
nbytes=	word ptr  0Eh
EmmHndl= word ptr  10h
push	bp
mov	bp, sp
sub	sp, 2
push	es
push	ds
push	si
push	di
cmp	[bp+Src_Sgmt], 0
jnz	short @@DoIt
mov	ax, ds
mov	[bp+Src_Sgmt], ax
@@DoIt:
mov	ax, [bp+Dst_Ofst]
mov	dx, [bp+Dst_Sgmt]
shl	ax, 1
rcl	dx, 1
shl	ax, 1
rcl	dx, 1
mov	bx, dx
mov	[cs:EMM_Logical_Page], bx
shr	ax, 1
shr	ax, 1
mov	di, ax
mov	ax, 4400h
mov	dx, [bp+EmmHndl]
int	67h
mov	ax, 4401h
mov	bx, [cs:EMM_Logical_Page]
inc	bx
mov	dx, [bp+EmmHndl]
int	67h
mov	ax, 4402h
mov	bx, [cs:EMM_Logical_Page]
inc	bx
inc	bx
mov	dx, [bp+EmmHndl]
int	67h
mov	ax, 4403h
mov	bx, [cs:EMM_Logical_Page]
inc	bx
inc	bx
inc	bx
mov	dx, [bp+EmmHndl]
int	67h
mov	ax, [sa_EMM_PFBA]
mov	es, ax
mov	cx, [bp+nbytes]
loc_163A4:
mov	si, [bp+Src_Ofst]
mov	ax, [bp+Src_Sgmt]
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
mov	ax, seg	dseg
mov	ds, ax
mov	ax, [sa_EMM_PFBA]
pop	ds
retf
endp EMM_GetPageFrame
; Attributes: bp-based frame
proc EMM_MapFourPages far
EmmHandle= word	ptr  6
EmmLogicalPage=	word ptr  8
push	bp
mov	bp, sp
push	es
push	ds
mov	ax, 4400h
mov	dx, [bp+EmmHandle]
mov	bx, [bp+EmmLogicalPage]
int	67h
mov	ax, 4401h
mov	bx, [bp+EmmLogicalPage]
inc	bx
mov	dx, [bp+EmmHandle]
int	67h
mov	ax, 4402h
mov	bx, [bp+EmmLogicalPage]
inc	bx
inc	bx
mov	dx, [bp+EmmHandle]
loc_163E8:
int	67h
mov	ax, 4403h
mov	bx, [bp+EmmLogicalPage]
inc	bx
inc	bx
inc	bx
mov	dx, [bp+EmmHandle]
int	67h
pop	ds
pop	es
pop	bp
retf
endp EMM_MapFourPages
; Attributes: bp-based frame
proc EMM_MapMulti4 far
argFirstLogicalPage= word ptr  6
argEmsHandle= word ptr	8
push	bp
mov	bp, sp
push	ax
push	bx
push	cx
push	dx
push	si
push	ds
loc_16405:
mov	bx, [bp+argFirstLogicalPage]
loc_16408:
mov	dx, [bp+argEmsHandle]
mov	ax, seg	dseg
loc_1640E:
mov	ds, ax
mov	[word ptr g_EMM_Log2Phys_Map], bx
inc	bx
mov	[word ptr g_EMM_Log2Phys_Map+4], bx
inc	bx
loc_1641A:
mov	[word ptr g_EMM_Log2Phys_Map+8], bx
loc_1641E:
inc	bx
loc_1641F:
mov	[word ptr g_EMM_Log2Phys_Map+0Ch], bx
inc	bx
loc_16424:
mov	cx, 4
loc_16427:
mov	si, offset g_EMM_Log2Phys_Map
loc_1642A:
mov	ax, 5000h
loc_1642D:
int	67h
pop	ds
loc_16430:
pop	si
pop	dx
loc_16432:
pop	cx
pop	bx
loc_16434:
pop	ax
pop	bp
locret_16436:
retf
endp EMM_MapMulti4
ends seg012
; Segment type:	Pure code
segment	seg013 byte public 'CODE' use16
assume cs:seg013
;org 7
assume es:nothing, ss:nothing, ds:dseg,	fs:nothing, gs:nothing
; Attributes: bp-based frame
proc EMS_Startup far
buffer=	byte ptr -66h
EMM_Open_Handle_Count= word ptr	-2
push	bp
loc_16438:
mov	bp, sp
loc_1643A:
sub	sp, 66h
push	si
call	EMM_DetectDevice
or	ax, ax
jnz	short @@EMM_Detected
jmp	@@EMM_NotPresent
@@EMM_Detected:
call	EMM_GetActiveHandleCount
mov	[bp+EMM_Open_Handle_Count], ax
mov	[EmmHndlNbr_YOMOMA], 0
mov	si, 1
jmp	short @@LoopActiveHandles
@@GetHandleName:
push	si
lea	ax, [bp+buffer]
push	ax
call	EMM_GetHandleName
pop	cx
pop	cx
or	ax, ax
jz	short @@NextHandle
@@GotHandleName:
lea	ax, [bp+buffer]
push	ax
mov	ax, offset g_EmmHndlNm_YOMOMA1
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short @@NextHandle
@@FoundYomomaHndlName:
mov	[EmmHndlNbr_YOMOMA], si
@@NextHandle:
inc	si
@@LoopActiveHandles:
cmp	si, [bp+EMM_Open_Handle_Count]
jge	short @@DoOurEmmHandles
cmp	[EmmHndlNbr_YOMOMA], 0
jz	short @@GetHandleName
@@DoOurEmmHandles:
mov	[EMM_Open_Handles], 0
cmp	[EmmHndlNbr_YOMOMA], 0
jnz	short @@HadFoundYomomaHndlNm
@@NeedYomomaHndl:
xor	ax, ax
push	ax
mov	ax, offset g_EmmHndlNm_YOMOMA2
push	ax
mov	ax, 1
push	ax
nop
push	cs
call	near ptr EMM_GetHandle
add	sp, 6
mov	[EmmHndlNbr_YOMOMA], ax
jmp	short @@DoneYomomaHndl
@@HadFoundYomomaHndlNm:
mov	ax, offset g_EmmHndlNm_YOMOMA1
push	ax
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	[EMM_Table.reserved+bx], 0
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	dx, [EmmHndlNbr_YOMOMA]
mov	bx, ax
mov	[EMM_Table.handle+bx], dx
mov	[EMM_Open_Handles], 1
@@DoneYomomaHndl:
cmp	[EmmHndlNbr_YOMOMA], 0
jg	short @@Success_CrtHndl_Yomoma
@@Failure_CrtHndl_Yomoma:
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
@@Success_CrtHndl_Yomoma:
mov	[g_EMM_OK], 1
mov	ax, [EmmHndlNbr_YOMOMA]
inc	ax
mov	si, ax
jmp	short @@Loop50Tries
@@CheckForHandleName:
push	si
lea	ax, [bp+buffer]
push	ax
call	EMM_GetHandleName
pop	cx
pop	cx
or	ax, ax
jz	short @@NextHandle2
cmp	[bp+buffer], 0
jz	short @@NextHandle2
@@AddHandleToTable:
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
@@NextHandle2:
inc	si
@@Loop50Tries:
cmp	si, 50
jge	short @@CheckEnoughPages
cmp	[EMM_Open_Handles], 40
jl	short @@CheckForHandleName
@@CheckEnoughPages:
call	EMM_GetFreePageCnt
cmp	ax, [EMM_Pages_Reserved]
jge	short @@DoBaseGameAllocs
@@ErrorOutOfPages:
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
@@DoBaseGameAllocs:
mov	ax, 1
push	ax
mov	ax, offset str_VGAFILEH__seg013
push	ax
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
mov	[g_EMMDATAH_Level], 0
jmp	short @@Done
@@EMM_NotPresent:
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
@@Done:
pop	si
mov	sp, bp
pop	bp
retf
endp EMS_Startup
; Attributes: bp-based frame
proc EMM_Load_LBX_File_0 far
LBX_Name= word ptr  6
push	bp
mov	bp, sp
xor	ax, ax
push	ax
push	[bp+LBX_Name]
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
argLbxFileName=	word ptr  6
push	bp
mov	bp, sp
mov	ax, 1
push	ax
push	[bp+argLbxFileName]
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
varEmmHndlNm= byte ptr -3Eh
varEmmHndlFileName= byte ptr -2Ah
UU_varNbytesRead= dword	ptr -16h
varLbxFileSize=	dword ptr -12h
varEmmLogicalPage= word	ptr -0Eh
varReadNbytes= word ptr	-0Ch
varTmpEmmPageFrameSgmtAddr= word ptr -0Ah
varLbxFileSize16kBlocks= word ptr -8
Handle_Exists= word ptr	-6
varEmmHndlNbr= word ptr	-4
varLbxFileHandle= word ptr -2
argLbxFileName=	word ptr  6
argEmmRsvdFlag=	word ptr  8
push	bp
mov	bp, sp
sub	sp, 3Eh
push	si
push	di
push	[bp+argLbxFileName]
call	File_Name_Base
pop	cx
push	[bp+argLbxFileName]
lea	ax, [bp+varEmmHndlNm]
push	ax
call	strcpy
pop	cx
pop	cx
xor	si, si
jmp	short @@FindZero
@@NextChar:
inc	si
@@FindZero:
cmp	[bp+si+varEmmHndlNm], 0
jnz	short @@NextChar
@@GotLen:
mov	di, si
jmp	short @@LoopMaxLen
@@ZeroPad:
mov	[bp+di+varEmmHndlNm], 0
inc	di
@@LoopMaxLen:
cmp	di, 9
jl	short @@ZeroPad
mov	[bp+Handle_Exists], 0
xor	si, si
jmp	short @@LoopOpenHandles
@@CheckEmmTableEmmHndlNm:
lea	ax, [bp+varEmmHndlNm]
push	ax
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short @@NextHandle
mov	[bp+Handle_Exists], 1
@@NextHandle:
inc	si
@@LoopOpenHandles:
cmp	si, [EMM_Open_Handles]
jl	short @@CheckEmmTableEmmHndlNm
cmp	[bp+Handle_Exists], 0
jz	short @@CheckEmmOk
xor	ax, ax
push	ax
mov	ax, LBXErr_release_first
push	ax
lea	ax, [bp+varEmmHndlNm]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
@@CheckEmmOk:
cmp	[g_EMM_OK], 0
jnz	short @@EmmOk
@@NOT_EMM_OK:
jmp	@@Return_varEmmHndlNbr
@@EmmOk:
cmp	[EMM_Open_Handles], 40
jl	short @@HandlesAvailable
@@g_EMM_Open_Handles_GTE_40:
jmp	@@Return_varEmmHndlNbr
@@HandlesAvailable:
lea	ax, [bp+varEmmHndlNm]
push	ax
lea	ax, [bp+varEmmHndlFileName]
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, offset str_LBX_extension__seg013
push	ax
lea	ax, [bp+varEmmHndlFileName]
push	ax
call	strcat
pop	cx
pop	cx
lea	ax, [bp+varEmmHndlFileName]
push	ax
call	LOF
pop	cx
mov	[word ptr bp+varLbxFileSize+2],	dx
mov	[word ptr bp+varLbxFileSize], ax
mov	ax, [word ptr bp+varLbxFileSize]
or	ax, [word ptr bp+varLbxFileSize+2]
jnz	short @@CalcBlocks
@@LbxFileSizeZero:
jmp	@@Return_varEmmHndlNbr
@@CalcBlocks:
xor	ax, ax
mov	dx, e_EMM_Page_Size
push	ax
push	dx
push	[word ptr bp+varLbxFileSize+2]
push	[word ptr bp+varLbxFileSize]
call	LDIV@
mov	[bp+varLbxFileSize16kBlocks], ax
xor	ax, ax
mov	dx, e_EMM_Page_Size
push	ax
push	dx
push	[word ptr bp+varLbxFileSize+2]
push	[word ptr bp+varLbxFileSize]
call	LMOD@
or	ax, dx
jz	short @@NoRemainder
@@AddOneForPartialPage:
inc	[bp+varLbxFileSize16kBlocks]
@@NoRemainder:
cmp	[bp+argEmmRsvdFlag], 0
jnz	short @@RsvdFlagSet
call	EMM_GetFreePageCnt
mov	dx, [EMM_Pages_Reserved]
add	dx, [bp+varLbxFileSize16kBlocks]
cmp	ax, dx
jnb	short @@RsvdFlagSet
@@NotEnoughEmsPages:
xor	ax, ax
@@JmpDone:
jmp	@@Done
@@RsvdFlagSet:
push	[bp+argEmmRsvdFlag]
lea	ax, [bp+varEmmHndlNm]
push	ax
push	[bp+varLbxFileSize16kBlocks]
nop
push	cs
call	near ptr EMM_GetHandle
add	sp, 6
mov	[bp+varEmmHndlNbr], ax
cmp	[bp+varEmmHndlNbr], 0
jnz	short @@GotEmmHndl
jmp	@@MaybeLbxToEmmLoadFail
@@GotEmmHndl:
call	EMM_GetPageFrame
mov	[bp+varTmpEmmPageFrameSgmtAddr], ax
mov	[bp+varEmmLogicalPage],	0
cmp	[bp+varTmpEmmPageFrameSgmtAddr], 0
jnz	short @@OpenLbxFile
@@EmmPgFrmAddr_NULL:
jmp	@@JmpReturn_varEmmHndlNbr
@@OpenLbxFile:
lea	ax, [bp+varEmmHndlFileName]
push	ax
call	lbx_open
pop	cx
mov	[bp+varLbxFileHandle], ax
cmp	[bp+varLbxFileHandle], 0
jnz	short @@SetNbytes
@@JmpJmpReturn_varEmmHndlNbr:
jmp	@@JmpReturn_varEmmHndlNbr
@@SetNbytes:
mov	[word ptr bp+UU_varNbytesRead+2], 0
mov	[word ptr bp+UU_varNbytesRead],	0
mov	[bp+varReadNbytes], e_EMM_Page_Size
jmp	short @@DoReadChunks
@@ReadChunksFull:
sub	[word ptr bp+varLbxFileSize], e_EMM_Page_Size
sbb	[word ptr bp+varLbxFileSize+2],	0
push	[bp+varEmmLogicalPage]
push	[bp+varEmmHndlNbr]
call	EMM_MapFourPages
pop	cx
pop	cx
push	[bp+varLbxFileHandle]
push	[bp+varReadNbytes]
push	[bp+varTmpEmmPageFrameSgmtAddr]
call	lbx_read_sgmt
add	sp, 6
inc	[bp+varEmmLogicalPage]
add	[word ptr bp+UU_varNbytesRead],	e_EMM_Page_Size
adc	[word ptr bp+UU_varNbytesRead+2], 0
@@DoReadChunks:
cmp	[word ptr bp+varLbxFileSize+2],	0
jg	short @@ReadChunksFull
@@Total_LTE_:
jl	short @@Total_LT
@@Total_EQ:
cmp	[word ptr bp+varLbxFileSize], e_EMM_Page_Size
jnb	short @@ReadChunksFull
@@Total_LT:
cmp	[word ptr bp+varLbxFileSize+2],	0
jl	short @@CloseLbxFile
jg	short @@ReadChunkRemainder
cmp	[word ptr bp+varLbxFileSize], 0
jbe	short @@CloseLbxFile
@@ReadChunkRemainder:
mov	ax, [word ptr bp+varLbxFileSize]
mov	[bp+varReadNbytes], ax
push	[bp+varEmmLogicalPage]
push	[bp+varEmmHndlNbr]
call	EMM_MapFourPages
pop	cx
pop	cx
push	[bp+varLbxFileHandle]
push	[bp+varReadNbytes]
push	[bp+varTmpEmmPageFrameSgmtAddr]
call	lbx_read_sgmt
add	sp, 6
@@CloseLbxFile:
push	[bp+varLbxFileHandle]
call	lbx_close
pop	cx
@@JmpReturn_varEmmHndlNbr:
jmp	short @@Return_varEmmHndlNbr
@@MaybeLbxToEmmLoadFail:
cmp	[bp+argEmmRsvdFlag], 1
jnz	short @@Return_varEmmHndlNbr
xor	ax, ax
push	ax
mov	ax, LBXErr_EM_swap_fail
push	ax
push	[bp+argLbxFileName]
call	Error_Handler
db  83h
db 0C4h
db    6
@@Return_varEmmHndlNbr:
mov	ax, [bp+varEmmHndlNbr]
jmp	@@JmpDone
@@Done:
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
jmp	short loc_16884
loc_16883:
inc	si
loc_16884:
cmp	[bp+si+Handle_Name], 0
jnz	short loc_16883
mov	di, si
jmp	short loc_16893
loc_1688E:
mov	[bp+di+Handle_Name], 0
inc	di
loc_16893:
cmp	di, 9
jl	short loc_1688E
loc_16898:
mov	[bp+Handle_Found], 0
xor	si, si
jmp	short loc_168C1
loc_168A1:
lea	ax, [bp+Handle_Name]
push	ax
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short loc_168C0
mov	[bp+Handle_Found], 1
loc_168C0:
inc	si
loc_168C1:
cmp	si, [EMM_Open_Handles]
jl	short loc_168A1
cmp	[bp+Handle_Found], 0
jz	short loc_168E0
xor	ax, ax
push	ax
mov	ax, LBXErr_release_first
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_168E0:
cmp	[g_EMM_OK], 0
jnz	short loc_168EA
jmp	loc_16A2F
loc_168EA:
cmp	[EMM_Open_Handles], 40
jl	short loc_168F4
jmp	loc_16A2F
loc_168F4:
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
jnz	short loc_16939
push	[bp+Entry_Index]
mov	ax, le_corrupted
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_16939:
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
jz	short loc_16967
inc	[bp+Page_Count]
loc_16967:
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
cmp	[bp+EMM_Handle], 0
jnz	short loc_16995
xor	ax, ax
push	ax
mov	ax, LBXErr_low_EMS
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_16995:
call	EMM_GetPageFrame
mov	[bp+Page_Frame_Seg], ax
mov	[bp+Logical_Page], 0
mov	ax, [word ptr bp+Entry_Size+2]
mov	dx, [word ptr bp+Entry_Size]
mov	[word ptr bp+Total_Bytes_Left+2], ax
mov	[word ptr bp+Total_Bytes_Left],	dx
mov	[bp+Bytes_To_Copy], 4000h
jmp	short loc_169DF
loc_169B5:
sub	[word ptr bp+Total_Bytes_Left],	4000h
sbb	[word ptr bp+Total_Bytes_Left+2], 0
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
inc	[bp+Logical_Page]
loc_169DF:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jg	short loc_169B5
jl	short loc_169EE
cmp	[word ptr bp+Total_Bytes_Left],	4000h
jnb	short loc_169B5
loc_169EE:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jl	short loc_16A2F
jg	short loc_169FC
cmp	[word ptr bp+Total_Bytes_Left],	0
jbe	short loc_16A2F
loc_169FC:
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
loc_16A2F:
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
push	ax
call	strcpy
pop	cx
pop	cx
push	[bp+HndName@]
lea	ax, [bp+EMM_Handle_Name]
push	ax
call	strcpy
pop	cx
pop	cx
xor	si, si
jmp	short loc_16AB2
loc_16AB1:
inc	si
loc_16AB2:
cmp	[bp+si+EMM_Handle_Name], 0
jnz	short loc_16AB1
mov	di, si
jmp	short loc_16AC1
loc_16ABC:
mov	[bp+di+EMM_Handle_Name], 0
inc	di
loc_16AC1:
cmp	di, 9
jl	short loc_16ABC
mov	[bp+Handle_Found], 0
xor	si, si
jmp	short loc_16AEF
loc_16ACF:
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
jnz	short loc_16AEE
mov	[bp+Handle_Found], 1
loc_16AEE:
inc	si
loc_16AEF:
cmp	si, [EMM_Open_Handles]
jl	short loc_16ACF
cmp	[bp+Handle_Found], 0
jz	short loc_16B0E
xor	ax, ax
push	ax
mov	ax, LBXErr_release_first
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_16B0E:
cmp	[g_EMM_OK], 0
jnz	short loc_16B18
jmp	@@Done
loc_16B18:
cmp	[EMM_Open_Handles], 40
jl	short loc_16B22
jmp	@@Done
loc_16B22:
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
jg	short loc_16B78
jnz	short loc_16B65
cmp	[word ptr bp+LBX_Entry_Size], 0
ja	short loc_16B78
loc_16B65:
push	[bp+Entry]
mov	ax, le_corrupted
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_16B78:
mov	ax, [word ptr bp+Bytes+2]
mov	dx, [word ptr bp+Bytes]
cmp	ax, [word ptr bp+LBX_Entry_Size+2]
jg	short loc_16B96
jl	short loc_16B8A
cmp	dx, [word ptr bp+LBX_Entry_Size]
jnb	short loc_16B96
loc_16B8A:
mov	ax, [word ptr bp+Bytes+2]
mov	dx, [word ptr bp+Bytes]
mov	[word ptr bp+LBX_Entry_Size+2],	ax
mov	[word ptr bp+LBX_Entry_Size], dx
loc_16B96:
push	[bp+File_Handle]
push	[word ptr bp+File_Offset+2]
push	[word ptr bp+File_Offset]
call	lbx_seek
add	sp, 6
or	ax, ax
jnz	short loc_16BBE
push	[bp+Entry]
mov	ax, le_corrupted
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_16BBE:
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+LBX_Entry_Size+2]
push	[word ptr bp+LBX_Entry_Size]
call	LDIV@
mov	[bp+Page_Count], ax
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+LBX_Entry_Size+2]
push	[word ptr bp+LBX_Entry_Size]
call	LMOD@
or	ax, dx
jz	short loc_16BEC
inc	[bp+Page_Count]
loc_16BEC:
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
jnz	short loc_16C16
xor	ax, ax
push	ax
mov	ax, LBXErr_low_EMS
push	ax
lea	ax, [bp+LBX_FileName]
push	ax
call	Error_Handler
db  83h
db 0C4h
db    6
loc_16C16:
call	EMM_GetPageFrame
mov	[bp+Buffer_Seg], ax
mov	[bp+First_Log_Page], 0
mov	ax, [word ptr bp+LBX_Entry_Size+2]
mov	dx, [word ptr bp+LBX_Entry_Size]
mov	[word ptr bp+Total_Bytes_Left+2], ax
mov	[word ptr bp+Total_Bytes_Left],	dx
mov	[bp+Bytes_To_Copy], 16384
jmp	short loc_16C60
loc_16C36:
sub	[word ptr bp+Total_Bytes_Left],	16384
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
loc_16C60:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jg	short loc_16C36
jl	short loc_16C6F
cmp	[word ptr bp+Total_Bytes_Left],	16384
jnb	short loc_16C36
loc_16C6F:
cmp	[word ptr bp+Total_Bytes_Left+2], 0
jl	short @@Done
jg	short loc_16C7D
cmp	[word ptr bp+Total_Bytes_Left],	0
jbe	short @@Done
loc_16C7D:
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
@@Done:
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp UU_EMM_CustomLBXLoad
; Attributes: bp-based frame
proc EMM_LBX_EntryLoader far
EntryTableOffset= dword	ptr -18h
DataOffset_End__DataSize_Bytes=	dword ptr -14h
DataOffsetBegin= dword ptr -10h
tmp_DataSize_Paras= word ptr -0Ch
tmp_SAMB_data= word ptr	-0Ah
ReadNbytes= word ptr -8
LbxType= word ptr -6
LbxEntryCount= word ptr	-4
EMM_Handle= word ptr -2
EmmHndlNm= word	ptr  6
LbxEntry= word ptr  8
SAMB_head= word	ptr  0Ah
LoadType= word ptr  0Ch
FormatType= word ptr  0Eh
arg_10=	word ptr  10h
arg_12=	word ptr  12h
arg_14=	word ptr  14h
_DI_SAMB_data =	di
_SI_SAMB_head =	si
push	bp
mov	bp, sp
sub	sp, 18h
push	_SI_SAMB_head
push	_DI_SAMB_data
mov	_SI_SAMB_head, [bp+SAMB_head]
xor	_DI_SAMB_data, _DI_SAMB_data
push	[bp+EmmHndlNm]
nop
push	cs
call	near ptr EMM_CheckHandleOpen
pop	cx
mov	[bp+EMM_Handle], ax
cmp	[bp+EMM_Handle], e_RE_FAILURE
jnz	short @@FoundHandleInTable
@@HandleNotInTable:
jmp	@@GotoExit
@@FoundHandleInTable:
cmp	[g_EMM_Reserved], 1
jnz	short @@GetCheckEntryCount
@@LoadEmmRsvd:
push	[bp+EMM_Handle]
mov	ax, 2
push	ax
xor	ax, ax
mov	dx, 6
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+LbxType]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
cmp	[bp+LbxType], 0
jz	short @@LbxFileTypePictures
push	[bp+LbxEntry]
mov	ax, LBXErr_cantload_reserved
push	ax
push	[bp+EmmHndlNm]
call	Error_Handler
db  83h
db 0C4h
db    6
db 0EBh
db  1Ah
@@LbxFileTypePictures:
push	[bp+FormatType]
push	[bp+LoadType]
push	_SI_SAMB_head
push	[bp+LbxEntry]
push	[bp+EmmHndlNm]
push	[bp+EMM_Handle]
nop
push	cs
call	near ptr EMM_LBX_Load_Picture_Header
add	sp, 0Ch
mov	_DI_SAMB_data, ax
@@JmpDone_LoadFlicHeader:
jmp	@@GotoExit
@@GetCheckEntryCount:
push	[bp+EMM_Handle]
mov	ax, 2
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
lea	ax, [bp+LbxEntryCount]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [bp+LbxEntry]
cmp	ax, [bp+LbxEntryCount]
jl	short @@CheckFormat
@@GotoError:
xor	ax, ax
@@JmpDone:
jmp	@@Done
@@CheckFormat:
cmp	[bp+FormatType], 0
jnz	short @@LbxHdrFmt_1
@@LbxHdrFmt_0:
mov	ax, [bp+LbxEntry]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
jmp	short @@EntryOffsetBeginEndSize
@@LbxHdrFmt_1:
mov	ax, [bp+LbxEntry]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 520
@@EntryOffsetBeginEndSize:
adc	dx, 0
mov	[word ptr bp+EntryTableOffset+2], dx
mov	[word ptr bp+EntryTableOffset],	ax
push	[bp+EMM_Handle]
mov	ax, 4
push	ax
push	[word ptr bp+EntryTableOffset+2]
push	[word ptr bp+EntryTableOffset]
xor	ax, ax
push	ax
lea	ax, [bp+DataOffsetBegin]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	[bp+EMM_Handle]
mov	ax, 4
push	ax
mov	ax, [word ptr bp+EntryTableOffset+2]
mov	dx, [word ptr bp+EntryTableOffset]
add	dx, 4
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+DataOffset_End__DataSize_Bytes]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [word ptr bp+DataOffsetBegin+2]
mov	dx, [word ptr bp+DataOffsetBegin]
sub	[word ptr bp+DataOffset_End__DataSize_Bytes], dx
sbb	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	ax
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
jge	short loc_16DCF
jmp	@@GotoExit
loc_16DCF:
jg	short @@EntrySizeParas
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes], 0
ja	short @@EntrySizeParas
@@EntrySize_Lo0:
jmp	@@GotoExit
@@EntrySizeParas:
xor	ax, ax
mov	dx, e_SZ_PARAGRAPH_B
push	ax
push	dx
push	[word ptr bp+DataOffset_End__DataSize_Bytes+2]
push	[word ptr bp+DataOffset_End__DataSize_Bytes]
call	LDIV@
mov	[bp+tmp_DataSize_Paras], ax
test	[bp+tmp_DataSize_Paras], 1111b
jz	short @@CheckLoadType
inc	[bp+tmp_DataSize_Paras]
@@CheckLoadType:
mov	ax, [bp+LoadType]
or	ax, ax
jz	short @@LoadTypeZero
cmp	ax, 1
jz	short @@LoadTypeOne
@@NotLoadTypeZeroOrOne:
cmp	ax, 2
jz	short @@LoadTypeTwo
@@ErrorInvalidLoadType:
jmp	@@CheckNullSamb
@@LoadTypeZero:
push	[bp+tmp_DataSize_Paras]
call	Allocate_Space_No_Header__EntrySize
pop	cx
mov	_DI_SAMB_data, ax
or	_DI_SAMB_data, _DI_SAMB_data
jnz	short @@DoneLoadTypeZero
@@ErrorLoadTypeZeroAlloc:
jmp	@@GotoError
@@DoneLoadTypeZero:
jmp	@@CheckNullSamb
@@LoadTypeOne:
push	_SI_SAMB_head
call	Check_Allocation
pop	cx
or	ax, ax
jnz	short @@CheckAllocSize
@@LoadTypeOneValidateZero:
jmp	@@GotoError
@@CheckAllocSize:
mov	ax, s_SAMB.size
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
dec	ax
cmp	ax, [bp+tmp_DataSize_Paras]
jnb	short loc_16E45
@@LoadTypeOneExceedsEntries:
jmp	@@GotoError
loc_16E45:
mov	ax, _SI_SAMB_head
inc	ax
mov	_DI_SAMB_data, ax
mov	ax, [bp+tmp_DataSize_Paras]
inc	ax
push	ax
@@EndLoadTypeOneOrTwo:
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpokew
add	sp, 6
jmp	short @@CheckNullSamb
@@LoadTypeTwo:
or	_SI_SAMB_head, _SI_SAMB_head
jnz	short @@LoadTypeTwoWithSeg
@@LoadTypeTwoWithoutSeg:
mov	ax, 1
jmp	short @@LoadTypeTwoCheckMemSig
@@LoadTypeTwoWithSeg:
xor	ax, ax
@@LoadTypeTwoCheckMemSig:
push	ax
call	Check_Allocation
pop	cx
or	ax, ax
jz	short @@CheckAllocFree
@@LoadTypeTwoValidateNotZero:
jmp	@@GotoError
@@CheckAllocFree:
push	_SI_SAMB_head
call	Get_Free_Blocks
pop	cx
cmp	ax, [bp+tmp_DataSize_Paras]
jnb	short loc_16E86
@@LoadTypeTwoExceedsEntries:
jmp	@@GotoError
loc_16E86:
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
mov	dx, _SI_SAMB_head
add	dx, ax
mov	_DI_SAMB_data, dx
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
mov	dx, [bp+tmp_DataSize_Paras]
add	dx, ax
push	dx
jmp	short @@EndLoadTypeOneOrTwo
@@CheckNullSamb:
or	_DI_SAMB_data, _DI_SAMB_data
jnz	short @@DoRead
@@bxDataSegNull:
jmp	@@GotoExit
@@DoRead:
mov	[bp+tmp_SAMB_data], _DI_SAMB_data
mov	[bp+ReadNbytes], e_SZ_32K_B
jmp	short @@DoReadChunks
@@ReadChunkFull:
sub	[word ptr bp+DataOffset_End__DataSize_Bytes], e_SZ_32K_B
sbb	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
push	[bp+EMM_Handle]
push	[bp+ReadNbytes]
push	[word ptr bp+DataOffsetBegin+2]
push	[word ptr bp+DataOffsetBegin]
push	[bp+tmp_SAMB_data]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
add	[word ptr bp+DataOffsetBegin], e_SZ_32K_B
adc	[word ptr bp+DataOffsetBegin+2], 0
add	[bp+tmp_SAMB_data], e_SZ_32K_PR
@@DoReadChunks:
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
jg	short @@ReadChunkFull
jnz	short loc_16EFD
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes], e_SZ_32K_B
ja	short @@ReadChunkFull
loc_16EFD:
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
jl	short @@GotoExit
jg	short @@ReadChunkRemainder
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes], 0
jbe	short @@GotoExit
@@ReadChunkRemainder:
xor	ax, ax
mov	dx, e_SZ_32K_B
push	ax
push	dx
push	[word ptr bp+DataOffset_End__DataSize_Bytes+2]
push	[word ptr bp+DataOffset_End__DataSize_Bytes]
call	LMOD@
mov	[bp+ReadNbytes], ax
push	[bp+EMM_Handle]
push	[bp+ReadNbytes]
push	[word ptr bp+DataOffsetBegin+2]
push	[word ptr bp+DataOffsetBegin]
push	[bp+tmp_SAMB_data]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
@@GotoExit:
mov	ax, _DI_SAMB_data
jmp	@@JmpDone
@@Done:
pop	_DI_SAMB_data
pop	_SI_SAMB_head
mov	sp, bp
pop	bp
retf
endp EMM_LBX_EntryLoader
; Attributes: bp-based frame
proc EMM_LBX_Load_Picture_Header far
EntryTableOffset= dword	ptr -10h
DataOffset_Begin= dword	ptr -0Ch
Entry_Begin_Logical_Page_Number= word ptr -8
Entry_Begin_Logical_Page_Offset= word ptr -6
DataSize_Paras=	word ptr -4
LbxEntryCount= word ptr	-2
EMM_Handle= word ptr  6
EMM_Handle_Name= word ptr  8
LbxEntryIndex= word ptr	 0Ah
SAMB_head= word	ptr  0Ch
Load_Type= word	ptr  0Eh
_DI_SAMB_data =	di
_SI_SAMB_head =	si
push	bp
mov	bp, sp
sub	sp, 10h
push	_SI_SAMB_head
push	_DI_SAMB_data
mov	_SI_SAMB_head, [bp+SAMB_head]
xor	_DI_SAMB_data, _DI_SAMB_data
push	[bp+EMM_Handle]
mov	ax, 2
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
lea	ax, [bp+LbxEntryCount]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [bp+LbxEntryIndex]
cmp	ax, [bp+LbxEntryCount]
jl	short @@EntryOffsetBegin
push	[bp+LbxEntryIndex]
mov	ax, le_entries_exceeded
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
@@EntryOffsetBegin:
mov	ax, [bp+LbxEntryIndex]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
adc	dx, 0
mov	[word ptr bp+EntryTableOffset+2], dx
mov	[word ptr bp+EntryTableOffset],	ax
push	[bp+EMM_Handle]
mov	ax, 4
push	ax
push	[word ptr bp+EntryTableOffset+2]
push	[word ptr bp+EntryTableOffset]
xor	ax, ax
push	ax
lea	ax, [bp+DataOffset_Begin]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	[bp+DataSize_Paras], 1
mov	ax, [bp+Load_Type]
or	ax, ax
jz	short @@LoadTypeZero
@@LoadTypeNotZero:
cmp	ax, 1
jz	short @@LoadTypeOne
@@NotLoadTypeZeroOne:
cmp	ax, 2
jnz	short @@ErrorInvalidLoadType
@@jLoadTypeTwo:
jmp	@@LoadTypeTwo
@@ErrorInvalidLoadType:
jmp	@@DoRead
@@LoadTypeZero:
push	[bp+DataSize_Paras]
call	Allocate_Space_No_Header__EntrySize
pop	cx
mov	_DI_SAMB_data, ax
or	_DI_SAMB_data, _DI_SAMB_data
jnz	short @@DoneLoadTypeZero
push	[bp+LbxEntryIndex]
mov	ax, LBXErr_low_RAM
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
@@DoneLoadTypeZero:
jmp	@@DoRead
@@LoadTypeOne:
push	_SI_SAMB_head
call	Check_Allocation
pop	cx
or	ax, ax
jnz	short @@CheckAllocSize
push	[bp+LbxEntryIndex]
mov	ax, LBXErr_alloc_short
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
@@CheckAllocSize:
mov	ax, s_SAMB.size
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
dec	ax
cmp	ax, [bp+DataSize_Paras]
jnb	short loc_17040
mov	ax, 1
push	ax
push	[bp+LbxEntryIndex]
mov	ax, LBXErr_reload_fail
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    8
loc_17040:
mov	ax, _SI_SAMB_head
inc	ax
mov	_DI_SAMB_data, ax
mov	ax, [bp+DataSize_Paras]
inc	ax
push	ax
@@EndLoadTypeOneOrTwo:
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpokew
add	sp, 6
jmp	short @@DoRead
@@LoadTypeTwo:
or	_SI_SAMB_head, _SI_SAMB_head
jnz	short @@LoadTypeTwoWithSeg
@@LoadTypeTwoWithoutSeg:
mov	ax, 1
jmp	short loc_17064
@@LoadTypeTwoWithSeg:
xor	ax, ax
loc_17064:
push	ax
call	Check_Allocation
pop	cx
or	ax, ax
jz	short @@CheckAllocFree
push	[bp+LbxEntryIndex]
mov	ax, LBXErr_alloc_short
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    6
@@CheckAllocFree:
push	_SI_SAMB_head
call	Get_Free_Blocks
pop	cx
cmp	ax, [bp+DataSize_Paras]
jnb	short @@UsedPlusSize
mov	ax, 1
push	ax
push	[bp+LbxEntryIndex]
mov	ax, LBXErr_reload_fail
push	ax
push	[bp+EMM_Handle_Name]
call	Error_Handler
db  83h
db 0C4h
db    8
@@UsedPlusSize:
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
mov	dx, _SI_SAMB_head
add	dx, ax
mov	_DI_SAMB_data, dx
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
mov	dx, [bp+DataSize_Paras]
add	dx, ax
push	dx
jmp	short @@EndLoadTypeOneOrTwo
@@DoRead:
xor	ax, ax
mov	dx, e_EMM_Page_Size
push	ax
push	dx
push	[word ptr bp+DataOffset_Begin+2]
push	[word ptr bp+DataOffset_Begin]
call	LDIV@
mov	[bp+Entry_Begin_Logical_Page_Number], ax
xor	ax, ax
mov	dx, e_EMM_Page_Size
push	ax
push	dx
push	[word ptr bp+DataOffset_Begin+2]
push	[word ptr bp+DataOffset_Begin]
call	LMOD@
mov	[bp+Entry_Begin_Logical_Page_Offset], ax
push	[bp+EMM_Handle]
mov	ax, 16
push	ax
push	[word ptr bp+DataOffset_Begin+2]
push	[word ptr bp+DataOffset_Begin]
push	_DI_SAMB_data
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	[bp+EMM_Handle]
mov	ax, s_FLIC_HDR.EMM_Handle_Number
push	ax
push	_DI_SAMB_data
call	farpokeb
add	sp, 6
push	[bp+Entry_Begin_Logical_Page_Number]
mov	ax, s_FLIC_HDR.EMM_Logical_Page_Number
push	ax
push	_DI_SAMB_data
call	farpokeb
add	sp, 6
push	[bp+Entry_Begin_Logical_Page_Offset]
mov	ax, s_FLIC_HDR.EMM_Logical_Page_Offset
push	ax
push	_DI_SAMB_data
call	farpokew
add	sp, 6
mov	ax, _DI_SAMB_data
jmp	short $+2
pop	_DI_SAMB_data
pop	_SI_SAMB_head
mov	sp, bp
pop	bp
retf
endp EMM_LBX_Load_Picture_Header
; Attributes: bp-based frame
proc EMM_LBX_RecLoader far
EntryTableOffset= dword	ptr -1Ah
DataOffset_End__DataSize_Bytes=	dword ptr -16h
DataOffset_Begin= dword	ptr -12h
Record_Count= word ptr -0Eh
Record_Size= word ptr -0Ch
tmp_DataSize_Paras= word ptr -0Ah
tmp_SAMB_data= word ptr	-8
ReadNbytes= word ptr -6
LbxEntryCount= word ptr	-4
SAMB_data= word	ptr -2
EmmHndlNm= word	ptr  6
LbxEntry= word ptr  8
SAMB_head= word	ptr  0Ah
LoadType= word ptr  0Ch
RecFirst= word ptr  0Eh
RecCount= word ptr  10h
RecSize= word ptr  12h
_SI_SAMB_head =	si
_DI_EmmHndl = di
push	bp
mov	bp, sp
sub	sp, 1Ah
push	_SI_SAMB_head
push	_DI_EmmHndl
mov	_SI_SAMB_head, [bp+SAMB_head]
mov	[bp+SAMB_data],	e_RE_FAILURE
push	[bp+EmmHndlNm]
nop
push	cs
call	near ptr EMM_CheckHandleOpen
pop	cx
mov	_DI_EmmHndl, ax
or	_DI_EmmHndl, _DI_EmmHndl
jnz	short @@GetCheckEntryCount
jmp	@@GotoExit
@@GetCheckEntryCount:
push	_DI_EmmHndl
mov	ax, 2
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
xor	ax, ax
push	ax
lea	ax, [bp+LbxEntryCount]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [bp+LbxEntry]
cmp	ax, [bp+LbxEntryCount]
jl	short @@EntryOffsetBeginEndSize
@@GotoError:
xor	ax, ax
@@JmpDone:
jmp	@@Done
@@EntryOffsetBeginEndSize:
mov	ax, [bp+LbxEntry]
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
adc	dx, 0
mov	[word ptr bp+EntryTableOffset+2], dx
mov	[word ptr bp+EntryTableOffset],	ax
push	_DI_EmmHndl
mov	ax, 4
push	ax
push	[word ptr bp+EntryTableOffset+2]
push	[word ptr bp+EntryTableOffset]
xor	ax, ax
push	ax
lea	ax, [bp+DataOffset_Begin]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	_DI_EmmHndl
mov	ax, 4
push	ax
mov	ax, [word ptr bp+EntryTableOffset+2]
mov	dx, [word ptr bp+EntryTableOffset]
add	dx, 4
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+DataOffset_End__DataSize_Bytes]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [word ptr bp+DataOffset_Begin+2]
mov	dx, [word ptr bp+DataOffset_Begin]
sub	[word ptr bp+DataOffset_End__DataSize_Bytes], dx
sbb	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	ax
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
jge	short loc_171F7
jmp	@@GotoExit
loc_171F7:
jg	short @@GetRecordCountSize
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes], 0
ja	short @@GetRecordCountSize
jmp	@@GotoExit
@@GetRecordCountSize:
mov	ax, [word ptr bp+DataOffset_Begin+2]
mov	dx, [word ptr bp+DataOffset_Begin]
mov	[word ptr bp+EntryTableOffset+2], ax
mov	[word ptr bp+EntryTableOffset],	dx
push	_DI_EmmHndl
mov	ax, 2
push	ax
push	[word ptr bp+EntryTableOffset+2]
push	[word ptr bp+EntryTableOffset]
xor	ax, ax
push	ax
lea	ax, [bp+Record_Count]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	_DI_EmmHndl
mov	ax, 2
push	ax
mov	ax, [word ptr bp+EntryTableOffset+2]
mov	dx, [word ptr bp+EntryTableOffset]
add	dx, 2
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+Record_Size]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
cmp	[bp+Record_Size], 0
jz	short loc_17258
mov	ax, [bp+RecSize]
cmp	ax, [bp+Record_Size]
jz	short loc_1725B
loc_17258:
jmp	@@GotoError
loc_1725B:
mov	ax, [bp+RecFirst]
add	ax, [bp+RecCount]
cmp	ax, [bp+Record_Count]
jbe	short loc_17269
jmp	@@GotoError
loc_17269:
mov	ax, [bp+RecFirst]
cwd
push	ax
mov	ax, [bp+Record_Size]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
add	ax, 4
adc	dx, 0
add	[word ptr bp+DataOffset_Begin],	ax
adc	[word ptr bp+DataOffset_Begin+2], dx
mov	ax, [bp+RecCount]
cwd
push	ax
mov	ax, [bp+Record_Size]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
mov	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	dx
mov	[word ptr bp+DataOffset_End__DataSize_Bytes], ax
xor	ax, ax
mov	dx, e_SZ_PARAGRAPH_B
push	ax
push	dx
push	[word ptr bp+DataOffset_End__DataSize_Bytes+2]
push	[word ptr bp+DataOffset_End__DataSize_Bytes]
call	LDIV@
inc	ax
mov	[bp+tmp_DataSize_Paras], ax
mov	ax, [bp+LoadType]
or	ax, ax
jz	short @@LoadTypeZero
cmp	ax, 1
jz	short loc_172E1
cmp	ax, 2
jz	short loc_1731E
jmp	@@CheckNullSamb
@@LoadTypeZero:
push	[bp+tmp_DataSize_Paras]
call	Allocate_Space_No_Header__EntrySize
pop	cx
mov	[bp+SAMB_data],	ax
cmp	[bp+SAMB_data],	0
jnz	short loc_172DE
jmp	@@GotoError
loc_172DE:
jmp	@@CheckNullSamb
loc_172E1:
push	_SI_SAMB_head
call	Check_Allocation
pop	cx
or	ax, ax
jnz	short loc_172EF
jmp	@@GotoError
loc_172EF:
mov	ax, s_SAMB.size
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
dec	ax
cmp	ax, [bp+tmp_DataSize_Paras]
jnb	short loc_17304
jmp	@@GotoError
loc_17304:
mov	ax, _SI_SAMB_head
inc	ax
mov	[bp+SAMB_data],	ax
mov	ax, [bp+tmp_DataSize_Paras]
inc	ax
push	ax
@@EndLoadTypeOneOrTwo:
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpokew
add	sp, 6
jmp	short @@CheckNullSamb
loc_1731E:
or	_SI_SAMB_head, _SI_SAMB_head
jnz	short loc_17327
mov	ax, 1
jmp	short loc_17329
loc_17327:
xor	ax, ax
loc_17329:
push	ax
call	Check_Allocation
pop	cx
or	ax, ax
jz	short loc_17337
jmp	@@GotoError
loc_17337:
push	_SI_SAMB_head
call	Get_Free_Blocks
pop	cx
cmp	ax, [bp+tmp_DataSize_Paras]
jnb	short loc_17346
jmp	@@GotoError
loc_17346:
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
mov	dx, _SI_SAMB_head
add	dx, ax
mov	[bp+SAMB_data],	dx
mov	ax, s_SAMB.used
push	ax
push	_SI_SAMB_head
call	farpeekw
pop	cx
pop	cx
mov	dx, [bp+tmp_DataSize_Paras]
add	dx, ax
push	dx
jmp	short @@EndLoadTypeOneOrTwo
@@CheckNullSamb:
cmp	[bp+SAMB_data],	e_RE_FAILURE
jnz	short @@DoRead
jmp	@@GotoExit
@@DoRead:
mov	ax, [bp+SAMB_data]
mov	[bp+tmp_SAMB_data], ax
mov	[bp+ReadNbytes], e_SZ_32K_B
jmp	short @@DoReadChunks
@@ReadChunkFull:
sub	[word ptr bp+DataOffset_End__DataSize_Bytes], e_SZ_32K_B
sbb	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
push	_DI_EmmHndl
push	[bp+ReadNbytes]
push	[word ptr bp+DataOffset_Begin+2]
push	[word ptr bp+DataOffset_Begin]
push	[bp+tmp_SAMB_data]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
add	[word ptr bp+DataOffset_Begin],	e_SZ_32K_B
adc	[word ptr bp+DataOffset_Begin+2], 0
add	[bp+tmp_SAMB_data], e_SZ_32K_PR
@@DoReadChunks:
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
jg	short @@ReadChunkFull
jnz	short loc_173C1
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes], e_SZ_32K_B
ja	short @@ReadChunkFull
loc_173C1:
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
jl	short @@GotoExit
jg	short @@ReadChunkRemainder
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes], 0
jbe	short @@GotoExit
@@ReadChunkRemainder:
xor	ax, ax
mov	dx, e_SZ_32K_B
push	ax
push	dx
push	[word ptr bp+DataOffset_End__DataSize_Bytes+2]
push	[word ptr bp+DataOffset_End__DataSize_Bytes]
call	LMOD@
mov	[bp+ReadNbytes], ax
push	_DI_EmmHndl
push	[bp+ReadNbytes]
push	[word ptr bp+DataOffset_Begin+2]
push	[word ptr bp+DataOffset_Begin]
push	[bp+tmp_SAMB_data]
xor	ax, ax
push	ax
call	EMM_MapnRead
add	sp, 0Ch
@@GotoExit:
mov	ax, [bp+SAMB_data]
jmp	@@JmpDone
@@Done:
pop	_DI_EmmHndl
pop	_SI_SAMB_head
mov	sp, bp
pop	bp
retf
endp EMM_LBX_RecLoader
; Attributes: bp-based frame
proc EMM_LBXR_DirectLoad far
varEntryTableOffset= dword ptr -18h
DataOffset_End__DataSize_Bytes=	dword ptr -14h
DataOffset_Begin= dword	ptr -10h
varLbxEntryRecordCount=	word ptr -0Ch
varLbxEntryRecordSize= word ptr	-0Ah
UU_DataSize_Paras= word	ptr -8
ReadNbytes= word ptr -6
Entry_Count= word ptr -4
UU_SAMB_data= word ptr -2
EmmHndlNm= word	ptr  6
LbxEntry= word ptr  8
SAMB_data= word	ptr  0Ah
RecFirst= word ptr  0Ch
RecCount= word ptr  0Eh
RecSize= word ptr  10h
arg_12=	word ptr  12h
_SI_EmmHndl = si
_DI_LbxEntry = di
push	bp
mov	bp, sp
sub	sp, 18h
push	_SI_EmmHndl
push	_DI_LbxEntry
mov	_DI_LbxEntry, [bp+LbxEntry]
mov	[bp+UU_SAMB_data], e_RE_FAILURE
push	[bp+EmmHndlNm]
nop
push	cs
call	near ptr EMM_CheckHandleOpen
pop	cx
mov	_SI_EmmHndl, ax
or	_SI_EmmHndl, _SI_EmmHndl
jnz	short @@CheckEntryCount
jmp	@@JmpFailure
@@CheckEntryCount:
push	_SI_EmmHndl
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
cmp	_DI_LbxEntry, [bp+Entry_Count]
jl	short @@EntryOffsetBeginEndSize
@@GotoError:
xor	ax, ax
@@JmpDone:
jmp	@@Done
@@EntryOffsetBeginEndSize:
mov	ax, _DI_LbxEntry
cwd
mov	cl, 2
call	LXLSH@
add	ax, 8
adc	dx, 0
mov	[word ptr bp+varEntryTableOffset+2], dx
mov	[word ptr bp+varEntryTableOffset], ax
push	_SI_EmmHndl
mov	ax, 4
push	ax
push	[word ptr bp+varEntryTableOffset+2]
push	[word ptr bp+varEntryTableOffset]
xor	ax, ax
push	ax
lea	ax, [bp+DataOffset_Begin]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	_SI_EmmHndl
mov	ax, 4
push	ax
mov	ax, [word ptr bp+varEntryTableOffset+2]
mov	dx, [word ptr bp+varEntryTableOffset]
add	dx, 4
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+DataOffset_End__DataSize_Bytes]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
mov	ax, [word ptr bp+DataOffset_Begin+2]
mov	dx, [word ptr bp+DataOffset_Begin]
sub	[word ptr bp+DataOffset_End__DataSize_Bytes], dx
sbb	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	ax
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	0
jge	short loc_174B5
jmp	@@JmpGotoExit
loc_174B5:
jg	short @@GetRecordCountSize
cmp	[word ptr bp+DataOffset_End__DataSize_Bytes], 0
ja	short @@GetRecordCountSize
jmp	@@JmpGotoExit
@@GetRecordCountSize:
mov	ax, [word ptr bp+DataOffset_Begin+2]
mov	dx, [word ptr bp+DataOffset_Begin]
mov	[word ptr bp+varEntryTableOffset+2], ax
mov	[word ptr bp+varEntryTableOffset], dx
push	_SI_EmmHndl
mov	ax, 2
push	ax
push	[word ptr bp+varEntryTableOffset+2]
push	[word ptr bp+varEntryTableOffset]
xor	ax, ax
push	ax
lea	ax, [bp+varLbxEntryRecordCount]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
push	_SI_EmmHndl
mov	ax, 2
push	ax
mov	ax, [word ptr bp+varEntryTableOffset+2]
mov	dx, [word ptr bp+varEntryTableOffset]
add	dx, 2
adc	ax, 0
push	ax
push	dx
xor	ax, ax
push	ax
lea	ax, [bp+varLbxEntryRecordSize]
push	ax
call	EMM_MapnRead
add	sp, 0Ch
cmp	[bp+varLbxEntryRecordSize], 0
jz	short loc_17516
mov	ax, [bp+RecSize]
cmp	ax, [bp+varLbxEntryRecordSize]
jz	short loc_17519
loc_17516:
jmp	@@GotoError
loc_17519:
mov	ax, [bp+RecFirst]
add	ax, [bp+RecCount]
cmp	ax, [bp+varLbxEntryRecordCount]
jbe	short loc_17527
jmp	@@GotoError
loc_17527:
mov	ax, [bp+RecFirst]
cwd
push	ax
mov	ax, [bp+varLbxEntryRecordSize]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
add	ax, 4
adc	dx, 0
add	[word ptr bp+DataOffset_Begin],	ax
adc	[word ptr bp+DataOffset_Begin+2], dx
mov	ax, [bp+RecCount]
cwd
push	ax
mov	ax, [bp+varLbxEntryRecordSize]
push	dx
xor	dx, dx
pop	cx
pop	bx
call	LXMUL@
mov	[word ptr bp+DataOffset_End__DataSize_Bytes+2],	dx
mov	[word ptr bp+DataOffset_End__DataSize_Bytes], ax
xor	ax, ax
mov	dx, e_SZ_PARAGRAPH_B
push	ax
push	dx
push	[word ptr bp+DataOffset_End__DataSize_Bytes+2]
push	[word ptr bp+DataOffset_End__DataSize_Bytes]
call	LDIV@
inc	ax
mov	[bp+UU_DataSize_Paras],	ax
xor	ax, ax
mov	dx, e_SZ_32K_B
push	ax
push	dx
push	[word ptr bp+DataOffset_End__DataSize_Bytes+2]
push	[word ptr bp+DataOffset_End__DataSize_Bytes]
call	LMOD@
mov	[bp+ReadNbytes], ax
push	_SI_EmmHndl
push	[bp+ReadNbytes]
push	[word ptr bp+DataOffset_Begin+2]
push	[word ptr bp+DataOffset_Begin]
xor	ax, ax
push	ax
push	[bp+SAMB_data]
call	EMM_MapnRead
add	sp, 0Ch
@@JmpGotoExit:
jmp	short @@GotoExit
@@JmpFailure:
jmp	@@GotoError
@@GotoExit:
mov	ax, e_RE_SUCCESS
jmp	@@JmpDone
@@Done:
pop	_DI_LbxEntry
pop	_SI_EmmHndl
mov	sp, bp
pop	bp
retf
endp EMM_LBXR_DirectLoad
; Attributes: bp-based frame
proc EMM_CheckHandleOpen far
EMM_HandleName=	word ptr  6
push	bp
mov	bp, sp
push	si
push	di
xor	di, di
xor	si, si
jmp	short @@LoopOpenHandles
@@CheckEmmHndlNm:
push	[bp+EMM_HandleName]
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short @@NextHandle
@@FoundHandle:
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	di, [EMM_Table.handle+bx]
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	al, [EMM_Table.reserved+bx]
cbw
mov	[g_EMM_Reserved], ax
@@NextHandle:
inc	si
@@LoopOpenHandles:
cmp	si, [EMM_Open_Handles]
jge	short @@Done
or	di, di
jz	short @@CheckEmmHndlNm
@@Done:
mov	ax, di
jmp	short $+2
pop	di
pop	si
pop	bp
retf
endp EMM_CheckHandleOpen
; Attributes: bp-based frame
proc EMM_Release_All far
push	bp
mov	bp, sp
push	si
xor	si, si
jmp	short loc_17622
loc_1760E:
mov	ax, si
mov	dx, size s_EMM_RECORD
imul	dx
mov	bx, ax
push	[EMM_Table.handle+bx]
call	EMM_ReleaseHandle__STUB
pop	cx
inc	si
loc_17622:
cmp	si, [EMM_Open_Handles]
jl	short loc_1760E
cmp	[g_EmmHndl_OVERXYZ], 0
jz	short loc_17639
push	[g_EmmHndl_OVERXYZ]
call	EMM_ReleaseHandle__STUB
pop	cx
loc_17639:
pop	si
pop	bp
retf
endp EMM_Release_All
; Attributes: bp-based frame
proc EMM_Release_Resd far
push	bp
mov	bp, sp
push	si
xor	si, si
jmp	short loc_17668
@@NextHandle:
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
cmp	[EMM_Table.reserved+bx], 1
jnz	short @@Continue
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
push	[EMM_Table.handle+bx]
call	EMM_ReleaseHandle__STUB
pop	cx
@@Continue:
inc	si
loc_17668:
cmp	si, [EMM_Open_Handles]
jl	short @@NextHandle
pop	si
pop	bp
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
loc_17678:
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
jnz	short loc_176C1
jmp	loc_17756
loc_176C1:
lea	ax, [bp+Handle_Name]
push	ax
lea	ax, [bp+File_Name]
push	ax
call	DIR
pop	cx
pop	cx
or	ax, ax
jnz	short @@CheckLastLoadRsvd
jmp	loc_17756
@@CheckLastLoadRsvd:
cmp	[g_EMM_Reserved], 1
jnz	short loc_176E9
@@WasRsvd_UpdRsvdLeft:
push	di
call	EMM_GetPageCount
pop	cx
add	[EMM_Pages_Reserved], ax
loc_176E9:
push	di
call	EMM_ReleaseHandle__STUB
pop	cx
mov	si, di
jmp	short loc_1774A
loc_176F4:
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, (offset EMM_Table.name+0Ch)
push	ax
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	ax, [(EMM_Table.handle+0Ch)+bx]
push	ax
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
pop	ax
mov	[EMM_Table.handle+bx], ax
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	al, [(EMM_Table.reserved+0Ch)+bx]
push	ax
mov	ax, si
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
pop	ax
mov	[EMM_Table.reserved+bx], al
inc	si
loc_1774A:
mov	ax, [EMM_Open_Handles]
dec	ax
cmp	ax, si
jg	short loc_176F4
dec	[EMM_Open_Handles]
loc_17756:
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
mov	ax, [g_EMM_OK]
jmp	short $+2
pop	bp
retf
endp UU_EMM_Available
; Attributes: bp-based frame
proc EMM_GetHandle far
varTmpStrEmsRsvdErr= byte ptr -0Ah
varExistingEmsHndlNbr= word ptr	-4
varItrOurOpenHandleCount= word ptr -2
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
cmp	[byte ptr _SI_handle_name], e_ST_NULL
jnz	short @@NameNotNull
@@Error_NoName:
xor	ax, ax
@@JmpDone:
jmp	@@Done
@@NameNotNull:
mov	[bp+varExistingEmsHndlNbr], e_ST_UNDEFINED
mov	[bp+varItrOurOpenHandleCount], 0
jmp	short @@LoopHandles
@@CheckName:
mov	ax, [bp+varItrOurOpenHandleCount]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
push	_SI_handle_name
call	stricmp
pop	cx
pop	cx
or	ax, ax
jnz	short @@Continue
@@FoundHandle:
mov	ax, [bp+varItrOurOpenHandleCount]
mov	[bp+varExistingEmsHndlNbr], ax
@@Continue:
inc	[bp+varItrOurOpenHandleCount]
@@LoopHandles:
mov	ax, [bp+varItrOurOpenHandleCount]
cmp	ax, [EMM_Open_Handles]
jl	short @@CheckName
@@CheckHadFoundHandle:
cmp	[bp+varExistingEmsHndlNbr], e_ST_UNDEFINED
jz	short @@HadNotFoundHandle
@@JmpCheckPageCounts:
jmp	@@CheckPageCounts
@@HadNotFoundHandle:
push	_SI_handle_name
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
cmp	[bp+reserved_flag], e_ST_TRUE
jnz	short @@NotReserved
@@Reserved:
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	al, [byte ptr EMM_Table.name+bx]
neg	al
push	ax
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
pop	ax
mov	[byte ptr EMM_Table.name+bx], al
@@NotReserved:
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table
push	ax
push	[bp+page_count]
call	EMM_MakeNamedHandle
pop	cx
pop	cx
mov	_DI_EmmHndlNbr,	ax
or	_DI_EmmHndlNbr,	_DI_EmmHndlNbr
jnz	short @@Success_MkNmHndl
@@Failure_MkNmHndl:
jmp	@@NewJmpJmpJmpDone
@@Success_MkNmHndl:
push	_SI_handle_name
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
add	ax, offset EMM_Table.name
push	ax
call	strcpy
pop	cx
pop	cx
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	dl, [byte ptr bp+reserved_flag]
mov	bx, ax
mov	[EMM_Table.reserved+bx], dl
mov	ax, [EMM_Open_Handles]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	[EMM_Table.handle+bx], _DI_EmmHndlNbr
inc	[EMM_Open_Handles]
cmp	[bp+reserved_flag], 1
jnz	short @@CheckEmsRsvdLeft
@@DecrEmsRsvdLeft:
mov	ax, [bp+page_count]
sub	[EMM_Pages_Reserved], ax
@@CheckEmsRsvdLeft:
cmp	[EMM_Pages_Reserved], 0
jge	short @@NewJmpJmpJmpDone
@@OutOfEmsRsvdPages:
mov	ax, 0Ah
push	ax
lea	ax, [bp+varTmpStrEmsRsvdErr]
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
lea	ax, [bp+varTmpStrEmsRsvdErr]
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
@@NewJmpJmpJmpDone:
jmp	@@Return_DI
@@CheckPageCounts:
mov	ax, [bp+varExistingEmsHndlNbr]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
mov	_DI_EmmHndlNbr,	[EMM_Table.handle+bx]
push	_DI_EmmHndlNbr
call	EMM_GetPageCount
pop	cx
cmp	ax, [bp+page_count]
jl	short @@NotEnoughEmsHndlPages
@@JmpJmpJmpDone:
jmp	@@Return_DI
@@NotEnoughEmsHndlPages:
mov	ax, 10
push	ax
mov	ax, (offset near_buffer+64h)
push	ax
mov	ax, [bp+varExistingEmsHndlNbr]
mov	dx, e_sizeof_EMM_RECORD
imul	dx
mov	bx, ax
push	[EMM_Table.handle+bx]
call	itoa
add	sp, 6
mov	ax, 0Ah
push	ax
mov	ax, (offset near_buffer+78h)
push	ax
push	_DI_EmmHndlNbr
call	itoa
add	sp, 6
mov	ax, 0Ah
push	ax
mov	ax, (offset near_buffer+96h)
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
mov	ax, (offset near_buffer+64h)
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
mov	ax, (offset near_buffer+78h)
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
mov	ax, (offset near_buffer+96h)
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
@@Return_DI:
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
mov	ax, 4000h
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
EMMDATAH_Nparas_Free= word ptr -2
nparas=	word ptr  6
push	bp
mov	bp, sp
sub	sp, 2
push	si
push	di
mov	si, [bp+nparas]
mov	[g_EMMDATAH_Level], 0
mov	ax, [sa_EMM_PFBA]
add	ax, [g_EMMDATAH_Level]
mov	di, ax
push	cs
call	near ptr EMMDATAH_Map
mov	ax, si
inc	ax
mov	si, ax
mov	ax, 4090
sub	ax, [g_EMMDATAH_Level]
mov	[bp+EMMDATAH_Nparas_Free], ax
cmp	[bp+EMMDATAH_Nparas_Free], si
jge	short loc_17A93
mov	ax, si
sub	ax, [bp+EMMDATAH_Nparas_Free]
push	ax
mov	ax, Alloc_EMM_too_small
push	ax
call	Allocation_Error
pop	cx
pop	cx
loc_17A93:
mov	ax, e_SAMB_MemSig1
push	ax
mov	ax, s_SAMB.MemSig1
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, e_SAMB_MemSig2
push	ax
mov	ax, s_SAMB.MemSig2
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
dec	ax
push	ax
mov	ax, s_SAMB.size
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, s_SAMB.used
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, s_SAMB.mark
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
inc	ax
add	[g_EMMDATAH_Level], ax
mov	ax, di
inc	ax
jmp	short $+2
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_EMMDATAH_AllocFirst ; sp-analysis failed
; Attributes: bp-based frame
proc EMM_EMMDATAH_AllocNext far
EMMDATAH_Nparas_Free= word ptr -2
nparas=	word ptr  6
push	bp
mov	bp, sp
sub	sp, 2
push	si
push	di
mov	si, [bp+nparas]
push	cs
call	near ptr EMMDATAH_Map
mov	ax, [sa_EMM_PFBA]
add	ax, [g_EMMDATAH_Level]
mov	di, ax
push	cs
call	near ptr EMMDATAH_Map
mov	ax, si
inc	ax
mov	si, ax
mov	ax, 4090
sub	ax, [g_EMMDATAH_Level]
mov	[bp+EMMDATAH_Nparas_Free], ax
cmp	[bp+EMMDATAH_Nparas_Free], si
jge	short loc_17B3B
mov	ax, si
sub	ax, [bp+EMMDATAH_Nparas_Free]
push	ax
mov	ax, Alloc_EMM_too_small
push	ax
call	Allocation_Error
pop	cx
pop	cx
loc_17B3B:
mov	ax, e_SAMB_MemSig1
push	ax
mov	ax, s_SAMB.MemSig1
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, e_SAMB_MemSig2
push	ax
mov	ax, s_SAMB.MemSig2
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
dec	ax
push	ax
mov	ax, s_SAMB.size
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, s_SAMB.used
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, 1
push	ax
mov	ax, s_SAMB.mark
push	ax
push	di
call	farpokew
add	sp, 6
mov	ax, si
inc	ax
add	[g_EMMDATAH_Level], ax
mov	ax, di
inc	ax
jmp	short $+2
pop	di
pop	si
mov	sp, bp
pop	bp
retf
endp EMM_EMMDATAH_AllocNext ; sp-analysis failed
; Attributes: bp-based frame
proc EMMDATAH_Save_Last_Offset far
push	bp
mov	bp, sp
mov	ax, [g_EMMDATAH_Level]
mov	[EMMDATAH_offset], ax
pop	bp
retf
endp EMMDATAH_Save_Last_Offset
; Attributes: bp-based frame
proc EMMDATAH_Restore_Last_Offset far
push	bp
mov	bp, sp
mov	ax, [EMMDATAH_offset]
mov	[g_EMMDATAH_Level], ax
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
jmp	short loc_17BDD
loc_17BD2:
cmp	[bp+si+File_Name], '.'
jnz	short loc_17BDC
mov	[bp+si+File_Name], 0
loc_17BDC:
inc	si
loc_17BDD:
cmp	si, 13
jl	short loc_17BD2
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
jnz	short loc_17C1B
xor	ax, ax
push	ax
mov	ax, LBXErr_EM_swap_fail
push	ax
push	[bp+FileName@]
call	Error_Handler
db 83h,	0C4h, 6
loc_17C1B:
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+Total_Bytes_left+2]
push	[word ptr bp+Total_Bytes_left]
call	LDIV@
mov	[bp+Needed_Pages], ax
xor	ax, ax
mov	dx, 16384
push	ax
push	dx
push	[word ptr bp+Total_Bytes_left+2]
push	[word ptr bp+Total_Bytes_left]
call	LMOD@
or	ax, dx
jz	short loc_17C49
inc	[bp+Needed_Pages]
loc_17C49:
mov	ah, 4Ch
mov	dx, [bp+EMM_Handle]
int	67h
mov	[bp+Handle_Page_Count],	bx
mov	ax, [bp+Needed_Pages]
cmp	ax, [bp+Handle_Page_Count]
jbe	short loc_17C65
mov	ax, offset str_LBX_File_Too_Long
push	ax
call	Exit_With_Message
pop	cx
loc_17C65:
call	EMM_GetPageFrame
mov	[bp+Buffer_Seg], ax
mov	[bp+First_Log_Page], 0
cmp	[bp+Buffer_Seg], 0
jnz	short loc_17C7B
jmp	loc_17D15
loc_17C7B:
push	[bp+FileName@]
call	lbx_open
pop	cx
mov	di, ax
or	di, di
jnz	short loc_17C8D
jmp	loc_17D15
loc_17C8D:
mov	[word ptr bp+EMM_Target_Offset+2], 0
mov	[word ptr bp+EMM_Target_Offset], 0
mov	[bp+Bytes_To_Copy], 16384
jmp	short loc_17CCF
loc_17C9E:
sub	[word ptr bp+Total_Bytes_left],	16384
sbb	[word ptr bp+Total_Bytes_left+2], 0
push	[bp+First_Log_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	di
push	[bp+Bytes_To_Copy]
push	[bp+Buffer_Seg]
call	lbx_read_sgmt
add	sp, 6
inc	[bp+First_Log_Page]
add	[word ptr bp+EMM_Target_Offset], 16384
loc_17CCB:
adc	[word ptr bp+EMM_Target_Offset+2], 0
loc_17CCF:
cmp	[word ptr bp+Total_Bytes_left+2], 0
loc_17CD3:
jg	short loc_17C9E
jl	short loc_17CDE
cmp	[word ptr bp+Total_Bytes_left],	16384
jnb	short loc_17C9E
loc_17CDE:
cmp	[word ptr bp+Total_Bytes_left+2], 0
loc_17CE2:
jl	short loc_17D0E
jg	short loc_17CEC
cmp	[word ptr bp+Total_Bytes_left],	0
jbe	short loc_17D0E
loc_17CEC:
mov	ax, [word ptr bp+Total_Bytes_left]
mov	[bp+Bytes_To_Copy], ax
loc_17CF2:
push	[bp+First_Log_Page]
push	[bp+EMM_Handle]
call	EMM_MapFourPages
pop	cx
pop	cx
push	di
push	[bp+Bytes_To_Copy]
push	[bp+Buffer_Seg]
call	lbx_read_sgmt
add	sp, 6
loc_17D0E:
push	di
call	lbx_close
pop	cx
loc_17D15:
pop	di
pop	si
loc_17D17:
mov	sp, bp
pop	bp
retf
endp UU_EMM_LBX_Load2Hnd ; sp-analysis failed
; Attributes: bp-based frame
proc EMM_Set_Minimum far
varEMS_MinKB= word ptr	6
push	bp
mov	bp, sp
mov	ax, [bp+varEMS_MinKB]
mov	[EMS_MinKB], ax
pop	bp
retf
endp EMM_Set_Minimum
; Attributes: bp-based frame
proc EMM_Error_Message far
varEmsErrTmpStr= byte ptr -14h
argStrBuff= word ptr  6
push	bp
mov	bp, sp
sub	sp, 14h
push	si
mov	si, [bp+argStrBuff]
mov	ax, 0Ah
push	ax
loc_17D34:
lea	ax, [bp+varEmsErrTmpStr]
push	ax
loc_17D38:
push	[EMS_MinKB]
loc_17D3C:
call	itoa
add	sp, 6
loc_17D44:
mov	ax, offset cnst_EMMErr_TooLow1
push	ax
loc_17D48:
push	si
loc_17D49:
call	strcpy
loc_17D4E:
pop	cx
pop	cx
loc_17D50:
lea	ax, [bp+varEmsErrTmpStr]
push	ax
loc_17D54:
push	si
loc_17D55:
call	strcat
loc_17D5A:
pop	cx
pop	cx
loc_17D5C:
mov	ax, offset cnst_EMMErr_TooLow2
push	ax
loc_17D60:
push	si
loc_17D61:
call	strcat
loc_17D66:
pop	cx
pop	cx
loc_17D68:
pop	si
loc_17D69:
mov	sp, bp
loc_17D6B:
pop	bp
locret_17D6C:
retf
endp EMM_Error_Message
ends seg013
