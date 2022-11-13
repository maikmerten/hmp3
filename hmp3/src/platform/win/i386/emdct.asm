; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: emdct.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
; emdct.asm   
;
;  Layer 3 encode mdct
;
;  masm 6.11
;
;  Pentium II target
;
;-------------------------------------------------
.486
;;.XMM
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT
;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

align 4

;;/*------ 18 point xform -------*/
w       dd  18 dup (0)
w2      dd   9 dup (0)
coef    dd 9*4 dup (0)  



abuf    dd  9 dup (0)
bbuf    dd  9 dup (0)

con5  real4   0.5 


;;/*------ 6 point xform -------*/
vv   dd  6  dup (0)
vv2  dd  3  dup (0)
coef87  dd 0 

;;------------
mdct_info_18 label dword
    dd  offset w
    dd  offset w2
    dd  offset coef
mdct_info_6 label dword
    dd  offset vv
    dd  offset vv2
    dd  offset coef87


_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
_mdct_init_addr_18 proc public
    mov eax, offset mdct_info_18 
    ret
_mdct_init_addr_18 endp
;;----------------------------------------------------------
_mdct_init_addr_6 proc public
    mov eax, offset mdct_info_6 
    ret
_mdct_init_addr_6 endp
;;==========================================================
;;==========================================================
_mdct18 proc public
;;
;; void  mdct18(float f[18], float y[])
;;            
;;
;; arguments
f_addr    textequ <dword ptr [esp+4*(1+npush)]>
y_addr    textequ <dword ptr [esp+4*(2+npush)]>

npush = (4)
    push    esi
    push    edi
    push    ebx
    push    ecx
;;----------
    mov esi, f_addr
    xor ebx, ebx    ;; up addr count
    mov edx, 4*8    ;; dn addr

fa0  textequ <dword ptr [esi+ebx]>
fb0  textequ <dword ptr [esi+edx][4*9]>

fa1  textequ <dword ptr [esi+ebx][4*9]>
fb1  textequ <dword ptr [esi+edx]>

wa0  textequ <dword ptr w[ebx]>
wb0  textequ <dword ptr w[edx][4*9]>

wa1  textequ <dword ptr w[ebx][4*9]>
wb1  textequ <dword ptr w[edx]>

w2a  textequ <dword ptr w2[ebx]>
w2b  textequ <dword ptr w2[edx]>


    fldz    ;; accum for y0 y1
    fldz    ;; done during loop

    mov ecx, 4
a100:
    fld fa0
    fld wa0
    fmulp st(1), st
    fld st(0)       

    fld fb0
    fld wb0
    fmulp st(1), st     ;;  g2 g1 g1

    fsub    st(2), st   ;;  g2 g1 g1-g2
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld w2a
    fmulp   st(2), st   ;;       ap bp
    ;;-----
    fld fb1
    fld wb1
    fmulp st(1), st
    fld st(0)       

    fld fa1
    fld wa1
    fmulp st(1), st     ;;  g2 g1 g1

    fsub    st(2), st   ;;  g2 g1 g1-g2
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld w2b
    fmulp   st(2), st   ;;  a8p b8p ap bp
    ;;------------
    fld st(2)           ;;  ap a8p b8p ap bp
    fxch st(1)          ;;  a8p ap b8p ap bp
    fsub st(3), st
    faddp st(1), st     ;;  a[p] b8p a[5+p] bp
    
    fld st(3)           ;;  bp a[p] b8p a[5+p] bp
    fxch st(2)          ;;  b8p a[p] bp a[5+p] bp
    fsub st(4), st      ;;  b8p a[p] bp a[5+p] b[5+p]
    faddp st(2), st     ;;    a[p] b[p] a[5+p] b[5+p]


        fadd st(5), st  ;; sum a[p]
    fstp abuf[ebx]
        fadd st(3), st  ;; sum b[p]
    fstp bbuf[ebx]
    fstp abuf[ebx][4*5]
    fstp bbuf[ebx][4*5]

    add ebx, 4
    sub edx, 4
    dec ecx
    jg  a100
;;----
;;----
    fld fa0
    fld wa0
    fmulp st(1), st
    fld st(0)       

    fld fb0
    fld wb0
    fmulp st(1), st     ;;  g2 g1 g1

    fsub    st(2), st   ;;  g2 g1 g1-g2
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld w2a
    fmulp   st(2), st   ;;       ap bp

        fadd st(3), st      ;; sum a[p]
    fstp abuf[ebx]
        fadd st(1), st      ;; sum b[p]
    fstp bbuf[ebx]      ;; sum(b) sum(a)
