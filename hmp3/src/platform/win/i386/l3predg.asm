; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: l3predg.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
; L3apredG.asm   
;
;  Layer 3 Geometric pred;
;
;  masm 6.13
;
; Pentium II
;
;-------------------------------------------------
.486P
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT
;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;
ALIGN 4

con20   dd  20.0
con30   dd  30.0
con200  dd  200.0


;; sqrt and recip tables and algor in aac\etest
include sqrt.inc
include recip.inc

tbuf dd 128 dup (0)


_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
;;==========================================================
sqrt_macro   macro mem_arg

        movzx   eax, byte ptr mem_arg[2]
        mov edx, mem_arg
        mov eax, look_sqrt[eax*4]
        shr edx, 1
        add eax, edx
        mov mem_arg, eax


endm
;;==========================================================
recip_macro   macro mem_arg

        mov edx, mem_arg
        xor edx, 07FFFFFFFh
        mov eax, edx
        shr edx, (23-8)
        and edx, 255
        mov edx, look_rec[edx*4]
        sub eax, edx
        mov mem_arg, eax


endm
;;==========================================================
minval = 03D088889h   ;; 1/30.0
;; r = 1/x
;; r = min(r, minval)

recip_min_macro   macro mem_arg

        mov edx, mem_arg
        xor edx, 07FFFFFFFh
        mov eax, edx
        shr edx, (23-8)
        sub eax, minval
        and edx, 255
        mov edx, look_rec[edx*4]
        sub eax, edx
        cdq
        and eax, edx
        add eax, minval

        mov mem_arg, eax


endm
;;==========================================================
align 4
_aL3pred_longGeo proc public
;;
;; z dimensions fixed
;;void aL3pred_longGeo(float z[][256][2], float fftbuf[][2], 
;;                          float pmeas[], int npred)
;; note: result can be > 1.000
;;
;; arguments
z         textequ <dword ptr [esp+4*(1+npush)]>
fftbuf    textequ <dword ptr [esp+4*(2+npush)]>
pmeas     textequ <dword ptr [esp+4*(3+npush)]>
n         textequ <dword ptr [esp+4*(4+npush)]>


x0     textequ <dword ptr [esi]>
y0     textequ <dword ptr [esi][4]>
x1     textequ <dword ptr [esi][4*2*256]>
y1     textequ <dword ptr [esi][4*2*256][4]>
x2     textequ <dword ptr [ebx]>
y2     textequ <dword ptr [ebx][4]>

;; tmps on stack
r02     textequ <dword ptr [esp]>
r11     textequ <dword ptr [esp][4]>
r       textequ <dword ptr [esp][2*4]>


npush = (4+3)
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub esp, 4*3
;;-----
    mov esi, z
    mov ebx, fftbuf
    mov edi, pmeas

    mov ecx, n
    test  ecx, ecx
    jle exit

    fld con20
    fld con30

a100:
    fld x0      ;;      x0  con30 con20
    fld x2      ;;   x2 x0
    fld st(1)   ;;   x0 x2 x0
    fmul st, st(1)      ;;  x0*x2 x2 x0

    fld y0
    fld y2      ;;   y2 y0
    fld st(1)   ;;   y0 y2 y0
    fmul st, st(1)      ;;  y0*y2 y2 y0 x0*x2 x2 x0   ;; stack maxed

    fsubp   st(3), st   ;;   y2 y0 x02 x2 x0
    fmulp   st(4), st   ;;   y0 x02 x2 x0*y2
    fmulp   st(2), st   ;;   x02 y0*x2 x0*y2
    fxch                ;;   y0*x2 x02 x0*y2
    faddp   st(2), st   ;;   x02 y02

    fld st(0)
    fmul    st, st(0)   ;;   x02*x02 x02 y02
    fld st(2)
    fmul    st, st(0)   ;;   y02*y02 x02*x02 x02 y02
    faddp   st(1), st   ;;   r02*r02 x02 y02
    fstp    r02         ;;   x02 y02
;;--
    fld x1
    fld y1
    fld st(1)       ;; x1 y1 x1
    fmul    st(2), st   ;;  x1 y1 x1*x1
    fmul    st, st(1)   ;;  x1*y1 y1 x1*x1
    fxch    st(1)       ;;  y1 x1*y1 x1*x1
    fmul    st, st(0)   ;;  y1*y1 x1*y1 x1*x1
    fsubp   st(2), st   ;;  x1*y1 x11
    fadd    st, st(0)   ;;  y11 x11
    ;;
    fld st(1)
    fmul    st, st(0)   ;;  x11*x11 y11 x11
    fld st(1)
    fmul    st, st(0)   ;;  y11*y11 x11*x11 y11 x11
    faddp   st(1), st   ;;  r11*r11 y11 x11
    fstp    r11
;;
;;                      ;;  y11 x11 x02 y02
    fsubp   st(3), st   ;;  x11 x02 y02-y11
    fsubp   st(1), st   ;;  x y
    fmul    st, st(0)
    fxch    
    fmul    st, st(0)   ;;  y*y x*x
    faddp   st(1), st   ;;  r*r
    fstp    r
;;---------
;; shift buffer  (runs faster with shift here instead of below)
    mov eax, x1
    mov edx, y1
    mov x0, eax
    mov y0, edx
    
    mov eax, x2
    mov edx, y2
    mov x1, eax
    mov y1, edx

    sqrt_macro r02
    fld r02             ;; r02 con30 con20
    fadd    st, st(1)
    sqrt_macro r11
    fld r11
    faddp st(1), st
    fstp    r02
    recip_macro r02  
    fld r02

    sqrt_macro r
    fld r           ;; r 1/rx con30 con20
    fadd    st, st(3)

    fmulp st(1), st
    fstp    dword ptr [edi]

    add esi, 4*2
    add ebx, 4*2
    add edi, 4
    dec ecx
    jne a100

    fstp    st(0)   ;; pop cons
    fstp    st(0)

