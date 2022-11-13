; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: xfft.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
; xFFT.asm   
;
;  Katmai new instructions
;
;   256 pt and 1024 pt real input fft w/hann window
;   NO inplace
;
;  masm 6.14
;
; Pentium III  
;
;-------------------------------------------------
.686
.XMM
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT

movmem16  textequ <movaps>
;;movmem16  textequ <movups>


half1024output = 1


;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;
ALIGN 16
;


;;static int bitrev[512];
bitrev  dd 256 dup (0)      ;; half size table

align 16
hann    dd   256*2  dup (0)    ;; half window in pairs
align 16
hann256 dd    64*2  dup (0)    ;; half window in pairs

;; radix 2 twid factors
align 16
wcs     dd   256*2  dup (0)   ;;      // sized for 512 pt fft

;;// radix 4 twid factors
;;// wc1 ws1  wc2 ws2  wc3 ws3    // sized for 512 pt fft
align 16
wcs4    dd   64*3*2 dup (0)

;;// convert to real twid factors, 
;;// 1024 pt real input FFT
align 16
        dd   0, 0   ;; dummy to align wr[1]
wr      dd   256*2  dup (0)

;; 512 pt 
align 16
        dd   0, 0   ;; dummy to align wr[1]
wr512   dd   128*2  dup (0)

align 16
        dd   0, 0   ;; dummy to align wr[1]
wr256   dd   64*2  dup (0)



align 16
wsign_flip  label oword
    dd  000000000h
    dd  080000000h
    dd  000000000h
    dd  080000000h


;; FFT/DFT 512 approx to 1024 point
;;
align 16
xcon25 dd  0.25, 0.25, 0.25, 0.25 
xcon50 dd  0.50, 0.50, 0.50, 0.50 

align 16
hann512 dd   256  dup (0)    ;; half window  FFT/DFT 512 approx
con25   dd  0.25
con50   dd  0.50




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
xFFT_init  PROC C PUBLIC \
		uses esi edi ebx ecx 

    sub esp, 8      ;; two args
arg1 textequ <dword ptr [esp]>
arg2 textequ <dword ptr [esp+4]>

;; 512 pt dft/fft init
    mov arg1,    offset hann512
    call _b3FFT_init_Hann512
;;    mov arg1,    offset atc1
;;    call _DFT_init_wcs512
;;    mov arg1,    offset tc1
;;    call _DFT_init_wcs1024
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
    xor edi, edi
a100:
    mov eax, bitrev[esi]
    shl eax, 3
    mov bitrev[edi], eax
    add esi, 4
    add edi, 4
    dec ecx
    jne a100
;;-------
;; gen cvt to real wr256 from wr 
    xor esi, esi
    xor edi, edi
    mov ecx, 64
a200:
    mov eax, wr[esi]
    mov edx, wr[esi][4]
    mov wr256[edi], eax
    mov wr256[edi][4], edx
    add edi, 8
    add esi, 4*8
    dec ecx
    jne a200
;;-------
;; gen cvt to real wr512 from wr 
    xor esi, esi
    xor edi, edi
    mov ecx, 128
a300:
    mov eax, wr[esi]
    mov edx, wr[esi][4]
    mov wr512[edi], eax
    mov wr512[edi][4], edx
    add edi, 8
    add esi, 2*8
    dec ecx
    jne a300
;;-------

    add esp, 8

    ret
xFFT_init endp
;;===================================================
;;===================================================
;;===================================================
R4bfly  macro

;; *** user must load xmm5 with wsign_flip ***
;;
;; does two complex instances withh same coefs
;;

    movmem16  xmm0, x0
    movmem16  xmm4, x2
    movaps  xmm2, xmm0
    addps   xmm0, xmm4      ;; xmm0 = txa tya
    subps   xmm2, xmm4      ;; xmm2 = txc tyc

    movmem16  xmm1, x1
    movmem16  xmm4, x3
    movaps  xmm3, xmm1
    addps   xmm1, xmm4      ;; xmm1 = txb tyb
    subps   xmm3, xmm4      ;; xmm3 = txd tyd

    movaps  xmm4, xmm0
    addps   xmm0, xmm1      ;; txa+txb  tya+tyb
    movmem16  xx0, xmm0    

    subps   xmm4, xmm1      ;; xmm4 = txa-txb   tya-tyb = u v
                            ;; xmm0 xmm1 free
    movaps  xmm1, xmm4              ;; xmm1 = u v
    shufps  xmm4, xmm4, 10110001B   ;; xmm4 = v u


    ;; load coefs
    movss   xmm7, ws2
    shufps  xmm7, xmm7, 0   ;; dup 
    xorps   xmm7, xmm5
    movss   xmm6, wc2
    shufps  xmm6, xmm6, 0   ;; dup 
    ;; complex mult
    mulps   xmm4, xmm7
    mulps   xmm1, xmm6
    addps   xmm1, xmm4
    movmem16  xx2, xmm1    
;;
;;---								 ;; xmm2 = txc tyc
   	shufps	xmm3, xmm3,   10110001b  ;; xmm3 = tyd txd
    xorps   xmm3, xmm5         ;; xmm3 = tyd -txd
	movaps	xmm0, xmm2

	addps	xmm0, xmm3			;; xmm0 = u v
    movaps  xmm1, xmm0              ;; xmm1 = u v
    shufps  xmm0, xmm0, 10110001B   ;; xmm0 = v u

    ;; load coefs
    movss   xmm7, ws1
    shufps  xmm7, xmm7, 0   ;; dup 
    movss   xmm6, wc1
    shufps  xmm6, xmm6, 0   ;; dup 
    xorps   xmm7, xmm5
    ;; complex mult
    mulps   xmm0, xmm7
    mulps   xmm1, xmm6
    addps   xmm1, xmm0
    movmem16  xx1, xmm1    