;;----------------

    mov edi, y_addr
y0   textequ <dword ptr [edi]>
y1   textequ <dword ptr [edi][4]>
y2   textequ <dword ptr [edi][2*4]>
y3   textequ <dword ptr [edi][3*4]>
y4   textequ <dword ptr [edi][4*4]>
y5   textequ <dword ptr [edi][5*4]>
y6   textequ <dword ptr [edi][6*4]>
y7   textequ <dword ptr [edi][7*4]>
y8   textequ <dword ptr [edi][8*4]>
y9   textequ <dword ptr [edi][9*4]>
y10  textequ <dword ptr [edi][10*4]>
y11  textequ <dword ptr [edi][11*4]>
y12  textequ <dword ptr [edi][12*4]>
y13  textequ <dword ptr [edi][13*4]>
y14  textequ <dword ptr [edi][14*4]>
y15  textequ <dword ptr [edi][15*4]>
y16  textequ <dword ptr [edi][16*4]>
y17  textequ <dword ptr [edi][17*4]>

c0   textequ <dword ptr coef[4*4*0]>
c1   textequ <dword ptr coef[4*4*1]>
c2   textequ <dword ptr coef[4*4*2]>
c3   textequ <dword ptr coef[4*4*3]>
c4   textequ <dword ptr coef[4*4*4]>
c5   textequ <dword ptr coef[4*4*5]>
c6   textequ <dword ptr coef[4*4*6]>
c7   textequ <dword ptr coef[4*4*7]>
c8   textequ <dword ptr coef[4*4*8]>

a0  textequ <abuf[4*0]>
a1  textequ <abuf[4*1]>
a2  textequ <abuf[4*2]>
a3  textequ <abuf[4*3]>
a4  textequ <abuf[4*4]>
a5  textequ <abuf[4*5]>
a6  textequ <abuf[4*6]>
a7  textequ <abuf[4*7]>
a8  textequ <abuf[4*8]>

b0  textequ <bbuf[4*0]>
b1  textequ <bbuf[4*1]>
b2  textequ <bbuf[4*2]>
b3  textequ <bbuf[4*3]>
b4  textequ <bbuf[4*4]>
b5  textequ <bbuf[4*5]>
b6  textequ <bbuf[4*6]>
b7  textequ <bbuf[4*7]>
b8  textequ <bbuf[4*8]>

    fld con5
    fmul    st(2), st
    fmulp   st(1), st       ;;  y1 y0

;y[2] = coef[1][0]*a[5] + coef[1][1]*a[6] + coef[1][2]*a[7]
;        + coef[1][3]*a[8];
;y[3] = coef[1][0]*b[5] + coef[1][1]*b[6] + coef[1][2]*b[7]
;        + coef[1][3]*b[8]                   - y[1];

    fld b5
    fld a5
    fld c1[4*0]
    fmul     st(2), st
    fmulp    st(1), st
    fxch    ;; y3 y2   partial

k = 0
rept 3
    fld a6[4*k]
    fld b6[4*k]
    fld c1[4*1][4*k]
    fmul     st(1), st
    fmulp    st(2), st
    faddp st(2), st
    faddp st(2), st    ;; y3 y2  y1 y0 partial 
k = k + 1
endm
    fsub st, st(2)      ;; y3 y2 y1 y0
    fxch st(3)          ;; y0 y2 y1 y3
    fsub st(2), st
    fstp y0             ;; y2 y1 y3
    fsub st, st(1)      ;; y2 y1 y3
    fxch
    fstp y1             ;; y2 y3

;y[4] = coef[2][0]*a[0] + coef[2][1]*a[1] + coef[2][2]*a[2]
;        + coef[2][3]*a[3] - a[4];
;y[5] = coef[2][0]*b[0] + coef[2][1]*b[1] + coef[2][2]*b[2]
;        + coef[2][3]*b[3] - b[4]            - y[3];
    fld a0
    fld b0
    fld c2[4*0]
    fmul     st(1), st
    fmulp    st(2), st    ;; y5 y4 partial

k = 1
rept 3
    fld a0[4*k]
    fld b0[4*k]
    fld c2[4*0][4*k]
    fmul     st(1), st
    fmulp    st(2), st
    faddp st(2), st
    faddp st(2), st    ;; y5 y4  y2 y3 partial 
