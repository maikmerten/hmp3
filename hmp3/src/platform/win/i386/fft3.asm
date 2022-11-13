; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: fft3.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
; FFT3.asm   
;
;   256 pt and 1024 pt real input fft w/hann window
;   NO inplace
;
;  masm 6.0
;
; Pentium II
;
;-------------------------------------------------
.486P
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT


half1024output = 0
;; no savings by doing half out

;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;
ALIGN 4
;


;;static int bitrev[512];
bitrev  dd 256 dup (0)      ;; half size table

hann    dd   256*2  dup (0)    ;; half window in pairs
hann256 dd    64*2  dup (0)    ;; half window in pairs

;; radix 2 twid factors
wcs     dd   256*2  dup (0)   ;;      // sized for 512 pt fft

;;// radix 4 twid factors
;;// wc1 ws1  wc2 ws2  wc3 ws3    // sized for 512 pt fft
wcs4    dd   64*3*2 dup (0)

;;// convert to real wwid factors, 1024 pt real input FFT
wr      dd   256*2  dup (0)

;; DFT  1024 point
;; sin coefs obtained from cosine coefs by reflections and offsets
;; must keep this order for init
tc1 dd 257 dup (0)
;ts1 dd 257 dup (0)

tc2 dd (256+128) dup (0)
;ts2 dd (256+128) dup (0)

tc3 dd 257 dup (0)
;ts3 dd 257 dup (0)

tc4 dd (256+64) dup (0)
;ts4 dd (256+64) dup (0)

tc5 dd 257 dup (0)
;ts5 dd 257 dup (0)


;; DFT 512 approx to 1024 point
;;

con25   dd  0.25
con50   dd  0.50
hann512 dd   256  dup (0) ;; half window  512pt FFT/DFT approx to 1024

;; sin coefs obtained from cosine coefs by reflections and offsets
;; must keep this order for init
atc1 dd 129 dup (0)
atc2 dd (128+64) dup (0)
atc3 dd 129 dup (0)
atc4 dd (128+32) dup (0)
atc5 dd 129 dup (0)




_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
;;---------------------------------------------------
;; external init functions

extrn _b3FFT_init_R2Twid:near
extrn _b3FFT_init_R4Twid:near
extrn _b3FFT_init_RealTwid:near
extrn _b3FFT_init_Hann1024:near
extrn _b3FFT_init_Hann256:near
extrn _b3FFT_init_BitReverse:near
extrn _DFT_init_wcs1024:near
extrn _DFT_init_wcs512:near
extrn _b3FFT_init_Hann512:near      ;; for 512 point DFT


;;  void b3FFT_init_R2Twid( float wcs[][2]);
;;  void b3FFT_init_R4Twid( float wcs4[][3][2]);
;;  void b3FFT_init_RealTwid(float wr[][2]);
;;  void b3FFT_init_Hann1024( float hann[][2]);
;;  void b3FFT_init_Hann512( float hann[][2]);
;;  void b3FFT_init_Hann256( float hann256[][2]);
;;  void b3FFT_init_BitReverse( int bitrev[], int n);
;;  void DFT_init_wcs1024( float wcs[]) 
;;  void DFT_init_wcs512( float wcs[]) 

;;---------------------------------------------------
b3FFT_init  PROC C PUBLIC \
		uses esi edi ebx ecx 


    sub esp, 8      ;; two args
arg1 textequ <dword ptr [esp]>
arg2 textequ <dword ptr [esp+4]>

;; dft init
    mov arg1,    offset hann512
    call _b3FFT_init_Hann512

    mov arg1,    offset atc1
    call _DFT_init_wcs512

    mov arg1,    offset tc1
    call _DFT_init_wcs1024
;;
;;
    mov arg1,    offset wcs
    call _b3FFT_init_R2Twid

    mov arg1,    offset wcs4
    call _b3FFT_init_R4Twid

    mov arg1,    offset wr
    call _b3FFT_init_RealTwid

    mov arg1,    offset hann
    call _b3FFT_init_Hann1024

    mov arg1,    offset hann256
    call _b3FFT_init_Hann256

    mov arg1,    offset bitrev
    mov arg2,    256     ;; half table
    call _b3FFT_init_BitReverse


;; scale bitreverse table up by 8
    mov ecx, 256
    xor esi, esi
a100:
    mov eax, bitrev[esi]
    shl eax, 3
    mov bitrev[esi], eax
    add esi, 4
    dec ecx
    jne a100


    add esp, 8

    ret
b3FFT_init endp
;;===================================================
R2Stage1    proc near
;;
;;  esi = buffer
;;  ecx = loop count
;;
;;

a textequ <dword ptr [esi]>
b textequ <dword ptr [esi][8]>

    xor ebx, ebx

a100:
    fld b[0]
    fld a[0]
    fsub    st, st(1)       ;; u 
        
    fld b[4]
    fld a[4]
    fsub    st, st(1)       ;; v
        
    fld  wcs[ebx][0]
    fmul st, st(3)
    
    fld  wcs[ebx][4]
    fmul st, st(2)
    faddp   st(1), st       
    fstp    b[0]
       
    fld  wcs[ebx][4]
    fmulp   st(3), st   ;; u*ws

    fld  wcs[ebx][0]
    fmulp   st(1), st   ;; u*ws
    fsubrp  st(2), st


    fld a[4]
    faddp   st(1), st
    fstp    a[4]

    fstp    b[4]

    fld a[0]
    faddp   st(1), st
    fstp    a[0]

    add esi, 16
    add ebx, 8
    dec ecx
    jne a100


    ret
R2Stage1    endp
;;---------------------------------------------------
R4_bfly macro  slot1
;;
;;
    fld x0
    fld x2
    fadd    st(1), st   ;; txa
    fsubr   x0          ;; txc txa

    fld x1
    fld x3
    fadd    st(1), st
    fsubr x1            ;; txd txb txc txa

    fld st(3)           ;; txa txd txb txc txa
    fadd    st, st(2)   ;; 
    fstp    xx0          ;; txd txb txc txa

    fxch                ;; txb txd txc txa
    fsubp   st(3), st   ;; txd txc uxa
;;--
    fld y0
    fld y2
    fadd    st(1), st   ;; tya
    fsubr   y0          ;; tyc tya

    fld y1
    fld y3
    fadd    st(1), st
    fsubr y1            ;; tyd tyb tyc tya

    fld st(3)           ;; tya tyd tyb tyc tya  ;; txd txc uxa
    fadd    st, st(2)   ;; 
    fstp    yy0          ;; tyd tyb tyc tya      ;; txd txc uxa

    fxch                ;; tyb tyd tyc tya
    fsubp   st(3), st   ;; tyd tyc uya         ;; txd txc uxa
;;---
    fld wc2[ebx]
    fmul    st, st(6)   ;; wc2*uxa tyd tyc uya   txd txc uxa


    fld ws2[ebx]
    fmul    st, st(4)   ;; ws2*uya wc2*uxa tyd tyc uya   txd txc uxa

;; 6344

    faddp   st(1), st   ;; 
    fstp    xx2          ;; tyd tyc uya   txd txc uxa

    fld st(4)
    fadd    st, st(1)   ;;* uxc tyd tyc uya   txd txc uxa

    fld ws2[ebx]
    fmulp   st(7), st   ;; uxc tyd tyc uya   txd txc ws2*uxa

    fxch    st(3)       ;; uya tyd tyc uxc   txd txc ws2*uxa

    fmul    wc2[ebx]    ;; wc2*uya tyd tyc uxc   txd txc ws2*uxa
    fsubrp  st(6), st   ;; tyd tyc uxc   txd txc y1
    fxch    st(5)
    fstp    yy2          ;; tyc uxc   txd txc tyd

    fld st(0)
    fsub    st, st(3)   ;;* uyc tyc uxc   txd txc tyd

ifnb <slot1>
    slot1
endif
;; 6265

    fld wc1[ebx]
    fmul st, st(3)      ;; wc1*uxc uyc tyc uxc   txd txc tyd

    fld ws1[ebx]
    fmul st, st(2)      ;; ws1*uyc wc1*uxc uyc tyc uxc   txd txc tyd
    faddp   st(1), st
    fstp    xx1          ;; uyc tyc uxc   txd txc tyd

