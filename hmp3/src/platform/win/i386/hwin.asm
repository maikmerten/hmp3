; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: hwin.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
;   
; Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.  
;       
; The contents of this file, and the files included with this file, 
; are subject to the current version of the RealNetworks Public 
; Source License (the "RPSL") available at 
; http://www.helixcommunity.org/content/rpsl unless you have licensed 
; the file under the current version of the RealNetworks Community 
; Source License (the "RCSL") available at 
; http://www.helixcommunity.org/content/rcsl, in which case the RCSL 
; will apply. You may also obtain the license terms directly from 
; RealNetworks.  You may not use this file except in compliance with 
; the RPSL or, if you have a valid RCSL with RealNetworks applicable 
; to this file, the RCSL.  Please see the applicable RPSL or RCSL for 
; the rights, obligations and limitations governing use of the 
; contents of the file. 
;   
; This file is part of the Helix DNA Technology. RealNetworks is the 
; developer of the Original Code and owns the copyrights in the 
; portions it created. 
;   
; This file, and the files included with this file, is distributed 
; and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY 
; KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS 
; ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES 
; OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET 
; ENJOYMENT OR NON-INFRINGEMENT. 
;  
; Technology Compatibility Kit Test Suite(s) Location:  
;    http://www.helixcommunity.org/content/tck  
;  
; Contributor(s):  
;   
; ***** END LICENSE BLOCK *****  
        
;--------------------------------------------
; hwin.asm   
;
;  Layer 3 hybrid window;
;
;  masm 6.11
;
;  target machine Pentium II
;
;-------------------------------------------------
.486P
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT
;
;
MIXED_BLOCK = 0
;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;; align 16 for table sharing with xHwin.asm
;
ALIGN 16
_win  dd 4*36 dup (0)


ybuf   real4  18 dup (0.0)

ALIGN 16
_csa  dd 2*8 dup (0)


;; public for sharing with xHwin.asm
public _win
public _csa



_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
extrn _mdct18:near
extrn _mdct6_3:near
extrn _mdct6_3B:near
;;==========================================================
_alias_init_addr proc public
    mov eax, offset _csa
    ret
_alias_init_addr endp
;;-----------------------------------------
_hwin_init_addr proc public
    mov eax, offset _win
    ret
_hwin_init_addr endp
;;==========================================================
_FreqInvert proc public
;;
;; void FreqInvert(float y[32][18], int nsb)
;;
;; arguments
y_addr    textequ <dword ptr [esp+4*(1+npush)]>
nsb       textequ <dword ptr [esp+4*(2+npush)]>



npush = 2
    push    esi
    push    ecx
;;-----
y   textequ <dword ptr [esi][4*18]>

    mov esi, y_addr
    mov ecx, nsb
    mov edx, 80000000h


a100:

k = 1
rept 9
    mov eax, y[k*4]
    xor eax, edx
    mov y[k*4], eax
k = k + 2
endm
    
    add esi, 4*36
    sub ecx, 2
    jg  a100

exit:
    pop ecx
    pop esi


    ret
_FreqInvert endp
;;==========================================================
;;==========================================================
IF MIXED_BLOCK
_hybrid proc public
;;
;; void  hybrid(float x1[], float x2[], float yout[],
;;            int btype, int nlong, int ntot)
;;
;; arguments
x1_addr    textequ <dword ptr [esp+4*(1+npush)]>
x2_addr    textequ <dword ptr [esp+4*(2+npush)]>
yout_addr  textequ <dword ptr [esp+4*(3+npush)]>
btype      textequ <dword ptr [esp+4*(4+npush)]>
nlong      textequ <dword ptr [esp+4*(5+npush)]>
ntot       textequ <dword ptr [esp+4*(6+npush)]>

arg1 textequ <dword ptr [esp]>
arg2 textequ <dword ptr [esp+4]>

npush = (4+2)
    push    esi
    push    edi
    push    ebx
    push    ecx

    sub esp, 2*4
;;-----

;; eax counts up  0,       8
;; ecx count down 8, 7, ...0