k = k + 1
endm
    fld b0[4*k]
    fsubp st(1), st
    fld a0[4*k]
    fsubp st(2), st

    fxch st(3)          ;; y3 y4 y2 y5
    fsub st(3), st      ;; y3 y4 y2 y5
    fsub st, st(2)      ;; y3 y4 y2 y5
    fsub st(1), st      ;; y3 y4 y2 y5
    fstp    y3
    fxch
    fstp    y2          ;;  y4 y5


;y[6] = coef[3][0]*( a[5] - a[7] - a[8]);
;y[7] = coef[3][0]*( b[5] - b[7] - b[8])     - y[5];
;y[5] = y[5] - y[4];
;y[6] = y[6] - y[5];
    fld a5
    fld b5
    ;;
    fld b7
    fsubp st(1), st
    fld a7
    fsubp st(2), st
    fld b8
    fsubp st(1), st
    fld a8
    fsubp st(2), st    ;; y7 y6 y4 y5 partial
    fld c3
    fmul st(1), st
    fmulp st(2), st
    
    fxch st(3)          ;; y5 y6 y4 y7
    fsub st(3), st
    fsub st, st(2)
    fsub st(1), st
    fstp y5
    fxch
    fstp y4             ;; y6 y7
;y[8] = coef[4][0]*a[0] + coef[4][1]*a[1] + coef[4][2]*a[2]
;        + coef[4][3]*a[3] + a[4];
;y[9] = coef[4][0]*b[0] + coef[4][1]*b[1] + coef[4][2]*b[2]
;        + coef[4][3]*b[3] + b[4]            - y[7];
;y[7] = y[7] - y[6];
;y[8] = y[8] - y[7];

    fld a0
    fld b0
    fld c4
    fmul     st(1), st
    fmulp    st(2), st    ;; y9 y8 y6 y7 partial
k = 1
rept 3
    fld a0[4*k]
    fld b0[4*k]
    fld c4[4*k]
    fmul     st(1), st
    fmulp    st(2), st
    faddp st(2), st
    faddp st(2), st    ;; y9 y8  y6 y7 partial 
k = k + 1
endm
    fld b0[4*k]
    faddp st(1), st
    fld a0[4*k]
    faddp st(2), st    ;; y9 y8  y6 y7 partial

    fxch st(3)         ;; y7 y8 y6 y9
    fsub st(3), st
    fsub st, st(2)
    fsub st(1), st
    fstp y7
    fxch
    fstp y6             ;; y8 y9
;y[10]= coef[5][0]*a[5] + coef[5][1]*a[6] + coef[5][2]*a[7]
;        + coef[5][3]*a[8];
;y[11]= coef[5][0]*b[5] + coef[5][1]*b[6] + coef[5][2]*b[7]
;        + coef[5][3]*b[8]                    - y[9];
;y[9]  = y[9]  - y[8];
;y[10] = y[10] - y[9];
    fld a5
    fld b5
    fld c5
    fmul     st(1), st
    fmulp    st(2), st    ;; y11 y10 y8 y9 partial
k = 1
rept 3
    fld a5[4*k]
    fld b5[4*k]
    fld c5[4*k]
    fmul     st(1), st
    fmulp    st(2), st
    faddp st(2), st
    faddp st(2), st    ;; y11 y10 y8 y9 partial 
k = k + 1
endm
    fxch st(3)          ;; y9 y10 y8 y11 
    fsub st(3), st
    fsub st, st(2)
    fsub st(1), st
    fstp y9
    fxch
    fstp y8            ;; y10 y11
;y[12]= 0.5f*(a[0] + a[2] + a[3]) - a[1] - a[4];
;y[13]= 0.5f*(b[0] + b[2] + b[3]) - b[1] - b[4] - y[11];
;y[11] = y[11] - y[10];
;y[12] = y[12] - y[11];
    fld a0
    fld b0

    fld b2
    faddp st(1), st
    fld a2
    faddp st(2), st
    fld b3
    faddp st(1), st
    fld a3
    faddp st(2), st

    fld con5
    fmul st(1), st
    fmulp st(2), st

    fld a1
    fld b1
    fld b4
    faddp st(1), st
    fld a4
    faddp st(2), st
    fsubp   st(2), st
    fsubp   st(2), st   ;; y13 y12 y10 y11

    fxch st(3)          ;; y11 y12 y10 y13 
    fsub st(3), st
    fsub st, st(2)
    fsub st(1), st
    fstp y11
    fxch
    fstp y10            ;; y12 y13