;; 6299

    fmul    wc1[ebx]    ;; wc1*uyc tyc uxc   txd txc tyd
    fld ws1[ebx]
    fmulp   st(3), st   ;; wc1*uyc tyc ws1*uxc   txd txc tyd
    fsubrp  st(2), st   ;; tyc y2   txd txc tyd
    fxch
    fstp    yy1          ;; tyc txd txc tyd

    faddp  st(1), st    ;;* uyd  txc tyd
    fxch
    fsubrp  st(2), st   ;;* uyd  uxd

    fld wc3[ebx]
    fmul    st, st(2)   ;; wc3*uxd uyd uxd

    fld ws3[ebx]
    fmul    st, st(2)   ;; ws3*uyd  wc3*uxd uyd uxd

    faddp   st(1), st
    fstp    xx3          ;; uyd uxd

    fld wc3[ebx]
    fmulp   st(1), st   ;; wc3*uyd uxd

    fld ws3[ebx]
    fmulp   st(2), st   ;; wc3*uyd ws3*uxd


    fsubrp  st(1), st
    fstp    yy3


;; 6442
endm
;;---------------------------------------------------
;;---------------------------------------------------
R4_bfly0    macro slot1
;;
;; special coef case
;;
;;
    fld x0
    fld x2
    fadd    st, st(1)   ;; txa x0

    fld x1
    fld x3
    fadd    st, st(1)   ;; txb x1 txa x0

    fld x2
    fsubp st(4), st     ;; txb x1 txa txc

    fld x3
    fsubp st(2), st     ;; txb txd txa txc

    fld st(2)           ;; txa txb txd txa txc
    fadd st, st(1)
    fstp    xx0         ;; txb txd txa txc

    fsubp   st(2), st
    fxch    
    fstp    xx2         ;; txd txc

    fld y0
    fld y2
    fadd    st, st(1)   ;; tya y0 txd txc

    fld y1
    fld y3
    fadd    st, st(1)   ;; tyb y1 tya y0 txd txc

    fld y2
    fsubp   st(4), st   ;; tyb y1 tya tyc txd txc

    fld y3
    fsubp   st(2), st   ;; tyb tyd tya tyc txd txc
;;
;; 5678

    fld st(2)
    fadd st, st(1)
    fstp yy0            ;; tyb tyd tya tyc txd txc

    fsubp   st(2), st   ;; tyd yy1 tyc txd txc
    fxch
    fstp    yy2         ;; tyd tyc txd txc

    fld st(3)
    fsub    st, st(1)
    fstp    xx3         ;;* tyd tyc txd txc

    faddp   st(3), st   ;;* tyc txd xx1

;; 5703

    fld st(0)           ;;* tyc tyc txd xx1
    fsub st, st(2)
    fstp yy1            ;; tyc txd xx1
    faddp st(1), st     ;;*
    fstp  yy3
    fstp  xx1

ifnb <slot1>
    slot1
endif
;; 5646

endm
;;---------------------------------------------------
;;---------------------------------------------------
R4Stage1 proc near
;;
;; input output = esi
;; uses esi edi ecx
;;
;; caller sets loop count in ecx
;;
;; middle reads exchanged by text equates
;;


wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>


    mov edi, esi
    xor ebx, ebx

a100:
;;----------------------
;; even
    x0 textequ <dword ptr [esi][2*0*8]>
    x1 textequ <dword ptr [esi][2*2*8]>
    x2 textequ <dword ptr [esi][2*1*8]>
    x3 textequ <dword ptr [esi][2*3*8]>

    y0 textequ <dword ptr [esi][2*0*8][4]>
    y1 textequ <dword ptr [esi][2*2*8][4]>
    y2 textequ <dword ptr [esi][2*1*8][4]>
    y3 textequ <dword ptr [esi][2*3*8][4]>

    xx0 textequ <dword ptr [edi][2*0*8]>
    xx1 textequ <dword ptr [edi][2*1*8]>
    xx2 textequ <dword ptr [edi][2*2*8]>
    xx3 textequ <dword ptr [edi][2*3*8]>

    yy0 textequ <dword ptr [edi][2*0*8][4]>
    yy1 textequ <dword ptr [edi][2*1*8][4]>
    yy2 textequ <dword ptr [edi][2*2*8][4]>
    yy3 textequ <dword ptr [edi][2*3*8][4]>

    R4_bfly
;;-----------------------
;; odd
    x0 textequ <dword ptr [esi][8][2*0*8]>
    x1 textequ <dword ptr [esi][8][2*2*8]>
    x2 textequ <dword ptr [esi][8][2*1*8]>
    x3 textequ <dword ptr [esi][8][2*3*8]>

    y0 textequ <dword ptr [esi][8][2*0*8][4]>
    y1 textequ <dword ptr [esi][8][2*2*8][4]>
    y2 textequ <dword ptr [esi][8][2*1*8][4]>
    y3 textequ <dword ptr [esi][8][2*3*8][4]>

    xx0 textequ <dword ptr [edi][8][2*0*8]>
    xx1 textequ <dword ptr [edi][8][2*1*8]>
    xx2 textequ <dword ptr [edi][8][2*2*8]>
    xx3 textequ <dword ptr [edi][8][2*3*8]>

    yy0 textequ <dword ptr [edi][8][2*0*8][4]>
    yy1 textequ <dword ptr [edi][8][2*1*8][4]>
    yy2 textequ <dword ptr [edi][8][2*2*8][4]>
    yy3 textequ <dword ptr [edi][8][2*3*8][4]>

    R4_bfly  <add esi, 64>

;;-----------------------
;;add esi, 64     ;; inc by 8 complex values
add edi, 64     ;; inc by 8 complex values
add ebx, 24

    dec ecx
    jne a100



    ret
R4Stage1 endp
;;---------------------------------------------------
;;---------------------------------------------------
R4Stage2 proc near
;;
;; input output = esi
;; uses esi edi ecx edx
;;
;; caller sets outer loop count in ecx
;;
;; middle reads exchanged by text equates
;;
wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>

    mov edi, esi
    xor ebx, ebx

a200:
    mov edx, 4
a100:
;;----------------------
;; even
    x0 textequ <dword ptr [esi][2*0*32]>
    x1 textequ <dword ptr [esi][2*2*32]>
    x2 textequ <dword ptr [esi][2*1*32]>
    x3 textequ <dword ptr [esi][2*3*32]>

    y0 textequ <dword ptr [esi][2*0*32][4]>
    y1 textequ <dword ptr [esi][2*2*32][4]>
    y2 textequ <dword ptr [esi][2*1*32][4]>
    y3 textequ <dword ptr [esi][2*3*32][4]>

    xx0 textequ <dword ptr [edi][2*0*32]>
    xx1 textequ <dword ptr [edi][2*1*32]>
    xx2 textequ <dword ptr [edi][2*2*32]>
    xx3 textequ <dword ptr [edi][2*3*32]>

    yy0 textequ <dword ptr [edi][2*0*32][4]>
    yy1 textequ <dword ptr [edi][2*1*32][4]>
    yy2 textequ <dword ptr [edi][2*2*32][4]>
    yy3 textequ <dword ptr [edi][2*3*32][4]>

    R4_bfly
;;-----------------------
;; odd
    x0 textequ <dword ptr [esi][8][2*0*32]>
    x1 textequ <dword ptr [esi][8][2*2*32]>
    x2 textequ <dword ptr [esi][8][2*1*32]>
    x3 textequ <dword ptr [esi][8][2*3*32]>

    y0 textequ <dword ptr [esi][8][2*0*32][4]>
    y1 textequ <dword ptr [esi][8][2*2*32][4]>
    y2 textequ <dword ptr [esi][8][2*1*32][4]>
    y3 textequ <dword ptr [esi][8][2*3*32][4]>

    xx0 textequ <dword ptr [edi][8][2*0*32]>
    xx1 textequ <dword ptr [edi][8][2*1*32]>
    xx2 textequ <dword ptr [edi][8][2*2*32]>
    xx3 textequ <dword ptr [edi][8][2*3*32]>

    yy0 textequ <dword ptr [edi][8][2*0*32][4]>
    yy1 textequ <dword ptr [edi][8][2*1*32][4]>
    yy2 textequ <dword ptr [edi][8][2*2*32][4]>
    yy3 textequ <dword ptr [edi][8][2*3*32][4]>

    R4_bfly  <add esi, 16>

;;-----------------------
;;add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec edx
    jne a100

add esi, 8*(2*(4*4-4))
add edi, 8*(2*(4*4-4))

add ebx, 24

    dec ecx
    jne a200



    ret
R4Stage2 endp
;;---------------------------------------------------
;;---------------------------------------------------
R4Stage3 proc near
;;
;; input output = esi
;; uses esi edi ecx edx
;;
;; middle reads exchanged by text equates
;;
wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>

    mov edi, esi

;;------- special case coefs
    mov edx, 16
