; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: xsbt.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
;
;  Layer 3 encode sbt
;
;  masm 6.14
;
;  Pentium III simd
;
;-------------------------------------------------
        
.686
.XMM
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT

;;movmem16  textequ <movaps>
movmem16  textequ <movups>

;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'


ALIGN 16
_xwincoef label real4 
include tableawx.inc
;;(512+8) dup (0.0)

;; buf must be padded for window dummy output
align 16
bufb dd 33 dup (0)
align 16
bufa dd 33 dup (0)

align 16
coef      dd        32 dup (0)


;;public _xwincoef

;;
input_ptr   dd 0
output_ptr  dd 0

;;
counter     db        0
first_pass  db        1


_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
window proc
;;
;;
;;****** caution, writes 33 output values (last is dummy 0.0) ******
;;

;;--- middle block plus first (first by fpu) ----------------
b       textequ <dword ptr bufb>

wcoef    textequ <dword ptr _xwincoef[ebx][4*8]>
xor ebx, ebx

;; 4 accums, total of 16 results
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    xorps   xmm3, xmm3


;; accum bx components
vbuf    textequ <dword ptr [esi][4*(15-3)]>
	mov esi, input_ptr
    mov ecx, 8
a200:
    movups  xmm4, vbuf
    movups  xmm5, vbuf[-16]
    movups  xmm6, vbuf[-2*16]
    movups  xmm7, vbuf[-3*16]
    mulps   xmm4, wcoef
    mulps   xmm5, wcoef[16]
    mulps   xmm6, wcoef[2*16]
    mulps   xmm7, wcoef[3*16]

    addps   xmm0, xmm4
    addps   xmm1, xmm5
    addps   xmm2, xmm6
    addps   xmm3, xmm7

    add esi, 4*64   ;; hop vbuf by 64 
    add ebx, 4*16   ;; coefs
    dec ecx
    jg  a200

;; reorder accum
    shufps  xmm0, xmm0, 00011011b
    shufps  xmm1, xmm1, 00011011b
    shufps  xmm2, xmm2, 00011011b
    shufps  xmm3, xmm3, 00011011b


;; accum si components
vbuf    textequ <dword ptr [esi][4*(17)]>
fvbuf   textequ <dword ptr [esi][4*(16)]>
fcoef   textequ <dword ptr _xwincoef[eax]>
	mov esi, input_ptr
    mov ecx, 8

    xor eax, eax
    fldz

a210:
    movups  xmm4, vbuf
    movups  xmm5, vbuf[16]
    movups  xmm6, vbuf[2*16]
    movups  xmm7, vbuf[3*16]
    mulps   xmm4, wcoef
    mulps   xmm5, wcoef[16]
    mulps   xmm6, wcoef[2*16]
    mulps   xmm7, wcoef[3*16]

    addps   xmm0, xmm4
        fld fvbuf
    addps   xmm1, xmm5
        fld fcoef
    addps   xmm2, xmm6
        fmulp   st(1), st
    addps   xmm3, xmm7
        faddp   st(1), st

    add esi, 4*64   ;; hop vbuf by 64 
    add ebx, 4*16   ;; coefs
    add eax, 4      ;; fpu coefs
    dec ecx
    jg  a210


    movups  b[4],       xmm0
    movups  b[4][16],   xmm1
    movups  b[4][2*16], xmm2
    movups  b[4][3*16], xmm3
    fstp    b


;;--- last block ---------------------

;; 4 accums, total of 15 (plus one dummy) results
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
    xorps   xmm2, xmm2
    xorps   xmm3, xmm3

;; accum bx components
vbuf    textequ <dword ptr [esi][4*(63-3)]>
	mov esi, input_ptr
    mov ecx, 8
a300:
    movups  xmm4, vbuf
    movups  xmm5, vbuf[-16]
    movups  xmm6, vbuf[-2*16]
    movups  xmm7, vbuf[-3*16]
    mulps   xmm4, wcoef
    mulps   xmm5, wcoef[16]
    mulps   xmm6, wcoef[2*16]
    mulps   xmm7, wcoef[3*16]

    addps   xmm0, xmm4
    addps   xmm1, xmm5
    addps   xmm2, xmm6
    addps   xmm3, xmm7

    add esi, 4*64   ;; hop vbuf by 64 
    add ebx, 4*16   ;; coefs
    dec ecx
    jg  a300

;; reorder accum
    shufps  xmm0, xmm0, 00011011b
    shufps  xmm1, xmm1, 00011011b
    shufps  xmm2, xmm2, 00011011b
    shufps  xmm3, xmm3, 00011011b