w1a  textequ <dword ptr _win[edx][eax]>
w1b  textequ <dword ptr _win[edx][4*9+ecx]>

w2a  textequ <dword ptr _win[edx][4*27+eax]>
w2b  textequ <dword ptr _win[edx][4*18+ecx]>

x1a  textequ <dword ptr [esi][eax]>
x1b  textequ <dword ptr [esi][4*9+ecx]>
x2a  textequ <dword ptr [edi][4*9+eax]>
x2b  textequ <dword ptr [edi][ecx]>

y2   textequ <ybuf[eax]>
y1   textequ <ybuf[4*9][eax]>


    mov edx, btype
    cmp edx, 2
    jne a001
    xor edx, edx
a001:
    imul    edx, 4*36
    mov btype, edx      ;; preserve during mdct call

    mov bl, byte ptr nlong
    mov bh, byte ptr ntot
    sub bh, bl

    mov esi, x1_addr
    mov edi, x2_addr

    test bl, bl
    jle short_blocks

a200:
    mov ecx, 4*8
    xor eax, eax

a100:
    fld w1a
    fld w2a
    fld x2a
    fmulp   st(1), st
    fld x1a
    fmulp   st(2), st

    fld w2b
    fld x2b
    fmulp   st(1), st

    fld w1b
    fld x1b
    fmulp   st(1), st

    fxch
    faddp   st(2), st
    faddp   st(2), st
    
    fstp    y2
    fstp    y1

    add eax, 4
    sub ecx, 4
    jge a100

        mov ecx, yout_addr
        mov arg2, ecx
        add ecx, 4*18
        mov yout_addr, ecx
        mov arg1, offset ybuf
        call _mdct18

        mov edx, btype      ;; not preserved by call
        add esi, 4*18
        add edi, 4*18
        dec bl
        jg  a200
;;---------------------
short_blocks:
    test bh, bh
    jg do_short_blocks

exit:
    add esp, 2*4
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret


;;---------------------
;;---------------------
do_short_blocks:
;; as of 11/15/99 not tested

w   textequ <dword ptr _win[2*4*36]>
x1  textequ <dword ptr [esi]>
x2  textequ <dword ptr [edi]>
y   textequ <ybuf>


b100:
;;    y[0]   = win[2][8]*x1[12+2] + win[2][9] *x1[12+3];
;;    y[1]   = win[2][7]*x1[12+1] + win[2][10]*x1[12+4];
    fld w[4*8]
    fmul    x1[4*(12+2)]
    fld w[4*7]
    fmul    x1[4*(12+1)]
    ;;
    fld w[4*9]
    fmul    x1[4*(12+3)]
    faddp   st(2), st
    fld w[4*10]
    fmul    x1[4*(12+4)]
    faddp   st(1), st
    fxch
    fstp    y[4*0]
    fstp    y[4*1]
;;    y[2]   = win[2][6]*x1[12+0] + win[2][11]*x1[12+5];
;;    y[3]   = win[2][0]*x1[6+0]  + win[2][5] *x1[6+5];
    fld w[4*6]
    fmul    x1[4*(12+0)]
    fld w[4*0]
    fmul    x1[4*(6+0)]
    ;;
    fld w[4*11]
    fmul    x1[4*(12+5)]
    faddp   st(2), st
    fld w[4*5]
    fmul    x1[4*(6+5)]
    faddp   st(1), st
    fxch
    fstp    y[4*2]
    fstp    y[4*3]
;;    y[4]   = win[2][1]*x1[6+1] + win[2][4] *x1[6+4];
;;    y[5]   = win[2][2]*x1[6+2] + win[2][3] *x1[6+3];
    fld w[4*1]
    fmul    x1[4*(6+1)]
    fld w[4*2]
    fmul    x1[4*(6+2)]
    ;;
    fld w[4*4]
    fmul    x1[4*(6+4)]
    faddp   st(2), st
    fld w[4*3]
    fmul    x1[4*(6+3)]
    faddp   st(1), st
    fxch
    fstp    y[4*4]
    fstp    y[4*5]
