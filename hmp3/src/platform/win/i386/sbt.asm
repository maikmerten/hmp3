; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: sbt.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
        
;  SBT.ASM
;
;  MPEG Layer III
;     window and dct
;
;---------------------------------------
;
; idct algor computes  f(p) = SUM u(k)*cos((pi/2n)*k*(2p+1))*x(k)
;                  u(k)=1.0 if k=0, 2.0 otherwise
;  window is scaled to produce
;             f(p) = SUM cos((pi/2n)*k*(2p+1))*x(k)
;--------------------------------------------------
.486
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT
;--------------------------------- 
;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'


align 4

WINCOEF   LABEL DWORD
include tableaw2.inc

bufa      dd        32 dup (0)
bufb      dd        32 dup (0)
coef      dd        32 dup (0)
;;
input_ptr   dd 0
output_ptr  dd 0

;;
counter     db        0
first_pass  db        1
;

_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
;----------------------------------
LOOPX	MACRO COUNTER, LOOP_LABEL
	DEC	COUNTER
	JNE	LOOP_LABEL
ENDM
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
;=========================================
FORWARD_IBF      MACRO     M, N, X, F
          LOCAL L100, L200
N2 = N/2
D  = 4*(N-1)
;; assumes st(0) = 0, leaves with st(0) = 0
;; caller needs to clear st on first stage
	xor       esi, esi
	xor       edi, edi
	mov       ch, m
L100:
	mov       cl, N2
L200:
	fsubr     x[D+esi]
	mov       eax, x[D+esi-4]
	sub       esi, 8
	mov       f[D+edi-4*N2], eax
	fst       f[D+edi]
	sub       edi, 4
	loopx     cl, L200
	fsub      st, st(0)         ;; clear st for next go
	add       esi, 4*N + 8*N2
	add       edi, 4*N + 4*N2
	loopx     ch, l100

ENDM
;-----------------------------------------
FORWARD_IBF0     MACRO     M, N, X, F
          LOCAL L100, L200
N2 = N/2
D  = 4*(N-1)
;; first stage forward butterfly  m = 1
;; assumes st(0) = 0, leaves with st(0) = 0
;; caller needs to clear st on first stage
	xor       esi, esi
	xor       edi, edi
	mov       cl, n2
L200:
	fsubr     x[d+esi]
	mov       eax, x[d+esi-4]
	sub       esi, 8
	mov       f[d+edi-4*n2], eax
	fst       f[d+edi]
	sub       edi, 4
	loopx     cl, l200
	fsub      st, st(0)         ;; clear st for next go
ENDM
;-------------------------------------------
BACK_IBF      MACRO     M, N, X, F, COEF
          LOCAL L100, L200
N2 = N/2
	xor       esi, esi
	mov       ch, m
L100:
	lea       edi, [esi+4*n]
	xor       ebx, ebx
	mov       cl, n2
L200:
	fld       x[esi+ebx+4*n2]
	fmul      coef[ebx]
	sub       edi, 4
	fld       x[esi+ebx]
	fadd      st, st(1)
	fxch
	fsubr     x[esi+ebx]
	fxch
	fstp      f[esi+ebx]
	fstp      f[edi]
	add       ebx, 4
	loopx     cl, L200
	add       esi, 4*n
	loopx     ch, l100

ENDM
;-------------------------------------
;-------------------------------------
FORWARD_BACK      MACRO     M, X, COEF
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
	mov       ch, m
L100:
	fld       x[esi+4*2]
	fmul      coef
	fld       x[esi+4*1]
	fsub      x[esi+4*3]
	fld       x[esi+4*3]
	fmul      coef
	;;
	fld       x[esi]
	fadd      st, st(3)
	fxch      st(3)
	fsubr     x[esi]
	fxch      st(2)
	;;
	fld       st(0)
	fadd      st, st(2)
	fxch      st(2)
	fsub
	fxch      st(3)
	fstp      x[esi]
	fstp      x[esi+4*2]
	fstp      x[esi+4*1]
	fstp      x[esi+4*3]
	add       esi, 4*4
	loopx     ch, L100