;;---
   	subps   xmm2, xmm3			;; xmm2 = u v
    movaps  xmm1, xmm2              ;; xmm1 = u v
    shufps  xmm2, xmm2, 10110001B   ;; xmm2 = v u

    ;; load coefs
    movss   xmm6, wc3
    shufps  xmm6, xmm6, 0   ;; dup 
    movss   xmm7, ws3
    shufps  xmm7, xmm7, 0   ;; dup 
    xorps   xmm7, xmm5
    ;; complex mult
    mulps   xmm1, xmm6
    mulps   xmm2, xmm7
    addps   xmm1, xmm2
    movmem16  xx3, xmm1    


endm
;;===================================================
;;===================================================
R4bfly0  macro

;; *** user must load xmm5 with wsign_flip ***

    movmem16  xmm0, x0
    movmem16  xmm4, x2
    movaps  xmm2, xmm0
    addps   xmm0, xmm4      ;; xmm0 = txa tya
    subps   xmm2, xmm4      ;; xmm2 = txc tyc

    movmem16  xmm1, x1
    movmem16  xmm4, x3
    movaps  xmm3, xmm1
    addps   xmm1, xmm4      ;; xmm1 = txb tyb
    subps   xmm3, xmm4      ;; xmm3 = txd tyd

    movaps  xmm4, xmm0
    addps   xmm0, xmm1      ;; txa+txb  tya+tyb
    movmem16  xx0, xmm0    

    subps   xmm4, xmm1      ;; xmm4 = txa-txb   tya-tyb = u v
                            ;; xmm0 xmm1 free
    movmem16  xx2, xmm4

;;---								 ;; xmm2 = txc  tyc
	shufps	xmm3, xmm3,   10110001b  ;; xmm3 = tyd  txd
    xorps   xmm3, xmm5  ;; sign flip ;; xmm3 = tyd -txd

    movaps  xmm0, xmm2
    addps   xmm2, xmm3
    movmem16  xx1, xmm2
    subps   xmm0, xmm3
    movmem16  xx3, xmm0

endm
;;===================================================
CvtReal1024 proc near
;;
;;
;;  caller sets input = esi
;;  uses esi edi ebx ecx
;;  caller must load xmm5 with sign flip
;;
;; /* conversion has factor of 2 scaling over actual */
;;
;; *** mem access is unaligned even if buffer aligned!

    w  textequ <oword ptr wr[ebx]>
    xi textequ <oword ptr [esi+ebx]>
    xj textequ <oword ptr [edi]>

    lea edi, [esi+8*(512-2)]
    mov ecx, 127    ;; does two per iteration
    mov ebx, 8

;; special case
    movss   xmm0, dword ptr [esi]
    movss   xmm1, dword ptr [esi][4]
    addss   xmm0, xmm1
    addss   xmm0, xmm0
    movss   dword ptr [esi], xmm0
    mov     dword ptr [esi][4], 0
    
a100:
    movups  xmm1, xj
    shufps  xmm1, xmm1, 01001110b
    xorps   xmm1, xmm5  ;;  x -y 
    movups  xmm0, xi

    movaps  xmm2, xmm0
    subps   xmm0, xmm1      ;;  txb tya
    addps   xmm2, xmm1      ;;  txa tyb
    
    movaps  xmm3, xmm0              ;; txb tya
    shufps  xmm0, xmm0, 10110001b   ;; tya txb

    ;; load coefs
    movaps  xmm7, w
    movaps  xmm6, xmm7
    shufps  xmm7, xmm7,    11110101b    ;; w1
    shufps  xmm6, xmm6,    10100000b    ;; w0
    xorps   xmm7, xmm5  ;; w1 -w1
    
    mulps   xmm3, xmm6 
    mulps   xmm0, xmm7
    addps   xmm3, xmm0      ;; qx qy

if half1024output eq 0
        movaps  xmm1, xmm2      ;; txa tyb
endif
    addps   xmm2, xmm3      ;; xi
    movups  xi, xmm2

    ;; skip if second half freq not needed
if half1024output eq 0
        subps   xmm1, xmm3      ;; x -y
        xorps   xmm1, xmm5      ;;  xj
        shufps  xmm1, xmm1,  01001110b
        movups  xj, xmm1
endif

    add ebx, 16
    sub edi, 16
    dec ecx
    jne a100

;; special cases
;; i = 255
    movss   xmm0, xi
    movss   xmm4, xj[8]
    movss   xmm1, xmm0
    addss   xmm0, xmm4  ;; txa
    subss   xmm1, xmm4  ;; txb

    movss   xmm2, xi[4]
    movss   xmm4, xj[8][4]
    movss   xmm3, xmm2
    addss   xmm2, xmm4  ;; tya
    subss   xmm3, xmm4  ;; tyb

    movss   xmm6, w     ;; w0
    movss   xmm7, w[4]  ;; w1
    movss   xmm4, xmm1  ;; txb
    movss   xmm5, xmm2  ;; tya          ;; sign flip destroyed
    mulss   xmm1, xmm6  ;; txb*w0
    mulss   xmm2, xmm7  ;; tya*w1
    mulss   xmm5, xmm6  ;; tya*w0
    mulss   xmm4, xmm7  ;; txb*w1

    addss   xmm1, xmm2  ;; qx
    subss   xmm5, xmm4  ;; qy

    movss   xmm6, xmm0  ;; txa
    addss   xmm0, xmm1  ;; xi
    movss   xi, xmm0

    movss   xmm7, xmm3  ;; tyb
    addss   xmm3, xmm5  ;; yi
    movss   xi[4], xmm3

    subss   xmm6, xmm1  ;; xj
    movss   xj[8], xmm6
    subss   xmm5, xmm7  ;; yj
    movss   xj[8][4], xmm5