a100:
;;----------------------
;; even
    x0 textequ <dword ptr [esi][2*0*128]>
    x1 textequ <dword ptr [esi][2*2*128]>
    x2 textequ <dword ptr [esi][2*1*128]>
    x3 textequ <dword ptr [esi][2*3*128]>

    y0 textequ <dword ptr [esi][2*0*128][4]>
    y1 textequ <dword ptr [esi][2*2*128][4]>
    y2 textequ <dword ptr [esi][2*1*128][4]>
    y3 textequ <dword ptr [esi][2*3*128][4]>

    xx0 textequ <dword ptr [edi][2*0*128]>
    xx1 textequ <dword ptr [edi][2*1*128]>
    xx2 textequ <dword ptr [edi][2*2*128]>
    xx3 textequ <dword ptr [edi][2*3*128]>

    yy0 textequ <dword ptr [edi][2*0*128][4]>
    yy1 textequ <dword ptr [edi][2*1*128][4]>
    yy2 textequ <dword ptr [edi][2*2*128][4]>
    yy3 textequ <dword ptr [edi][2*3*128][4]>

    R4_bfly0
;;-----------------------
;; odd
    x0 textequ <dword ptr [esi][8][2*0*128]>
    x1 textequ <dword ptr [esi][8][2*2*128]>
    x2 textequ <dword ptr [esi][8][2*1*128]>
    x3 textequ <dword ptr [esi][8][2*3*128]>

    y0 textequ <dword ptr [esi][8][2*0*128][4]>
    y1 textequ <dword ptr [esi][8][2*2*128][4]>
    y2 textequ <dword ptr [esi][8][2*1*128][4]>
    y3 textequ <dword ptr [esi][8][2*3*128][4]>

    xx0 textequ <dword ptr [edi][8][2*0*128]>
    xx1 textequ <dword ptr [edi][8][2*1*128]>
    xx2 textequ <dword ptr [edi][8][2*2*128]>
    xx3 textequ <dword ptr [edi][8][2*3*128]>

    yy0 textequ <dword ptr [edi][8][2*0*128][4]>
    yy1 textequ <dword ptr [edi][8][2*1*128][4]>
    yy2 textequ <dword ptr [edi][8][2*2*128][4]>
    yy3 textequ <dword ptr [edi][8][2*3*128][4]>

    R4_bfly0  <add esi, 16>

;;-----------------------
;;add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values
    dec edx
    jne a100

add esi, 8*(2*(4*16-16))
add edi, 8*(2*(4*16-16))
mov ebx, 24
;;==========================
;;==========================
    mov ecx, 3
a300:
    mov edx, 16
a200:
;;----------------------
;; even
    x0 textequ <dword ptr [esi][2*0*128]>
    x1 textequ <dword ptr [esi][2*2*128]>
    x2 textequ <dword ptr [esi][2*1*128]>
    x3 textequ <dword ptr [esi][2*3*128]>

    y0 textequ <dword ptr [esi][2*0*128][4]>
    y1 textequ <dword ptr [esi][2*2*128][4]>
    y2 textequ <dword ptr [esi][2*1*128][4]>
    y3 textequ <dword ptr [esi][2*3*128][4]>

    xx0 textequ <dword ptr [edi][2*0*128]>
    xx1 textequ <dword ptr [edi][2*1*128]>
    xx2 textequ <dword ptr [edi][2*2*128]>
    xx3 textequ <dword ptr [edi][2*3*128]>

    yy0 textequ <dword ptr [edi][2*0*128][4]>
    yy1 textequ <dword ptr [edi][2*1*128][4]>
    yy2 textequ <dword ptr [edi][2*2*128][4]>
    yy3 textequ <dword ptr [edi][2*3*128][4]>

    R4_bfly
;;-----------------------
;; odd
    x0 textequ <dword ptr [esi][8][2*0*128]>
    x1 textequ <dword ptr [esi][8][2*2*128]>
    x2 textequ <dword ptr [esi][8][2*1*128]>
    x3 textequ <dword ptr [esi][8][2*3*128]>

    y0 textequ <dword ptr [esi][8][2*0*128][4]>
    y1 textequ <dword ptr [esi][8][2*2*128][4]>
    y2 textequ <dword ptr [esi][8][2*1*128][4]>
    y3 textequ <dword ptr [esi][8][2*3*128][4]>

    xx0 textequ <dword ptr [edi][8][2*0*128]>
    xx1 textequ <dword ptr [edi][8][2*1*128]>
    xx2 textequ <dword ptr [edi][8][2*2*128]>
    xx3 textequ <dword ptr [edi][8][2*3*128]>

    yy0 textequ <dword ptr [edi][8][2*0*128][4]>
    yy1 textequ <dword ptr [edi][8][2*1*128][4]>
    yy2 textequ <dword ptr [edi][8][2*2*128][4]>
    yy3 textequ <dword ptr [edi][8][2*3*128][4]>

    R4_bfly  <add esi, 16>

;;-----------------------
;;add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec edx
    jne a200

add esi, 8*(2*(4*16-16))
add edi, 8*(2*(4*16-16))

add ebx, 24

    dec ecx
    jne a300



    ret
R4Stage3 endp
;;---------------------------------------------------
R4StageLast512 proc near
;;
;; input output = esi
;; uses esi edi ecx
;;
;;
;; middle reads exchanged by text equates
;;
    mov edi, esi
    mov ecx, 64

a100:
;; even
    x0 textequ <dword ptr [esi][2*0*512]>
    x1 textequ <dword ptr [esi][2*2*512]>
    x2 textequ <dword ptr [esi][2*1*512]>
    x3 textequ <dword ptr [esi][2*3*512]>

    y0 textequ <dword ptr [esi][2*0*512][4]>
    y1 textequ <dword ptr [esi][2*2*512][4]>
    y2 textequ <dword ptr [esi][2*1*512][4]>
    y3 textequ <dword ptr [esi][2*3*512][4]>

    xx0 textequ <dword ptr [edi][2*0*512]>
    xx1 textequ <dword ptr [edi][2*1*512]>
    xx2 textequ <dword ptr [edi][2*2*512]>
    xx3 textequ <dword ptr [edi][2*3*512]>

    yy0 textequ <dword ptr [edi][2*0*512][4]>
    yy1 textequ <dword ptr [edi][2*1*512][4]>
    yy2 textequ <dword ptr [edi][2*2*512][4]>
    yy3 textequ <dword ptr [edi][2*3*512][4]>

    R4_bfly0

;; odd
    x0 textequ <dword ptr [esi][8][2*0*512]>
    x1 textequ <dword ptr [esi][8][2*2*512]>
    x2 textequ <dword ptr [esi][8][2*1*512]>
    x3 textequ <dword ptr [esi][8][2*3*512]>

    y0 textequ <dword ptr [esi][8][2*0*512][4]>
    y1 textequ <dword ptr [esi][8][2*2*512][4]>
    y2 textequ <dword ptr [esi][8][2*1*512][4]>
    y3 textequ <dword ptr [esi][8][2*3*512][4]>

    xx0 textequ <dword ptr [edi][8][2*0*512]>
    xx1 textequ <dword ptr [edi][8][2*1*512]>
    xx2 textequ <dword ptr [edi][8][2*2*512]>
    xx3 textequ <dword ptr [edi][8][2*3*512]>

    yy0 textequ <dword ptr [edi][8][2*0*512][4]>
    yy1 textequ <dword ptr [edi][8][2*1*512][4]>
    yy2 textequ <dword ptr [edi][8][2*2*512][4]>
    yy3 textequ <dword ptr [edi][8][2*3*512][4]>

    R4_bfly0

;;-----------------------
add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec ecx
    jne a100

    ret
R4StageLast512 endp
;;===================================================
;;---------------------------------------------------
R4StageLast128 proc near
;;
;; input output = esi
;; uses esi edi ecx
;;
;;
;; middle reads exchanged by text equates
;;
    mov edi, esi
    mov ecx, 16

a100:
;; even
    x0 textequ <dword ptr [esi][2*0*128]>
    x1 textequ <dword ptr [esi][2*2*128]>
    x2 textequ <dword ptr [esi][2*1*128]>
    x3 textequ <dword ptr [esi][2*3*128]>

    y0 textequ <dword ptr [esi][2*0*128][4]>
    y1 textequ <dword ptr [esi][2*2*128][4]>
    y2 textequ <dword ptr [esi][2*1*128][4]>
    y3 textequ <dword ptr [esi][2*3*128][4]>

    xx0 textequ <dword ptr [edi][2*0*128]>
    xx1 textequ <dword ptr [edi][2*1*128]>
    xx2 textequ <dword ptr [edi][2*2*128]>
    xx3 textequ <dword ptr [edi][2*3*128]>

    yy0 textequ <dword ptr [edi][2*0*128][4]>
    yy1 textequ <dword ptr [edi][2*1*128][4]>
    yy2 textequ <dword ptr [edi][2*2*128][4]>
    yy3 textequ <dword ptr [edi][2*3*128][4]>

    R4_bfly0