;; accum si components
vbuf    textequ <dword ptr [esi][4*(33)]>
	mov esi, input_ptr
    mov ecx, 8
a310:
    movups  xmm4, vbuf
    movups  xmm5, vbuf[16]
    movups  xmm6, vbuf[2*16]
    movups  xmm7, vbuf[3*16]
    mulps   xmm4, wcoef
    mulps   xmm5, wcoef[16]
    mulps   xmm6, wcoef[2*16]
    mulps   xmm7, wcoef[3*16]

    subps   xmm0, xmm4
    subps   xmm1, xmm5
    subps   xmm2, xmm6
    subps   xmm3, xmm7

    add esi, 4*64   ;; hop vbuf by 64 
    add ebx, 4*16   ;; coefs
    dec ecx
    jg  a310

    movups  b[4][4*16], xmm0
    movups  b[4][5*16], xmm1
    movups  b[4][6*16], xmm2
    movups  b[4][7*16], xmm3

    ret

window endp
;;==========================================================
;=========================================
_xsbt_init proc public
;;
;;  init dct icoef table
;;  returns address of coef table for test
;;
;;
;; arguments
;;z         textequ <dword ptr [esp+4*(1+npush)]>
;;

npush = (4+5)
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub esp, 5*4

;; temps on stack
tmp     textequ <dword ptr [esp+0*4]>
nn      textequ <dword ptr [esp+1*4]>
pp       textequ <dword ptr [esp+2*4]>
itmp    textequ <dword ptr [esp+3*4]>
n       textequ <dword ptr [esp+4*4]>


;;-----------------------
          cmp       first_pass, 0
          je        init_exit
          mov       first_pass, 0
;;
;;
          mov       nn, 4*16
          mov       n, 16
          xor       edi, edi
          mov       ch, 5
i_200:
          mov       cl, byte ptr n
          mov       pp, 1      ;; computes 2*pp+1
          fldpi
          fild      nn
          fdiv                   ;; pi/(4*n)
i_100:
          fild      pp
          fmul      st, st(1)
          fcos
          fadd      st, st(0)    ;; 2*cos(t)
          fstp      coef[edi]
          add       edi, 4
          add       pp, 2
          dec       cl
          jg        i_100
;;
          fstp      st(0)
          shr       n, 1
          shr       nn, 1
          dec       ch
          jg        i_200

init_exit:
          mov       eax, offset coef



    add esp, 5*4
    pop ecx
    pop ebx
    pop edi
    pop esi
	ret

_xsbt_init           endp
;======================================
;----------------------------------
forward_stage0  macro  x, f

k = 15
p = 15
    fld x[4*(2*k+1)]
    fst f[4*(16+p)]
    mov eax, x[4*(2*k)]
    mov f[4*p], eax
k = k - 1
p = p - 1

rept 14
    fld x[4*(2*k+1)]
    fsubrp  st(1), st
    fst f[4*(16+p)]
    mov eax, x[4*(2*k)]
    mov f[4*p], eax
k = k - 1
p = p - 1
endm

    fld x[4*(2*k+1)]
    fsubrp  st(1), st
    fstp f[4*(16+p)]
    mov eax, x[4*(2*k)]
    mov f[4*p], eax

endm
;----------------------------------
forward_stage1  macro  x, f

k = 7
p = 7;
    fld x[4*(2*k+1)]
    fst f[4*(8+p)]
    mov eax, x[4*(2*k)]
    mov f[4*p], eax

        fld x[4*(2*k+1)][4*16]
        fst f[4*(8+p)][4*16]
        mov eax, x[4*(2*k)][4*16]
        mov f[4*p][4*16], eax


k = k - 1
p = p - 1

rept 6
    fld x[4*(2*k+1)]
    fsubrp  st(2), st
    mov eax, x[4*(2*k)]
    mov f[4*p], eax
        fld x[4*(2*k+1)][4*16]
        fsubrp  st(1), st
        mov eax, x[4*(2*k)][4*16]
        mov f[4*p][4*16], eax
    fxch
    fst f[4*(8+p)]
    fxch
        fst f[4*(8+p)][4*16]
k = k - 1
p = p - 1
endm

    fld x[4*(2*k+1)]
    fsubrp  st(2), st
    mov eax, x[4*(2*k)]
    mov f[4*p], eax
        fld x[4*(2*k+1)][4*16]
        fsubrp  st(1), st
        mov eax, x[4*(2*k)][4*16]
        mov f[4*p][4*16], eax

        fstp f[4*(8+p)][4*16]
    fstp f[4*(8+p)]

endm
;----------------------------------
forward_stage2  macro  x, f


m = 0

rept 2