;;    y[6+0] = win[2][8]*x2[2] + win[2][9] *x2[3];
;;    y[6+1] = win[2][7]*x2[1] + win[2][10]*x2[4];
    fld w[4*8]
    fmul    x2[4*(2)]
    fld w[4*7]
    fmul    x2[4*(1)]
    ;;
    fld w[4*9]
    fmul    x2[4*(3)]
    faddp   st(2), st
    fld w[4*10]
    fmul    x2[4*(4)]
    faddp   st(1), st
    fxch
    fstp    y[4*6]
    fstp    y[4*7]
;;    y[6+2] = win[2][6]*x2[0]    + win[2][11]*x2[5];
;;    y[6+3] = win[2][0]*x1[12+0] + win[2][5] *x1[12+5];
    fld w[4*6]
    fmul    x2[4*(0)]
    fld w[4*0]
    fmul    x1[4*(12+0)]
    ;;
    fld w[4*11]
    fmul    x2[4*(5)]
    faddp   st(2), st
    fld w[4*5]
    fmul    x1[4*(12+5)]
    faddp   st(1), st
    fxch
    fstp    y[4*8]
    fstp    y[4*9]
;;    y[6+4] = win[2][1]*x1[12+1] + win[2][4] *x1[12+4];
;;    y[6+5] = win[2][2]*x1[12+2] + win[2][3] *x1[12+3];
    fld w[4*1]
    fmul    x1[4*(12+1)]
    fld w[4*2]
    fmul    x1[4*(12+2)]
    ;;
    fld w[4*4]
    fmul    x1[4*(12+4)]
    faddp   st(2), st
    fld w[4*3]
    fmul    x1[4*(12+3)]
    faddp   st(1), st
    fxch
    fstp    y[4*10]
    fstp    y[4*11]
;;    y[12+0] = win[2][8]*x2[6+2] + win[2][9] *x2[6+3];
;;    y[12+1] = win[2][7]*x2[6+1] + win[2][10]*x2[6+4];
    fld w[4*8]
    fmul    x2[4*(6+2)]
    fld w[4*7]
    fmul    x2[4*(6+1)]
    ;;
    fld w[4*9]
    fmul    x2[4*(6+3)]
    faddp   st(2), st
    fld w[4*10]
    fmul    x2[4*(6+4)]
    faddp   st(1), st
    fxch
    fstp    y[4*12]
    fstp    y[4*13]
;;    y[12+2] = win[2][6]*x2[6+0] + win[2][11]*x2[6+5];
;;    y[12+3] = win[2][0]*x2[0] +   win[2][5] *x2[5];
    fld w[4*6]
    fmul    x2[4*(6+0)]
    fld w[4*0]
    fmul    x2[4*(0)]
    ;;
    fld w[4*11]
    fmul    x2[4*(6+5)]
    faddp   st(2), st
    fld w[4*5]
    fmul    x2[4*(5)]
    faddp   st(1), st
    fxch
    fstp    y[4*14]
    fstp    y[4*15]
;;    y[12+4] = win[2][1]*x2[1] + win[2][4] *x2[4];
;;    y[12+5] = win[2][2]*x2[2] + win[2][3] *x2[3];
    fld w[4*1]
    fmul    x2[4*(1)]
    fld w[4*2]
    fmul    x2[4*(2)]
    ;;
    fld w[4*4]
    fmul    x2[4*(4)]
    faddp   st(2), st
    fld w[4*3]
    fmul    x2[4*(3)]
    faddp   st(1), st
    fxch
    fstp    y[4*16]
    fstp    y[4*17]


        mov ecx, yout_addr
        mov arg2, ecx
        add ecx, 4*18
        mov yout_addr, ecx
        mov arg1, offset ybuf
        call _mdct6_3

        add esi, 4*18
        add edi, 4*18
        dec bh
        jg  b100


    jmp exit
;;---------------------