;; odd
    x0 textequ <dword ptr [esi][8][2*0*128]>
    x1 textequ <dword ptr [esi][8][2*2*128]>
    x2 textequ <dword ptr [esi][8][2*1*128]>
    x3 textequ <dword ptr [esi][8][2*3*128]>

    y0 textequ <dword ptr [esi][8][2*0*128][4]>
    y1 textequ <dword ptr [esi][8][2*2*128][4]>
    y2 textequ <dword ptr [esi][8][2*1*128][4]>
    y3 textequ <dword ptr [esi][8][2*3*128][4]>

    xx0 textequ <dword ptr [edi][8][2*0*128]>
    xx1 textequ <dword ptr [edi][8][2*1*128]>
    xx2 textequ <dword ptr [edi][8][2*2*128]>
    xx3 textequ <dword ptr [edi][8][2*3*128]>

    yy0 textequ <dword ptr [edi][8][2*0*128][4]>
    yy1 textequ <dword ptr [edi][8][2*1*128][4]>
    yy2 textequ <dword ptr [edi][8][2*2*128][4]>
    yy3 textequ <dword ptr [edi][8][2*3*128][4]>

    R4_bfly0

;;-----------------------
add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec ecx
    jne a100

    ret
R4StageLast128 endp
;;===================================================
bFFT1024RealHann  PROC C PUBLIC \
		uses esi edi ebx ecx     \
        xin:dword,
        yout:dword
;;
;;
;;----------------------------
;; apply window and bit reverse 
    mov esi, xin
    mov edi, yout
x textequ <dword ptr [esi]>
y textequ <dword ptr [edi]>
    mov eax, 0
    mov edx, 8*511
    mov ecx, 256

a100:
    mov ebx, bitrev[eax]

    fld hann[eax*2][0]
    fld hann[eax*2][4]

    fld x[eax*2][0]
    fmul   st, st(2)       ;; hann[i][0]*x[i][0]

    fld x[eax*2][4]
    fmul   st, st(2)       ;; hann[i][1]*x[i][1]
    add eax, 4

    fld x[edx][0]
    fmulp  st(3), st       ;; hann[i][1]*x[j][0]

    fld x[edx][4]
    fmulp  st(4), st       ;; hann[i][1]*x[j][1]
    sub edx, 8

    fstp    y[ebx][4]
    fstp    y[ebx][0]
    xor ebx, (511 shl 3)    ;; table scaled by 8
    fstp    y[ebx][0]
    fstp    y[ebx][4]


    dec ecx
    jne a100
;;----------------------------
    mov esi, yout
    mov ecx, 256
    call R2Stage1

    mov esi, yout
    mov ecx, 64
    call R4Stage1

    mov esi, yout
    mov ecx, 16
    call R4Stage2

    mov esi, yout
    call R4Stage3

    mov esi, yout
    call R4StageLast512

;;-----------
;; convert to real FFT
;;  conversion has factor of 2 scaling over actual

a0 textequ <dword ptr [esi]>  
a  textequ <dword ptr [esi+ebx]>  
b  textequ <dword ptr [edi]>

x100:
    mov esi, yout
    mov ebx, 8
    lea edi, [esi+8*511]

;; special case z[0]
    fld     a0[0]
    fadd    a0[4]
    fadd    st, st(0)
    fstp    a0[0]
    mov     a0[4], 0

    mov ecx, 255

c100:
    fld a[0]
    fld b[0]
    fsubr    st, st(1)  ;; txb

    fld a[4]
    fld b[4]
    fadd     st, st(1)  ;; tya ay txb ax

    fld     wr[ebx][0]
    fmul    st, st(3)   ;; wc*txb tya ay txb ax

    fld     wr[ebx][4]
    fmul    st, st(2)   ;; ws*tya wc*txb tya ay txb ax

    fld b[0]
    faddp   st(6), st   ;; ws*tya wc*txb tya ay txb txa

    faddp   st(1), st   ;; qx tya ay txb txa

    fld     wr[ebx][4]
    fmulp   st(4), st   ;; qx tya ay ws*txb txa

    fld     wr[ebx][0]
    fmulp   st(2), st   ;; qx wc*tya ay ws*txb txa

    fld b[4]
    fsubp   st(3), st   ;; qx wc*tya tyb ws*txb txa
    
    fxch                ;; wc*tya qx tyb ws*txb txa
    fsubrp  st(3), st   ;; qx tyb qy txa

    ;;ax = txa+qx;
    ;;ay = tyb+qy;
    ;;bx = txa-qx;
    ;;by = qy-tyb;

    ;;  qx tyb qy txa

if half1024output
    faddp st(3), st
    faddp st(1), st
    fstp    a[4]
    fstp    a[0]
    add ebx, 8
else
    fld st(0)
    fadd    st, st(4)   ;; ax qx tyb qy txa
    fld st(3)
    fadd    st, st(3)   ;; ay ax qx tyb qy txa
    fxch    st(2)       ;; qx ax ay tyb qy txa
    fsubp   st(5), st   ;; ax ay tyb qy bx
    fxch    st(2)       ;; tyb ay ax qy bx
    fsubp   st(3), st   ;; ay ax by bx
    fstp    a[4]
    fstp    a[0]
    add ebx, 8
    fstp    b[4]
    fstp    b[0]
endif


    sub edi, 8
    dec ecx
    jne c100

;;-----
;; special case z[256]
    mov edx, a[4]
    mov eax, a[0]

    add edx, (1 shl 23)     ;; x = 2.0*x
    add eax, (1 shl 23)

    xor edx, (1 shl 31)     ;; x = -x

    mov a[4], edx
    mov a[0], eax

;;
;;
;;
;;
    ret
bFFT1024RealHann  endp
;====================================
;;===================================================
bFFT256RealHann  PROC C PUBLIC \
		uses esi edi ebx ecx     \
        xin:dword,
        yout:dword
;;
;;
;;----------------------------
;; apply window and bit reverse 
    mov esi, xin
    mov edi, yout
x textequ <dword ptr [esi]>
y textequ <dword ptr [edi]>
    mov eax, 0
    mov edx, 8*127
    mov ecx, 64

a100:
    mov ebx, bitrev[eax*4]

    fld hann256[eax*2][0]
    fld hann256[eax*2][4]

    fld x[eax*2][0]
    fmul   st, st(2)       ;; hann[i][0]*x[i][0]

    fld x[eax*2][4]
    fmul   st, st(2)       ;; hann[i][1]*x[i][1]
    add eax, 4

    fld x[edx][0]
    fmulp  st(3), st       ;; hann[i][1]*x[j][0]

    fld x[edx][4]
    fmulp  st(4), st       ;; hann[i][1]*x[j][1]
    sub edx, 8

    fstp    y[ebx][4]
    fstp    y[ebx][0]
    xor ebx, (127 shl 3)    ;; table scaled by 8
    fstp    y[ebx][0]
    fstp    y[ebx][4]


    dec ecx
    jne a100

;;----------------------------
    mov esi, yout
    mov ecx, 64
    call R2Stage1

    mov esi, yout
    mov ecx, 16
    call R4Stage1

    mov esi, yout
    mov ecx, 4
    call R4Stage2

    mov esi, yout
    call R4StageLast128

;;-----------
;; convert to real FFT
;;  conversion has factor of 2 scaling over actual

a0 textequ <dword ptr [esi]>  
a  textequ <dword ptr [esi+ebx]>  
b  textequ <dword ptr [edi]>

x100:
    mov esi, yout
    mov ebx, 8
    lea edi, [esi+8*127]

;; special case z[0]
    fld     a0[0]
    fadd    a0[4]
    fadd    st, st(0)
    fstp    a0[0]
    mov     a0[4], 0

    mov ecx, 63

c100:
    fld a[0]
    fld b[0]
    fsubr    st, st(1)  ;; txb

    fld a[4]
    fld b[4]
    fadd     st, st(1)  ;; tya ay txb ax

    fld     wr[ebx*4][0]
    fmul    st, st(3)   ;; wc*txb tya ay txb ax

    fld     wr[ebx*4][4]
    fmul    st, st(2)   ;; ws*tya wc*txb tya ay txb ax

    fld b[0]
    faddp   st(6), st   ;; ws*tya wc*txb tya ay txb txa

    faddp   st(1), st   ;; qx tya ay txb txa

    fld     wr[ebx*4][4]
    fmulp   st(4), st   ;; qx tya ay ws*txb txa

    fld     wr[ebx*4][0]
    fmulp   st(2), st   ;; qx wc*tya ay ws*txb txa

    fld b[4]
    fsubp   st(3), st   ;; qx wc*tya tyb ws*txb txa
    
    fxch                ;; wc*tya qx tyb ws*txb txa
    fsubrp  st(3), st   ;; qx tyb qy txa

    ;;ax = txa+qx;
    ;;ay = tyb+qy;
    ;;bx = txa-qx;
    ;;by = qy-tyb;


    fld st(0)
    fadd    st, st(4)   ;; ax qx tyb qy txa

    fld st(3)
    fadd    st, st(3)   ;; ay ax qx tyb qy txa

    fxch    st(2)       ;; qx ax ay tyb qy txa
    fsubp   st(5), st   ;; ax ay tyb qy bx

    fxch    st(2)       ;; tyb ay ax qy bx
    fsubp   st(3), st   ;; ay ax by bx


    fstp    a[4]
    fstp    a[0]

    add ebx, 8
    
    fstp    b[4]
    fstp    b[0]

    sub edi, 8
    dec ecx
    jne c100