;;-----------
;; i= 256
if half1024output eq 0
    movlps  xmm1, wsign_flip
    movlps  xmm0, xi[8]
    xorps   xmm0, xmm1
    addps   xmm0, xmm0
    movlps  xi[8], xmm0
endif

    ret

CvtReal1024 endp
;;===================================================
CvtReal256 proc near
;;
;;
;;  caller sets input = esi
;;  uses esi edi ebx ecx
;;  caller must load xmm5 with sign flip
;;
;; /* conversion has factor of 2 scaling over actual */
;;
;; *** mem access is unaligned even if buffer aligned!

    w  textequ <oword ptr wr256[ebx]>
    xi textequ <oword ptr [esi+ebx]>
    xj textequ <oword ptr [edi]>

    lea edi, [esi+8*(128-2)]
    mov ecx, 31    ;; does two per iteration
    mov ebx, 8

;; special case
    movss   xmm0, dword ptr [esi]
    movss   xmm1, dword ptr [esi][4]
    addss   xmm0, xmm1
    addss   xmm0, xmm0
    movss   dword ptr [esi], xmm0
    mov     dword ptr [esi][4], 0
    
a100:
    movups  xmm1, xj
    shufps  xmm1, xmm1, 01001110b
    xorps   xmm1, xmm5  ;;  x -y 
    movups  xmm0, xi

    movaps  xmm2, xmm0
    subps   xmm0, xmm1      ;;  txb tya
    addps   xmm2, xmm1      ;;  txa tyb
    
    movaps  xmm3, xmm0              ;; txb tya
    shufps  xmm0, xmm0, 10110001b   ;; tya txb

    ;; load coefs
    movaps  xmm7, w
    movaps  xmm6, xmm7
    shufps  xmm7, xmm7,    11110101b    ;; w1
    shufps  xmm6, xmm6,    10100000b    ;; w0
    xorps   xmm7, xmm5  ;; w1 -w1
    
    mulps   xmm3, xmm6 
    mulps   xmm0, xmm7
    addps   xmm3, xmm0      ;; qx qy

    movaps  xmm1, xmm2      ;; txa tyb
    addps   xmm2, xmm3      ;; xi
    movups  xi, xmm2

        subps   xmm1, xmm3      ;; x -y
        xorps   xmm1, xmm5      ;;  xj
        shufps  xmm1, xmm1,  01001110b
        movups  xj, xmm1
    add ebx, 16
    sub edi, 16
    dec ecx
    jne a100

;; special cases
;; i = 63
    movss   xmm0, xi
    movss   xmm4, xj[8]
    movss   xmm1, xmm0
    addss   xmm0, xmm4  ;; txa
    subss   xmm1, xmm4  ;; txb

    movss   xmm2, xi[4]
    movss   xmm4, xj[8][4]
    movss   xmm3, xmm2
    addss   xmm2, xmm4  ;; tya
    subss   xmm3, xmm4  ;; tyb

    movss   xmm6, w     ;; w0
    movss   xmm7, w[4]  ;; w1
    movss   xmm4, xmm1  ;; txb
    movss   xmm5, xmm2  ;; tya          ;; sign flip destroyed
    mulss   xmm1, xmm6  ;; txb*w0
    mulss   xmm2, xmm7  ;; tya*w1
    mulss   xmm5, xmm6  ;; tya*w0
    mulss   xmm4, xmm7  ;; txb*w1

    addss   xmm1, xmm2  ;; qx
    subss   xmm5, xmm4  ;; qy

    movss   xmm6, xmm0  ;; txa
    addss   xmm0, xmm1  ;; xi
    movss   xi, xmm0

    movss   xmm7, xmm3  ;; tyb
    addss   xmm3, xmm5  ;; yi
    movss   xi[4], xmm3

    subss   xmm6, xmm1  ;; xj
    movss   xj[8], xmm6
    subss   xmm5, xmm7  ;; yj
    movss   xj[8][4], xmm5
;;-----------
;; i= 64
    movlps  xmm1, wsign_flip
    movlps  xmm0, xi[8]
    xorps   xmm0, xmm1
    addps   xmm0, xmm0
    movlps  xi[8], xmm0

    ret

CvtReal256 endp
;;===================================================
R2Stage1 proc near
;;
;; uses esi edi ecx ebx
;;
;; caller set input=esi, output=edi  inplace OK
;; caller sets loop count in ecx
;; caller must set    movaps   xmm5, wsign_flip
;;
;; does two r4 bflys per loop iteration
;; for use with R2 first stage with bit reversed input
;;
;;----------------------
    w   textequ <wcs[ebx]>
    ;; input
    x0 textequ <oword ptr [esi][0*8]>
    x1 textequ <oword ptr [esi][1*8]>
    x2 textequ <oword ptr [esi][2*8]>
    x3 textequ <oword ptr [esi][3*8]>
    ;; y component at x[4]
    ;; output
    xx0 textequ <oword ptr [edi][0*8]>
    xx1 textequ <oword ptr [edi][1*8]>
    xx2 textequ <oword ptr [edi][2*8]>
    xx3 textequ <oword ptr [edi][3*8]>
    ;; y component at x[4]