k = 3
p = 3

    fld x[4*(2*k+1)][4*16*m]
    fst f[4*(4+p)][4*16*m]
    mov eax, x[4*(2*k)][4*16*m]
    mov f[4*p][4*16*m], eax
        fld x[4*(2*k+1)][4*8][4*16*m]
        fst f[4*(4+p)][4*8][4*16*m]
        mov eax, x[4*(2*k)][4*8][4*16*m]
        mov f[4*p][4*8][4*16*m], eax


k = k - 1
p = p - 1

rept 2
    fld x[4*(2*k+1)][4*16*m]
    fsubrp  st(2), st
    mov eax, x[4*(2*k)][4*16*m]
    mov f[4*p][4*16*m], eax
        fld x[4*(2*k+1)][4*8][4*16*m]
        fsubrp  st(1), st
        mov eax, x[4*(2*k)][4*8][4*16*m]
        mov f[4*p][4*8][4*16*m], eax
    fxch
    fst f[4*(4+p)][4*16*m]
    fxch
        fst f[4*(4+p)][4*8][4*16*m]
k = k - 1
p = p - 1
endm

    fld x[4*(2*k+1)][4*16*m]
    fsubrp  st(2), st
    mov eax, x[4*(2*k)][4*16*m]
    mov f[4*p][4*16*m], eax
        fld x[4*(2*k+1)][4*8][4*16*m]
        fsubrp  st(1), st
        mov eax, x[4*(2*k)][4*8][4*16*m]
        mov f[4*p][4*8][4*16*m], eax

        fstp f[4*(4+p)][4*8][4*16*m]
    fstp f[4*(4+p)][4*16*m]

m = m + 1
endm


endm
;=====================================
;-------------------------------------
forward_back_combo  macro     X, COEF
          LOCAL L100, L200
;;
;; combines      FORWARD_IBF           m,  4,   BUFA, BUFB
;;                 BACK_IBF           2*m, 2, BUFB, BUFA, COEF
;; INPLACE ONLY!
;;  x0 = x0 + a*x2
;;  x1 = x0 - a*x2
;;  x2 = (x1-x3)  + a*x3
;;  x3 = (x1-x3)  - a*x3
;;

	xor       esi, esi
	mov       ch, 8

    fld coef
L100:
    fld x[esi][4*3]
    fld st(0)
    fmul    st, st(2)   ;;  a*x3 x3 a
    fld x[esi][4*2]
    fmul    st, st(3)   ;; a*x2 a*x3 x3 a
    fld x[esi][4*1]     ;; x1 a*x2 a*x3 x3
    fsubrp st(3), st    ;; a*x2 a*x3 x1-x3
    fld x[esi][4*0]     ;; x0 a*x2 a*x3 x1-x3
    fld st(0)           ;; x0 x0 a*x2 a*x3 x1-x3

    fld st(3)           ;; a*x3 x0 x0 a*x2 a*x3 x1-x3
    fxch st(5)          ;; x1-x3 x0 x0 a*x2 a*x3 a*x3
    fadd st(4), st      ;; x1-x3 x0 x0 a*x2 y2 a*x3
    fsubrp st(5), st    ;; x0 x0 a*x2 y2 y3
    fxch    st(2)       ;; a*x2 x0 x0 y2 y3
    fadd    st(1), st   ;; a*x2 y0 x0 y2 y3
    fsubp   st(2), st   ;;  y0 y1 y2 y3

    fstp    x[esi][4*0]
    fstp    x[esi][4*1]
    fstp    x[esi][4*2]
    fstp    x[esi][4*3]

    add esi, 4*4
    dec ch
    jg  L100


    fstp    st(0)   ;; pop coef off stack

endm
;-------------------------------------------
;-------------------------------------------
;; back stage 1 and 2 combo
;;f0 = x0 + w0*x2         f4 = x4 + w0*x6    
;;f3 = x0 - w0*x2         f7 = x4 - w0*x6    
;;    y0 = f0 + c0*f4
;;    y7 = f0 - c0*f4
;;    y3 = f3 + c3*f7
;;    y4 = f3 - c3*f7
;;
;;f1 = x1 + w1*x3         f5 = x5 + w1*x7
;;f2 = x1 - w1*x3         f6 = x5 - w1*x7    
;;    y1 = f1 + c1*f5
;;    y6 = f1 - c1*f5
;;    y2 = f2 + c2*f6
;;    y5 = f2 - c2*f6
;-------------------------------------------
;-------------------------------------------
back_ibf_r4      macro     m, n, x, f, coef
          local a100, a200
;; uses ebx, ecx

n2 = n/2
n4 = n/4

