

/*

unit_race < 15

mov     ax, [bp+unit_race]
mov     _DX_print_y_offset, 18h         ; sizeof(s_RACE)
imul    _DX_print_y_offset
mov     bx, ax
push    [word ptr bx+1D86h]             ; src
push    [word ptr _unit_race_table.Name+bx] ; src


unit_race >= 15

mov     bx, [bp+unit_race]
shl     bx, 1                           ; sizeof
push    [word ptr bx+1EBAh]
push    [(_unit_race_table.Growth_Mod+120h)+bx]

WTF?
unit_race * 2 (sizeof),
because value to retrieve is 2-byte
segment offset / near pointer


1D86


1EBA

dseg:1D86

1EBA - 1D86 = 134  308 / 24 = 12.833333333333333333333333333333

120  288 / 24 = 12

            / * 0x14 * / int16_t Growth_Mod;

1D86 + 120 + 14 + (2 * unit_race)

1D86 + 120 = 1EA6 + 14 = 1EBA + 1E = 1ED8
dseg:1ED8 DE 26                                           rtn1 dw offset cnst_Rlm5_Arcane         ; "Arcane"


*/