;;------------------------



exit:
    add esp, 4*3
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_aL3pred_longGeo endp
;;==========================================================
align 4
_aL3pred_shortGeo proc public
;;
;; z dimensions fixed
;;void aL3pred_shortGeo(float z[3][128][2],  
;;                          float pmeas[], int npred, int npred0)
;; note: result can be > 1.000
;;
;; arguments
z         textequ <dword ptr [esp+4*(1+npush)]>
pmeas     textequ <dword ptr [esp+4*(2+npush)]>
n         textequ <dword ptr [esp+4*(3+npush)]>
n0        textequ <dword ptr [esp+4*(4+npush)]>


x0     textequ <dword ptr [esi]>
y0     textequ <dword ptr [esi][4]>
x1     textequ <dword ptr [esi][4*2*128]>
y1     textequ <dword ptr [esi][4*2*128][4]>
x2     textequ <dword ptr [esi][2*4*2*128]>
y2     textequ <dword ptr [esi][2*4*2*128][4]>

;; tmps on stack
r02     textequ <dword ptr [esp]>
r11     textequ <dword ptr [esp][4]>
r       textequ <dword ptr [esp][2*4]>


npush = (4+3)
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub esp, 4*3
;;-----
    mov esi, z
    xor edi, edi

    mov ecx, n
    test  ecx, ecx
    jle exit

    fld con200
a100:
    fld x0      ;;      x0  con200
    fld x2      ;;   x2 x0
    fld st(1)   ;;   x0 x2 x0
    fmul st, st(1)      ;;  x0*x2 x2 x0

    fld y0
    fld y2      ;;   y2 y0
    fld st(1)   ;;   y0 y2 y0
    fmul st, st(1)      ;;  y0*y2 y2 y0 x0*x2 x2 x0   ;; stack maxed

    fsubp   st(3), st   ;;   y2 y0 x02 x2 x0
    fmulp   st(4), st   ;;   y0 x02 x2 x0*y2
    fmulp   st(2), st   ;;   x02 y0*x2 x0*y2
    fxch                ;;   y0*x2 x02 x0*y2
    faddp   st(2), st   ;;   x02 y02

    fld st(0)
    fmul    st, st(0)   ;;   x02*x02 x02 y02
    fld st(2)
    fmul    st, st(0)   ;;   y02*y02 x02*x02 x02 y02
    faddp   st(1), st   ;;   r02*r02 x02 y02
    fstp    r02         ;;   x02 y02
;;--
    fld x1
    fld y1
    fld st(1)       ;; x1 y1 x1
    fmul    st(2), st   ;;  x1 y1 x1*x1
    fmul    st, st(1)   ;;  x1*y1 y1 x1*x1
    fxch    st(1)       ;;  y1 x1*y1 x1*x1
    fmul    st, st(0)   ;;  y1*y1 x1*y1 x1*x1
    fsubp   st(2), st   ;;  x1*y1 x11
    fadd    st, st(0)   ;;  y11 x11
    ;;
    fld st(1)
    fmul    st, st(0)   ;;  x11*x11 y11 x11
    fld st(1)
    fmul    st, st(0)   ;;  y11*y11 x11*x11 y11 x11
    faddp   st(1), st   ;;  r11*r11 y11 x11
    fstp    r11
;;
;;                      ;;  y11 x11 x02 y02
    fsubp   st(3), st   ;;  x11 x02 y02-y11
    fsubp   st(1), st   ;;  x y
    fmul    st, st(0)
    fxch    
    fmul    st, st(0)   ;;  y*y x*x
    faddp   st(1), st   ;;  r*r
    fstp    r
;;---------
    sqrt_macro r02
    fld r02             ;; r02 con200
    fadd    st, st(1)
    sqrt_macro r11
    fld r11
    faddp st(1), st
    fstp    r02
    recip_macro r02  
    fld r02

    sqrt_macro r
    fld r           ;; r 1/rx con200
    fadd    st, st(2)

    fmulp st(1), st
    fstp    dword ptr tbuf[edi]

    add esi, 4*2
    add edi, 4
    dec ecx
    jne a100

    fstp    st(0)   ;; pop con
;;------------------------

    xor esi, esi
    mov edi, pmeas
    mov ecx, n0
    test ecx, ecx
    jle a300
a200:
    mov ebx, tbuf[esi]

    mov eax, [edi]
    sub eax, ebx
    cdq
    and eax, edx
    add eax, ebx
    mov [edi], eax

    mov eax, [edi][4]
    sub eax, ebx
    cdq
    and eax, edx
    add eax, ebx
    mov [edi][4], eax

    mov eax, [edi][2*4]
    sub eax, ebx
    cdq
    and eax, edx
    add eax, ebx
    mov [edi][2*4], eax

    mov eax, [edi][3*4]
    sub eax, ebx
    cdq
    and eax, edx
    add eax, ebx
    mov [edi][3*4], eax


    add edi, 4*4
    add esi, 4

    dec ecx
    jg  a200


a300:
    mov ecx, n
    sub ecx, n0
    jle exit

a400:
    mov eax, tbuf[esi]
    mov [edi], eax
    mov [edi][4], eax
    mov [edi][2*4], eax
    mov [edi][3*4], eax

    add edi, 4*4
    add esi, 4

    dec ecx
    jg  a400


exit:
    add esp, 4*3
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_aL3pred_shortGeo endp
;====================================
;====================================
;====================================
_TEXT ENDS
;====================================
          END