k0 = 4*(0)
k1 = 4*(n4)
k2 = 4*(n4+n4)
k3 = 4*(n4+n4+n4)

i0 = 4*(0)
i1 = 4*(n2 - 1)
i2 = 4*(n2)
i3 = 4*(n - 1)


xor ebx, ebx
mov ecx, m
a100:

rept n4
;;---------------
    fld     x[ebx][k3]
    fmul    coef[4*n2][k0]
    fld     x[ebx][k1]
    fmul    coef[4*n2][k0]

    fld     x[ebx][k2]
    fadd    st, st(2)
    fxch    st(2)
    fsubr   x[ebx][k2]

    fld     x[ebx][k0]
    fadd    st, st(2)
    fxch    st(3)
    fmul    coef[i0]
    fxch    st(2)
    fsubr   x[ebx][k0]
    fxch    st(1)
    fmul    coef[i1]

    fld     st(3)
    fadd    st, st(3)
    fxch    st(3)
    fsubp   st(4), st

    fld     st(0)
    fadd    st, st(2)
    fxch    st(2)
    fsubrp  st(1), st

    fxch    st(2)
    fstp    f[ebx][i0]
    fstp    f[ebx][i1]
    fstp    f[ebx][i2]
    fstp    f[ebx][i3]

k0 = k0 + 4
k1 = k1 + 4
k2 = k2 + 4
k3 = k3 + 4

i0 = i0 + 4
i1 = i1 - 4
i2 = i2 + 4
i3 = i3 - 4
;;--------
endm

add ebx, 4*n
dec ecx
jne a100

endm
;-------------------------------------------
back_ibf_r4_last      macro     m, n, x, f, coef, delta
          local a100, a200
;; m == 1

n2 = n/2
n4 = n/4

k0 = 4*(0)
k1 = 4*(n4)
k2 = 4*(n4+n4)
k3 = 4*(n4+n4+n4)

i0 = 4*(0)
i1 = 4*(n2 - 1)
i2 = 4*(n2)
i3 = 4*(n - 1)



rept n4
;;---------------
    fld     x[k3]
    fmul    coef[4*n2][k0]
    fld     x[k1]
    fmul    coef[4*n2][k0]

    fld     x[k2]
    fadd    st, st(2)
    fxch    st(2)
    fsubr   x[k2]

    fld     x[k0]
    fadd    st, st(2)
    fxch    st(3)
    fmul    coef[i0]
    fxch    st(2)
    fsubr   x[k0]
    fxch    st(1)
    fmul    coef[i1]

    fld     st(3)
    fadd    st, st(3)
    fxch    st(3)
    fsubp   st(4), st

    fld     st(0)
    fadd    st, st(2)
    fxch    st(2)
    fsubrp  st(1), st

    fxch    st(2)
    fstp    f[delta*i0]
    fstp    f[delta*i1]
    fstp    f[delta*i2]
    fstp    f[delta*i3]

k0 = k0 + 4
k1 = k1 + 4
k2 = k2 + 4
k3 = k3 + 4

i0 = i0 + 4
i1 = i1 - 4
i2 = i2 + 4
i3 = i3 - 4
;;--------
endm

endm
;--------------------------------------
;----------------------------------------
;;--------------------------------------
idct_L3 proc
;;
;;
;;  input from bufb
;; output to output_ptr (interleaved)

;-------------
	forward_stage0        bufb, bufa
	forward_stage1        bufa, bufb
	forward_stage2        bufb, bufa
	forward_back_combo    bufa, coef[4*(16+8+4+2)]
;
    back_ibf_r4          4, 8, bufa, bufb, coef[4*(16+8)]
    mov       esi, output_ptr
    back_ibf_r4_last     1,32, bufb, dword ptr [esi], coef, 18
;-------------

    ret
idct_L3 endp
;--------------------------------------
;--------------------------------------
_xsbt_L3 proc public
;;
;; arguments
vbuf_addr   textequ <dword ptr [esp+4*(1+npush)]>
samp_addr   textequ <dword ptr [esp+4*(2+npush)]>
;;
npush = (4)
    push    esi
    push    edi
    push    ebx
    push    ecx
;;
;;
	mov       eax, vbuf_addr
	mov       ebx, samp_addr
	add       eax, 4*576
	mov       input_ptr, eax
	mov       output_ptr, ebx

	mov       counter, 18   ;; one granule
sbt_100:
	sub       input_ptr, 4*32
    call window
	call idct_L3
	add       output_ptr, 4    ;; output 
	dec counter
    jg  sbt_100

;;----
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret

_xsbt_L3 endp
;;==========================================================
;;==========================================================
;;==========================================================
_TEXT ENDS
;====================================
          END