;;----------------------


    xor ebx, ebx
a100:

    movlps  xmm2, x0
    movlps  xmm1, x1
    movhps  xmm2, x2
    movhps  xmm1, x3

    movaps  xmm0, xmm2

    subps   xmm2, xmm1      ;; u v

        addps   xmm0, xmm1   ;; x0 y0 x2 y2

    movaps  xmm1, xmm2              ;; xmm1 = u v 
    shufps  xmm2, xmm2, 10110001B   ;; xmm2 = v u

    movaps  xmm7, w
    movaps  xmm6, xmm7
    shufps  xmm7, xmm7, 11110101b  ;; w1 w1 w3 w3  (big end notation)
    shufps  xmm6, xmm6, 10100000b  ;; w0 w0 w2 w2
    xorps   xmm7, xmm5             ;; w1 -w1 w3 -w3

    mulps   xmm1, xmm6
    mulps   xmm2, xmm7
    addps   xmm1, xmm2

    movlps  xx0, xmm0
    movhps  xx2, xmm0

    movlps  xx1, xmm1
    movhps  xx3, xmm1

    add esi, 8*4
    add edi, 8*4
    add ebx, 4*4


    dec ecx
    jne a100



    ret
R2Stage1 endp
;;=========================================================
;;===================================================
R4Stage1 proc near
;;
;; uses esi edi ecx ebx
;;
;; caller set input=esi, output=edi  inplace OK
;; caller sets loop count in ecx
;; caller must set    movaps   xmm5, wsign_flip
;;
;; middle reads exchanged by text equates
;;
;;----------------------
    wc1 textequ <wcs4[ebx][8*0][0]>
    ws1 textequ <wcs4[ebx][8*0][4]>
    wc2 textequ <wcs4[ebx][8*1][0]>
    ws2 textequ <wcs4[ebx][8*1][4]>
    wc3 textequ <wcs4[ebx][8*2][0]>
    ws3 textequ <wcs4[ebx][8*2][4]>

;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][2*0*8]>
    x1 textequ <oword ptr [esi][2*2*8]>
    x2 textequ <oword ptr [esi][2*1*8]>
    x3 textequ <oword ptr [esi][2*3*8]>
    ;; y component at x[4]
    ;; output
    xx0 textequ <oword ptr [edi][2*0*8]>
    xx1 textequ <oword ptr [edi][2*1*8]>
    xx2 textequ <oword ptr [edi][2*2*8]>
    xx3 textequ <oword ptr [edi][2*3*8]>
    ;; y component at x[4]
;;----------------------


    xor ebx, ebx

a100:
    R4bfly
    add esi, 64     ;; inc by 8 complex values
    add edi, 64     ;; inc by 8 complex values
    add ebx, 24

    dec ecx
    jne a100



    ret
R4Stage1 endp
;;=========================================================
R4Stage2 proc near
;;
;; input = esi   output = edi
;; uses esi edi ebx ecx edx
;;
;; caller sets outer loop count in ecx
;; caller must set    movaps   xmm5, wsign_flip
;;
;; middle reads exchanged by text equates
;;
    wc1 textequ <wcs4[ebx][8*0][0]>
    ws1 textequ <wcs4[ebx][8*0][4]>
    wc2 textequ <wcs4[ebx][8*1][0]>
    ws2 textequ <wcs4[ebx][8*1][4]>
    wc3 textequ <wcs4[ebx][8*2][0]>
    ws3 textequ <wcs4[ebx][8*2][4]>

    xor ebx, ebx
a200:
    mov edx, 4
a100:
;;----------------------
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][2*0*32]>
    x1 textequ <oword ptr [esi][2*2*32]>
    x2 textequ <oword ptr [esi][2*1*32]>
    x3 textequ <oword ptr [esi][2*3*32]>
    ;; y component at x[4]
    ;; ouput
    xx0 textequ <oword ptr [edi][2*0*32]>
    xx1 textequ <oword ptr [edi][2*1*32]>
    xx2 textequ <oword ptr [edi][2*2*32]>
    xx3 textequ <oword ptr [edi][2*3*32]>
    ;; y component at x[4]

    R4bfly
;;-----------------------
add esi, 16     ;; inc by 2 complex values
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
;====================================================
R4Stage3 proc near
;;
;; input = esi   output = edi
;; input output = esi
;; uses esi edi ebx ecx edx
;; caller must set    movaps   xmm5, wsign_flip
;;
;; middle reads exchanged by text equates
;;
wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>

;;------- special case coefs
    mov edx, 16
a100:
;;----------------------
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][2*0*128]>
    x1 textequ <oword ptr [esi][2*2*128]>
    x2 textequ <oword ptr [esi][2*1*128]>
    x3 textequ <oword ptr [esi][2*3*128]>
    ;; y component at x[4]
    ;; ouput
    xx0 textequ <oword ptr [edi][2*0*128]>
    xx1 textequ <oword ptr [edi][2*1*128]>
    xx2 textequ <oword ptr [edi][2*2*128]>
    xx3 textequ <oword ptr [edi][2*3*128]>
    ;; y component at x[4]

    R4bfly0

    add esi, 16     ;; inc by 2 complex values
    add edi, 16     ;; inc by 2 complex values
    dec edx
    jne a100

;;==========================
add esi, 8*(2*(4*16-16))
add edi, 8*(2*(4*16-16))
mov ebx, 24
mov ecx, 3

wc1 textequ <wcs4[ebx][8*0][0]>
ws1 textequ <wcs4[ebx][8*0][4]>
wc2 textequ <wcs4[ebx][8*1][0]>
ws2 textequ <wcs4[ebx][8*1][4]>
wc3 textequ <wcs4[ebx][8*2][0]>
ws3 textequ <wcs4[ebx][8*2][4]>

