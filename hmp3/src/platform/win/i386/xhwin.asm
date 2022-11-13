; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: xhwin.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
; xHwin.asm   
;
;  Layer 3 hybrid window;
;
;  masm 6.14
;
;  Pentium III simd
;
;  hybrid is slower that fpu.  Revisit later
;  use fpu for 9th value, special mdct with aligned/padded ybuf
;  aligned read on some window coefs or special aligned window
;  or... just use fpu, most cycles are in mdct
;
;-------------------------------------------------
.686
.XMM
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT

;;movmem16  textequ <movaps>
movmem16  textequ <movups>

load16  textequ <movups>

;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;; tables from/shared with Hwin.asm
;
ALIGN 16
extrn _win:dword     ;;; test test test


ALIGN 16
ybuf label real4  
    yb0 real4 0.0
    yb1 real4 0.0
    yb2 real4 0.0
    yb3 real4 0.0
    yb4 real4 0.0
    yb5 real4 0.0
    yb6 real4 0.0
    yb7 real4 0.0
    yb8 real4 0.0
    yb9 real4 0.0
    yb10 real4 0.0
    yb11 real4 0.0
    yb12 real4 0.0
    yb13 real4 0.0
    yb14 real4 0.0
    yb15 real4 0.0
    yb16 real4 0.0
    yb17 real4 0.0


ALIGN 16
extrn _csa:dword     ;;; test test test

_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
extrn _mdct18:near
extrn _mdct6_3:near
;;==========================================================
_xhybrid proc public
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

w1a  textequ <dword ptr _win[edx][4*k]>
x1a  textequ <dword ptr [esi][4*k]>

w1b  textequ <dword ptr _win[edx][4*14-4*j]>
x1b  textequ <dword ptr [esi][4*14-4*j]>

w2a  textequ <dword ptr _win[edx][4*27+4*k]>
x2a  textequ <dword ptr [edi][4*9+4*k]>

w2b  textequ <dword ptr _win[edx][4*23-4*j]>
x2b  textequ <dword ptr [edi][4*5-4*j]>

y2   textequ <ybuf[4*k]>
y1   textequ <ybuf[4*9][4*k]>


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
k = 0
j = 0
rept 2
    movups  xmm2, w1b
    movups  xmm3, x1b
    mulps   xmm2, xmm3

    movups  xmm0, w1a
    movups  xmm1, x1a
    mulps   xmm0, xmm1
    shufps  xmm2, xmm2, 00011011b
    addps   xmm0, xmm2
    movups  y1, xmm0
    ;;
    movups  xmm6, w2b
    movups  xmm7, x2b
    mulps   xmm6, xmm7

    movups  xmm4, w2a
    movups  xmm5, x2a
    mulps   xmm4, xmm5
    shufps  xmm6, xmm6, 00011011b
    addps   xmm4, xmm6
    movups  y2, xmm4
k = k + 4
j = j + 4
endm
j = 5
    movss  xmm2, w1b
    movss  xmm3, x1b
    mulss  xmm2, xmm3

    movss  xmm0, w1a
    movss  xmm1, x1a
    mulss  xmm0, xmm1
    addss  xmm0, xmm2
    movss  y1, xmm0
    ;;
    movss  xmm6, w2b
    movss  xmm7, x2b
    mulss  xmm6, xmm7

    movss  xmm4, w2a
    movss  xmm5, x2a
    mulss  xmm4, xmm5
    addss  xmm4, xmm6
    movss  y2, xmm4

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
        jg  a200


    jmp exit
;;---------------------



_xhybrid endp
;;==========================================================
;;==========================================================
_xantialias proc public
;;
;; void antialias(float x[], int n)
;;
;; arguments
x_addr    textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>


npush = (0)
;; push
;;-----

w0  textequ <dword ptr _csa>
w1  textequ <dword ptr _csa[4*8]>


;; pre load all coefs
    load16  xmm2, w0
    load16  xmm3, w1
    load16  xmm6, w0[16]
    load16  xmm7, w1[16]
    

    mov edx, n
    mov eax, x_addr
    dec edx


a100:
    movups    xmm0, [eax][4*(18-4)]   ;; a
    shufps      xmm0, xmm0, 00011011b
    movups    xmm1, [eax][4*(18)]     ;; b

    movaps  xmm4, xmm0  ;; a
    mulps   xmm0, xmm2  ;; a*w0
    
    movaps  xmm5, xmm1  ;; b
    mulps   xmm1, xmm3  ;; b*w1

    mulps   xmm4, xmm3  ;; a*w1
    mulps   xmm5, xmm2  ;; b*w0
    
    addps   xmm0, xmm1  ;;  a
    subps   xmm5, xmm4  ;;  b  
    shufps  xmm0, xmm0, 00011011b

    movups    [eax][4*(18-4)], xmm0  
    movups    [eax][4*(18)], xmm5
;;---
    movups    xmm0, [eax][4*(18-2*4)]   ;; a
    shufps      xmm0, xmm0, 00011011b
    movups    xmm1, [eax][4*(18+4)]     ;; b

    movaps  xmm4, xmm0  ;; a
    mulps   xmm0, xmm6  ;; a*w0
    
    movaps  xmm5, xmm1  ;; b
    mulps   xmm1, xmm7  ;; b*w1

    mulps   xmm4, xmm7  ;; a*w1
    mulps   xmm5, xmm6  ;; b*w0
    
    addps   xmm0, xmm1  ;;  a
    subps   xmm5, xmm4  ;;  b
    shufps  xmm0, xmm0, 00011011b

    movups    [eax][4*(18-2*4)], xmm0  
    movups    [eax][4*(18+4)], xmm5

    add eax, 4*18
    dec edx
    jg  a100


;; half size bfly on last

    movups    xmm0, [eax][4*(18-4)]   ;; a
    shufps      xmm0, xmm0, 00011011b
    mulps   xmm0, xmm2
    shufps      xmm0, xmm0, 00011011b
    movups    [eax][4*(18-4)], xmm0  

    movups    xmm0, [eax][4*(18-2*4)]   ;; a
    shufps      xmm0, xmm0, 00011011b
    mulps   xmm0, xmm6
    shufps      xmm0, xmm0, 00011011b
    movups    [eax][4*(18-2*4)], xmm0  

exit:
;; pop

    ret

_xantialias endp
;====================================
;====================================
_TEXT ENDS
;====================================
          END

