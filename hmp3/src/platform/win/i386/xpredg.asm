; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: xpredg.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
;  Katmai new instructions
;
;
;   geometric predict measure
; 
;  masm 6.14
;
; Pentium III  
;
;   npred_long  should be multiple of 16
;   npred_short should be multiple of  4
;
;-------------------------------------------------
        
.686
.XMM
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT

movmem16  textequ <movaps>
;;movmem16  textequ <movups>

;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;


align 16
con20   dd  4 dup (20.0)
con30   dd  4 dup (30.0)
con200  dd  4 dup (200.0)



_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
;;==========================================================
_xL3pred_longGeo proc public
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


x0     textequ <oword ptr [esi]>
y0     textequ <oword ptr [esi][4*4]>
x1     textequ <oword ptr [esi][4*2*256]>
y1     textequ <oword ptr [esi][4*2*256][4*4]>
x2     textequ <oword ptr [ebx]>
y2     textequ <oword ptr [ebx][4*4]>

npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----
    mov esi, z
    mov ebx, fftbuf
    mov edi, pmeas

    mov ecx, n
    test  ecx, ecx
    jle exit

a100:
;;------------------------
    movmem16    xmm4, x2
    movmem16    xmm7, y2    
    movaps  xmm5, xmm4
    shufps  xmm4, xmm7, 10001000b   ;; x2
    shufps  xmm5, xmm7, 11011101b   ;; y2

    movmem16    xmm0, x0
    movmem16    xmm1, y0
    movmem16    xmm2, x1
    movmem16    xmm3, y1

        ;; buffer shift
        movmem16    x0, xmm2
        movmem16    y0, xmm3
        movmem16    x1, xmm4
        movmem16    y1, xmm5


    movaps  xmm6, xmm4  ;; x2
    mulps   xmm4, xmm0  ;; x0*x2

    movaps  xmm7, xmm5  ;; y2
    mulps   xmm5, xmm1  ;; y0*y2 

    subps   xmm4, xmm5  ;; x02

    mulps   xmm6, xmm1  ;; y0*x2
    mulps   xmm7, xmm0  ;; x0*y2

    addps   xmm6, xmm7  ;; y02
    ;; xmm4 = x02,   xmm6 = y02
    ;; xmm2 = x1   xmm3 = y1

    movaps  xmm5, xmm2      ;; x1
    mulps   xmm2, xmm3      ;; x1*y1
    mulps   xmm5, xmm5      ;; x1*x1
    mulps   xmm3, xmm3      ;; y1*y1
    subps   xmm5, xmm3      ;; x11 = x1*x1-y1*y1
    addps   xmm2, xmm2      ;; y11 = 2*x1*y1

    ;;
    movaps  xmm0, xmm4  ;; x02
    mulps   xmm4, xmm4  ;; x02**2
    subps   xmm0, xmm5  ;; x02-x11 = xhat

    movaps  xmm1, xmm6  ;; y02
    mulps   xmm6, xmm6  ;; y02**2
    subps   xmm1, xmm2  ;; y02-y11 = yhat

    mulps   xmm5, xmm5  ;; x11**2
    mulps   xmm2, xmm2  ;; y11**2

    addps   xmm4, xmm6  ;; rr02
    addps   xmm2, xmm5  ;; rr11
    rsqrtps   xmm4, xmm4    ;; 1/r02
    rsqrtps   xmm2, xmm2    ;; 1/r11
    rcpps   xmm4, xmm4  ;; r02
    rcpps   xmm2, xmm2  ;; r11
    addps   xmm4, con30 ;; 
    addps   xmm4, xmm2  ;; 30 + r02 + r11
    rcpps   xmm4, xmm4  ;; 1/(30 + r02 + r11)

    mulps   xmm0, xmm0  ;; xhat**2
    mulps   xmm1, xmm1  ;; yhat**2

    addps   xmm0, xmm1  ;; rrhat
    rsqrtps xmm0, xmm0  ;; 1/rhat
    rcpps   xmm0, xmm0  ;; rhat
    addps   xmm0, con20

    mulps   xmm0, xmm4  ; pmeas
    
    movmem16 [edi], xmm0


    add esi, 2*16
    add ebx, 2*16
    add edi, 16
    sub ecx, 4      ;; 4 pmeas per iteration
    jg  a100

;;------------------------

;;stmxcsr z
;;mov eax, z



