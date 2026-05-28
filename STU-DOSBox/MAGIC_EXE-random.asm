; Attributes: bp-based frame
proc Set_Random_Seed far
n= dword ptr  6
push    bp
mov     bp, sp
mov     ax, [word ptr bp+n]
mov     [word ptr random_seed], ax
mov     ax, [word ptr bp+n+2]
mov     [word ptr random_seed+2], ax
pop     bp
retf
endp Set_Random_Seed
; Attributes: bp-based frame
proc Get_Random_Seed far
return_value= dword ptr -4
push    bp
mov     bp, sp
sub     sp, 4
mov     ax, [word ptr random_seed+2]
xor     dx, dx
add     dx, [word ptr random_seed]
adc     ax, 0
mov     [word ptr bp+return_value+2], ax
mov     [word ptr bp+return_value], dx
mov     dx, [word ptr bp+return_value+2]
mov     ax, [word ptr bp+return_value]
jmp     short $+2
mov     sp, bp
pop     bp
retf
endp Get_Random_Seed
; Attributes: bp-based frame
proc Randomize far
timer_value= dword ptr -4
push    bp
loc_1CC5A:
mov     bp, sp
sub     sp, 4
push    es
mov     ax, 0
mov     es, ax
assume es:nothing
mov     ax, [es:46Ch]
mov     [word ptr bp+timer_value+2], ax
mov     ax, [es:46Eh]
mov     [word ptr bp+timer_value], ax
pop     es
assume es:nothing
mov     ax, [word ptr bp+timer_value+2]
mov     [word ptr random_seed], ax
mov     ax, [word ptr bp+timer_value]
mov     [word ptr random_seed+2], ax
mov     sp, bp
pop     bp
retf
endp Randomize
; Attributes: bp-based frame
proc Random far
result= word ptr -4
return_value= word ptr -2
n= word ptr  6
push    bp
mov     bp, sp
sub     sp, 4
push    si
push    di
cmp     [bp+n], 0
jnz     short loc_1CC9C
mov     ax, offset cnst_RND_Error
push    ax
call    Exit_With_Message
pop     cx
loc_1CC9C:
mov     [bp+result], 0
push    si
push    di
mov     si, [word ptr random_seed]
mov     di, [word ptr random_seed+2]
mov     cx, 9
loc_1CCAE:
mov     ax, si
mov     bx, si
mov     dx, di
shr     dx, 1
rcr     bx, 1
xor     ax, bx
shr     dx, 1
rcr     bx, 1
xor     ax, bx
shr     dx, 1
rcr     bx, 1
shr     dx, 1
rcr     bx, 1
xor     ax, bx
shr     dx, 1
rcr     bx, 1
shr     dx, 1
rcr     bx, 1
xor     ax, bx
shr     dx, 1
xor     al, dh
mov     dx, ax
shr     dx, 1
rcl     [bp+result], 1
shr     ax, 1
rcr     di, 1
rcr     si, 1
loop    loc_1CCAE
cmp     si, 0
jnz     short @@Done
cmp     di, 0
jnz     short @@Done
mov     si, 12478
@@Done:
mov     [word ptr random_seed], si
mov     [word ptr random_seed+2], di
pop     di
pop     si
mov     ax, [bp+result]
cwd
idiv    [bp+n]
inc     dx
mov     [bp+return_value], dx
mov     ax, [bp+return_value]
jmp     short $+2
pop     di
pop     si
mov     sp, bp
pop     bp
retf
endp Random