_hybrid endp
ENDIF
;;==========================================================
;;==========================================================
;;==========================================================
_hybridLong proc public
;;
;; void  hybrid(float x1[], float x2[], float yout[],
;;            int btype, int nlong, int clear_flag)
;;
;; long blocks only  block types  0, 1, 3
;; no mixed
;;
;; arguments
x1_addr    textequ <dword ptr [esp+4*(1+npush)]>
x2_addr    textequ <dword ptr [esp+4*(2+npush)]>
yout_addr  textequ <dword ptr [esp+4*(3+npush)]>
btype      textequ <dword ptr [esp+4*(4+npush)]>
nlong      textequ <dword ptr [esp+4*(5+npush)]>
clear_flag textequ <dword ptr [esp+4*(6+npush)]>



arg1 textequ <dword ptr [esp]>
arg2 textequ <dword ptr [esp+4]>

npush = (4+2)
    push    esi
    push    edi
    push    ebx
    push    ecx

    sub esp, 2*4
;;-----

;; eax counts up  0,       8
;; ecx count down 8, 7, ...0

w1a  textequ <dword ptr _win[edx][eax]>
w1b  textequ <dword ptr _win[edx][4*9+ecx]>

w2a  textequ <dword ptr _win[edx][4*27+eax]>
w2b  textequ <dword ptr _win[edx][4*18+ecx]>

x1a  textequ <dword ptr [esi][eax]>
x1b  textequ <dword ptr [esi][4*9+ecx]>
x2a  textequ <dword ptr [edi][4*9+eax]>
x2b  textequ <dword ptr [edi][ecx]>

y2   textequ <ybuf[eax]>
y1   textequ <ybuf[4*9][eax]>


    mov edx, btype
a001:
    imul    edx, 4*36
    mov btype, edx      ;; preserve during mdct call

    mov bl, byte ptr nlong

    mov esi, x1_addr
    mov edi, x2_addr

a200:
    mov ecx, 4*8
    xor eax, eax

a100:
    fld w1a
    fld w2a
    fld x2a
    fmulp   st(1), st
    fld x1a
    fmulp   st(2), st

    fld w2b
    fld x2b
    fmulp   st(1), st

    fld w1b
    fld x1b
    fmulp   st(1), st

    fxch
    faddp   st(2), st
    faddp   st(2), st
    
    fstp    y2
    fstp    y1

    add eax, 4
    sub ecx, 4
    jge a100

        mov ecx, yout_addr
        mov arg2, ecx
        add ecx, 4*18
        mov yout_addr, ecx
        mov arg1, offset ybuf
        call _mdct18        ;; C may not preserve ecx

        mov edx, btype      ;; not preserved by call
        add esi, 4*18
        add edi, 4*18
        dec bl
        jg  a200

    mov eax, clear_flag
    test eax, eax
    jne clear_yout

;;---------------------
exit:
    add esp, 2*4
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret

;;---------------------
clear_yout:
;;-------------
;; clear remainder of output buffer, 
;; may have leftover data from short block
;;
    mov ebx, nlong
    sub ebx, 32
    jge exit        ;; insurance, should not happen
    mov esi, yout_addr
    xor eax, eax
    xor ecx, ecx
    lea ebx, [ebx+ebx*8]   ;; loop count = 9*(32-nlong) 
a400:
    mov [esi], eax
    mov [esi][4], ecx
    add esi, 2*4
    inc ebx
    jl  a400

    jmp exit

_hybridLong endp
;;==========================================================
;;==========================================================
;;==========================================================
_hybridShort proc public
;;
;; void  hybrid(float x1[], float x2[], float yout[], int ntot)
;;
;;  short blocks only
;;
;;
;;
;; arguments
x1_addr    textequ <dword ptr [esp+4*(1+npush)]>
x2_addr    textequ <dword ptr [esp+4*(2+npush)]>
yout_addr  textequ <dword ptr [esp+4*(3+npush)]>
ntot       textequ <dword ptr [esp+4*(4+npush)]>

arg1 textequ <dword ptr [esp]>
arg2 textequ <dword ptr [esp+4]>