ENDM
;----------------------------------------
BACK_IBF0      MACRO     M, N, X, F, COEF
          LOCAL L100, L200
;; first stage back butterfly   N==2
;; can be done inplace
	xor       esi, esi
	mov       ch, m
L100:
	fld       x[esi+4]
	fmul      coef
	fld       x[esi]
	fadd      st, st(1)
	fxch
	fsubr     x[esi]
	fxch
	fstp      f[esi]
	fstp      f[esi+4]
	add       esi, 8
	loopx     ch, L100

ENDM
;----------------------------------------
BACK_IBF_LAST      MACRO     M, N, X, F, COEF
          LOCAL L100, L200
;; last stage m = 1
;; output to [esi], caller must load, interleaved
N2 = N/2
	lea       edi, [esi+8*n]
	xor       ebx, ebx
	mov       cl, n2
L200:
	fld       x[ebx+4*n2]
	fmul      coef[ebx]
	add       esi, 8
	sub       edi, 8
	fld       x[ebx]
	fadd      st, st(1)
	fxch
	fsubr     x[ebx]
	fxch
	fstp      dword ptr f[esi-8]
	fstp      dword ptr f[edi]
	add       ebx, 4
	loopx     cl, L200

ENDM
;----------------------------------------
;----------------------------------------
BACK_IBF_LAST_L3      MACRO     M, N, X, F, COEF
          LOCAL L100, L200
;; last stage m = 1
;; output to [esi], caller must load, interleaved
N2 = N/2
	lea       edi, [esi+4*18*n]
	xor       ebx, ebx
	mov       cl, n2
L200:
	fld       x[ebx+4*n2]
	fmul      coef[ebx]
	add       esi, 4*18
	sub       edi, 4*18
	fld       x[ebx]
	fadd      st, st(1)
	fxch
	fsubr     x[ebx]
	fxch
	fstp      dword ptr f[esi-4*18]
	fstp      dword ptr f[edi]
	add       ebx, 4
	loopx     cl, L200

ENDM
;----------------------------------------
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
;----------------------------------------
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
;=========================================
_sbt_init proc public
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
          loopx     cl, i_100
;;
          fstp      st(0)
          shr       n, 1
          shr       nn, 1
          loopx     ch, i_200

init_exit:
          mov       eax, offset coef



    add esp, 5*4
    pop ecx
    pop ebx
    pop edi
    pop esi
	ret

_sbt_init           endp
;======================================
window2  proc
;; code for Pentium II
;; 
;;  caller loads
;;      input from vbuf start es:input_ptr
;;      ds = local data
;;  uses
;;          vbuf  [esi], [ebx]
;;          wincoef  [edi]
;;  output to bufb[edx]
;;
;;
;;  adjust pointers
	mov       esi, input_ptr
	lea       ebx, [esi+4*15]
	add       esi, 4*16
	xor       edi, edi
	xor       edx, edx
	;;
;-------------------------------
;;  first 1
k = 0
	fld       dword ptr [esi + 4*64*k]
	fld       wincoef[edi + 4*k]
	fmul
k = k + 1
	fld       dword ptr [esi + 4*64*k]
	fld      wincoef[edi + 4*k]
	fmul
k = k + 1
rept 3
	fld       dword ptr [esi + 4*64*k]
	fld       wincoef[edi + 4*k]
	fmul
    faddp    st(2), st
k = k + 1
	fld       dword ptr [esi + 4*64*k]
	fld       wincoef[edi + 4*k]
	fmul
    faddp    st(1), st
k = k + 1
endm
    add       edi, 4*8
	add       edx, 4
    fadd
	add       esi, 4
mov       ecx, 16       ;; counter for next 16
	fstp      bufb[edx-4]
;-------------------------------
	; next 16