;;-----
;; special case z[64]
    mov edx, a[4]
    mov eax, a[0]

    add edx, (1 shl 23)     ;; x = 2.0*x
    add eax, (1 shl 23)

    xor edx, (1 shl 31)     ;; x = -x

    mov a[4], edx
    mov a[0], eax

;;
;;
;;
;;
exit:
    ret
bFFT256RealHann  endp
;;===================================================
if 0 
DFT1024RealHann01  PROC C PUBLIC \
		uses esi edi ebx ecx     \
        xin:dword,
        yout:dword
;;
;; computes only the first 6 frequency bins!
;;
;;----------------------------
    mov esi, xin
    mov edi, yout
    mov ecx, 4*255
    xor ebx, ebx

;; ecx counts down from 4*255
h0 textequ <hann[ebx]>
h1 textequ <hann[ebx][4*256]>
h2 textequ <hann[ecx][4*256]>
h3 textequ <hann[ecx]>
    
xin0 textequ <dword ptr [esi]>
xin1 textequ <dword ptr [esi][1*4*256]>
xin2 textequ <dword ptr [esi][2*4*256]>
xin3 textequ <dword ptr [esi][3*4*256]>
    
cx0  textequ <dword ptr [edi]>
cx1  textequ <dword ptr [edi][1*8]>
cx2  textequ <dword ptr [edi][2*8]>
cx3  textequ <dword ptr [edi][3*8]>
cx4  textequ <dword ptr [edi][4*8]>
cx5  textequ <dword ptr [edi][5*8]>

cy0  textequ <dword ptr [edi][4]>
cy1  textequ <dword ptr [edi][1*8][4]>
cy2  textequ <dword ptr [edi][2*8][4]>
cy3  textequ <dword ptr [edi][3*8][4]>
cy4  textequ <dword ptr [edi][4*8][4]>
cy5  textequ <dword ptr [edi][5*8][4]>

        mov cx0, 0
        mov cx1, 0
        mov cx2, 0
        mov cx3, 0
        mov cx4, 0
        mov cx5, 0

        mov cy0, 0
        mov cy1, 0
        mov cy2, 0
        mov cy3, 0
        mov cy4, 0
        mov cy5, 0

a100:
    fld xin0
    fld h0
    fmulp st(1), st

    fld xin2
    fld h2
    fmulp st(1), st

    fld xin1
    fld h1
    fmulp st(1), st

    fld xin3
    fld h3
    fmulp st(1), st         ;;  f3 f1 f2 f0

    fld st(3)               ;;  f0 f3 f1 f2 f0
    fxch st(3)              ;;  f2 f3 f1 f0 f0
    fadd    st(4), st
    fsubp   st(3), st       ;;  f3 f1 x1 g0

    fld st(1)
    fxch                    ;;  f3 f1 f1 x1 g0
    fadd    st(2), st
    fsubp   st(1), st       ;;  y g1 x1 g0

    fld st(3)
    fxch    st(2)           ;; g1 y g0 x1 g0
    fadd    st(4), st       ;;  
    fsubp   st(2), st       ;;  y x2 x1 x0

    fxch    st(3)           ;;  x0 x2 x1 y
;;c0
    fld cx0
    fadd    st, st(1)
    fstp    cx0
;;c4
    fld tc4[ebx]
    fmul    st, st(1)
    fld cx4
    faddp    st(1), st
    fstp    cx4

    fld ts4[ebx]        
    fmulp   st(1), st       ;; done with x0
    fld cy4
    faddp   st(1), st
    fstp    cy4             ;; x2 x1 y

;; c2
    fld tc2[ebx]
    fmul    st, st(1)
    fld cx2
    faddp    st(1), st
    fstp    cx2

    fld ts2[ebx]        
    fmulp   st(1), st       ;; done with x2
    fld cy2
    faddp   st(1), st
    fstp    cy2             ;; x1 y
;; c1 ------
    fld st(1)
    fld st(1)           ;; x y x y

    fld tc1[ebx]
    fmul    st(1), st   ;; wc wc*x   y x y
    fmul    st, st(4)   ;; wc*y wc*x y x y

    fld ts1[ebx]        ;; ws wc*y wc*x    y x y
    fmul    st(3), st   ;; ws wc*y wc*x ws*y x y
    fmul    st, st(4)   ;; ws*x wc*y wc*x ws*y x y
    
    fld cx1
    faddp   st(3), st

    fld cy1
    faddp   st(1), st

    fxch    st(2)       ;; wc*x wc*y ws*x ws*y x y
    faddp   st(3), st   ;;      wc*y ws*x cx1 x y

    fsubp   st(1), st   ;; cy1 cx1 x y    
    fstp    cy1
    fstp    cx1
;; c3 ------------
    fld st(1)
    fld st(1)           ;; x y x y

    fld tc3[ebx]
    fmul    st(1), st   ;; wc wc*x   y x y
    fmul    st, st(4)   ;; wc*y wc*x y x y

    fld ts3[ebx]        ;; ws wc*y wc*x    y x y
    fmul    st(3), st   ;; ws wc*y wc*x ws*y x y
    fmul    st, st(4)   ;; ws*x wc*y wc*x ws*y x y
    
    fld cx3
    faddp   st(3), st

    fld cy3
    faddp   st(1), st

    fxch    st(2)       ;; wc*x wc*y ws*x ws*y x y
    fsubrp  st(3), st   ;;      wc*y ws*x cx1 x y

    faddp   st(1), st   ;; cy3 cx3 x y    
    fstp    cy3
    fstp    cx3
;; c5 ------------
    fld st(1)           ;; y x y

    fld tc5[ebx]
    fmul    st(1), st   ;; wc wc*y x y
    fmul    st, st(2)   ;; wc*x wc*y x y

    fld ts5[ebx]        ;; ws wc*x wc*y x y
    fmul    st(4), st   ;; ws wc*x wc*y x ws*y
    fmulp   st(3), st   ;; wc*x wc*y ws*x ws*y
    
    fld cx5             
    faddp   st(4), st

    fld cy5
    faddp   st(3), st

    faddp   st(3), st    ;; wc*y ws*x cx5

    fsubp   st(1), st    ;; cy5 cx5
    fstp    cy5
    fstp    cx5


    add ebx, 4
    add esi, 4
    sub ecx, 4
    jge a100        ;; do cx = 0
    

    fld cx0
    fadd    st, st(0)
    fstp    cx0

;;
;;
    ret
DFT1024RealHann01  endp
endif
;====================================
;;===================================================
DFT1024RealHann  PROC C PUBLIC \
		uses esi edi ebx ecx     \
        xin:dword,
        yout:dword
;;
;; computes only the first 6 frequency bins!
;;
;;----------------------------
    mov esi, xin
    mov edi, yout
    mov ecx, 4*255
    xor ebx, ebx

;; ecx counts down from 4*255
h0 textequ <hann[ebx]>
h1 textequ <hann[ebx][4*256]>
h2 textequ <hann[ecx][4*256]>
h3 textequ <hann[ecx]>
    
xin0 textequ <dword ptr [esi]>
xin1 textequ <dword ptr [esi][1*4*256]>
xin2 textequ <dword ptr [esi][2*4*256]>
xin3 textequ <dword ptr [esi][3*4*256]>
    
cx0  textequ <dword ptr [edi]>
cx1  textequ <dword ptr [edi][1*8]>
cx2  textequ <dword ptr [edi][2*8]>
cx3  textequ <dword ptr [edi][3*8]>
cx4  textequ <dword ptr [edi][4*8]>
cx5  textequ <dword ptr [edi][5*8]>

cy0  textequ <dword ptr [edi][4]>
cy1  textequ <dword ptr [edi][1*8][4]>
cy2  textequ <dword ptr [edi][2*8][4]>
cy3  textequ <dword ptr [edi][3*8][4]>
cy4  textequ <dword ptr [edi][4*8][4]>
cy5  textequ <dword ptr [edi][5*8][4]>

wc1 textequ <tc1[ebx]>
wc2 textequ <tc2[ebx]>
wc3 textequ <tc3[ebx]>
wc4 textequ <tc4[ebx]>
wc5 textequ <tc5[ebx]>