a300:
    mov edx, 16
a200:
;;----------------------
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][2*0*128]>
    x1 textequ <oword ptr [esi][2*2*128]>
    x2 textequ <oword ptr [esi][2*1*128]>
    x3 textequ <oword ptr [esi][2*3*128]>
    ;; y component at x[4]
    ;; ouput
    xx0 textequ <oword ptr [edi][2*0*128]>
    xx1 textequ <oword ptr [edi][2*1*128]>
    xx2 textequ <oword ptr [edi][2*2*128]>
    xx3 textequ <oword ptr [edi][2*3*128]>
    ;; y component at x[4]

    R4bfly
;;-----------------------
add esi, 16     ;; inc by 2 complex values
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
;;===================================================
R4StageLast512 proc near
;;
;; caller sets input = esi output = edi
;; uses esi edi ebx ecx
;; caller must set    movaps   xmm5, wsign_flip
;;
;;
;; middle reads exchanged by text equates
;;
    mov ecx, 64

a100:
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][2*0*512]>
    x1 textequ <oword ptr [esi][2*2*512]>
    x2 textequ <oword ptr [esi][2*1*512]>
    x3 textequ <oword ptr [esi][2*3*512]>
    ;; y component at x[4]
    ;; ouput

    xx0 textequ <oword ptr [edi][2*0*512]>
    xx1 textequ <oword ptr [edi][2*1*512]>
    xx2 textequ <oword ptr [edi][2*2*512]>
    xx3 textequ <oword ptr [edi][2*3*512]>
    ;; y component at x[4]

    R4bfly0

;;-----------------------
add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec ecx
    jne a100

    ret

R4StageLast512 endp
;====================================================
;;===================================================
R4StageLast128 proc near
;;
;; caller sets input = esi output = edi
;; uses esi edi ebx ecx
;; caller must set    movaps   xmm5, wsign_flip
;;
;;
;; middle reads exchanged by text equates
;;
    mov ecx, 16

a100:
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][2*0*128]>
    x1 textequ <oword ptr [esi][2*2*128]>
    x2 textequ <oword ptr [esi][2*1*128]>
    x3 textequ <oword ptr [esi][2*3*128]>
    ;; y component at x[4]
    ;; ouput

    xx0 textequ <oword ptr [edi][2*0*128]>
    xx1 textequ <oword ptr [edi][2*1*128]>
    xx2 textequ <oword ptr [edi][2*2*128]>
    xx3 textequ <oword ptr [edi][2*3*128]>
    ;; y component at x[4]

    R4bfly0

;;-----------------------
add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec ecx
    jne a100

    ret

R4StageLast128 endp
;====================================================
HannWindow1024 proc near
;; apply window and bit reverse 
;; input pcm may not be aligned! use movups
;;      esi = input
;;      edi = output
x  textequ <oword ptr [esi]>
y  textequ <oword ptr [edi]>
    xor eax, eax
    mov edx, 8*510
    mov ecx, 128

a100:
    mov ebx, bitrev[eax]

    movaps  xmm7, hann[eax*2]
    movups  xmm0, x[eax*2]
    mulps   xmm0, xmm7

    movlps  y[ebx], xmm0
    movhps  y[ebx][8*256], xmm0

    shufps  xmm7, xmm7, 00011011b
    movups  xmm1, x[edx]
    mulps   xmm1, xmm7    

    xor ebx, (511 shl 3)    ;; table scaled by 8
    movlps  y[ebx][-8*256], xmm1
    movhps  y[ebx], xmm1

    add eax,  2*4
    sub edx,  2*8

    dec ecx
    jne a100

    ret