;y[14]= coef[7][0]*a[5] + coef[7][1]*a[6] + coef[7][2]*a[7]
;        + coef[7][3]*a[8];
;y[15]= coef[7][0]*b[5] + coef[7][1]*b[6] + coef[7][2]*b[7]
;        + coef[7][3]*b[8]                    - y[13];
;y[13] = y[13] - y[12];
;y[14] = y[14] - y[13];
    fld a5
    fld b5
    fld c7
    fmul     st(1), st
    fmulp    st(2), st    ;; y15 y14 y12 y13 partial
k = 1
rept 3
    fld a5[4*k]
    fld b5[4*k]
    fld c7[4*k]
    fmul     st(1), st
    fmulp    st(2), st
    faddp st(2), st
    faddp st(2), st    ;; y15 y14 y12 y13 partial 
k = k + 1
endm
    fxch st(3)          ;; y13 y14 y12 y15 
    fsub st(3), st
    fsub st, st(2)
    fsub st(1), st
    fstp y13
    fxch
    fstp y12            ;; y14 y15
;y[16]= coef[8][0]*a[0] + coef[8][1]*a[1] + coef[8][2]*a[2]
;        + coef[8][3]*a[3] + a[4];
;y[17]= coef[8][0]*b[0] + coef[8][1]*b[1] + coef[8][2]*b[2]
;        + coef[8][3]*b[3] + b[4]             - y[15];
;y[15] = y[15] - y[14];
;y[16] = y[16] - y[15];
;y[17] = y[17] - y[16];
    fld a0
    fld b0
    fld c8
    fmul     st(1), st
    fmulp    st(2), st    ;; y17 y16 y14 y15 partial
k = 1
rept 3
    fld a0[4*k]
    fld b0[4*k]
    fld c8[4*k]
    fmul     st(1), st
    fmulp    st(2), st
    faddp st(2), st
    faddp st(2), st    ;; y17 y16 y14 y15 partial 
k = k + 1
endm
    fld b0[4*k]
    faddp st(1), st
    fld a0[4*k]
    faddp st(2), st


    fxch st(3)          ;; y15 y16 y14 y17 
    fsub st(3), st
    fsub st, st(2)
    fsub st(1), st
    fstp y15
    fxch
    fstp y14            ;; y16 y17
    fsub st(1), st
    fstp y16
    fstp y17


exit:
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret

_mdct18 endp
;;==========================================================
;;==========================================================
_mdct6_3 proc public
;;
;; void  mdct6_3(float f[18], float y[])
;;            
;;
;; arguments
f_addr    textequ <dword ptr [esp+4*(1+npush)]>
y_addr    textequ <dword ptr [esp+4*(2+npush)]>

npush = (4)
    push    esi
    push    edi
    push    ebx
    push    ecx
;;----------


f0   textequ <dword ptr [esi][4*0]>
f1   textequ <dword ptr [esi][4*1]>
f2   textequ <dword ptr [esi][4*2]>
f3   textequ <dword ptr [esi][4*3]>
f4   textequ <dword ptr [esi][4*4]>
f5   textequ <dword ptr [esi][4*5]>

v0   textequ <dword ptr vv[4*0]>
v1   textequ <dword ptr vv[4*1]>
v2   textequ <dword ptr vv[4*2]>
v3   textequ <dword ptr vv[4*3]>
v4   textequ <dword ptr vv[4*4]>
v5   textequ <dword ptr vv[4*5]>

a0   textequ <dword ptr [ebx][4*0]>
a1   textequ <dword ptr [ebx][4*1]>
a2   textequ <dword ptr [ebx][4*2]>
a3   textequ <dword ptr [ebx][4*3]>
a4   textequ <dword ptr [ebx][4*4]>
a5   textequ <dword ptr [ebx][4*5]>

    mov esi, f_addr
    mov ebx, offset abuf
    mov ecx, 3
