
; https://masm32.com/board/index.php?topic=4190.msg66573#msg66573

; GUI #
include win64a.inc
MI_PROCESS_CREATE	equ 1
MI_PROCESS_TERMINATE	equ 2
MI_EXIT 		equ 3
IDR_MAINMENU	equ 30

.code
WinMain proc
local msg:MSG

      xor ebx,ebx

	mov edi,offset ClassName
	mov esi,IMAGE_BASE
	mov ecx,offset FileName
	invoke LoadCursorFromFile
	push rax 	;hIconSm
	push rdi	;lpszClassName
	push IDR_MAINMENU;lpszMenuName
	push COLOR_WINDOW;hbrBackground
	push 10003h	;hCursor
	push rax        ;hIcon
	push rsi	;hInstance
	push rbx        ;cbClsExtra & cbWndExtra
	pushaddr WndProc;lpfnWndProc
	push sizeof WNDCLASSEX;cbSize & style
	invoke RegisterClassEx,esp	;addr WNDCLASSEX	
	push rbx
	push rsi	;rsi=400000h
	shl esi,9	;rsi=CW_USEDEFAULT
	push rbx
	push rbx
	push rsi
	push rsi
	push rsi
	push rsi
	sub esp,20h
    	invoke CreateWindowEx,WS_EX_CLIENTEDGE,edi,edi,WS_OVERLAPPEDWINDOW or WS_VISIBLE
	invoke GetMenu,eax
	mov hMenu,rax
    	lea edi,msg
@@:   invoke GetMessage,edi,0,0,0
	invoke DispatchMessage,edi
      jmp @b
WinMain endp

WndProc proc hWnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM

local progStartInfo:STARTUPINFO

	mov edi,offset processInfo
	mov esi,offset proExitCode
	mov hWnd,rcx
	mov wParam,r8
	mov lParam,r9

	cmp  edx,WM_DESTROY
	je   wmDESTROY
	
	cmp  edx,WM_COMMAND
	je   wmCOMMAND
	
	cmp  edx,WM_INITMENUPOPUP
	je   wmINITMENUPOPUP
	
	leave
	jmp DefWindowProc

wmDESTROY:invoke ExitProcess,NULL
wmINITMENUPOPUP:invoke GetExitCodeProcess,[rdi+PROCESS_INFORMATION.hProcess],esi
	or eax,eax
	jz @f;GetExitCodeProcess_TRUE
	cmp dword ptr [rsi],STILL_ACTIVE;cmp     [proExitCode],STILL_ACTIVE
	jne @f;     GetExitCodeProcess_STILL_ACTIVE
	xor r9d,r9d;MF_ENABLED
	mov r8d,MF_GRAYED
	jmp @0
@@:	mov r9d,MF_GRAYED
	xor r8d,r8d;MF_ENABLED
@0:	invoke EnableMenuItem,hMenu,0;MI_PROCESS_CREATE
      invoke EnableMenuItem,hMenu,MI_PROCESS_TERMINATE,MF_ENABLED,MF_GRAYED
      jmp wmBYE
wmCOMMAND:movzx eax,word ptr wParam
      or r9,r9	;cmp lParam,0
	jnz wmBYE
	cmp rax,MI_EXIT
	ja  wmBYE
	jmp  [menu_handlers+rax*8]

PROCESS_CREATE:cmp [rdi+PROCESS_INFORMATION.hProcess],rbx
	je pi_hProcess_IS_0
	invoke CloseHandle,[rdi+PROCESS_INFORMATION.hProcess]
	mov [rdi+PROCESS_INFORMATION.hProcess],rbx
pi_hProcess_IS_0:
	lea esi,progStartInfo
	invoke GetStartupInfo,esi
	mov edx,offset progName;command line
	invoke CreateProcess,0,,0,0,0,NORMAL_PRIORITY_CLASS,0,0,rsi,rdi ;receives PROCESS_INFORMATION
	invoke CloseHandle,[rdi+PROCESS_INFORMATION.hThread]
	invoke Sleep,20600
	invoke TerminateProcess,[rdi+PROCESS_INFORMATION.hProcess],0
	invoke CloseHandle,[rdi+PROCESS_INFORMATION.hProcess]
	mov [rdi+PROCESS_INFORMATION.hProcess],rbx;0
        jmp wmBYE
TERMINATE:invoke GetExitCodeProcess,[rdi+PROCESS_INFORMATION.hProcess],esi;proExitCode
	cmp dword ptr [rsi],STILL_ACTIVE
	jne proExitCode_NOT_STILL_ACTIVE;a4;
	invoke TerminateProcess,[rdi+PROCESS_INFORMATION.hProcess],0
proExitCode_NOT_STILL_ACTIVE:
	invoke CloseHandle,[rdi+PROCESS_INFORMATION.hProcess]
	mov [rdi+PROCESS_INFORMATION.hProcess],rbx;0
        jmp wmBYE

EXIT:	;ax=MI_EXIT
        invoke DestroyWindow
wmBYE:  leave
        retn
menu_handlers	dq wmBYE,PROCESS_CREATE, TERMINATE, EXIT
WndProc endp
;---------------------------------------
ClassName       db 'Uncle Remus tales:#14 Process',0
hMenu		dq ?
proExitCode	dq ?;process exit code
progName	db '"C:\Program Files (x86)\Windows Media Player\wmplayer.exe" ';,0
arg db '"E:\Uncle Remus tales\37\37c\movie3.wmv"',0
FileName	db "br_Rabbit3.cur",0
processInfo	PROCESS_INFORMATION <>
end