npush = (4+2)
    push    esi
    push    edi
    push    ebx
    push    ecx

    sub esp, 2*4
;;-----

    mov bh, byte ptr ntot
    mov esi, x1_addr
    mov edi, x2_addr

;;---------------------
;;---------------------

w   textequ <dword ptr _win[2*4*36]>
x1  textequ <dword ptr [esi]>
x2  textequ <dword ptr [edi]>
y   textequ <ybuf>


b100:
;;    y[0]   = win[2][8]*x1[12+2] + win[2][9] *x1[12+3];
;;    y[1]   = win[2][7]*x1[12+1] + win[2][10]*x1[12+4];
    fld w[4*8]
    fmul    x1[4*(12+2)]
    fld w[4*7]
    fmul    x1[4*(12+1)]
    ;;
    fld w[4*9]
    fmul    x1[4*(12+3)]
    faddp   st(2), st
    fld w[4*10]
    fmul    x1[4*(12+4)]
    faddp   st(1), st
    fxch
    fstp    y[4*0]
    fstp    y[4*1]
;;    y[2]   = win[2][6]*x1[12+0] + win[2][11]*x1[12+5];
;;    y[3]   = win[2][0]*x1[6+0]  + win[2][5] *x1[6+5];
    fld w[4*6]
    fmul    x1[4*(12+0)]
    fld w[4*0]
    fmul    x1[4*(6+0)]
    ;;
    fld w[4*11]
    fmul    x1[4*(12+5)]
    faddp   st(2), st
    fld w[4*5]
    fmul    x1[4*(6+5)]
    faddp   st(1), st
    fxch
    fstp    y[4*2]
    fstp    y[4*3]
;;    y[4]   = win[2][1]*x1[6+1] + win[2][4] *x1[6+4];
;;    y[5]   = win[2][2]*x1[6+2] + win[2][3] *x1[6+3];
    fld w[4*1]
    fmul    x1[4*(6+1)]
    fld w[4*2]
    fmul    x1[4*(6+2)]
    ;;
    fld w[4*4]
    fmul    x1[4*(6+4)]
    faddp   st(2), st
    fld w[4*3]
    fmul    x1[4*(6+3)]
    faddp   st(1), st
    fxch
    fstp    y[4*4]
    fstp    y[4*5]
;;    y[6+0] = win[2][8]*x2[2] + win[2][9] *x2[3];
;;    y[6+1] = win[2][7]*x2[1] + win[2][10]*x2[4];
    fld w[4*8]
    fmul    x2[4*(2)]
    fld w[4*7]
    fmul    x2[4*(1)]
    ;;
    fld w[4*9]
    fmul    x2[4*(3)]
    faddp   st(2), st
    fld w[4*10]
    fmul    x2[4*(4)]
    faddp   st(1), st
    fxch
    fstp    y[4*6]
    fstp    y[4*7]
;;    y[6+2] = win[2][6]*x2[0]    + win[2][11]*x2[5];
;;    y[6+3] = win[2][0]*x1[12+0] + win[2][5] *x1[12+5];
    fld w[4*6]
    fmul    x2[4*(0)]
    fld w[4*0]
    fmul    x1[4*(12+0)]
    ;;
    fld w[4*11]
    fmul    x2[4*(5)]
    faddp   st(2), st
    fld w[4*5]
    fmul    x1[4*(12+5)]
    faddp   st(1), st
    fxch
    fstp    y[4*8]
    fstp    y[4*9]
;;    y[6+4] = win[2][1]*x1[12+1] + win[2][4] *x1[12+4];
;;    y[6+5] = win[2][2]*x1[12+2] + win[2][3] *x1[12+3];
    fld w[4*1]
    fmul    x1[4*(12+1)]
    fld w[4*2]
    fmul    x1[4*(12+2)]
    ;;
    fld w[4*4]
    fmul    x1[4*(12+4)]
    faddp   st(2), st
    fld w[4*3]
    fmul    x1[4*(12+3)]
    faddp   st(1), st
    fxch
    fstp    y[4*10]
    fstp    y[4*11]