a100:
    fld f0
    fld v0
    fmulp st(1), st
    fld f5
    fld v5
    fmulp st(1), st     ;; g2 g1
    fld st(1)
    fxch                ;; g2 g1 g1
    fsub    st(2), st
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld vv2[4*0]
    fmulp st(2), st
    fstp a0
    fstp a3
    ;;
    fld f1
    fld v1
    fmulp st(1), st
    fld f4
    fld v4
    fmulp st(1), st     ;; g2 g1
    fld st(1)
    fxch                ;; g2 g1 g1
    fsub    st(2), st
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld vv2[4*1]
    fmulp st(2), st
    fstp a1
    fstp a4
    ;;
    fld f2
    fld v2
    fmulp st(1), st
    fld f3
    fld v3
    fmulp st(1), st     ;; g2 g1
    fld st(1)
    fxch                ;; g2 g1 g1
    fsub    st(2), st
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld vv2[4*2]
    fmulp st(2), st
    fstp a2
    fstp a5
    ;;
    add esi, 4*6
    add ebx, 4*6
    dec ecx
    jg  a100

;;-----------
y0   textequ <dword ptr [edi]>
y1   textequ <dword ptr [edi][4]>
y2   textequ <dword ptr [edi][2*4]>
y3   textequ <dword ptr [edi][3*4]>
y4   textequ <dword ptr [edi][4*4]>
y5   textequ <dword ptr [edi][5*4]>
    mov edi, y_addr
    mov ebx, offset abuf
    mov ecx, 3

a200:
;    a02 = (a[0] + a[2]);
;    b02 = (a[3+0] + a[3+2]);
    fld a0
    fld a2
    faddp st(1), st
    fld a3
    fld a5
    faddp st(1), st     ;; b02 a02

;    c[0] = a02 + a[1];
;    c[1] = b02 + a[3+1];
    fld a1
    fadd st, st(2)
    fst y0          
    fld a4
    fadd st, st(2) ;; y1 y0 b02 a02   
;;
;;    y[2] = coef87*(a[0] - a[2]);
;;    y[3] = coef87*(a[3+0] - a[3+2]) - y[1];
    fld a0
    fld a3
    fld a5
    fsubp st(1), st     ;; a3-a5 a0
    fld a2
    fsubp st(2), st     ;; a3-a5 a0-a2 y1 y0 b02 a02
    fld coef87
    fmul st(1), st
    fmulp st(2), st      ;; y3 y2 y1 y0 b02 a02
    fsub st, st(2)
;c[1] = c[1] - c[0];
;c[2] = c[2] - c[1];
    fxch st(3)          ;; y0 y2 y1 y3 b02 a02
    fsubp st(2), st     ;; y2 y1 y3 b02 a02
    fxch                ;; y1 y2 y3 b02 a02
    fsub st(1), st      ;; y1 y2 y3 b02 a02
    fstp y1             ;; y2 y3 b02 a02
;c[4] = a02 - a[1] - a[1];
;c[5] = b02 - a[3+1] - a[3+1]    - c[3];
    fxch                ;; y3 y2 b02 a02
    fld a4
    fsub st(3), st
    fsubp st(3), st
    fld a1
    fsub st(4), st
    fsubp st(4), st     ;; y3 y2 y5 y4
    fsub st(2), st
;c[3] = c[3] - c[2];
;c[4] = c[4] - c[3];
;c[5] = c[5] - c[4];
    fxch
    fsub st(1), st
    fstp y2             ;; y3 y5 y4
    fsub    st(2), st
    fstp y3
    fxch
    fsub st(1), st
    fstp y4
    fstp y5

    add ebx, 4*6
    add edi, 4*6
    dec ecx
    jg a200



exit:
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret

_mdct6_3 endp
;;==========================================================
;;==========================================================
_mdct6_3B proc public
;; output ordered for x[3][192]
;; void  mdct6_3(float f[18], float y[])
;;            
;;
;; arguments
f_addr    textequ <dword ptr [esp+4*(1+npush)]>
y_addr    textequ <dword ptr [esp+4*(2+npush)]>

npush = (4)
    push    esi
    push    edi
    push    ebx
    push    ecx
;;----------


f0   textequ <dword ptr [esi][4*0]>
f1   textequ <dword ptr [esi][4*1]>
f2   textequ <dword ptr [esi][4*2]>
f3   textequ <dword ptr [esi][4*3]>
f4   textequ <dword ptr [esi][4*4]>
f5   textequ <dword ptr [esi][4*5]>

v0   textequ <dword ptr vv[4*0]>
v1   textequ <dword ptr vv[4*1]>
v2   textequ <dword ptr vv[4*2]>
v3   textequ <dword ptr vv[4*3]>
v4   textequ <dword ptr vv[4*4]>
v5   textequ <dword ptr vv[4*5]>