w200:
k = 0
	fld       dword ptr [esi + 4*64*k]
	fld       wincoef[edi + 8*k]
	fmul
	fld       dword ptr [ebx + 4*64*k]
	fld       wincoef[edi+4 + 8*k]
	fmul
k = k + 1
rept 7
	fld       dword ptr [esi + 4*64*k]
	fld       wincoef[edi + 8*k]
	fmul
    faddp   st(2), st
	fld       dword ptr [ebx + 4*64*k]
	fld       wincoef[edi+4 + 8*k]
	fmul
    faddp   st(1), st
k = k + 1
endm
	add       edx, 4
    faddp   st(1), st
	add       esi, 4
	sub       ebx, 4
    add     edi, 4*16
    dec ecx
	fstp      bufb[edx-4]
    jne w200
;-------------------------------
; last 15
	mov       edi, 4*247
	add       ebx, 4*64
	mov       ecx, 15
W300:
k = 0
	fld       dword ptr [ebx + 4*64*k]
	fld       wincoef[edi - 8*k]
	fmul
	fld       dword ptr [esi + 4*64*k]
	fld       wincoef[edi - 4 - 8*k]
	fmul
k = k + 1
rept 7
	fld       dword ptr [ebx + 4*64*k]
	fld       wincoef[edi-8*k]
	fmul
    faddp    st(2), st
	fld       dword ptr [esi + 4*64*k]
	fld      wincoef[edi-4-8*k]
	fmul
    faddp   st(1), st
k = k + 1
endm
	add       edx, 4
    fsubp     st(1), st
	add       esi, 4
	sub       ebx, 4
    sub     edi, 4*16
    dec ecx
	fstp      bufb[edx-4]
    jne w300
;--------
          ret
window2   endp
;------------------------------------
;; for Layer I/II
;; idct       PROC
;; ;;
;; ;;
;; ;;  input from bufb
;; ;; output to output_ptr (interleaved)
;; 
;; 	fldz
;; 	forward_ibf0        1, 32, bufb, bufa
;; 	forward_ibf         2, 16, bufa, bufb
;; 	forward_ibf         4,  8, bufb, bufa
;; 	fstp      st(0)         ;;  pop stack for forward_ibf
;; 	;
;; 	forward_back        8,     bufa, coef[4*(16+8+4+2)]
;; 	;
;; ;;	back_ibf             8, 4, bufa, bufb, coef[4*(16+8+4)];
;; ;;	back_ibf             4, 8, bufb, bufa, coef[4*(16+8)]
;; ;;	back_ibf             2,16, bufa, bufb, coef[4*(16)]
;; ;;	mov       esi, output_ptr
;; ;;	back_ibf_last        1,32, bufb, [0], coef
;; 
;; 
;; 	back_ibf_r4          4, 8, bufa, bufb, coef[4*(16+8)]
;; 	mov       esi, output_ptr
;; 	back_ibf_r4_last     1,32, bufb, dword ptr [esi], coef, 2
;; 
;; 
;;           RET
;; idct     ENDP
;; ;--------------------------------------
;; ;--------------------------------------
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
;; Layer I/II
;;asbt PROC C PUBLIC \
;;		USES ESI EDI EBX ECX   \
;;		vbuf_addr:DWORD, 
;;		samp_addr:DWORD
;;	mov       eax, vbuf_addr
;;	mov       ebx, samp_addr
;;	add       eax, 4*1152
;;	mov       input_ptr, eax
;;	mov       output_ptr, ebx
;;
;;
;;	mov       counter, 36
;;sbt_100:
;;	sub       input_ptr, 4*32
;;	call window
;;	call idct
;;	add       output_ptr, 4*64    ;; output interleaved
;;	loopx     counter, sbt_100
;;
;;	ret
;;
;;asbt       ENDP
;--------------------------------------
_sbt_L3 proc public
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
	call window2
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

_sbt_L3 endp
;===========================================
;====================================
;====================================
;====================================
_TEXT ENDS
;====================================
          END