;;    y[12+0] = win[2][8]*x2[6+2] + win[2][9] *x2[6+3];
;;    y[12+1] = win[2][7]*x2[6+1] + win[2][10]*x2[6+4];
    fld w[4*8]
    fmul    x2[4*(6+2)]
    fld w[4*7]
    fmul    x2[4*(6+1)]
    ;;
    fld w[4*9]
    fmul    x2[4*(6+3)]
    faddp   st(2), st
    fld w[4*10]
    fmul    x2[4*(6+4)]
    faddp   st(1), st
    fxch
    fstp    y[4*12]
    fstp    y[4*13]
;;    y[12+2] = win[2][6]*x2[6+0] + win[2][11]*x2[6+5];
;;    y[12+3] = win[2][0]*x2[0] +   win[2][5] *x2[5];
    fld w[4*6]
    fmul    x2[4*(6+0)]
    fld w[4*0]
    fmul    x2[4*(0)]
    ;;
    fld w[4*11]
    fmul    x2[4*(6+5)]
    faddp   st(2), st
    fld w[4*5]
    fmul    x2[4*(5)]
    faddp   st(1), st
    fxch
    fstp    y[4*14]
    fstp    y[4*15]
;;    y[12+4] = win[2][1]*x2[1] + win[2][4] *x2[4];
;;    y[12+5] = win[2][2]*x2[2] + win[2][3] *x2[3];
    fld w[4*1]
    fmul    x2[4*(1)]
    fld w[4*2]
    fmul    x2[4*(2)]
    ;;
    fld w[4*4]
    fmul    x2[4*(4)]
    faddp   st(2), st
    fld w[4*3]
    fmul    x2[4*(3)]
    faddp   st(1), st
    fxch
    fstp    y[4*16]
    fstp    y[4*17]


        mov ecx, yout_addr
        mov arg2, ecx
        add ecx, 4*6       ;; mdct output organized x[3][192]
        mov yout_addr, ecx
        mov arg1, offset ybuf
        call _mdct6_3B

        add esi, 4*18
        add edi, 4*18
        dec bh
        jg  b100

;;-------------
;; clear remainder of output buffer, 
;; may have leftover data from long block
;;
    mov ebx, ntot
    sub ebx, 32
    jge exit        ;; insurance, should not happen
    add ebx, ebx
    mov esi, yout_addr
    xor eax, eax
    xor ecx, ecx
    lea ebx, [ebx+ebx*2]
b200:
    mov [esi], eax
    mov [esi][4*192], ecx
    mov [esi][2*4*192], eax
    add esi, 4
    inc ebx
    jl  b200
    
;;-------------
exit:
    add esp, 2*4
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret


_hybridShort endp
;;==========================================================
;;==========================================================
;;==========================================================
_antialias proc public
;;
;; void antialias(float x[], int n)
;;
;; arguments
x_addr    textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>


npush = (0)
;; push
;;-----

a   textequ <dword ptr [eax][4*(17-k)]>
b   textequ <dword ptr [eax][4*(18+k)]>
w0  textequ <dword ptr _csa[4*k]>
w1  textequ <dword ptr _csa[4*k][4*8]>


    mov edx, n
    mov eax, x_addr
    dec edx


a100:

k = 0
rept 8
    fld w1
    fld w0

    fld st(0)
    fld st(2)

    fld a
    fmul st(3), st

    fld b
    fmul    st(2), st

    fmulp   st(3), st
    
    fmulp   st(4), st
    faddp   st(2), st
    fsubrp  st(2), st
    
    fstp    a
    fstp    b   

k = k + 1
endm

    add eax, 4*18
    dec edx
    jg  a100


;; half size bfly on last

k = 0
rept 8
    fld a
    fld w0
    fmulp   st(1), st
    fstp    a
k = k + 1
endm
            



exit:
;; pop

    ret

_antialias endp
;====================================
;====================================
_TEXT ENDS
;====================================
          END