HannWindow1024 endp
;====================================================
_xFFTtest proc public
;;
;;void (float x[], float y[])
;; map transform data to FFT energy bins
;;
;;
;; arguments
xin        textequ <dword ptr [esp+4*(1+npush)]>
xout       textequ <dword ptr [esp+4*(2+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----
    mov esi, xin
    mov edi, xout

    call HannWindow1024

    movaps   xmm5, wsign_flip

;;    mov ecx, 128    ;; two r2 bflys per loop iter
;;    call R2stage1

;;    mov ecx, 64
;;    call R4stage1

;;    mov ecx, 16
;;    call R4stage2

;;    call R4stage3   ;; sets own ecx counter

;;    call R4StageLast512  ;; sets own ecx counter

;;    call CvtReal1024
;;-------
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_xFFTtest endp
;;===================================================
;====================================================
_xFFT1024RealHann proc public
;;
;;void FFT(float x[], float y[])
;;
;; xin = pcm may not be aligned!
;;
;; arguments
xin        textequ <dword ptr [esp+4*(1+npush)]>
xout       textequ <dword ptr [esp+4*(2+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----
    mov esi, xin
    mov edi, xout

;;-----------------
;; apply window and bit reverse
x textequ <oword ptr [esi]>
y textequ <oword ptr [edi]>
    xor eax, eax
    mov edx, 8*510
    mov ecx, 128

a100:
    mov ebx, bitrev[eax]

    movaps  xmm7, hann[eax*2]
    movups  xmm0, x[eax*2]
    mulps   xmm0, xmm7

    movlps  y[ebx], xmm0
    movhps  y[ebx][8*256], xmm0

    shufps  xmm7, xmm7, 00011011b
    movups  xmm1, x[edx]
    mulps   xmm1, xmm7    

    xor ebx, (511 shl 3)    ;; table scaled by 8
    movlps  y[ebx][-8*256], xmm1
    movhps  y[ebx], xmm1

    add eax,  2*4
    sub edx,  2*8

    dec ecx
    jne a100

;;-----------


    movaps   xmm5, wsign_flip

    mov esi, xout
    mov edi, esi
    mov ecx, 128    ;; two r2 bflys per loop iter
    call R2stage1

    mov esi, xout
    mov edi, esi
    mov ecx, 64
    call R4stage1

    mov esi, xout
    mov edi, esi
    mov ecx, 16
    call R4stage2

    mov esi, xout
    mov edi, esi
    call R4stage3   ;; sets own ecx counter

    mov esi, xout
    mov edi, esi
    call R4StageLast512  ;; sets own ecx counter

    mov esi, xout
    call CvtReal1024
;;-------
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_xFFT1024RealHann endp
;;===================================================
_xFFT256RealHann proc public
;;
;;void FFT(float x[], float y[])
;;
;; xin = pcm may not be aligned!
;;
;; arguments
xin        textequ <dword ptr [esp+4*(1+npush)]>
xout       textequ <dword ptr [esp+4*(2+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----
    mov esi, xin
    mov edi, xout

;;-----------------
;; apply window and bit reverse
x textequ <oword ptr [esi]>
y textequ <oword ptr [edi]>
    xor eax, eax
    mov edx, 8*126
    mov ecx, 32

a100:
    mov ebx, bitrev[eax*2]

    movaps  xmm7, hann256[eax]
    movups    xmm0, x[eax]
    mulps   xmm0, xmm7

    movlps  y[ebx], xmm0
    movhps  y[ebx][8*64], xmm0

    shufps  xmm7, xmm7, 00011011b
    movups  xmm1, x[edx]
    mulps   xmm1, xmm7    

    xor ebx, (127 shl 3)    ;; table scaled by 8
    movlps  y[ebx][-8*64], xmm1
    movhps  y[ebx], xmm1

    add eax,  2*2*4
    sub edx,  2*8

    dec ecx
    jne a100

;;-----------

    movaps   xmm5, wsign_flip

    mov esi, xout
    mov edi, esi
    mov ecx, 32    ;; two r2 bflys per loop iter
    call R2stage1


    mov esi, xout
    mov edi, esi
    mov ecx, 16
    call R4stage1

    mov esi, xout
    mov edi, esi
    mov ecx, 4
    call R4stage2

    mov esi, xout
    mov edi, esi
    call R4StageLast128  ;; sets own ecx counter

    mov esi, xout
    call CvtReal256
;;-------

exit:
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_xFFT256RealHann endp
;;===================================================
;;===================================================
;;===================================================
;;===================================================
;;===================================================
;;===================================================
;;========= 256pt complex =============================
;;===================================================
;;---------------------------------------------------
aR4_bfly macro  slot1
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
aR4_256Stage1 proc near
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

    aR4_bfly  <add esi, 32>

;;-----------------------
;;add esi, 32     ;; inc by 4 complex values
add edi, 32     ;; inc by 4 complex values
add ebx, 24     ;; inc coefs

    dec ecx
    jne a100



    ret
aR4_256Stage1 endp
;;---------------------------------------------------
R4_256Stage2 proc near
;;
;; input = esi   output = edi
;; uses esi edi ebx ecx edx
;;
;; caller sets outer loop count in ecx
;; caller must set    movaps   xmm5, wsign_flip
;;
;; middle reads exchanged by text equates
;;
    wc1 textequ <wcs4[ebx][8*0][0]>
    ws1 textequ <wcs4[ebx][8*0][4]>
    wc2 textequ <wcs4[ebx][8*1][0]>
    ws2 textequ <wcs4[ebx][8*1][4]>
    wc3 textequ <wcs4[ebx][8*2][0]>
    ws3 textequ <wcs4[ebx][8*2][4]>

    xor ebx, ebx
a200:
    mov edx, 2
a100:
;;----------------------
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][0*32]>
    x1 textequ <oword ptr [esi][2*32]>
    x2 textequ <oword ptr [esi][1*32]>
    x3 textequ <oword ptr [esi][3*32]>
    ;; y component at x[4]
    ;; ouput
    xx0 textequ <oword ptr [edi][0*32]>
    xx1 textequ <oword ptr [edi][1*32]>
    xx2 textequ <oword ptr [edi][2*32]>
    xx3 textequ <oword ptr [edi][3*32]>
    ;; y component at x[4]

    R4bfly
;;-----------------------
add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

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
;; input = esi   output = edi
;; input output = esi
;; uses esi edi ebx ecx edx
;; caller must set    movaps   xmm5, wsign_flip
;;
;; middle reads exchanged by text equates
;;
wc1 textequ <wcs4[8*0][0]>
ws1 textequ <wcs4[8*0][4]>
wc2 textequ <wcs4[8*1][0]>
ws2 textequ <wcs4[8*1][4]>
wc3 textequ <wcs4[8*2][0]>
ws3 textequ <wcs4[8*2][4]>

;;------- special case coefs
    mov edx, 8
a100:
;;----------------------
;; does two in parallel
    ;; input
    x0 textequ <oword ptr [esi][0*128]>
    x1 textequ <oword ptr [esi][2*128]>
    x2 textequ <oword ptr [esi][1*128]>
    x3 textequ <oword ptr [esi][3*128]>
    ;; y component at x[4]
    ;; ouput
    xx0 textequ <oword ptr [edi][0*128]>
    xx1 textequ <oword ptr [edi][1*128]>
    xx2 textequ <oword ptr [edi][2*128]>
    xx3 textequ <oword ptr [edi][3*128]>
    ;; y component at x[4]

    R4bfly0

    add esi, 16     ;; inc by 2 complex values
    add edi, 16     ;; inc by 2 complex values
    dec edx
    jne a100

;;==========================
add esi, 8*((4*16-16))
add edi, 8*((4*16-16))
mov ebx, 24
mov ecx, 3

wc1 textequ <wcs4[ebx][8*0][0]>
ws1 textequ <wcs4[ebx][8*0][4]>
wc2 textequ <wcs4[ebx][8*1][0]>
ws2 textequ <wcs4[ebx][8*1][4]>
wc3 textequ <wcs4[ebx][8*2][0]>
ws3 textequ <wcs4[ebx][8*2][4]>

a300:
    mov edx, 8
a200:
;;----------------------
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][0*128]>
    x1 textequ <oword ptr [esi][2*128]>
    x2 textequ <oword ptr [esi][1*128]>
    x3 textequ <oword ptr [esi][3*128]>
    ;; y component at x[4]
    ;; ouput
    xx0 textequ <oword ptr [edi][0*128]>
    xx1 textequ <oword ptr [edi][1*128]>
    xx2 textequ <oword ptr [edi][2*128]>
    xx3 textequ <oword ptr [edi][3*128]>
    ;; y component at x[4]

    R4bfly
;;-----------------------
add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec edx
    jne a200

add esi, 8*((4*16-16))
add edi, 8*((4*16-16))

add ebx, 24

    dec ecx
    jne a300

    ret
R4_256Stage3 endp
;;===================================================
;;===================================================
R4StageLast256 proc near
;;
;; caller sets input = esi output = edi
;; uses esi edi ebx ecx
;; caller must set    movaps   xmm5, wsign_flip
;;
;;
;; middle reads exchanged by text equates
;;
    mov ecx, 32

a100:
;; even (does both even/odd)
    ;; input
    x0 textequ <oword ptr [esi][0*512]>
    x1 textequ <oword ptr [esi][2*512]>
    x2 textequ <oword ptr [esi][1*512]>
    x3 textequ <oword ptr [esi][3*512]>
    ;; y component at x[4]
    ;; ouput

    xx0 textequ <oword ptr [edi][0*512]>
    xx1 textequ <oword ptr [edi][1*512]>
    xx2 textequ <oword ptr [edi][2*512]>
    xx3 textequ <oword ptr [edi][3*512]>
    ;; y component at x[4]

    R4bfly0

;;-----------------------
add esi, 16     ;; inc by 2 complex values
add edi, 16     ;; inc by 2 complex values

    dec ecx
    jne a100

    ret

R4StageLast256 endp
;;===================================================
CvtReal512 proc near
;;
;;
;;  caller sets input = esi
;;  uses esi edi ebx ecx
;;  caller must load xmm5 with sign flip
;;
;; /* conversion has factor of 2 scaling over actual */
;;
;; *** mem access is unaligned even if buffer aligned!

    w  textequ <oword ptr wr512[ebx]>
    xi textequ <oword ptr [esi+ebx]>
    xj textequ <oword ptr [edi]>

    lea edi, [esi+8*(256-2)]
    mov ecx, 63    ;; does two per iteration
    mov ebx, 8

;; special case
    movss   xmm0, dword ptr [esi]
    movss   xmm1, dword ptr [esi][4]
    addss   xmm0, xmm1
    addss   xmm0, xmm0
    movss   dword ptr [esi], xmm0
    mov     dword ptr [esi][4], 0
    
a100:
    movups  xmm1, xj
    shufps  xmm1, xmm1, 01001110b
    xorps   xmm1, xmm5  ;;  x -y 
    movups  xmm0, xi

    movaps  xmm2, xmm0
    subps   xmm0, xmm1      ;;  txb tya
    addps   xmm2, xmm1      ;;  txa tyb
    
    movaps  xmm3, xmm0              ;; txb tya
    shufps  xmm0, xmm0, 10110001b   ;; tya txb

    ;; load coefs
    movaps  xmm7, w
    movaps  xmm6, xmm7
    shufps  xmm7, xmm7,    11110101b    ;; w1
    shufps  xmm6, xmm6,    10100000b    ;; w0
    xorps   xmm7, xmm5  ;; w1 -w1
    
    mulps   xmm3, xmm6 
    mulps   xmm0, xmm7
    addps   xmm3, xmm0      ;; qx qy

        movaps  xmm1, xmm2      ;; txa tyb

    addps   xmm2, xmm3      ;; xi
    movups  xi, xmm2

        subps   xmm1, xmm3      ;; x -y
        xorps   xmm1, xmm5      ;;  xj
        shufps  xmm1, xmm1,  01001110b
        movups  xj, xmm1

    add ebx, 16
    sub edi, 16
    dec ecx
    jne a100

;; special cases
;; i = 127
    movss   xmm0, xi
    movss   xmm4, xj[8]
    movss   xmm1, xmm0
    addss   xmm0, xmm4  ;; txa
    subss   xmm1, xmm4  ;; txb

    movss   xmm2, xi[4]
    movss   xmm4, xj[8][4]
    movss   xmm3, xmm2
    addss   xmm2, xmm4  ;; tya
    subss   xmm3, xmm4  ;; tyb

    movss   xmm6, w     ;; w0
    movss   xmm7, w[4]  ;; w1
    movss   xmm4, xmm1  ;; txb
    movss   xmm5, xmm2  ;; tya          ;; sign flip destroyed
    mulss   xmm1, xmm6  ;; txb*w0
    mulss   xmm2, xmm7  ;; tya*w1
    mulss   xmm5, xmm6  ;; tya*w0
    mulss   xmm4, xmm7  ;; txb*w1

    addss   xmm1, xmm2  ;; qx
    subss   xmm5, xmm4  ;; qy

    movss   xmm6, xmm0  ;; txa
    addss   xmm0, xmm1  ;; xi
    movss   xi, xmm0

    movss   xmm7, xmm3  ;; tyb
    addss   xmm3, xmm5  ;; yi
    movss   xi[4], xmm3

    subss   xmm6, xmm1  ;; xj
    movss   xj[8], xmm6
    subss   xmm5, xmm7  ;; yj
    movss   xj[8][4], xmm5
;;-----------
;; i= 128
    movlps  xmm1, wsign_flip
    movlps  xmm0, xi[8]
    xorps   xmm0, xmm1
    addps   xmm0, xmm0
    movlps  xi[8], xmm0

    ret

CvtReal512 endp
;;===================================================
input256   proc near
;;
;; input for 1024 real hann aprox
;; subsample, hann, and bit reverse
;; input = esi, output = edi
;; uses eax ebx ecx edx
;;

x textequ <oword ptr [esi]>
y textequ <qword ptr [edi]>

    xor eax, eax
    mov edx, 4*1016
    mov ecx, 64

a100:
    movups  xmm5, x[eax*2][0]       ;; x0 x1 x2 x3 
    movups  xmm6, x[eax*2][16]      ;; x4 x5 x6 x7

    movaps  xmm0, xmm5
    shufps  xmm0, xmm6, 10001000b   ;; x0 x2 x4 x6

    movups  xmm7, x[eax*2][2*16]    ;; x8 x9 x10 x11
    movaps  xmm4, xmm6
    shufps  xmm4, xmm7, 10001000b   ;; x4 x6 x8 x10

    movaps  xmm1, xmm5
    shufps  xmm1, xmm6, 11011101b   ;; x1 x3 x5 x7

    shufps  xmm5, xmm6, 01001110b   ;; x2 x3 x4 x5
    shufps  xmm6, xmm7, 01001110b   ;; x6 x7 x8 x9

        addps   xmm0, xmm4
    movaps  xmm3, xmm5
    shufps  xmm3, xmm6, 11011101b   ;; x3 x5 x7 x9

    movaps  xmm2, xmm5
    shufps  xmm2, xmm6, 10001000b   ;; x2 x4 x6 x8

        addps   xmm1, xmm3
        mulps   xmm0, xcon25
        mulps   xmm1, xcon50
        addps   xmm0, xmm2
        addps   xmm0, xmm1
        mulps   xmm0, hann512[eax]

    mov ebx, bitrev[eax]
    movlps  y[ebx], xmm0
    movhps  y[ebx][8*128], xmm0
;;---
    movups  xmm5, x[edx][0]       ;; x0 x1 x2 x3 
    movups  xmm6, x[edx][16]      ;; x4 x5 x6 x7

    movaps  xmm0, xmm5
    shufps  xmm0, xmm6, 10001000b   ;; x0 x2 x4 x6

    movups  xmm7, x[edx][2*16]    ;; x8 x9 x10 x11
    movaps  xmm4, xmm6
    shufps  xmm4, xmm7, 10001000b   ;; x4 x6 x8 x10

    movaps  xmm1, xmm5
    shufps  xmm1, xmm6, 11011101b   ;; x1 x3 x5 x7

    shufps  xmm5, xmm6, 01001110b   ;; x2 x3 x4 x5
    shufps  xmm6, xmm7, 01001110b   ;; x6 x7 x8 x9

        addps   xmm0, xmm4
    movaps  xmm2, xmm5
    shufps  xmm2, xmm6, 10001000b   ;; x2 x4 x6 x8

    movaps  xmm3, xmm5
    shufps  xmm3, xmm6, 11011101b   ;; x3 x5 x7 x9

        mulps   xmm0, xcon25
        addps   xmm1, xmm3
        mulps   xmm1, xcon50
        addps   xmm0, xmm2
        addps   xmm0, xmm1

    movaps  xmm7, hann512[eax]
    shufps  xmm7, xmm7, 00011011b
    mulps   xmm0, xmm7

    xor ebx, (255 shl 3)    ;; table scaled by 8
    movlps  y[ebx][-8*128], xmm0
    movhps  y[ebx], xmm0

    add eax, 4*4
    sub edx, 8*4
    dec ecx
    jne a100



    ret
input256   endp
;;====================================================
;;===================================================
_xFFT1024RealHannApprox  proc public
;;
;;void FFT(float x[], float y[])
;;
;;
;; arguments
xin        textequ <dword ptr [esp+4*(1+npush)]>
yout       textequ <dword ptr [esp+4*(2+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----
;;
;;----------------------------
;; subsample, apply window and bit reverse 
    mov esi, xin
    mov edi, yout
    call input256

    movaps   xmm5, wsign_flip

    mov esi, yout
    mov edi, esi
    mov ecx, 64
    call aR4_256Stage1

    mov esi, yout
    mov edi, esi
    mov ecx, 16
    call R4_256Stage2

    mov esi, yout
    mov edi, esi
    call R4_256Stage3

    mov esi, yout
    mov edi, esi
    call R4StageLast256

    mov esi, yout
    call CvtReal512

exit:
;;
    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
_xFFT1024RealHannApprox  endp
;;===================================================
;;===================================================
;;===================================================
_TEXT ENDS
;====================================
          END
