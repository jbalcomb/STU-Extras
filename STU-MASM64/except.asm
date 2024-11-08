https://masm32.com/board/index.php?topic=6614.0


; 64-BIT
;\masm32\bin\ml64 -c -Zp8 except.asm
;\masm32\bin\link /ENTRY:main /SUBSYSTEM:console /MACHINE:X64 /FIXED except.obj

include macamd64.inc
include seh.inc
includelib \masm32\lib64\msvcrt.lib
printf proto :ptr, :vararg
includelib \masm32\lib64\kernel32.lib
RtlUnwindEx proto :ptr, :ptr, :ptr, :ptr, :ptr, :ptr
ExitProcess proto :dword


.data
align 16
	tempExceptContext _CONTEXT <>
	exceptContext1 _CONTEXT <>
	exceptRecord1 _EXCEPTION_RECORD <>
	exceptContext2 _CONTEXT <>
	exceptRecord2 _EXCEPTION_RECORD <>
	exceptContext3 _CONTEXT <>
	exceptRecord3 _EXCEPTION_RECORD <>

	fmtString0 db "(Trapped in Main) Exception 0x%llx at 0x%llx",10,0
	fmtString1 db "(Trapped in proc1_0) Exception 0x%llx at 0x%llx",10,0	
	;fmtString2 db "(Trapped in proc2_0) Exception 0x%llx at 0x%llx",10,0		
	fmtString2 db "(Trapped in proc2_0) Exception 0x%llx at 0x%llx, rbx=0x%llx, rsi=0x%llx, rsi=0x%llx",10,0
.code

level0Handler proc pExceptionRecord:ptr, EstablisherFrame:ptr, pContextRecord:ptr, pDispatcherContext:ptr
	; Copy the information to eventually report it
	cld
	lea rdi, exceptRecord1
	mov rsi, rcx
	mov rcx, sizeof _EXCEPTION_RECORD
	rep movsb
	lea rdi, exceptContext1
	mov rsi, r8
	mov rcx, sizeof _CONTEXT
	rep movsb
	
	mov rax, TRYEXCEPT0
	mov qword ptr [r8]._CONTEXT._Rip, rax
	mov eax, ExceptionContinueExecution  ;continue execution
	
	ret
level0Handler endp

level1HandlerA proc pExceptionRecord:ptr, EstablisherFrame:ptr, pContextRecord:ptr, pDispatcherContext:ptr

	cld
	lea rdi, exceptRecord2
	mov rsi, rcx
	mov rcx, sizeof _EXCEPTION_RECORD
	rep movsb
	lea rdi, exceptContext2
	mov rsi, r8
	mov rcx, sizeof _CONTEXT
	rep movsb
	
	mov rax, TRYEXCEPT1
	mov qword ptr [r8]._CONTEXT._Rip, rax
	mov eax, ExceptionContinueExecution 
	
	ret
level1HandlerA endp

level1HandlerB proc pExceptionRecord:ptr, pEstablisherFrame:ptr, pContextRecord:ptr, pDispatcherContext:ptr
	test dword ptr [rcx]._EXCEPTION_RECORD.ExceptionFlags, EXCEPTION_UNWIND
	mov eax, ExceptionContinueSearch
	jnz @exit
	
	mov pExceptionRecord, rcx
	mov pEstablisherFrame, rdx
	mov pContextRecord, r8
	mov pDispatcherContext, r9

	cld
	lea rdi, exceptRecord3
	mov rsi, rcx
	mov rcx, sizeof _EXCEPTION_RECORD
	rep movsb
	lea rdi, exceptContext3
	mov rsi, r8
	mov rcx, sizeof _CONTEXT
	rep movsb
	
	mov rcx, pEstablisherFrame
	mov rdx, TRYEXCEPT2
	mov r8, pExceptionRecord
	mov r9, 0
	sub rsp, 30h
	mov rax, pDispatcherContext
      mov rax, [rax].DISPATCHER_CONTEXT.HistoryTable
      mov [rsp+28h], rax	
	lea rax, tempExceptContext
	mov [rsp+20h], rax
	call RtlUnwindEx ; Must not return. If this function returns there is an error
	add rsp, 30h
	
	;mov eax, ExceptionContinueExecution  ;continue execution
@exit:	
	ret
level1HandlerB endp


proc1_1 proc FRAME
	rex_push_reg rbp
	set_frame rbp,0
	END_PROLOGUE

	mov rcx, 0 ; cause divide by zero exception
	div rcx

	mov rsp, rbp
	pop rbp
	ret
proc1_1 endp

proc1_0 proc private FRAME:level1HandlerA
	rex_push_reg rbp
	set_frame rbp,0
	END_PROLOGUE
TRYSTART1=$	
	sub rsp, 20h
	call proc1_1
	add rsp, 20h
	jmp TRYEND1
TRYEXCEPT1=$	
	lea rcx, fmtString1
	mov edx, exceptRecord2.ExceptionCode
	mov r8, exceptContext2._Rip
	sub rsp, 20h
	call printf
	add rsp, 20h
TRYEND1=$
	pop rbp
	ret
proc1_0 endp

proc2_0 proc FRAME:level1HandlerB
	rex_push_reg rdi
	push_reg rsi
	push_reg rbx
	set_frame rsp, 0	
	END_PROLOGUE
	mov rbx, 33h
	mov rsi, 44h
	mov rdi, 55h
	mov rax, 0 ; Cause an Access Violation
	mov rax, [rax]
	jmp TRYEND2
TRYEXCEPT2=$	
	lea rcx, fmtString2
	mov edx, exceptRecord3.ExceptionCode
	mov r8, exceptContext3._Rip
	mov r9, exceptContext3._Rbx
	sub rsp, 30h
	mov rax, exceptContext3._Rsi
	mov [rsp+20h], rax
	mov rax, exceptContext3._Rdi
	mov [rsp+28h], rax
	call printf
	add rsp, 30h	
TRYEND2=$	
	pop rbx
	pop rsi
	pop rdi
	ret
proc2_0 endp

proc3_0 proc
	HLT ; Prileged instruction
	ret
proc3_0 endp

main proc FRAME:level0Handler
	rex_push_reg rbp
	set_frame rbp,0
	END_PROLOGUE
TRYSTART0=$	
	sub rsp, 20h
	call proc1_0 ; method 1
	add rsp, 20h
	
	sub rsp, 20h
	call proc2_0 ; method 2
	add rsp, 20h
	
	sub rsp, 20h
	call proc3_0 ; Unhandled in the callee
	add rsp, 20h
	
	jmp TRYEND0
TRYEXCEPT0=$		
	lea rcx, fmtString0
	mov edx, exceptRecord1.ExceptionCode
	mov r8, exceptContext1._Rip
	sub rsp, 20h
	call printf
	add rsp, 20h
TRYEND0=$
	mov rsp, rbp
	pop rbp
	mov rcx, 0
	call ExitProcess
main endp

end