;; sin coefs obtained from cosine coefs by reflections and offsets
;;ws1 textequ <ts1[ebx]>
ws1 textequ <tc1[ecx][4]>
;;ws2 textequ <ts2[ebx]>
ws2 textequ <tc2[ebx][4*128]>
;;ws3 textequ <ts3[ebx]>
ws3 textequ <tc3[ecx][4]>
;;ws4 textequ <ts4[ebx]>
ws4 textequ <tc4[ebx][4*64]>
;;ws5 textequ <ts5[ebx]>
ws5 textequ <tc5[ecx][4]>


        mov cx0, 0
        mov cx1, 0
        mov cx2, 0
        mov cx3, 0
        mov cx4, 0
        mov cx5, 0

        mov cy0, 0
        mov cy1, 0
        mov cy2, 0
        mov cy3, 0
        mov cy4, 0
        mov cy5, 0

a100:
    fld xin0
    fld h0
    fmulp st(1), st

    fld xin2
    fld h2
    fmulp st(1), st

    fld xin1
    fld h1
    fmulp st(1), st

    fld xin3
    fld h3
    fmulp st(1), st         ;;  f3 f1 f2 f0

    fld st(3)               ;;  f0 f3 f1 f2 f0
    fxch st(3)              ;;  f2 f3 f1 f0 f0
    fadd    st(4), st
    fsubp   st(3), st       ;;  f3 f1 x1 g0

    fld st(1)
    fxch                    ;;  f3 f1 f1 x1 g0
    fadd    st(2), st
    fsubp   st(1), st       ;;  y g1 x1 g0

    fld st(3)
    fxch    st(2)           ;; g1 y g0 x1 g0
    fadd    st(4), st       ;;  
    fsubp   st(2), st       ;;  y x2 x1 x0

    fxch    st(3)           ;;  x0 x2 x1 y
;;c0
    fld cx0
    fadd    st, st(1)
    fstp    cx0
;;c4
    fld wc4
    fmul    st, st(1)
    fld cx4
    faddp    st(1), st
    fstp    cx4

    fld ws4        
    fmulp   st(1), st       ;; done with x0
    fld cy4
    faddp   st(1), st
    fstp    cy4             ;; x2 x1 y

;; c2
    fld wc2
    fmul    st, st(1)
    fld cx2
    faddp    st(1), st
    fstp    cx2

    fld ws2        
    fmulp   st(1), st       ;; done with x2
    fld cy2
    faddp   st(1), st
    fstp    cy2             ;; x1 y
;; c1 ------
    fld st(1)
    fld st(1)           ;; x y x y

    fld wc1
    fmul    st(1), st   ;; wc wc*x   y x y
    fmul    st, st(4)   ;; wc*y wc*x y x y

    fld ws1        ;; ws wc*y wc*x    y x y
    fmul    st(3), st   ;; ws wc*y wc*x ws*y x y
    fmul    st, st(4)   ;; ws*x wc*y wc*x ws*y x y
    
    fld cx1
    faddp   st(3), st

    fld cy1
    faddp   st(1), st

    fxch    st(2)       ;; wc*x wc*y ws*x ws*y x y
    fsubrp  st(3), st   ;;      wc*y ws*x cx1 x y

    faddp   st(1), st   ;; -cy1 cx1 x y    
    fstp    cy1
    fstp    cx1
;; c3 ------------
    fld st(1)
    fld st(1)           ;; x y x y

    fld wc3
    fmul    st(1), st   ;; wc wc*x   y x y
    fmul    st, st(4)   ;; wc*y wc*x y x y

    fld ws3        ;; ws wc*y wc*x    y x y
    fmul    st(3), st   ;; ws wc*y wc*x ws*y x y
    fmul    st, st(4)   ;; ws*x wc*y wc*x ws*y x y
    
    fld cx3
    faddp   st(3), st

    fld cy3
    faddp   st(1), st

    fxch    st(2)       ;; wc*x wc*y ws*x ws*y x y
    fsubrp  st(3), st   ;;      wc*y ws*x cx1 x y

    faddp   st(1), st   ;; cy3 cx3 x y    
    fstp    cy3
    fstp    cx3
;; c5 ------------
    fld st(1)           ;; y x y

    fld wc5
    fmul    st(1), st   ;; wc wc*y x y
    fmul    st, st(2)   ;; wc*x wc*y x y

    fld ws5             ;; ws wc*x wc*y x y
    fmul    st(3), st   ;; ws wc*x wc*y ws*x y
    fmulp   st(4), st   ;; wc*x wc*y ws*x ws*y
    
    fld cx5             
    faddp   st(1), st

    fld cy5
    faddp   st(3), st

    fsubrp  st(3), st   ;; wc*y ws*x cx5

    faddp   st(1), st    ;; cy5 cx5
    fstp    cy5
    fstp    cx5

    add ebx, 4
    add esi, 4
    sub ecx, 4
    jge a100        ;; do cx = 0
    
;; adjust dc
    fld cx0
    fadd    st, st(0)
    fstp    cx0
;; adjust signs
    mov eax, cy1
    xor eax, 80000000h
    mov cy1, eax
    mov eax, cy5
    xor eax, 80000000h
    mov cy5, eax

;;
;;
    ret
DFT1024RealHann  endp
;====================================
;;===================================================
DFT1024RealHannApprox  PROC C PUBLIC \
		uses esi edi ebx ecx     \
        xin:dword,
        yout:dword
;;
;; computes only the first 6 frequency bins!
;; approx using 512 point DFT
;; on random data average phase error is about 0.02 radians
;;----------------------------
    mov esi, xin
    mov edi, yout
    mov ecx, 4*127
    xor ebx, ebx

;; ecx counts down from 4*127
h0 textequ <hann512[ebx]>
h1 textequ <hann512[ebx][4*128]>
h2 textequ <hann512[ecx][4*128]>
h3 textequ <hann512[ecx]>
    
xin0 textequ <dword ptr [esi]>
xin1 textequ <dword ptr [esi][1*4*256]>
xin2 textequ <dword ptr [esi][2*4*256]>
xin3 textequ <dword ptr [esi][3*4*256]>
    
cx0  textequ <dword ptr [edi]>
cx1  textequ <dword ptr [edi][1*8]>
cx2  textequ <dword ptr [edi][2*8]>
cx3  textequ <dword ptr [edi][3*8]>
cx4  textequ <dword ptr [edi][4*8]>
cx5  textequ <dword ptr [edi][5*8]>

cy0  textequ <dword ptr [edi][4]>
cy1  textequ <dword ptr [edi][1*8][4]>
cy2  textequ <dword ptr [edi][2*8][4]>
cy3  textequ <dword ptr [edi][3*8][4]>
cy4  textequ <dword ptr [edi][4*8][4]>
cy5  textequ <dword ptr [edi][5*8][4]>

wc1 textequ <atc1[ebx]>
wc2 textequ <atc2[ebx]>
wc3 textequ <atc3[ebx]>
wc4 textequ <atc4[ebx]>
wc5 textequ <atc5[ebx]>

;; sin coefs obtained from cosine coefs by reflections and offsets
ws1 textequ <atc1[ecx][4]>
ws2 textequ <atc2[ebx][4*64]>
ws3 textequ <atc3[ecx][4]>
ws4 textequ <atc4[ebx][4*32]>
ws5 textequ <atc5[ecx][4]>


        mov cx0, 0
        mov cx1, 0
        mov cx2, 0
        mov cx3, 0
        mov cx4, 0
        mov cx5, 0

        mov cy0, 0
        mov cy1, 0
        mov cy2, 0
        mov cy3, 0
        mov cy4, 0
        mov cy5, 0

a100:
    fld xin0
    fld xin0[4]
    faddp   st(1), st
    fld h0
    fmulp st(1), st

    fld xin2
    fld xin2[4]
    faddp   st(1), st
    fld h2
    fmulp st(1), st

    fld xin1
    fld xin1[4]
    faddp   st(1), st
    fld h1
    fmulp st(1), st

    fld xin3
    fld xin3[4]
    faddp   st(1), st
    fld h3
    fmulp st(1), st         ;;  f3 f1 f2 f0

    fld st(3)               ;;  f0 f3 f1 f2 f0
    fxch st(3)              ;;  f2 f3 f1 f0 f0
    fadd    st(4), st
    fsubp   st(3), st       ;;  f3 f1 x1 g0

    fld st(1)
    fxch                    ;;  f3 f1 f1 x1 g0
    fadd    st(2), st
    fsubp   st(1), st       ;;  y g1 x1 g0

    fld st(3)
    fxch    st(2)           ;; g1 y g0 x1 g0
    fadd    st(4), st       ;;  
    fsubp   st(2), st       ;;  y x2 x1 x0

    fxch    st(3)           ;;  x0 x2 x1 y