exit:
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_xL3pred_longGeo endp
;;==========================================================
_xL3pred_shortGeo proc public
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


x0     textequ <oword ptr [esi]>
y0     textequ <oword ptr [esi][16]>
x1     textequ <oword ptr [esi][4*2*128]>
y1     textequ <oword ptr [esi][4*2*128][16]>
x2     textequ <oword ptr [esi][2*4*2*128]>
y2     textequ <oword ptr [esi][2*4*2*128][16]>

npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----
    mov esi, z
    mov edi, pmeas
    mov ecx, n0
    sub ecx,  3
    jle short_only

;;------------
a100:
    movmem16    xmm4, x2
    movmem16    xmm7, y2    
    movaps  xmm5, xmm4
    shufps  xmm4, xmm7, 10001000b   ;; x2
    shufps  xmm5, xmm7, 11011101b   ;; y2

    movmem16    xmm0, x0
    movmem16    xmm7, y0
    movaps  xmm1, xmm0
    shufps  xmm0, xmm7, 10001000b   ;; x0
    shufps  xmm1, xmm7, 11011101b   ;; y0

    movmem16    xmm2, x1
    movmem16    xmm7, y1
    movaps  xmm3, xmm2
    shufps  xmm2, xmm7, 10001000b   ;; x1
    shufps  xmm3, xmm7, 11011101b   ;; y1

    movaps  xmm6, xmm4  ;; x2
    mulps   xmm4, xmm0  ;; x0*x2

    movaps  xmm7, xmm5  ;; y2
    mulps   xmm5, xmm1  ;; y0*y2 

    subps   xmm4, xmm5  ;; x02

    mulps   xmm6, xmm1  ;; y0*x2
    mulps   xmm7, xmm0  ;; x0*y2

    addps   xmm6, xmm7  ;; y02
    ;; xmm4 = x02,   xmm6 = y02
    ;; xmm2 = x1   xmm3 = y1

    movaps  xmm5, xmm2      ;; x1
    mulps   xmm2, xmm3      ;; x1*y1
    mulps   xmm5, xmm5      ;; x1*x1
    mulps   xmm3, xmm3      ;; y1*y1
    subps   xmm5, xmm3      ;; x11 = x1*x1-y1*y1
    addps   xmm2, xmm2      ;; y11 = 2*x1*y1

    ;;
    movaps  xmm0, xmm4  ;; x02
    mulps   xmm4, xmm4  ;; x02**2
    subps   xmm0, xmm5  ;; x02-x11 = xhat

    movaps  xmm1, xmm6  ;; y02
    mulps   xmm6, xmm6  ;; y02**2
    subps   xmm1, xmm2  ;; y02-y11 = yhat

    mulps   xmm5, xmm5  ;; x11**2
    mulps   xmm2, xmm2  ;; y11**2

    addps   xmm4, xmm6  ;; rr02
    addps   xmm2, xmm5  ;; rr11
    rsqrtps   xmm4, xmm4    ;; 1/r02
    rsqrtps   xmm2, xmm2    ;; 1/r11
    rcpps   xmm4, xmm4  ;; r02
    rcpps   xmm2, xmm2  ;; r11
    addps   xmm4, con200 ;; 
    addps   xmm4, xmm2  ;; 200 + r02 + r11
    rcpps   xmm4, xmm4  ;; 1/(200 + r02 + r11)

    mulps   xmm0, xmm0  ;; xhat**2
    mulps   xmm1, xmm1  ;; yhat**2

    addps   xmm0, xmm1  ;; rrhat
    rsqrtps xmm0, xmm0  ;; 1/rhat
    rcpps   xmm0, xmm0  ;; rhat
    addps   xmm0, con200

    mulps   xmm0, xmm4  ; pmeas

    ;; expand result by 4
    movaps  xmm1, xmm0
    movaps  xmm2, xmm0
    movaps  xmm3, xmm0

    shufps  xmm0, xmm0, 00000000b
    shufps  xmm1, xmm1, 01010101b
    shufps  xmm2, xmm2, 10101010b
    shufps  xmm3, xmm3, 11111111b

        movmem16 xmm4, [edi]
        movmem16 xmm5, [edi][16]
        movmem16 xmm6, [edi][2*16]
        movmem16 xmm7, [edi][3*16]
        minps xmm0, xmm4
        minps xmm1, xmm5
        minps xmm2, xmm6
        minps xmm3, xmm7

    movmem16 [edi],       xmm0
    movmem16 [edi][16],   xmm1
    movmem16 [edi][2*16], xmm2
    movmem16 [edi][3*16], xmm3

    add esi, 2*16
    add edi, 4*16
    sub ecx, 4      ;; 4 pmeas per iteration
    jg  a100