a0   textequ <dword ptr [ebx][4*0]>
a1   textequ <dword ptr [ebx][4*1]>
a2   textequ <dword ptr [ebx][4*2]>
a3   textequ <dword ptr [ebx][4*3]>
a4   textequ <dword ptr [ebx][4*4]>
a5   textequ <dword ptr [ebx][4*5]>

    mov esi, f_addr
    mov ebx, offset abuf
    mov ecx, 3
a100:
    fld f0
    fld v0
    fmulp st(1), st
    fld f5
    fld v5
    fmulp st(1), st     ;; g2 g1
    fld st(1)
    fxch                ;; g2 g1 g1
    fsub    st(2), st
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld vv2[4*0]
    fmulp st(2), st
    fstp a0
    fstp a3
    ;;
    fld f1
    fld v1
    fmulp st(1), st
    fld f4
    fld v4
    fmulp st(1), st     ;; g2 g1
    fld st(1)
    fxch                ;; g2 g1 g1
    fsub    st(2), st
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld vv2[4*1]
    fmulp st(2), st
    fstp a1
    fstp a4
    ;;
    fld f2
    fld v2
    fmulp st(1), st
    fld f3
    fld v3
    fmulp st(1), st     ;; g2 g1
    fld st(1)
    fxch                ;; g2 g1 g1
    fsub    st(2), st
    faddp   st(1), st   ;;  g1+g2 g1-g2
    fld vv2[4*2]
    fmulp st(2), st
    fstp a2
    fstp a5
    ;;
    add esi, 4*6
    add ebx, 4*6
    dec ecx
    jg  a100

;;-----------
y0   textequ <dword ptr [edi]>
y1   textequ <dword ptr [edi][4]>
y2   textequ <dword ptr [edi][2*4]>
y3   textequ <dword ptr [edi][3*4]>
y4   textequ <dword ptr [edi][4*4]>
y5   textequ <dword ptr [edi][5*4]>
    mov edi, y_addr
    mov ebx, offset abuf
    mov ecx, 3

a200:
;    a02 = (a[0] + a[2]);
;    b02 = (a[3+0] + a[3+2]);
    fld a0
    fld a2
    faddp st(1), st
    fld a3
    fld a5
    faddp st(1), st     ;; b02 a02

;    c[0] = a02 + a[1];
;    c[1] = b02 + a[3+1];
    fld a1
    fadd st, st(2)
    fst y0          
    fld a4
    fadd st, st(2) ;; y1 y0 b02 a02   
;;
;;    y[2] = coef87*(a[0] - a[2]);
;;    y[3] = coef87*(a[3+0] - a[3+2]) - y[1];
    fld a0
    fld a3
    fld a5
    fsubp st(1), st     ;; a3-a5 a0
    fld a2
    fsubp st(2), st     ;; a3-a5 a0-a2 y1 y0 b02 a02
    fld coef87
    fmul st(1), st
    fmulp st(2), st      ;; y3 y2 y1 y0 b02 a02
    fsub st, st(2)
;c[1] = c[1] - c[0];
;c[2] = c[2] - c[1];
    fxch st(3)          ;; y0 y2 y1 y3 b02 a02
    fsubp st(2), st     ;; y2 y1 y3 b02 a02
    fxch                ;; y1 y2 y3 b02 a02
    fsub st(1), st      ;; y1 y2 y3 b02 a02
    fstp y1             ;; y2 y3 b02 a02
;c[4] = a02 - a[1] - a[1];
;c[5] = b02 - a[3+1] - a[3+1]    - c[3];
    fxch                ;; y3 y2 b02 a02
    fld a4
    fsub st(3), st
    fsubp st(3), st
    fld a1
    fsub st(4), st
    fsubp st(4), st     ;; y3 y2 y5 y4
    fsub st(2), st
;c[3] = c[3] - c[2];
;c[4] = c[4] - c[3];
;c[5] = c[5] - c[4];
    fxch
    fsub st(1), st
    fstp y2             ;; y3 y5 y4
    fsub    st(2), st
    fstp y3
    fxch
    fsub st(1), st
    fstp y4
    fstp y5

    add ebx, 4*6
    add edi, 4*192
    dec ecx
    jg a200



exit:
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret

_mdct6_3B endp
;;==========================================================
;;==========================================================
_TEXT ENDS
;====================================
          END