;;c0
    fld cx0
    fadd    st, st(1)
    fstp    cx0
;;c4
    fld wc4
    fmul    st, st(1)
    fld cx4
    faddp    st(1), st
    fstp    cx4

    fld ws4        
    fmulp   st(1), st       ;; done with x0
    fld cy4
    faddp   st(1), st
    fstp    cy4             ;; x2 x1 y

;; c2
    fld wc2
    fmul    st, st(1)
    fld cx2
    faddp    st(1), st
    fstp    cx2

    fld ws2        
    fmulp   st(1), st       ;; done with x2
    fld cy2
    faddp   st(1), st
    fstp    cy2             ;; x1 y
;; c1 ------
    fld st(1)
    fld st(1)           ;; x y x y

    fld wc1
    fmul    st(1), st   ;; wc wc*x   y x y
    fmul    st, st(4)   ;; wc*y wc*x y x y

    fld ws1        ;; ws wc*y wc*x    y x y
    fmul    st(3), st   ;; ws wc*y wc*x ws*y x y
    fmul    st, st(4)   ;; ws*x wc*y wc*x ws*y x y
    
    fld cx1
    faddp   st(3), st

    fld cy1
    faddp   st(1), st

    fxch    st(2)       ;; wc*x wc*y ws*x ws*y x y
    fsubrp  st(3), st   ;;      wc*y ws*x cx1 x y

    faddp   st(1), st   ;; -cy1 cx1 x y    
    fstp    cy1
    fstp    cx1
;; c3 ------------
    fld st(1)
    fld st(1)           ;; x y x y

    fld wc3
    fmul    st(1), st   ;; wc wc*x   y x y
    fmul    st, st(4)   ;; wc*y wc*x y x y

    fld ws3        ;; ws wc*y wc*x    y x y
    fmul    st(3), st   ;; ws wc*y wc*x ws*y x y
    fmul    st, st(4)   ;; ws*x wc*y wc*x ws*y x y
    
    fld cx3
    faddp   st(3), st

    fld cy3
    faddp   st(1), st

    fxch    st(2)       ;; wc*x wc*y ws*x ws*y x y
    fsubrp  st(3), st   ;;      wc*y ws*x cx1 x y

    faddp   st(1), st   ;; cy3 cx3 x y    
    fstp    cy3
    fstp    cx3
;; c5 ------------
    fld st(1)           ;; y x y

    fld wc5
    fmul    st(1), st   ;; wc wc*y x y
    fmul    st, st(2)   ;; wc*x wc*y x y

    fld ws5             ;; ws wc*x wc*y x y
    fmul    st(3), st   ;; ws wc*x wc*y ws*x y
    fmulp   st(4), st   ;; wc*x wc*y ws*x ws*y
    
    fld cx5             
    faddp   st(1), st

    fld cy5
    faddp   st(3), st

    fsubrp  st(3), st   ;; wc*y ws*x cx5

    faddp   st(1), st    ;; cy5 cx5
    fstp    cy5
    fstp    cx5

    add ebx, 4
    add esi, 2*4
    sub ecx, 4
    jge a100        ;; do cx = 0
    
;; adjust dc
    fld cx0
    fadd    st, st(0)
    fstp    cx0
;; adjust signs
    mov eax, cy1
    xor eax, 80000000h
    mov cy1, eax
    mov eax, cy5
    xor eax, 80000000h
    mov cy5, eax

;;
;;
    ret
DFT1024RealHannApprox  endp
;====================================
;;===================================================
;;===================================================
;;========= 256 complex =============================
;;===================================================
;;---------------------------------------------------
R4_256Stage1 proc near
;;
;;  first stage 256 point complex FFT
;;  with bit reversed input
;;
;; input output = esi
;; uses esi edi ecx
;;
;; caller sets loop count in ecx
;;
;; middle reads exchanged by text equates
;;


wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>


    mov edi, esi
    xor ebx, ebx

a100:
;;----------------------
;; 
    x0 textequ <dword ptr [esi][0*8]>
    x1 textequ <dword ptr [esi][2*8]>
    x2 textequ <dword ptr [esi][1*8]>
    x3 textequ <dword ptr [esi][3*8]>

    y0 textequ <dword ptr [esi][0*8][4]>
    y1 textequ <dword ptr [esi][2*8][4]>
    y2 textequ <dword ptr [esi][1*8][4]>
    y3 textequ <dword ptr [esi][3*8][4]>

    xx0 textequ <dword ptr [edi][0*8]>
    xx1 textequ <dword ptr [edi][1*8]>
    xx2 textequ <dword ptr [edi][2*8]>
    xx3 textequ <dword ptr [edi][3*8]>

    yy0 textequ <dword ptr [edi][0*8][4]>
    yy1 textequ <dword ptr [edi][1*8][4]>
    yy2 textequ <dword ptr [edi][2*8][4]>
    yy3 textequ <dword ptr [edi][3*8][4]>

    R4_bfly  <add esi, 32>

;;-----------------------
;;add esi, 32     ;; inc by 4 complex values
add edi, 32     ;; inc by 4 complex values
add ebx, 24     ;; inc coefs

    dec ecx
    jne a100



    ret
R4_256Stage1 endp
;;---------------------------------------------------
R4_256Stage2 proc near
;;
;;
;;  256 point complex FFT
;;
;; input output = esi
;; uses esi edi ecx edx
;;
;; caller sets outer loop count in ecx
;;
;; middle reads exchanged by text equates
;;
wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>

    mov edi, esi
    xor ebx, ebx

a200:
    mov edx, 4
a100:
;;----------------------
    x0 textequ <dword ptr [esi][0*32]>
    x1 textequ <dword ptr [esi][2*32]>
    x2 textequ <dword ptr [esi][1*32]>
    x3 textequ <dword ptr [esi][3*32]>

    y0 textequ <dword ptr [esi][0*32][4]>
    y1 textequ <dword ptr [esi][2*32][4]>
    y2 textequ <dword ptr [esi][1*32][4]>
    y3 textequ <dword ptr [esi][3*32][4]>

    xx0 textequ <dword ptr [edi][0*32]>
    xx1 textequ <dword ptr [edi][1*32]>
    xx2 textequ <dword ptr [edi][2*32]>
    xx3 textequ <dword ptr [edi][3*32]>

    yy0 textequ <dword ptr [edi][0*32][4]>
    yy1 textequ <dword ptr [edi][1*32][4]>
    yy2 textequ <dword ptr [edi][2*32][4]>
    yy3 textequ <dword ptr [edi][3*32][4]>

    R4_bfly  <add esi, 8>

;;-----------------------
;;add esi, 8     ;; inc by 1 complex value
add edi, 8     ;; inc by 1 complex value

    dec edx
    jne a100

add esi, 8*((4*4-4))
add edi, 8*((4*4-4))

add ebx, 24

    dec ecx
    jne a200



    ret
R4_256Stage2 endp
;;---------------------------------------------------
R4_256Stage3 proc near
;;
;;  256 point complex FFT, bit reversed input
;;
;; input output = esi
;; uses esi edi ecx edx
;;
;; middle reads exchanged by text equates
;;
wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>

    mov edi, esi

;;------- special case coefs
    mov edx, 16
a100:
;;----------------------
;;
    x0 textequ <dword ptr [esi][0*128]>
    x1 textequ <dword ptr [esi][2*128]>
    x2 textequ <dword ptr [esi][1*128]>
    x3 textequ <dword ptr [esi][3*128]>

    y0 textequ <dword ptr [esi][0*128][4]>
    y1 textequ <dword ptr [esi][2*128][4]>
    y2 textequ <dword ptr [esi][1*128][4]>
    y3 textequ <dword ptr [esi][3*128][4]>

    xx0 textequ <dword ptr [edi][0*128]>
    xx1 textequ <dword ptr [edi][1*128]>
    xx2 textequ <dword ptr [edi][2*128]>
    xx3 textequ <dword ptr [edi][3*128]>

    yy0 textequ <dword ptr [edi][0*128][4]>
    yy1 textequ <dword ptr [edi][1*128][4]>
    yy2 textequ <dword ptr [edi][2*128][4]>
    yy3 textequ <dword ptr [edi][3*128][4]>

    R4_bfly0  <add esi, 8>

;;-----------------------
;;add esi, 8     ;; inc by 1 complex value
add edi, 8     ;; inc by 1 complex value
    dec edx
    jne a100

add esi, 8*((4*16-16))
add edi, 8*((4*16-16))
mov ebx, 24
;;==========================
;;==========================
    mov ecx, 3
a300:
    mov edx, 16