;;------------
short_only:
    sub ecx, n0
    add ecx, n
    jle exit
;;------------
a200:
    movmem16    xmm4, x2
    movmem16    xmm7, y2    
    movaps  xmm5, xmm4
    shufps  xmm4, xmm7, 10001000b   ;; x2
    shufps  xmm5, xmm7, 11011101b   ;; y2

    movmem16    xmm0, x0
    movmem16    xmm7, y0
    movaps  xmm1, xmm0
    shufps  xmm0, xmm7, 10001000b   ;; x0
    shufps  xmm1, xmm7, 11011101b   ;; y0

    movmem16    xmm2, x1
    movmem16    xmm7, y1
    movaps  xmm3, xmm2
    shufps  xmm2, xmm7, 10001000b   ;; x1
    shufps  xmm3, xmm7, 11011101b   ;; y1

    movaps  xmm6, xmm4  ;; x2
    mulps   xmm4, xmm0  ;; x0*x2

    movaps  xmm7, xmm5  ;; y2
    mulps   xmm5, xmm1  ;; y0*y2 

    subps   xmm4, xmm5  ;; x02

    mulps   xmm6, xmm1  ;; y0*x2
    mulps   xmm7, xmm0  ;; x0*y2

    addps   xmm6, xmm7  ;; y02
    ;; xmm4 = x02,   xmm6 = y02
    ;; xmm2 = x1   xmm3 = y1

    movaps  xmm5, xmm2      ;; x1
    mulps   xmm2, xmm3      ;; x1*y1
    mulps   xmm5, xmm5      ;; x1*x1
    mulps   xmm3, xmm3      ;; y1*y1
    subps   xmm5, xmm3      ;; x11 = x1*x1-y1*y1
    addps   xmm2, xmm2      ;; y11 = 2*x1*y1

    ;;
    movaps  xmm0, xmm4  ;; x02
    mulps   xmm4, xmm4  ;; x02**2
    subps   xmm0, xmm5  ;; x02-x11 = xhat

    movaps  xmm1, xmm6  ;; y02
    mulps   xmm6, xmm6  ;; y02**2
    subps   xmm1, xmm2  ;; y02-y11 = yhat

    mulps   xmm5, xmm5  ;; x11**2
    mulps   xmm2, xmm2  ;; y11**2

    addps   xmm4, xmm6  ;; rr02
    addps   xmm2, xmm5  ;; rr11
    rsqrtps   xmm4, xmm4    ;; 1/r02
    rsqrtps   xmm2, xmm2    ;; 1/r11
    rcpps   xmm4, xmm4  ;; r02
    rcpps   xmm2, xmm2  ;; r11
    addps   xmm4, con200 ;; 
    addps   xmm4, xmm2  ;; 200 + r02 + r11
    rcpps   xmm4, xmm4  ;; 1/(200 + r02 + r11)

    mulps   xmm0, xmm0  ;; xhat**2
    mulps   xmm1, xmm1  ;; yhat**2

    addps   xmm0, xmm1  ;; rrhat
    rsqrtps xmm0, xmm0  ;; 1/rhat
    rcpps   xmm0, xmm0  ;; rhat
    addps   xmm0, con200

    mulps   xmm0, xmm4  ; pmeas

    ;; expand result by 4
    movaps  xmm1, xmm0
    movaps  xmm2, xmm0
    movaps  xmm3, xmm0

    shufps  xmm0, xmm0, 00000000b
    shufps  xmm1, xmm1, 01010101b
    shufps  xmm2, xmm2, 10101010b
    shufps  xmm3, xmm3, 11111111b
        
    movmem16 [edi],       xmm0
    movmem16 [edi][16],   xmm1
    movmem16 [edi][2*16], xmm2
    movmem16 [edi][3*16], xmm3

    add esi, 2*16
    add edi, 4*16
    sub ecx, 4      ;; 4 pmeas per iteration
    jg  a200
;;------------




exit:
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_xL3pred_shortGeo endp
;====================================
;====================================
;====================================
_TEXT ENDS
;====================================
          END