a200:
;;----------------------
;;
    x0 textequ <dword ptr [esi][0*128]>
    x1 textequ <dword ptr [esi][2*128]>
    x2 textequ <dword ptr [esi][1*128]>
    x3 textequ <dword ptr [esi][3*128]>

    y0 textequ <dword ptr [esi][0*128][4]>
    y1 textequ <dword ptr [esi][2*128][4]>
    y2 textequ <dword ptr [esi][1*128][4]>
    y3 textequ <dword ptr [esi][3*128][4]>

    xx0 textequ <dword ptr [edi][0*128]>
    xx1 textequ <dword ptr [edi][1*128]>
    xx2 textequ <dword ptr [edi][2*128]>
    xx3 textequ <dword ptr [edi][3*128]>

    yy0 textequ <dword ptr [edi][0*128][4]>
    yy1 textequ <dword ptr [edi][1*128][4]>
    yy2 textequ <dword ptr [edi][2*128][4]>
    yy3 textequ <dword ptr [edi][3*128][4]>

    R4_bfly  <add esi, 8>

;;-----------------------
;;add esi, 8     ;; inc by 1 complex value
add edi, 8     ;; inc by 1 complex value

    dec edx
    jne a200

add esi, 8*((4*16-16))
add edi, 8*((4*16-16))

add ebx, 24

    dec ecx
    jne a300



    ret
R4_256Stage3 endp
;;---------------------------------------------------
;;---------------------------------------------------
R4StageLast256 proc near
;;
;;  Last stage 256 point complex FFT
;;
;;
;; input output = esi
;; uses esi edi ecx
;;
;;
;; middle reads exchanged by text equates
;;
    mov edi, esi
    mov ecx, 64

a100:
;;
    x0 textequ <dword ptr [esi][0*512]>
    x1 textequ <dword ptr [esi][2*512]>
    x2 textequ <dword ptr [esi][1*512]>
    x3 textequ <dword ptr [esi][3*512]>

    y0 textequ <dword ptr [esi][0*512][4]>
    y1 textequ <dword ptr [esi][2*512][4]>
    y2 textequ <dword ptr [esi][1*512][4]>
    y3 textequ <dword ptr [esi][3*512][4]>

    xx0 textequ <dword ptr [edi][0*512]>
    xx1 textequ <dword ptr [edi][1*512]>
    xx2 textequ <dword ptr [edi][2*512]>
    xx3 textequ <dword ptr [edi][3*512]>

    yy0 textequ <dword ptr [edi][0*512][4]>
    yy1 textequ <dword ptr [edi][1*512][4]>
    yy2 textequ <dword ptr [edi][2*512][4]>
    yy3 textequ <dword ptr [edi][3*512][4]>

    R4_bfly0

;;-----------------------
add esi, 8     ;; inc by 1 complex value
add edi, 8     ;; inc by 1 complex value

    dec ecx
    jne a100

    ret
R4StageLast256 endp
;;---------------------------------------------------
;;===================================================
bFFT1024RealHannApprox  PROC C PUBLIC \
		uses esi edi ebx ecx     \
        xin:dword,
        yout:dword
;;
;;
;;----------------------------
;; subsample, apply window and bit reverse 
    mov esi, xin
    mov edi, yout
x textequ <dword ptr [esi]>
y textequ <dword ptr [edi]>
x0 textequ <dword ptr [esi]>
x1 textequ <dword ptr [esi][4]>
x2 textequ <dword ptr [esi][2*4]>
x3 textequ <dword ptr [esi][3*4]>
x4 textequ <dword ptr [esi][4*4]>

    xor eax, eax
    mov edx, 4*4*255
    mov ecx, 128
    fld con25
    fld con50
a100:
    mov ebx, bitrev[eax]
    
    fld x0[eax*2]
    fld x4[eax*2]
    fadd
    fmul    st, st(2)
    fld x1[eax*2]
    fld x3[eax*2]
    fadd
    fmul    st, st(2)
    fld x2[eax*2]
    faddp st(2), st
    faddp st(1), st     ;; x[i][0]
    ;;
    fld x0[eax*2][2*4]
    fld x4[eax*2][2*4]
    fadd
    fmul    st, st(3)
    fld x1[eax*2][2*4]
    fld x3[eax*2][2*4]
    fadd
    fmul    st, st(3)
    fld x2[eax*2][2*4]
    faddp st(2), st
    faddp st(1), st     ;; x[i][1]  x[i][0]
    ;;
    fld hann512[eax][0]
    fmulp st(2), st    
    fld hann512[eax][4]
    fmulp st(1), st     ;; fft[k][1] fft[k][0]

    fstp    y[ebx][4]
    fstp    y[ebx][0]
;;---------
    fld x0[edx]
    fld x4[edx]
    fadd
    fmul    st, st(2)
    fld x1[edx]
    fld x3[edx]
    fadd
    fmul    st, st(2)
    fld x2[edx]
    faddp st(2), st
    faddp st(1), st     ;; x[j][0]
    ;;
    fld x0[edx][2*4]
    fld x4[edx][2*4]
    fadd
    fmul    st, st(3)
    fld x1[edx][2*4]
    fld x3[edx][2*4]
    fadd
    fmul    st, st(3)
    fld x2[edx][2*4]
    faddp st(2), st
    faddp st(1), st     ;; x[j][1]  x[j][0]
    ;;
    fld hann512[eax][4]
    fmulp st(2), st    
    fld hann512[eax][0]
    fmulp st(1), st     ;; fft[k][1] fft[k][0]

    xor ebx, (255 shl 3)    ;; table scaled by 8
    fstp    y[ebx][4]
    fstp    y[ebx][0]
;;---------

    add eax,  8
    sub edx, 16

    dec ecx
    jne a100

    fstp st(0)      ;; pop cons off stack
    fstp st(0)

;;----------------------------
    mov esi, yout
    mov ecx, 64
    call R4_256Stage1

    mov esi, yout
    mov ecx, 16
    call R4_256Stage2

    mov esi, yout
    call R4_256Stage3


    mov esi, yout
    call R4StageLast256

;;-----------------------------
;; convert to real FFT
;;  conversion has factor of 2 scaling over actual

a0 textequ <dword ptr [esi]>  
a  textequ <dword ptr [esi+ebx]>  
b  textequ <dword ptr [edi]>

x100:
    mov esi, yout
    mov ebx, 8
    lea edi, [esi+8*255]

;; special case z[0]
    fld     a0[0]
    fadd    a0[4]
    fadd    st, st(0)
    fstp    a0[0]
    mov     a0[4], 0

    mov ecx, 127

c100:
    fld a[0]
    fld b[0]
    fsubr    st, st(1)  ;; txb

    fld a[4]
    fld b[4]
    fadd     st, st(1)  ;; tya ay txb ax

    fld     wr[ebx*2][0]
    fmul    st, st(3)   ;; wc*txb tya ay txb ax

    fld     wr[ebx*2][4]
    fmul    st, st(2)   ;; ws*tya wc*txb tya ay txb ax

    fld b[0]
    faddp   st(6), st   ;; ws*tya wc*txb tya ay txb txa

    faddp   st(1), st   ;; qx tya ay txb txa

    fld     wr[ebx*2][4]
    fmulp   st(4), st   ;; qx tya ay ws*txb txa

    fld     wr[ebx*2][0]
    fmulp   st(2), st   ;; qx wc*tya ay ws*txb txa

    fld b[4]
    fsubp   st(3), st   ;; qx wc*tya tyb ws*txb txa
    
    fxch                ;; wc*tya qx tyb ws*txb txa
    fsubrp  st(3), st   ;; qx tyb qy txa

    ;;ax = txa+qx;
    ;;ay = tyb+qy;
    ;;bx = txa-qx;
    ;;by = qy-tyb;


    fld st(0)
    fadd    st, st(4)   ;; ax qx tyb qy txa

    fld st(3)
    fadd    st, st(3)   ;; ay ax qx tyb qy txa

    fxch    st(2)       ;; qx ax ay tyb qy txa
    fsubp   st(5), st   ;; ax ay tyb qy bx

    fxch    st(2)       ;; tyb ay ax qy bx
    fsubp   st(3), st   ;; ay ax by bx


    fstp    a[4]
    fstp    a[0]

    add ebx, 8
    
    fstp    b[4]
    fstp    b[0]

    sub edi, 8
    dec ecx
    jne c100

;;-----
;; special case z[128]
    mov edx, a[4]
    mov eax, a[0]

    add edx, (1 shl 23)     ;; x = 2.0*x
    add eax, (1 shl 23)

    xor edx, (1 shl 31)     ;; x = -x

    mov a[4], edx
    mov a[0], eax

;;
;;
;;
exit:
;;
    ret
bFFT1024RealHannApprox  endp
;====================================
;====================================
_TEXT ENDS
;====================================
          END
