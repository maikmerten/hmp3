; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: l3math.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
; L3math.asm   
;
;  Layer 3 bitallo math functions
;
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
;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;
ALIGN 4


g_offset = 8


;;extrn _look_pigain:dword
;;extrn _look_f00_pmax:dword  ;; float in mb

extrn _look_34igain:dword
extrn _look_gain:dword
extrn _look_ix43:dword

;
con_1000log10_2   dd  301.029995664
con_10log10_2     dd  3.01029995664
con_1505          dd  1.50515
con_150500        dd  150.515
con_eminus12      dd  1.0e-12
con_11            dd  1.1
con_1100          dd  110.0
con_946           dd  0.0946
;;con_946           dd  0.15   ;; better snr mnr, worse xfer
con_5946          dd  0.5946
con_707			  dd  0.70710678
con_05            dd  0.5
con_4375          dd  0.4375
con_10            dd  10.0
con_4             dd   4.0
con_2             dd   2.0

gain_0_1        dd  0, 0

quant_table label dword
 dd  -0.34290 ;;  0.09460 -0.4375    0
;; dd  -0.25
;; dd  -0.30
 dd  -0.40951 ;;  0.02799 -0.4375    1
 dd  -0.42079 ;;  0.01671 -0.4375    2
 dd  -0.42558 ;;  0.01192 -0.4375    3
 dd  -0.42823 ;;  0.00927 -0.4375    4
 dd  -0.42992 ;;  0.00758 -0.4375    5
 dd  -0.43109 ;;  0.00641 -0.4375    6
 dd  -0.43194 ;;  0.00556 -0.4375    7
 dd  -0.43260 ;;  0.00490 -0.4375    8
 dd  -0.43311 ;;  0.00439 -0.4375    9
 dd  -0.43353 ;;  0.00397 -0.4375   10
 dd  -0.43388 ;;  0.00362 -0.4375   11
 dd  -0.43417 ;;  0.00333 -0.4375   12
 dd  -0.43441 ;;  0.00309 -0.4375   13
 dd  -0.43463 ;;  0.00287 -0.4375   14
 dd  -0.43481 ;;  0.00269 -0.4375   15
 dd  -0.43497 ;;  0.00253 -0.4375   16
 dd  -0.43512 ;;  0.00238 -0.4375   17
 dd  -0.43525 ;;  0.00225 -0.4375   18
 dd  -0.43536 ;;  0.00214 -0.4375   19
 dd  -0.43547 ;;  0.00203 -0.4375   20
 dd  -0.43556 ;;  0.00194 -0.4375   21
 dd  -0.43565 ;;  0.00185 -0.4375   22
 dd  -0.43573 ;;  0.00177 -0.4375   23
 dd  -0.43580 ;;  0.00170 -0.4375   24
 dd  -0.43587 ;;  0.00163 -0.4375   25
 dd  -0.43593 ;;  0.00157 -0.4375   26
 dd  -0.43598 ;;  0.00152 -0.4375   27
 dd  -0.43604 ;;  0.00146 -0.4375   28
 dd  -0.43609 ;;  0.00141 -0.4375   29
 dd  -0.43614 ;;  0.00136 -0.4375   30
 dd  -0.43618 ;;  0.00132 -0.4375   31



;;------------- log_2  coefs ordered [b, a] ntable = 8 = 3 bits
look_log_bits = 3
;; for n = 8 max eps approx 0.01  on log in db
look_log2  label dword
  dd   03FAE00D2h,   0BFAE00D2h
  dd   03F9BA6B3h,   0BF995B70h
  dd   03F8CCDBAh,   0BF86CC38h
  dd   03F808B2Bh,   0BF6BE167h
  dd   03F6C7F52h,   0BF4CFEE1h
  dd   03F5AF65Ch,   0BF308051h
  dd   03F4BD95Ch,   0BF160D92h
  dd   03F3EB025h,   0BEFAC094h

;; 256 pt table max relative error about 3.5e-7
include pow14.inc

;; 64 pt table adds error to fnc_actual (about 0.1db)
;; for now want less error for testing
;; 64 pt table max relative error about 1.5e-5
;; 256 pt table max eps = 1.0e-6
include pow43.inc

;;------------------------------------------------
;; LogSubberTable  size = 84
LogSubberTable  label dword
 dd        7,       22,       36,       49
 dd       61,       73,       83,       93
 dd      103,      112,      120,      128
 dd      136,      143,      150,      156
 dd      163,      168,      174,      179
 dd      184,      189,      194,      198
 dd      202,      206,      210,      214
 dd      217,      220,      223,      227
 dd      229,      232,      235,      237
 dd      240,      242,      244,      247
 dd      249,      251,      253,      254
 dd      256,      258,      259,      261
 dd      263,      264,      265,      267
 dd      268,      269,      270,      271
 dd      273,      274,      275,      276
 dd      277,      277,      278,      279
 dd      280,      281,      281,      282
 dd      283,      283,      284,      285
 dd      285,      286,      286,      287
 dd      287,      288,      288,      289
 dd      289,      290,      290,      290



align 4
map_table label dword
dd  1.000
dd  0.875
dd  0.750
dd  0.625
dd  0.500
dd  0.375
dd  0.250
dd  0.125

align 4
;; mbLogC all integer lookup
;; eps ave    4.41e-001    5.62e-001 max    2.19e+000
look_mbLogC label dword
  dd   0FFFF6AAEh, 0FFFF6AB0h, 0FFFF6AB1h, 0FFFF6AB3h, 0FFFF6AB5h, 0FFFF6AB6h
  dd   0FFFF6AB8h, 0FFFF6ABAh, 0FFFF6ABBh, 0FFFF6ABDh, 0FFFF6ABEh, 0FFFF6AC0h
  dd   0FFFF6AC2h, 0FFFF6AC3h, 0FFFF6AC5h, 0FFFF6AC7h, 0FFFF6AC8h, 0FFFF6ACAh
  dd   0FFFF6ACBh, 0FFFF6ACDh, 0FFFF6ACEh, 0FFFF6AD0h, 0FFFF6AD2h, 0FFFF6AD3h
  dd   0FFFF6AD5h, 0FFFF6AD6h, 0FFFF6AD8h, 0FFFF6AD9h, 0FFFF6ADBh, 0FFFF6ADCh
  dd   0FFFF6ADEh, 0FFFF6ADFh, 0FFFF6AE1h, 0FFFF6AE2h, 0FFFF6AE4h, 0FFFF6AE5h
  dd   0FFFF6AE7h, 0FFFF6AE8h, 0FFFF6AEAh, 0FFFF6AEBh, 0FFFF6AEDh, 0FFFF6AEEh
  dd   0FFFF6AF0h, 0FFFF6AF1h, 0FFFF6AF3h, 0FFFF6AF4h, 0FFFF6AF5h, 0FFFF6AF7h
  dd   0FFFF6AF8h, 0FFFF6AFAh, 0FFFF6AFBh, 0FFFF6AFDh, 0FFFF6AFEh, 0FFFF6AFFh
  dd   0FFFF6B01h, 0FFFF6B02h, 0FFFF6B04h, 0FFFF6B05h, 0FFFF6B06h, 0FFFF6B08h
  dd   0FFFF6B09h, 0FFFF6B0Bh, 0FFFF6B0Ch, 0FFFF6B0Dh, 0FFFF6B0Fh, 0FFFF6B10h
  dd   0FFFF6B11h, 0FFFF6B13h, 0FFFF6B14h, 0FFFF6B15h, 0FFFF6B17h, 0FFFF6B18h
  dd   0FFFF6B19h, 0FFFF6B1Bh, 0FFFF6B1Ch, 0FFFF6B1Dh, 0FFFF6B1Fh, 0FFFF6B20h
  dd   0FFFF6B21h, 0FFFF6B22h, 0FFFF6B24h, 0FFFF6B25h, 0FFFF6B26h, 0FFFF6B28h
  dd   0FFFF6B29h, 0FFFF6B2Ah, 0FFFF6B2Bh, 0FFFF6B2Dh, 0FFFF6B2Eh, 0FFFF6B2Fh
  dd   0FFFF6B30h, 0FFFF6B32h, 0FFFF6B33h, 0FFFF6B34h, 0FFFF6B35h, 0FFFF6B37h
  dd   0FFFF6B38h, 0FFFF6B39h, 0FFFF6B3Ah, 0FFFF6B3Ch, 0FFFF6B3Dh, 0FFFF6B3Eh
  dd   0FFFF6B3Fh, 0FFFF6B40h, 0FFFF6B42h, 0FFFF6B43h, 0FFFF6B44h, 0FFFF6B45h
  dd   0FFFF6B46h, 0FFFF6B48h, 0FFFF6B49h, 0FFFF6B4Ah, 0FFFF6B4Bh, 0FFFF6B4Ch
  dd   0FFFF6B4Eh, 0FFFF6B4Fh, 0FFFF6B50h, 0FFFF6B51h, 0FFFF6B52h, 0FFFF6B53h
  dd   0FFFF6B55h, 0FFFF6B56h, 0FFFF6B57h, 0FFFF6B58h, 0FFFF6B59h, 0FFFF6B5Ah
  dd   0FFFF6B5Bh, 0FFFF6B5Dh, 0FFFF6B5Eh, 0FFFF6B5Fh, 0FFFF6B60h, 0FFFF6B61h
  dd   0FFFF6B62h, 0FFFF6B63h, 0FFFF6B64h, 0FFFF6B65h, 0FFFF6B67h, 0FFFF6B68h
  dd   0FFFF6B69h, 0FFFF6B6Ah, 0FFFF6B6Bh, 0FFFF6B6Ch, 0FFFF6B6Dh, 0FFFF6B6Eh
  dd   0FFFF6B6Fh, 0FFFF6B70h, 0FFFF6B72h, 0FFFF6B73h, 0FFFF6B74h, 0FFFF6B75h
  dd   0FFFF6B76h, 0FFFF6B77h, 0FFFF6B78h, 0FFFF6B79h, 0FFFF6B7Ah, 0FFFF6B7Bh
  dd   0FFFF6B7Ch, 0FFFF6B7Dh, 0FFFF6B7Eh, 0FFFF6B7Fh, 0FFFF6B80h, 0FFFF6B81h
  dd   0FFFF6B82h, 0FFFF6B83h, 0FFFF6B85h, 0FFFF6B86h, 0FFFF6B87h, 0FFFF6B88h
  dd   0FFFF6B89h, 0FFFF6B8Ah, 0FFFF6B8Bh, 0FFFF6B8Ch, 0FFFF6B8Dh, 0FFFF6B8Eh
  dd   0FFFF6B8Fh, 0FFFF6B90h, 0FFFF6B91h, 0FFFF6B92h, 0FFFF6B93h, 0FFFF6B94h
  dd   0FFFF6B95h, 0FFFF6B96h, 0FFFF6B97h, 0FFFF6B98h, 0FFFF6B99h, 0FFFF6B9Ah
  dd   0FFFF6B9Bh, 0FFFF6B9Ch, 0FFFF6B9Dh, 0FFFF6B9Eh, 0FFFF6B9Fh, 0FFFF6BA0h
  dd   0FFFF6BA1h, 0FFFF6BA1h, 0FFFF6BA2h, 0FFFF6BA3h, 0FFFF6BA4h, 0FFFF6BA5h
  dd   0FFFF6BA6h, 0FFFF6BA7h, 0FFFF6BA8h, 0FFFF6BA9h, 0FFFF6BAAh, 0FFFF6BABh
  dd   0FFFF6BACh, 0FFFF6BADh, 0FFFF6BAEh, 0FFFF6BAFh, 0FFFF6BB0h, 0FFFF6BB1h
  dd   0FFFF6BB2h, 0FFFF6BB3h, 0FFFF6BB3h, 0FFFF6BB4h, 0FFFF6BB5h, 0FFFF6BB6h
  dd   0FFFF6BB7h, 0FFFF6BB8h, 0FFFF6BB9h, 0FFFF6BBAh, 0FFFF6BBBh, 0FFFF6BBCh
  dd   0FFFF6BBDh, 0FFFF6BBEh, 0FFFF6BBEh, 0FFFF6BBFh, 0FFFF6BC0h, 0FFFF6BC1h
  dd   0FFFF6BC2h, 0FFFF6BC3h, 0FFFF6BC4h, 0FFFF6BC5h, 0FFFF6BC6h, 0FFFF6BC7h
  dd   0FFFF6BC7h, 0FFFF6BC8h, 0FFFF6BC9h, 0FFFF6BCAh, 0FFFF6BCBh, 0FFFF6BCCh
  dd   0FFFF6BCDh, 0FFFF6BCEh, 0FFFF6BCEh, 0FFFF6BCFh, 0FFFF6BD0h, 0FFFF6BD1h
  dd   0FFFF6BD2h, 0FFFF6BD3h, 0FFFF6BD4h, 0FFFF6BD4h, 0FFFF6BD5h, 0FFFF6BD6h
  dd   0FFFF6BD7h, 0FFFF6BD8h, 0FFFF6BD9h, 0FFFF6BDAh

align 4
include mbExp.inc    ;; look_mbExp   inverse mbLog

_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
;====================================
;====================================
_align16  PROC PUBLIC 

;;  float *align(float *xin)
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 0

    mov eax, xin
    add eax, 15
    and eax, 0FFFFFFF0h


    ret

_align16 endp
;====================================
_round_to_int  PROC PUBLIC 
;;  (int) f(float x)
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 0

    fld xin
    fistp xin
    mov eax, xin

    ret

_round_to_int  endp
;====================================
;====================================
_pos_fmin  PROC PUBLIC 
;;  (float) min(float x, float y)
;;  x,y non-negative
;;
;; arguments
x     textequ <dword ptr [esp+4*(1+npush)]>
y     textequ <dword ptr [esp+4*(2+npush)]>
npush = 1

    push ebx


    mov eax, x
    mov ebx, y
    sub eax, ebx
    cdq
    and eax, edx
    add eax, ebx
    mov x, eax
    fld x

    pop ebx

    ret

_pos_fmin  endp
;====================================
;====================================
_pos_fmax  PROC PUBLIC 
;;  (float) max(float x, float y)
;;  x,y non-negative
;;
;; arguments
x     textequ <dword ptr [esp+4*(1+npush)]>
y     textequ <dword ptr [esp+4*(2+npush)]>
npush = 1

    push ebx


    mov eax, x
    mov ebx, y
    sub eax, ebx
    cdq
    not edx
    and eax, edx
    add eax, ebx
    mov x, eax
    fld x

    pop ebx

    ret

_pos_fmax  endp
;====================================
_dbLog  PROC PUBLIC 
;;  (float) dbLog10(float x)
;;     y = 10.0*log10(x)     x > 0
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 0

    fld con_10log10_2 
    fld xin
    fyl2x

    ret

_dbLog  endp
;====================================
;====================================
dbLogMacro  macro  xin
;;  (float) dbLog10(float x)
;;     y = 10.0*log10(x)     x > 0
;;
;;  xin  floating pt mem location  (destroyed)
;;  uses eax edx ebx
;;
;;---------
    mov eax, xin
    mov edx, eax
    mov ebx, eax

    shr eax, (23-look_log_bits)
    and eax, ((1 shl look_log_bits) - 1)

    and edx, 07FFFFFh
    or  edx, (127 shl 23)

    mov xin, edx

    fld con_10log10_2 

    fld look_log2[eax*8]
    fld xin
    fmulp   st(1), st

    fld look_log2[eax*8+4]
    faddp   st(1), st

    shr ebx, 23 
    sub ebx, 127
    mov xin, ebx
    fild    xin

    faddp   st(1), st

    fmulp   st(1), st

;; result in st(0)

endm
;====================================
_dbLogB  PROC PUBLIC 
;;  (float) dbLog10(float x)
;;     y = 10.0*log10(x)     x > 0
;;
;; arguments
;;
;;
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 1

    push    ebx

;;---------
    dbLogMacro  xin
;;---------

    pop ebx

    ret

_dbLogB  endp
;====================================
;====================================
;====================================
_mbLog  PROC PUBLIC 
;;  int  mbLog(float x)
;;     y = 1000.0*log10(x)     x > 0
;;  integer result in milli-bells
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 0

    fld con_1000log10_2 
    fld xin
    fyl2x
    fistp   xin
    mov eax, xin

    ret

_mbLog  endp
;====================================
;====================================
mbLogMacro  macro  xin
;;  int dbLog10(float x)
;;     y = 1000.0*log10(x)     x > 0
;;
;;  xin  floating pt mem location  (destroyed)
;;  uses eax edx ebx
;;
;;---------
    mov eax, xin
    mov edx, eax
    mov ebx, eax

    shr eax, (23-look_log_bits)
    and eax, ((1 shl look_log_bits) - 1)

    and edx, 07FFFFFh
    or  edx, (127 shl 23)

    mov xin, edx

    fld con_1000log10_2 

    fld look_log2[eax*8]
    fld xin
    fmulp   st(1), st

    fld look_log2[eax*8+4]
    faddp   st(1), st

    shr ebx, 23 
    sub ebx, 127
    mov xin, ebx
    fild    xin

    faddp   st(1), st

    fmulp   st(1), st

;; result in st(0)

endm
;====================================
mbLogMacroC macro xin

;; uses eax edx

    mov eax, xin   ;; float xin
    mov edx, eax
    shr eax, (23-8)
    shr edx, 23
    and eax, 255
    mov eax, look_mbLogC[eax*4]
    imul edx, 301
    add eax, edx

    ;; result in eax

endm
;====================================
_mbLogB  PROC PUBLIC 
;;  int mbLogB(float x)
;;     y = 1000.0*log10(x)     x > 0
;;  integer result in milli-bells
;;
;; arguments
;;
;;
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 1

    push    ebx

;;---------
    mbLogMacro  xin
;;---------
    fistp   xin
    mov eax, xin

    pop ebx

    ret

_mbLogB  endp
;====================================
;====================================
_mbLogC  PROC PUBLIC 
;;  int mbLogC(float x)
;;     y = 1000.0*log10(x)     x >= 0 (0 is OK)
;;  integer result in milli-bells
;;  max absolute eps = 4mb
;;  relative max eps = 1.5e-4
;;
;; arguments
;;
;;
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 0

    mov eax, xin
    mov edx, eax
    shr eax, (23-8)
    shr edx, 23
    and eax, 255
    mov eax, look_mbLogC[eax*4]
    imul edx, 301
    add eax, edx

    ret

_mbLogC  endp
;====================================
;====================================
_mbExp    PROC PUBLIC 
;;
;;  inverse msLog
;;  float mbExp(int mb)
;; arguments
;;
;;
xin     textequ <dword ptr [esp+4*(1+npush)]>
npush = 0

    mov eax, xin

    sub eax, 32000
    cdq 
    and eax, edx
    add eax, 64000
    cdq
    not edx
    and eax, edx
    sub eax, 32000

    movzx edx, ah
    movzx eax, al

    fld look_mbExpA[edx*4]
    fld look_mbExpB[eax*4]
    fmul

    ret
_mbExp endp 
;====================================
;====================================
_LogSubber  PROC PUBLIC 
;;  int LogSubber( int mbNoise, int mbNoise2)
;;
;; mbNoise in milli-bells
;; mbNoise > mbNoise2
;;  computes mbLog(2*antilog(mbNoise) - antilog(mbNoise2))
;;
;; arguments
;;
;;
mbNoise    textequ <dword ptr [esp+4*(1+npush)]>
mbNoise2   textequ <dword ptr [esp+4*(2+npush)]>
npush = 0


    mov eax, mbNoise
    mov edx, mbNoise2

    sub eax, edx
    shr eax, 4

    sub eax, 83     ;; limit range
    cdq 
    and eax, edx
    add eax, 83

    mov eax, LogSubberTable[eax*4]
    mov edx, mbNoise

    add eax, edx

    ret

_LogSubber  endp
;====================================
;====================================
_pow43ix  PROC PUBLIC 
;;  (float) pow43ix( int ix)
;;              ix > 0 !!!    
;;
;; arguments
;;
;;
xin    textequ <dword ptr [esp+4*(1+npush)]>
npush = 1

    push    ebx

;;---------
;;---------
    fild    xin
    fstp    xin

    mov eax, xin
    mov edx, eax
    mov ebx, eax

    shr eax, (23-pow43_bits)
    and eax, ((1 shl pow43_bits) - 1)

    and edx, 07FFFFFh
    or  edx, (127 shl 23)

    mov xin, edx

    fld look_pow43[eax*8]
    fld xin
    fmulp   st(1), st

    fld look_pow43[eax*8+4]
    faddp   st(1), st

    shr ebx, 23 
    and ebx, 255
    fld  look_exp43[ebx*4 - 127*4]

    fmulp   st(1), st

;;---------

    pop ebx

    ret

_pow43ix  endp
;====================================
;====================================
_vect_fmax1_01 proc public

;; float vect_fmax1(float x[], int n);   x[i]>=0
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
n       textequ <dword ptr [esp+4*(2+npush)]>

npush = 3
    push    esi
    push    ebx
    push    ecx



    mov esi, xin
    xor eax, eax
    mov ecx, n

a100:
    mov ebx, [esi]
    add esi, 4
    mov edx, -1
    sub eax, ebx
    adc edx, 0
    and eax, edx
    add eax, ebx
    dec ecx
    jg  a100

    push    eax         ;; result to fp stack
    fld dword ptr [esp]
    pop eax

    pop ecx
    pop ebx
    pop esi

    ret
_vect_fmax1_01 endp
;====================================
;====================================
_vect_fmax1 proc public

;; float vect_fmax1(float x[], int n);   x[i]>=0
;;
;; n must be even, n>=4
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
n       textequ <dword ptr [esp+4*(2+npush)]>

npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx


    mov esi, xin
;;    xor eax, eax
;;    xor edi, edi
    mov eax, [esi]
    mov edi, [esi][4]
    add esi, 2*4
    mov ecx, n
    sub ecx, 2

a100:
    mov ebx, [esi]
    sub eax, ebx
    cdq
    not edx
    and eax, edx
    add eax, ebx

    mov ebx, [esi][4]
    add esi, 2*4
    mov edx, -1
    sub edi, ebx
    adc edx, 0
    and edi, edx
    add edi, ebx

    sub ecx, 2
    jg  a100
;;--
    mov edx, -1
    sub eax, edi
    adc edx, 0
    and eax, edx
    add eax, edi

    mov xin, eax         ;; result to fp stack
    fld xin

    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
_vect_fmax1 endp
;====================================
;====================================
_vect_fmax201 proc public

;; void vect_fmax(float x[], int n, float *yout );   x[i]>=0
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
n       textequ <dword ptr [esp+4*(2+npush)]>
yout    textequ <dword ptr [esp+4*(3+npush)]>

npush = 3
    push    esi
    push    ebx
    push    ecx



    mov esi, xin
    xor eax, eax
    mov ecx, n

a100:
    mov ebx, [esi]
    add esi, 4
    mov edx, -1
    sub eax, ebx
    adc edx, 0
    and eax, edx
    add eax, ebx
    
    dec ecx
    jg  a100

    mov edx, yout
    mov [edx], eax

    pop ecx
    pop ebx
    pop esi

    ret
_vect_fmax201 endp
;====================================
;====================================
_vect_fmax2 proc public

;; void vect_fmax(float x[], int n, float *yout );   x[i]>=0
;;
;; n must be even, n>=4
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
n       textequ <dword ptr [esp+4*(2+npush)]>
yout    textequ <dword ptr [esp+4*(3+npush)]>

npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx


    mov esi, xin
;;    xor eax, eax
;;    xor edi, edi
    mov eax, [esi]
    mov edi, [esi][4]
    add esi, 2*4
    mov ecx, n
    sub ecx, 2

a100:
    mov ebx, [esi]
    sub eax, ebx
    cdq
    not edx
    and eax, edx
    add eax, ebx

    mov ebx, [esi][4]
    add esi, 2*4
    mov edx, -1
    sub edi, ebx
    adc edx, 0
    and edi, edx
    add edi, ebx

    sub ecx, 2
    jg  a100
;;--
    mov edx, -1
    sub eax, edi
    adc edx, 0
    and eax, edx
    add eax, edi

    mov esi, yout
    mov [esi], eax         ;; result

    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
_vect_fmax2 endp
;====================================
;====================================
_vect_imax proc public

;; int vect_imax(int x[], int n);   neg x ok
;; but always returns ixmax >= 0
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
n       textequ <dword ptr [esp+4*(2+npush)]>

npush = 3
    push    esi
    push    ebx
    push    ecx



    mov esi, xin
    xor eax, eax
    mov ecx, n

a100:
    mov ebx, [esi]
    add esi, 4
    sub eax, ebx
    cdq 
    not edx
    and eax, edx
    add eax, ebx
    dec ecx
    jg  a100


    pop ecx
    pop ebx
    pop esi

    ret
_vect_imax endp
;====================================
;====================================
_vect_pow3414A proc public

;; void vect_pow3414(float x[], float x34[], float x14[], 
;;      int n);   x[i]>=0
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
x34     textequ <dword ptr [esp+4*(2+npush)]>
x14     textequ <dword ptr [esp+4*(3+npush)]>
n       textequ <dword ptr [esp+4*(4+npush)]>


npush = 3
    push    esi
    push    ebx
    push    ecx

    mov esi, xin
    mov eax, x34
    mov edx, x14
    mov ecx, n


a100:
    fld dword ptr [esi]
    add esi, 4
    fsqrt
    fsqrt
    fst dword ptr [edx]     ;; x14
    add edx, 4
    fld st(0)
    fmul    st, st(0)
    fmulp   st(1), st
    fstp    dword ptr [eax] ;; x34
    add eax, 4

    dec ecx
    jg  a100


    pop ecx
    pop ebx
    pop esi

    ret
_vect_pow3414A endp
;====================================
_fnc_pow14 proc public
;; float fnc_pow14(float x)     x>=0
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>

npush = 1
    push    ebx



    mov eax, xin

    mov edx, eax
    mov ebx, eax

    shr eax, (23-8)
    and edx, 07FFFFFh
    
    and eax, 255
    or  edx, (127 shl 23)

    mov xin, edx

    fld xin
    fld look_pow14[eax*8]
    fmulp   st(1), st

    shr ebx, 23
    and ebx, 255        ;; in case sign bit is set

    fld look_pow14[eax*8+4]
    faddp   st(1), st
    fld look_exp14[ebx*4]
    fmulp   st(1), st


    pop ebx

    ret

_fnc_pow14 endp
;====================================
;====================================
_vect_pow3414 proc public

;; void vect_pow3414(float x[], float x34[], float x14[], 
;;      int n);   x[i]>=0
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
x34     textequ <dword ptr [esp+4*(2+npush)]>
x14     textequ <dword ptr [esp+4*(3+npush)]>
n       textequ <dword ptr [esp+4*(4+npush)]>

npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    push    ebp

    mov esi, xin
    mov ecx, x34
    mov edi, x14
    mov ebp, n

a100:
    mov eax, dword ptr [esi]
    add esi, 4

    mov edx, eax
    mov ebx, eax

    shr eax, (23-8)
    and edx, 07FFFFFh
    
    and eax, 255
    or  edx, (127 shl 23)

    mov xin, edx

    fld look_pow14[eax*8]
    fld xin
    fmulp   st(1), st

    shr ebx, 23
    and ebx, 255        ;; in case sign bit is set

    fld look_pow14[eax*8+4]
    faddp   st(1), st
    fld look_exp14[ebx*4]
    fmulp   st(1), st

    fst dword ptr [edi]     ;; x14
    fld st(0)

    fmul    st(0), st
    fmulp   st(1), st
    fstp dword ptr [ecx]        ;; x34

    add edi, 4
    add ecx, 4

    dec ebp
    jg  a100

    pop     ebp
    pop     ecx
    pop     ebx
    pop     edi
    pop     esi

    ret
_vect_pow3414 endp
;====================================
_vect_sign proc public

;; void vect_sign(float x[], char sign[], int n);
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
xsign   textequ <dword ptr [esp+4*(2+npush)]>
n       textequ <dword ptr [esp+4*(3+npush)]>


npush = 3
    push    esi
    push    ebx
    push    ecx

    mov esi, xin
    mov ebx, xsign
    mov ecx, n


a100:
    mov eax, [esi]
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs

    mov [esi], eax
    add esi, 4

    shr edx, 31
    mov [ebx], dl
    inc ebx

    dec ecx
    jg  a100

    pop ecx
    pop ebx
    pop esi

    ret
_vect_sign endp
;====================================
;====================================
_vect_sign_sxx proc public

;; sxx = float vect_sign(float x[], char sign[], int n);
;;
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
xsign   textequ <dword ptr [esp+4*(2+npush)]>
n       textequ <dword ptr [esp+4*(3+npush)]>


npush = 3
    push    esi
    push    ebx
    push    ecx

    mov esi, xin
    mov ebx, xsign
    mov ecx, n


    fldz
a100:
    fld dword ptr [esi]
    fmul st(0), st
    mov eax, [esi]
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs

    faddp   st(1), st

    mov [esi], eax
    add esi, 4

    shr edx, 31
    mov [ebx], dl
    inc ebx

    dec ecx
    jg  a100

    pop ecx
    pop ebx
    pop esi

    ret
_vect_sign_sxx endp
;====================================
;====================================
_vect_limits proc public

;; void vect_limits(int x[], int upper[], int lower[], int n)
;;
;; arguments
xin     textequ <dword ptr [esp+4*(1+npush)]>
upper   textequ <dword ptr [esp+4*(2+npush)]>
lower   textequ <dword ptr [esp+4*(3+npush)]>
n       textequ <dword ptr [esp+4*(4+npush)]>

npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    push    ebp


    mov esi, xin
    mov edi, upper
    mov ebx, lower
    mov ebp, n

    sub edi, esi
    sub ebx, esi

a100:
    mov eax, [esi]
    mov ecx, [esi+edi]  ;; min(x, upper)
    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    mov ecx, [esi+ebx]  ;; max(x, lower)
    sub eax, ecx
    cdq
    not edx
    and eax, edx
    add eax, ecx

    mov [esi], eax
    add esi, 4

    dec ebp
    jg  a100

    pop ebp
    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
_vect_limits endp
;====================================
;====================================
;====================================
;====================================
;;_ixfnc_noise0P proc public       
;;;;
;;;;
;;;;  int noise = xfnc_noise0P(float x14max, float xgsf)
;;;;  integer noise in milli-bells
;;;;
;;;; gsf = xgsf + 0.5;
;;;; igain = look_pigain[gsf];
;;;; p   = (int)((0.5f) + x14max*igain);
;;;; sum = look_f00_pmax[p];    // inl mb
;;;; noise = sum + 110.0*(xgsf-gsf) + 150.515f*(gsf-g_offset);  
;;;;  // look_f_pmax is in db
;;;;  // look_f_pmax is in mb
;;;; 
;;;; 
;;;; arguments
;;x14max     textequ <dword ptr [esp+4*(1+npush)]>
;;xgsf       textequ <dword ptr [esp+4*(2+npush)]>
;;
;;npush = 0
;;
;;    fld     xgsf
;;    fist    xgsf
;;
;;    fild    xgsf
;;    fsubp   st(1), st
;;    fld con_1100
;;    fmulp   st(1), st
;;
;;    mov eax, xgsf
;;    mov edx, _look_pigain[eax*4]
;;    mov xgsf, edx
;;
;;
;;    fld x14max
;;    fld xgsf         ;; igain
;;    fmulp   st(1), st
;;    fistp   xgsf
;;    mov edx, xgsf
;;
;;    sub eax, g_offset       ;; gsf - g_offset
;;    mov x14max, eax
;;
;;    fld con_150500
;;    fild x14max      ;; gsf - goffset
;;    fmulp   st(1), st
;;
;;    fld _look_f00_pmax[edx*4]
;;    faddp   st(1), st
;;
;;    faddp   st(1), st   ;; + 1.1*(xs - s)
;;
;;    fistp   xgsf
;;    mov eax, xgsf
;;
;;
;;    ret
;;_ixfnc_noise0P endp
;====================================
;====================================
_fnc_noise_actual proc public
;;
;;  log precision max eps = 0.01
;;
;;  float fnc_noise_actual(float x34[], float x[], 
;;              int gsf, int n, float logn)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
logn       textequ <dword ptr [esp+4*(5+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov eax, gsf
    fldz
    fld con_946
    fld _look_34igain[eax*4]
    fld _look_gain[eax*4]       ;; gain  igain  0.946  accum
    mov esi, x34
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(2)       
    fsub    st, st(3)
    fistp   tmp
    
    mov eax, tmp
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    st, st(1)       ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(4), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain
    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

    fstp    tmp      ;; accum to tmp for log
    dbLogMacro tmp      ;; uses eax edx ebx


;;    fld con_10log10_2 
;;    fxch
;;    fyl2x


;;-----
    fld logn
    fsubp   st(1), st



    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_fnc_noise_actual endp
;====================================
;====================================
_ifnc_noise_actual proc public
;;
;; integer result in milli-bells
;;
;;  log precision max eps = 0.01db
;;
;;  int noise_mb fnc_noise_actual(float x34[], float x[], 
;;              int gsf, int n, int logn_mb)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
logn       textequ <dword ptr [esp+4*(5+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov eax, gsf
    fldz
    fld con_946
    fld _look_34igain[eax*4]
    fld _look_gain[eax*4]       ;; gain  igain  0.0946  accum
    mov esi, x34
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(2)       
    fsub    st, st(3)
    fistp   tmp
    
    mov eax, tmp
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    st, st(1)       ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(4), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain
    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946
    fstp    tmp      ;; accum to tmp for log
;;--------
;;    mbLogMacro tmp      ;; uses eax edx ebx
;;    fistp   tmp
;;    mov eax, tmp
;;--
    mbLogMacroC tmp      ;; uses eax edx  result = eax
;;---------
    mov edx, logn       ;; let caller do this ??
    sub eax, edx


    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_noise_actual endp
;====================================
;====================================
_ifnc_noise_actualB proc public
;; 
;; uses optimum forward quant
;; integer result in milli-bells
;;
;;  log precision max eps = 0.01db
;;
;;  int noise_mb fnc_noise_actual(float x34[], float x[], 
;;              int gsf, int n, int logn_mb)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
logn       textequ <dword ptr [esp+4*(5+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov eax, gsf
    fldz
    fld con_4375
    fld _look_34igain[eax*4]
    fld _look_gain[eax*4]       ;; gain  igain  0.0946  accum
    mov esi, x34
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(2)       
    fsub    st, st(3)
    fist    tmp
;;
    mov eax, tmp
    sub eax, 31
    cdq
    and eax, edx
    add eax, 31
    fld quant_table[eax*4]
    fsubp   st(1), st
    fistp   tmp
;;
    mov eax, tmp
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    st, st(1)       ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(4), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain
    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946
    fstp    tmp      ;; accum to tmp for log

;;--------
;;    mbLogMacro tmp      ;; uses eax edx ebx
;;    fistp   tmp
;;    mov eax, tmp
;;--
    mbLogMacroC tmp      ;; uses eax edx  result = eax
;;---------

    mov edx, logn       ;; let caller do this ??
    sub eax, edx


    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_noise_actualB endp
;====================================
;====================================
_ifnc_noise_actual_ms proc public
;;
;;
;; computes noise in Left and Right chans
;; based on sum/diff data
;;
;; integer result in milli-bells
;;
;;  log precision max eps = 0.01db
;;
;;  int (noiseL, noiseR) ifnc_noise_actual_ms(float x34[2][576], 
;;              float x[2][576], int *sign_mask,
;;              int gsf0, int gsf1, int n, int logn_mb)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
sign_mask  textequ <dword ptr [esp+4*(3+npush)]>
gsf0       textequ <dword ptr [esp+4*(4+npush)]>
gsf1       textequ <dword ptr [esp+4*(5+npush)]>
n          textequ <dword ptr [esp+4*(6+npush)]>
logn       textequ <dword ptr [esp+4*(7+npush)]>


npush = 8
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4*4
;;-----
tmp       textequ <dword ptr [esp]>
tmp2      textequ <dword ptr [esp][4]>
gain0     textequ <dword ptr [esp][2*4]>
gain1     textequ <dword ptr [esp][3*4]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov edx, gsf1
    mov eax, gsf0
    fldz
    fldz    
    fld con_946
    fld _look_34igain[edx*4]
    fld _look_34igain[eax*4]
                ;; igain0 igain1  0.0946  accL, accR

    mov eax, _look_gain[eax*4]       
    mov edx, _look_gain[edx*4]       
    mov gain0, eax
    mov gain1, edx

    mov esi, x34
    mov edi, x
    mov ebx, sign_mask
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(3)
    fistp   tmp
    
    mov eax, tmp
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    gain0       ;; xhat0

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat0 - x0

    fstp    tmp2
    mov eax, [ebx]
    mov edx, tmp2
    xor edx, eax        ;; apply sign
    mov tmp2, edx
    
;;-----
    fld dword ptr [esi][4*576]   ;; x34
    fmul    st, st(2)       
    fsub    st, st(3)
    fistp   tmp
    
    mov eax, tmp
    cmp eax, 256
    jge big_pow43B

    fld _look_ix43[eax*4]
big_pow43_returnB:
    fmul    gain1       ;; xhat1

    fld dword ptr [edi][4*576]
    fsubp   st(1), st       ;; xhat1 - x1

;;-----
    fld st(0)
    fld tmp2
    fadd    st(1), st
    fsubrp  st(2), st

    fmul    st(0), st   
    faddp   st(5), st   ;; noise left

    fmul    st(0), st
    faddp   st(5), st   ;; noise right


    add esi, 4
    add edi, 4
    add ebx, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop igain0
    fstp    st(0)       ;; pop igain1
    fstp    st(0)       ;; pop 946

    fstp    tmp      ;; left accum to tmp for log
    mbLogMacro tmp      ;; uses eax edx ebx
    fistp   tmp

    fstp    tmp2      ;; right accum to tmp for log
    mbLogMacro tmp2      ;; uses eax edx ebx
    fistp   tmp2

;;-----
    mov ecx, logn
    add ecx, 301        ;; correct by log10(2)

    mov eax, tmp
    mov edx, tmp2

    sub eax, ecx
    sub edx, ecx


    add esp, 4*4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

big_pow43B:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_returnB

_ifnc_noise_actual_ms endp
;====================================
;====================================
_compute_sign_mask  PROC PUBLIC 
;;  void compute_sign_mask(char signs[2][576], int signmask[], int n)
;;
;; arguments
ixsign     textequ <dword ptr [esp+4*(1+npush)]>
sign_mask  textequ <dword ptr [esp+4*(2+npush)]>
n          textequ <dword ptr [esp+4*(3+npush)]>

npush = 3
     push   esi
     push   edi
     push   ecx

    mov esi, ixsign
    mov edi, sign_mask
    mov ecx, n


a100:
    mov eax, [esi]
    mov edx, [esi][576]
    xor eax, edx
    and eax, 01010101h      ;; should not be needed
    shl eax, 7
    mov [edi+3], al
    mov [edi+3][4], ah
    shr eax, 16
    mov [edi+3][2*4], al
    mov [edi+3][3*4], ah

    add esi, 4
    add edi, 4*4

    sub ecx, 4
    jg a100


    pop ecx
    pop edi
    pop esi


    ret

_compute_sign_mask  endp
;====================================
;====================================
;====================================
_fnc_noise_actual2 proc public
;;
;;  log precision max eps = 0.01
;;
;;  float fnc_noise_actual2(float x34[], float x[], 
;;              int gsf, int n, float logn, int sf)
;;  gsf = forward quant
;;   sf = dequant
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
logn       textequ <dword ptr [esp+4*(5+npush)]>
sf         textequ <dword ptr [esp+4*(6+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov eax, gsf
    mov ecx, sf
    fldz
    fld con_946
    fld _look_34igain[eax*4]
    fld _look_gain[ecx*4]       ;; gain  igain  0.946  accum
    mov esi, x34
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(2)       
    fsub    st, st(3)
    fistp   tmp
    
    mov eax, tmp
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    st, st(1)       ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(4), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain
    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

    fstp    tmp      ;; accum to tmp for log
    dbLogMacro tmp      ;; uses eax edx ebx


;;    fld con_10log10_2 
;;    fxch
;;    fyl2x


;;-----
    fld logn
    fsubp   st(1), st



    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_fnc_noise_actual2 endp
;====================================
;====================================
_ifnc_noise_actual2 proc public
;;
;;  log precision max eps = 0.01
;;
;;  int fnc_noise_actual2(float x34[], float x[], 
;;              int gsf, int n, int logn, int sf)
;;  gsf = forward quant
;;   sf = dequant
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
logn       textequ <dword ptr [esp+4*(5+npush)]>
sf         textequ <dword ptr [esp+4*(6+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov eax, gsf
    mov ecx, sf
    fldz
    fld con_946
    fld _look_34igain[eax*4]
    fld _look_gain[ecx*4]       ;; gain  igain  0.946  accum
    mov esi, x34
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(2)       
    fsub    st, st(3)
    fistp   tmp
    
    mov eax, tmp
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    st, st(1)       ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(4), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain
    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

    fstp    tmp      ;; accum to tmp for log
    mbLogMacro tmp      ;; uses eax edx ebx
;;-----
    fistp   tmp
    mov eax, tmp
    mov edx, logn       ;; let caller do this ??
    sub eax, edx


    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_noise_actual2 endp
;====================================
;====================================
_ifnc_noise_actualX2 proc public
;;
;; float forward quant value passed as arg
;;
;;  log precision max eps = 0.01
;;
;;  int fnc_noise_actual2(float x34[], float x[], 
;;              float igain34, int n, int logn, int sf)
;;igain = forward quant = look_34igain[gsf];
;;   sf = dequant
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
igain      textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
logn       textequ <dword ptr [esp+4*(5+npush)]>
sf         textequ <dword ptr [esp+4*(6+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov ecx, sf
    fldz
    fld con_946
    fld igain;
    fld _look_gain[ecx*4]       ;; gain  igain  0.946  accum
    mov esi, x34
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(2)       
    fsub    st, st(3)
    fistp   tmp
    
    mov eax, tmp
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    st, st(1)       ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(4), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain
    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

    fstp    tmp      ;; accum to tmp for log
    mbLogMacro tmp      ;; uses eax edx ebx
;;-----
    fistp   tmp
    mov eax, tmp
    mov edx, logn       ;; let caller do this ??
    sub eax, edx


    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_noise_actualX2 endp
;====================================
;====================================
_ifnc_ixnoise_actual proc public
;;
;;  log precision max eps = 0.01
;;
;;  int ifnc_ixnoise_actual(int ix[], float x[], 
;;              int sf, int n, int logn)
;;   sf = dequant
;;
;; arguments
ix        textequ <dword ptr [esp+4*(1+npush)]>
x         textequ <dword ptr [esp+4*(2+npush)]>
sf        textequ <dword ptr [esp+4*(3+npush)]>
n         textequ <dword ptr [esp+4*(4+npush)]>
logn      textequ <dword ptr [esp+4*(5+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

;; gain = look_gain[gsf];

    mov ecx, sf
    fldz
    fld _look_gain[ecx*4]       ;; gain  accum
    mov esi, ix
    mov edi, x
    mov ecx, n
a100:
    mov eax, [esi]
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:
    fmul    st, st(1)       ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(2), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain

    fstp    ix      ;; accum to ix for log
    mbLogMacro ix      ;; uses eax edx ebx
;;-----
    fistp    ix
    mov eax, ix
    mov edx, logn       ;; let caller do this ??
    sub eax, edx


    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_ixnoise_actual endp
;====================================
;====================================
_fnc_noise_actual_clip1 proc public
;;  forward quant clipped to 1
;;  log precision max eps = 0.01
;;
;;  float fnc_noise_actual(float x34[], float x[], 
;;              int gsf, int n, float logn)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
x          textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
logn       textequ <dword ptr [esp+4*(5+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>


;;igain = look_34igain[gsf];
;; gain = look_gain[gsf];

    mov eax, gsf
    fldz
    fld con_946
    fld _look_34igain[eax*4]    ;; igain  0.946  accum

    mov edx, _look_gain[eax*4]
    mov gain_0_1[4], edx

    mov esi, x34
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fistp   tmp
    
    mov eax, tmp 
    cmp eax, 1
    sbb edx, edx    ;; if(x==0) edx=-1, else edx = 0

    fld gain_0_1[4+edx*4]   ;; xhat

    fld dword ptr [edi]
    fsubp   st(1), st       ;; xhat - x

    fmul    st, st(0)
    faddp   st(3), st

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

    fstp    tmp      ;; accum to tmp for log
    dbLogMacro tmp      ;; uses eax edx ebx

;;    fld con_10log10_2 
;;    fxch
;;    fyl2x

;;-----
    fld logn
    fsubp   st(1), st

    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

_fnc_noise_actual_clip1 endp
;====================================
;====================================
_vect_quant0 proc public
;;
;;
;;  void vect_quant(float x34[], int ix[], 
;;              int gsf, int n)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 1
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    mov eax, gsf
    fld con_946
    fld _look_34igain[eax*4]   ;; igain  0.946  
    mov eax, x34
    mov edx, ix
    mov ecx, n
a100:
    fld     dword ptr [eax]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fistp   dword ptr [edx]
 
    add eax, 4
    add edx, 4

    dec ecx
    jg  a100

    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

;;---------------

    pop ecx


    ret
_vect_quant0 endp
;====================================
;====================================
_vect_quant proc public
;;
;;
;;  ixmax = vect_quant(float x34[], int ix[], 
;;              int gsf, int n)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    mov eax, gsf
    fld con_946
    fld _look_34igain[eax*4]   ;; igain  0.946  
    mov esi, x34
    mov edi, ix
    mov ecx, n
    xor eax, eax        ;; ixmax = 0
a100:
    fld     dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fistp   dword ptr [edi]

    add esi, 4

    mov ebx, [edi]
    sub eax, ebx
    mov edx, -1
    adc edx, 0
    and eax, edx
    add eax, ebx
 
    add edi, 4

    dec ecx
    jg  a100

    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

;;---------------

    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_vect_quant endp
;====================================
;====================================
_vect_quantB proc public
;;
;;  optimum quant
;;
;;  ixmax = vect_quant(float x34[], int ix[], 
;;              int gsf, int n)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    mov eax, gsf
    fld con_4375
    fld _look_34igain[eax*4]   ;; igain  0.4375  
    mov esi, x34
    mov edi, ix
    mov ecx, n
    xor eax, eax        ;; ixmax = 0
a100:
    fld     dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fist    dword ptr [edi]

    add esi, 4
;;
    mov ebx, [edi]
    sub ebx, 31
    sbb edx, edx
    and ebx, edx
    add ebx, 31
    fld quant_table[ebx*4]
    fsubp   st(1), st
    fistp   dword ptr [edi]   
;;

    mov ebx, [edi]
    sub eax, ebx
    mov edx, -1
    adc edx, 0
    and eax, edx
    add eax, ebx
 
    add edi, 4

    dec ecx
    jg  a100

    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 4375

;;---------------

    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_vect_quantB endp
;====================================
;====================================
_vect_quantB10x proc public
;;
;;  optimum quant at 10x actual (for thresholding)
;;  ixmax =  vect_quant(float x34[], int ix[], 
;;              int gsf, int n)
;;  is ixmax needed? used? (for peak thresholding maybe)
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    mov eax, gsf
    fld con_10
    fld con_4375
    fld _look_34igain[eax*4]   ;; igain  0.4375  10.0
    mov esi, x34
    mov edi, ix
    mov ecx, n
    xor eax, eax        ;; ixmax = 0
a100:
    fld     dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fist    dword ptr [edi]

    add esi, 4
;;
    mov ebx, [edi]
    sub ebx, 31
    sbb edx, edx
    and ebx, edx
    add ebx, 31
    fld quant_table[ebx*4]
    fsubp   st(1), st
    fmul    st, st(3)
    fistp   dword ptr [edi]   
;;

    mov ebx, [edi]


    sub eax, ebx      ;; must handle negative values
    cdq 
    not edx
    and eax, edx
    add eax, ebx
 
    add edi, 4

    dec ecx
    jg  a100

    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 4375
    fstp    st(0)       ;; pop 10.0

;;---------------

    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_vect_quantB10x endp
;====================================
;====================================
_vect_ixmax_quantB proc public
;;
;;  optimum quant
;;
;;  void vect_quant(float x34max[], int ixmax[], 
;;              int gsf[], int n)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    fld con_4375
    mov ecx, n
    mov eax, gsf
    mov esi, x34
    mov edi, ix
    test ecx, ecx
    jle exit

a100:
    mov ebx, [eax]          ;; gsf
    fld _look_34igain[ebx*4]   ;; igain  0.4375  

    fld     dword ptr [esi]     ;; x34max
    fmulp   st(1), st       
    fsub    st, st(1)
    fist    dword ptr [edi]

    add eax, 4
    add esi, 4
;;
    mov ebx, [edi]
    sub ebx, 31
    sbb edx, edx
    and ebx, edx
    add ebx, 31
    fld quant_table[ebx*4]
    fsubp   st(1), st
    fistp   dword ptr [edi]   
;;
    add edi, 4

    dec ecx
    jg  a100

exit:
    fstp    st(0)       ;; pop 4375

;;---------------
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_vect_ixmax_quantB endp
;====================================
;====================================
_vect_ix10xmax_quantB proc public
;;
;;  optimum quant
;;
;;  void vect_quant(float x34max[], int ixmax[], 
;;              int gsf[], int n)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    fld con_10
    fld con_4375
    mov ecx, n
    mov eax, gsf
    mov esi, x34
    mov edi, ix
    test ecx, ecx
    jle exit

a100:
    mov ebx, [eax]          ;; gsf
    fld _look_34igain[ebx*4]   ;; igain  0.4375  10.0

    fld     dword ptr [esi]     ;; x34max
    fmulp   st(1), st       
    fsub    st, st(1)
    fist    dword ptr [edi]

    add eax, 4
    add esi, 4
;;
    mov ebx, [edi]
    sub ebx, 31
    sbb edx, edx
    and ebx, edx
    add ebx, 31
    fld quant_table[ebx*4]
    fsubp   st(1), st
    fmul    st, st(2)
    fistp   dword ptr [edi]   
;;
    add edi, 4

    dec ecx
    jg  a100

exit:
    fstp    st(0)       ;; pop 4375
    fstp    st(0)       ;; pop 10.0

;;---------------
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_vect_ix10xmax_quantB endp
;====================================
;====================================
_vect_quantB2 proc public
;;
;;  optimum quant - 0-1 threshold factor passed as arg
;;
;;  ixmax = vect_quant(float x34[], int ix[], 
;;              int gsf, int n, float qadjust)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
qadjust    textequ <dword ptr [esp+4*(5+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    push    quant_table
;;-----


;;igain = look_34igain[gsf];
    mov eax, qadjust
    mov quant_table, eax



    mov eax, gsf
    fld con_4375
    fld _look_34igain[eax*4]   ;; igain  0.4375  
    mov esi, x34
    mov edi, ix
    mov ecx, n
    xor eax, eax        ;; ixmax = 0
a100:
    fld     dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fist    dword ptr [edi]

    add esi, 4
;;
    mov ebx, [edi]
    sub ebx, 31
    sbb edx, edx
    and ebx, edx
    add ebx, 31
    fld quant_table[ebx*4]
    fsubp   st(1), st
    fistp   dword ptr [edi]   
;;

    mov ebx, [edi]
    sub eax, ebx
    mov edx, -1
    adc edx, 0
    and eax, edx
    add eax, ebx
 
    add edi, 4

    dec ecx
    jg  a100

    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 4375

;;---------------

    pop quant_table
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_vect_quantB2 endp
;====================================
;====================================
_vect_quantX proc public
;;
;;  
;;  ixmax = vect_quant(float x34[], int ix[], 
;;              float igain34, int n)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
igain      textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    fld con_946
    fld igain   ;; igain  0.946  
    mov esi, x34
    mov edi, ix
    mov ecx, n
    xor eax, eax        ;; ixmax = 0
a100:
    fld     dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fistp   dword ptr [edi]

    add esi, 4

    mov ebx, [edi]
    sub eax, ebx
    mov edx, -1
    adc edx, 0
    and eax, edx
    add eax, ebx
 
    add edi, 4

    dec ecx
    jg  a100

    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

;;---------------

    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_vect_quantX endp
;====================================
;====================================
_vect_quant2 proc public
;;
;;
;;  ixmax = vect_quant(float x34[], int ix[], 
;;              int gsf, int n, int gzero)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
gzero      textequ <dword ptr [esp+4*(5+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];

    mov ebx, gsf
    mov eax, gzero
    sub eax, ebx
    jle zero_quant

;;    sub eax, 16
;;    cdq
;;    and eax, edx
;;    add eax, 16
;;    fld con_946_table[eax*4]

    fld con_946


    fld _look_34igain[ebx*4]   ;; igain  0.946  
    mov esi, x34
    mov edi, ix
    mov ecx, n
    xor eax, eax        ;; ixmax = 0
a100:
    fld     dword ptr [esi]     ;; x34
    fmul    st, st(1)       
    fsub    st, st(2)
    fistp   dword ptr [edi]

    add esi, 4

    mov ebx, [edi]
    sub eax, ebx
    mov edx, -1
    adc edx, 0
    and eax, edx
    add eax, ebx
 
    add edi, 4

    dec ecx
    jg  a100

    fstp    st(0)       ;; pop igain
    fstp    st(0)       ;; pop 946

;;---------------
exit:

    pop ecx
    pop ebx
    pop edi
    pop esi
    ret

zero_quant:
    mov edi, ix
    mov ecx, n
    xor eax, eax        ;; ixmax
b100:
    mov dword ptr [edi], 0
    add edi, 4
    dec ecx
    jg  b100

    jmp exit

    ret
_vect_quant2 endp
;====================================
;====================================
;====================================
_vect_quant_clip1 proc public
;;
;;  clip to 1
;;  ixmax =  vect_quant_clip1(float x34[], int ix[], 
;;              int gsf, int n)
;;  n even!
;;
;;  fpu is faster for small n
;;  this function intended for last cb (n=76 at 44.1kHz)
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    push    ebp
;;-----


;;igain = look_34igain[gsf];
;; compute with integer unit by thresholding

    mov eax, gsf
    fld con_5946
    fld _look_34igain[eax*4]
    fdivp   st(1), st       ;; about 30 cycles
    fstp    gsf     ;; threshold
    mov esi, x34
    mov edi, ix
    mov ecx, n
    mov ebx, gsf        ;; threshold
    xor ebp, ebp        ;; ixmax
a100:
    mov     eax, [esi]        ;; x34
    sub eax, ebx
    cdq
    inc edx
    mov     [edi], edx
    or  ebp, edx    ;; ixmax

    mov     eax, [esi][4]     ;; x34
    add esi, 2*4
    sub eax, ebx
    cdq
    inc edx
    mov     [edi][4], edx
    or  ebp, edx    ;; ixmax

    add edi, 2*4

    sub ecx, 2
    jg  a100

;;--------------
    mov eax, ebp        ;; return ixmax

    pop ebp
    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
_vect_quant_clip1 endp
;====================================
;====================================
_shave_01pairs proc public
;;
;;
;;  void shave_01pairs(float x34[], int ix[], int g, int n, float thres)
;;
;;
;; arguments
x34        textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
gsf        textequ <dword ptr [esp+4*(3+npush)]>
n          textequ <dword ptr [esp+4*(4+npush)]>
thres      textequ <dword ptr [esp+4*(5+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----


;;igain = look_34igain[gsf];
;;
;;
;; compute threshold   t = thres/look_34igain[g];
;;
    mov eax, gsf
    fld thres
    fld _look_34igain[eax*4]
    fdiv    
    fstp    thres
    mov ebx, thres


    mov edi, ix
    mov esi, x34
    mov ecx, n
a100:
    mov eax, [edi]
    mov edx, [edi+4]
    add eax, edx
    cmp eax, 1
    je  b100
    add edi, 2*4
    add esi, 2*4
    sub ecx, 2
    jg  a100
    jmp exit



b100:
    mov eax, [esi]
    sub eax, ebx
    cdq
    not edx     ;; threshold mask
    mov eax, [edi]
    and eax, edx
    mov [edi], eax

    mov eax, [esi+4]
    sub eax, ebx
    cdq
    not edx     ;; threshold mask
    mov eax, [edi+4]
    and eax, edx
    mov [edi+4], eax


    add edi, 2*4
    add esi, 2*4
    sub ecx, 2
    jg  a100

exit:
;;---------------
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret
_shave_01pairs endp
;====================================
;====================================
_fnc_ms_process proc public
;;
;; basic ms process only w/sign,  w/sqrt(2)
;;	xr --> sum/diff
;;
;;    void fnc_ms_process(float xr[2][576], int n, 
;;                            char ixsign[2][576])
;;
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
ixsign    textequ <dword ptr [esp+4*(3+npush)]>

npush = 3
    push    esi
    push    edi
    push    ecx
;;-----

    fld con_707

	mov	esi, xr
	mov edi, ixsign
	mov	ecx, n

a100:
	fld	dword ptr [esi]         ;; left
	fld st(0)                   ;; left

	fld	dword ptr [esi][4*576]  ;; right

	fsub    st(1), st           ;; diff
	faddp   st(2), st           ;; sum

    fmul    st(0), st(2)
	fstp    dword ptr [esi][4*576]  ;; store diff

    fmul    st(0), st(1)
	fstp    dword ptr [esi]     ;; store sum


;;----
    mov eax, [esi][4*576]   ;; diff 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [esi][4*576], eax    ;; store fabs(diff)
    mov [edi][576], dl      ;; diff sign
;;----
    mov eax, [esi]          ;; sum 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [esi], eax          ;; store fabs(sum)
    mov [edi], dl           ;; sum sign


	add	esi, 4
	inc edi

	dec	ecx
	jne	a100


;;----------
exit:
    
    fstp    st(0)


    pop ecx
    pop edi
    pop esi

    ret

;;---------

_fnc_ms_process endp
;====================================
;====================================
_fnc_ms_process2 proc public
;;
;; basic ms process only w/sign,  NO sqrt(2) scaling!!
;;	xr --> sum/diff
;;
;;    void fnc_ms_process(float xr[2][576], int n, 
;;                            char ixsign[2][576])
;;
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
ixsign    textequ <dword ptr [esp+4*(3+npush)]>

npush = 3
    push    esi
    push    edi
    push    ecx
;;-----


	mov	esi, xr
	mov edi, ixsign
	mov	ecx, n

a100:
	fld	dword ptr [esi]         ;; left
	fld st(0)                   ;; left

	fld	dword ptr [esi][4*576]  ;; right

	fsub    st(1), st           ;; diff
	faddp   st(2), st           ;; sum

	fstp    dword ptr [esi][4*576]  ;; store diff
	fstp    dword ptr [esi]     ;; store sum
;;----
    mov eax, [esi][4*576]   ;; diff 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [esi][4*576], eax    ;; store fabs(diff)
    mov [edi][576], dl      ;; diff sign
;;----
    mov eax, [esi]          ;; sum 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [esi], eax          ;; store fabs(sum)
    mov [edi], dl           ;; sum sign


	add	esi, 4
	inc edi

	dec	ecx
	jne	a100


;;----------
exit:
    

    pop ecx
    pop edi
    pop esi

    ret

;;---------

_fnc_ms_process2 endp
;====================================
;====================================
_fnc_ms_sparse proc public
;;
;; basic ms process sparse only
;;
;;    void fnc_ms_sparse(float xr[2][576], int n, int thres)
;;  n even - processed in pairs
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
thres     textequ <dword ptr [esp+4*(3+npush)]>

;; reuse args for scratch
tmptot        textequ <dword ptr [esp+4*(1+npush)]>
tmpdiff       textequ <dword ptr [esp+4*(2+npush)]>

npush = 2
    push    esi
    push    ecx
;;-----

    fld thres

	mov	esi, xr
	mov	ecx, n

a100:
	fld	dword ptr [esi][4*576]  ;; diff
    fmul    st(0), st

	fld	dword ptr [esi]         ;; sum
    fmul    st(0), st

	fld	dword ptr [esi][4*576][4]  ;; diff
    fmul    st(0), st
    faddp   st(2), st

	fld	dword ptr [esi][4]      ;; sum
    fmul    st(0), st
    faddp   st(1), st


    fadd    st, st(1)           ;; tot
    fmul    st, st(2)           ;; thres*tot
    fstp    tmptot

    fstp    tmpdiff

;; gen mask
    mov eax, tmptot
    mov edx, tmpdiff
    sub eax, edx
    cdq

	mov eax, [esi][4*576]
    and eax, edx
	mov [esi][4*576], eax

	mov eax, [esi][4*576][4]
    and eax, edx
	mov [esi][4*576][4], eax
;;----
	add	esi, 2*4

	sub ecx, 2
	jg  a100
;;----------
    fstp    st(0)       ;; pop thres off stack
    pop ecx
    pop esi

    ret

;;---------

_fnc_ms_sparse endp
;====================================
;====================================
_fnc_ms_sparse_sum proc public
;;
;; basic ms process sparse only
;;
;;    void fnc_ms_sparse(float xr[2][576], int n, int thres)
;;  n even - processed in pairs
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
thres     textequ <dword ptr [esp+4*(3+npush)]>

;; reuse args for scratch
tmptot        textequ <dword ptr [esp+4*(1+npush)]>
tmpdiff       textequ <dword ptr [esp+4*(2+npush)]>

npush = 2
    push    esi
    push    ecx
;;-----

    fld thres

	mov	esi, xr
	mov	ecx, n

a100:
	fld	dword ptr [esi] ;; sum
    fmul    st(0), st

	fld	dword ptr [esi][4*576]         ;; diff
    fmul    st(0), st

	fld	dword ptr [esi][4]  ;; sum
    fmul    st(0), st
    faddp   st(2), st

	fld	dword ptr [esi][4*576][4]      ;; diff
    fmul    st(0), st
    faddp   st(1), st


    fadd    st, st(1)           ;; tot
    fmul    st, st(2)           ;; thres*tot
    fstp    tmptot

    fstp    tmpdiff

;; gen mask
    mov eax, tmptot
    mov edx, tmpdiff
    sub eax, edx
    cdq

	mov eax, [esi]
    and eax, edx
	mov [esi], eax

	mov eax, [esi][4]
    and eax, edx
	mov [esi][4], eax
;;----
	add	esi, 2*4

	sub ecx, 2
	jg  a100
;;----------
    fstp    st(0)       ;; pop thres off stack
    pop ecx
    pop esi

    ret

;;---------

_fnc_ms_sparse_sum endp
;====================================
;====================================
_fnc_ms_sparse1 proc public
;;
;; basic ms process sparse only
;; sparse if x1 < thres*x0  not energy pairs
;;
;;    void fnc_ms_sparse1(float xr[2][576], int n, int thres)
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
thres     textequ <dword ptr [esp+4*(3+npush)]>

;; reuse args for scratch
tmp        textequ <dword ptr [esp+4*(1+npush)]>

npush = 3
    push    ebx
    push    esi
    push    ecx
;;-----

    fld thres

	mov	esi, xr
	mov	ecx, n

a100:
	fld	dword ptr [esi]  ;; sum
    fmul    st, st(1)    ; thres*sum
    fstp    tmp

;; gen mask
    mov eax, tmp
    mov ebx, [esi][4*576]
    sub eax, ebx
    cdq
    and ebx, edx
	mov [esi][4*576], ebx

;;----
	add	esi, 4

	dec ecx
	jg  a100
;;----------
    fstp    st(0)       ;; pop thres off stack
    pop ecx
    pop esi
    pop ebx

    ret

;;---------

_fnc_ms_sparse1 endp
;====================================
;====================================
_fnc_sxx proc public
;;
;; 
;;
;;    void fnc_ms_sparse(float xr[2][576], int n, float *sxx)
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
sxx       textequ <dword ptr [esp+4*(3+npush)]>

npush = 2
    push    esi
    push    ecx
;;-----

	mov	esi, xr
	mov	ecx, n

    fldz    
    fldz
a100:
	fld	dword ptr [esi]     ;; x
    fmul    st(0), st       ;; xx
    faddp   st(1), st       ;; sxx

	fld	dword ptr [esi][4*576]  ;; y
    fmul    st(0), st           ;; yy
    faddp   st(2), st           ;; syy

;;---------
	add	esi, 4

	dec ecx
	jg  a100
;;----------

    mov esi, sxx
    fstp    dword ptr [esi]
    fstp    dword ptr [esi][4]
;;

    pop ecx
    pop esi

    ret

;;---------

_fnc_sxx endp
;====================================
;====================================
_fnc_ms_process_sparse proc public
;;
;; NOT scaled with sqrt(2), must compensate with g
;;	xr --> sum/diff
;;
;;    void fnc_ms_process_sparse(float xr[2][576], int n, 
;;                            float thres, 
;;                            char ixsign[2][576], 
;;                            float sxx[4])
;;
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
thres     textequ <dword ptr [esp+4*(3+npush)]>
ixsign    textequ <dword ptr [esp+4*(4+npush)]>
sxx       textequ <dword ptr [esp+4*(5+npush)]>

npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    fldz
    fldz
    fldz
    fldz

	fld	dword ptr thres

	mov	esi, xr
	mov edi, ixsign
	mov	ecx, n

a100:
	fld	dword ptr [esi]         ;; left
	fld	st(0)				
    fmul    st, st(0)
    faddp   st(3), st           ;;  sxx left

	fld	dword ptr [esi][4*576]  ;; right
	fld	st(0)				
    fmul    st, st(0)
    faddp   st(5), st           ;; sxx right

    fld st(1)
    fxch                        ;; xr xl xl

	fsub    st(1), st           ;; diff
	faddp   st(2), st           ;; sum

	fstp	dword ptr [esi][4*576]  ;; store diff

    fld st(0)
    fmul    st, st(0)
    faddp   st(5), st           ;; sxx sum

	fst 	dword ptr [esi]         ;; store sum

    fmul    st, st(1)
    fabs
    fstp    thres           ;; sum*thres
;;----
    mov eax, [esi]          ;; sum 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [esi], eax          ;; store sum
    mov [edi], dl           ;; sum sign
;;----
    mov eax, [esi][4*576]   ;; diff 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [edi][576], dl      ;; diff sign, don't care if sparsed to 0
;;----  eax = fabs(diff)
    mov edx, thres          ;; thres*sum
    sub edx, eax            ;; thres-diff, edx>0 if thres > diff
    sar edx, 31
    and eax, edx
    mov [esi][4*576], eax   ;; store diff 

    fld dword ptr [esi][4*576]
    fmul st, st(0)
    faddp   st(5), st       ;; sxx diff

	add	esi, 4
	inc edi

	dec	ecx
	jne	a100


;;----------
exit:
    fstp    st(0)   ;; thres pop fpu stack
    
;; store sxx
    mov edi, sxx
    fld con_05
    fmul    st(3), st
    fmulp   st(4), st
    fstp dword ptr [edi][0*4]   ;; sxx left
    fstp dword ptr [edi][1*4]   ;; sxx right
    fstp dword ptr [edi][2*4]   ;; sxx sum
    fstp dword ptr [edi][3*4]   ;; sxx diff




    pop ecx
    pop ebx
    pop edi
    pop esi

    ret

;;---------

_fnc_ms_process_sparse endp
;====================================
;====================================
_fnc_ms_process_sparse2 proc public
;;
;; with 1/sqrt(2) scaling
;;	xr --> sum/diff
;;
;;    void fnc_ms_process_sparse2(float xr[2][576], int n, 
;;                            float thres, 
;;                            char ixsign[2][576], 
;;                            float sxx[4])
;;
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
n         textequ <dword ptr [esp+4*(2+npush)]>
thres     textequ <dword ptr [esp+4*(3+npush)]>
ixsign    textequ <dword ptr [esp+4*(4+npush)]>
sxx       textequ <dword ptr [esp+4*(5+npush)]>

npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    fldz
    fldz
    fldz
    fldz

	fld	dword ptr thres

	mov	esi, xr
	mov edi, ixsign
	mov	ecx, n

a100:
	fld	dword ptr [esi]         ;; left
	fld	st(0)				
    fmul    st, st(0)
    faddp   st(3), st           ;;  sxx left

	fld	dword ptr [esi][4*576]  ;; right
	fld	st(0)				
    fmul    st, st(0)
    faddp   st(5), st           ;; sxx right

    fld st(1)
    fxch                        ;; xr xl xl

	fsub    st(1), st           ;; diff
	faddp   st(2), st           ;; sum

    fld con_707
    fmul    st(1), st
    fmulp   st(2), st


	fstp	dword ptr [esi][4*576]  ;; store diff

    fld st(0)
    fmul    st, st(0)
    faddp   st(5), st           ;; sxx sum

	fst 	dword ptr [esi]         ;; store sum

    fmul    st, st(1)
    fabs
    fstp    thres           ;; sum*thres
;;----
    mov eax, [esi]          ;; sum 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [esi], eax          ;; store sum
    mov [edi], dl           ;; sum sign
;;----
    mov eax, [esi][4*576]   ;; diff 
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    shr edx, 31
    mov [edi][576], dl      ;; diff sign, don't care if sparsed to 0
;;----  eax = fabs(diff)
    mov edx, thres          ;; thres*sum
    sub edx, eax            ;; thres-diff, edx>0 if thres > diff
    sar edx, 31
    and eax, edx
    mov [esi][4*576], eax   ;; store diff 

    fld dword ptr [esi][4*576]
    fmul st, st(0)
    faddp   st(5), st       ;; sxx diff

	add	esi, 4
	inc edi

	dec	ecx
	jne	a100


;;----------
exit:
    fstp    st(0)   ;; thres pop fpu stack
    
;; store sxx
    mov edi, sxx
    fstp dword ptr [edi][0*4]   ;; sxx left
    fstp dword ptr [edi][1*4]   ;; sxx right
    fstp dword ptr [edi][2*4]   ;; sxx sum
    fstp dword ptr [edi][3*4]   ;; sxx diff




    pop ecx
    pop ebx
    pop edi
    pop esi

    ret

;;---------

_fnc_ms_process_sparse2 endp
;====================================
;====================================
_fnc_ms_sign_sxx proc public

;;
;; void fnc(float x[2][576], float xms[2][576], 
;;              char ms_sign[2][576], int sign_mask[576],
;;               int n);
;;
;;
;; arguments
xin       textequ <dword ptr [esp+4*(1+npush)]>
xms       textequ <dword ptr [esp+4*(2+npush)]>
ms_sign   textequ <dword ptr [esp+4*(3+npush)]>
sign_mask textequ <dword ptr [esp+4*(4+npush)]>
n         textequ <dword ptr [esp+4*(5+npush)]>


npush = 5
    push    ebp
    push    esi
    push    edi
    push    ebx
    push    ecx

    mov esi, xin
    mov edi, xms
    mov ebx, ms_sign
    mov ecx, sign_mask


a100:

;;-----
    mov eax, [edi]          ;; sum chan
    mov edx, eax
    and eax, 7FFFFFFFh      ;; fabs
    mov [edi], eax          ;; store fabs(sum)
    and edx, 80000000h      ;; isolate sign of sum
;; mult left/right by sign_sum
    mov eax, [esi]          ;; left
    xor eax, edx
    mov [esi], eax
    mov eax, [esi][4*576]   ;; right
    xor eax, edx
    mov [esi][4*576], eax
;;
    mov ebp, [edi][4*576]   ;; diff chan
    mov edx, ebp
    and ebp, 7FFFFFFFh      ;; fabs
    mov [edi][4*576], ebp   ;; store fabs(diff)

    mov ebp, edx
    xor ebp, eax            ;; sign_sum*sign_diff
    and ebp, 80000000h      ;; isolate sign
    mov [ecx], ebp          ;; store sign mask

    shr eax, 31
    shr edx, 31
    mov [ebx], al           ;; store signs of sum diff
    mov [ebx][576], dl

    add esi, 4
    add edi, 4
    inc ebx
    add ecx, 4

    dec n
    jg  a100
;;---------

    pop ecx
    pop ebx
    pop edi
    pop esi
    pop ebp

    ret
_fnc_ms_sign_sxx endp
;====================================
;====================================
_ifnc_invq_noise_msLR proc public
;;
;;
;; computes noise total in Left and Right chans - result in mb
;; uses ix forward quant values 
;; based on Left/Right data
;; xrLR must have proper sign!!!
;;  gsf = inverse quant values
;;
;; integer result in milli-bells
;;
;;  log precision max eps = 0.01db
;;
;;  int (noiseLR) _ifnc_invq_noise_msLR(float xrLR[2][576], 
;;              int  ix[2][576], char signs[2][576]
;;              int gsf0, int gsf1, int n, int logn_mb)
;;
;;
;; arguments
xrLR       textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
sign       textequ <dword ptr [esp+4*(3+npush)]>
gsf0       textequ <dword ptr [esp+4*(4+npush)]>
gsf1       textequ <dword ptr [esp+4*(5+npush)]>
n          textequ <dword ptr [esp+4*(6+npush)]>
logn       textequ <dword ptr [esp+4*(7+npush)]>


npush = 6
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 2*4
;;-----
tmp       textequ <dword ptr [esp]>
tmp2      textequ <dword ptr [esp][4]>


;; gain = look_gain[gsf];

    mov edx, gsf1
    mov eax, gsf0
    fldz    
    fld _look_gain[edx*4 - 2*4]     ;; scale down gain by sqrt(2)
    fld _look_gain[eax*4 - 2*4]
                ;; gain0 gain1 accLR


    mov esi, xrLR
    mov edi, ix
    mov ebx, sign
    mov ecx, n
a100:
    mov eax, [edi]      ;; ix0
    cmp eax, 256
    jge big_pow43
    mov eax, _look_ix43[eax*4]  ;; eax = float value
big_pow43_return:
    movzx   edx, byte ptr [ebx]     ;; get sign
    shl edx, 31
    or  eax, edx
    mov tmp, eax
    fld tmp
    fmul    st, st(1)       ;; mul by gain -> x0hat
    fld st(0)               ;; dup x0hat
;;---
    mov eax, [edi][4*576]  ;; ix1
    cmp eax, 256
    jge big_pow43B
    mov eax, _look_ix43[eax*4]
big_pow43_returnB:
    movzx   edx, byte ptr [ebx][576]     ;; get sign
    shl edx, 31
    or  eax, edx
    mov tmp2, eax
    fld tmp2
    fmul    st, st(4)       ;; mul by gain -> x1hat
;;-------
    fadd    st(1), st       ;; xLhat
    fsubp   st(2), st       ;; xRhat
;;
    fld dword ptr [esi]     ;; xL
    fsubp   st(1), st
    fmul    st, st(0)       ;; noiseL
    faddp   st(4), st       ;; accum noise

    fld dword ptr [esi][4*576]     ;; xR
    fsubp   st(1), st
    fmul    st, st(0)       ;; noiseL
    faddp   st(3), st       ;; accum noise
;;-----
;;       


    add edi, 4
    inc ebx
    add esi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain0
    fstp    st(0)       ;; pop gain1

    fstp    tmp      ;; accum to tmp for log
    mbLogMacro tmp      ;; uses eax edx ebx
    fistp   tmp

;;-----
    mov ecx, logn
    mov eax, tmp
    sub eax, ecx

    add esp, 2*4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in eax, return will be in eax
    ;; tmp must be [esp]!
    mov tmp, eax
    call _pow43ix
    fstp   tmp
    mov eax, tmp
    jmp big_pow43_return

big_pow43B:
    ;; arg in eax, return will be in eax
    mov tmp, eax
    call _pow43ix
    fstp   tmp
    mov eax, tmp
    jmp big_pow43_returnB

_ifnc_invq_noise_msLR endp
;====================================
;====================================
_fnc_test_qbias proc public
;;
;;
;; uses ix forward quant values 
;;
;;  float _ifnc_test_qbias(float xr[576], 
;;              int  ix[576], gsf, int n)
;;
;;
;; arguments
xr       textequ <dword ptr [esp+4*(1+npush)]>
ix       textequ <dword ptr [esp+4*(2+npush)]>
gsf      textequ <dword ptr [esp+4*(3+npush)]>
n        textequ <dword ptr [esp+4*(4+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>
;;-----

    mov eax, gsf
    fldz    
    fld _look_gain[eax*4]
                ;; gain acc
    mov esi, xr
    mov edi, ix
    mov ecx, n
a100:
    mov eax, [edi]      ;; ix
    cmp eax, 256
    jge big_pow43
    fld _look_ix43[eax*4]  ;; eax = float value
big_pow43_return:
    fmul    st, st(1)       ;; mul by gain -> xhat

    fld dword ptr [esi]     ;; x
    fsubrp   st(1), st
    faddp   st(2), st       ;; accum noise

;;-----

    add edi, 4
    add esi, 4

    dec ecx
    jg  a100

    
;;-----
    fstp    st(0)       ;; pop gain

    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    ;; arg in eax, return will be in st
    ;; tmp must be [esp]!
    mov tmp, eax
    call _pow43ix
    jmp big_pow43_return


_fnc_test_qbias endp
;====================================
;====================================
_map_xform02 proc public
;;
;;void map_xform(float xr[], float rr[])
;; map transform data to FFT energy bins
;;
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
rr        textequ <dword ptr [esp+4*(2+npush)]>
n         textequ <dword ptr [esp+4*(3+npush)]>


npush = 2
    push    esi
    push    edi
;;-----

    mov esi, xr
    mov edi, rr
    mov eax, n

ka = 0
kb = 7

a100:
rept 8
    fld     dword ptr [esi+4*ka]
    fmul    st, st(0)
    fld     dword ptr [esi+4*ka+4]
    fmul    st, st(0)
    fld     map_table[4*ka]
    fmulp   st(2), st
    fld     map_table[4*kb]
    fmulp   st(1), st
    faddp   st(1), st
    fstp    dword ptr [edi+4*ka]
    ka = ka + 1
    kb = kb - 1
endm
    add esi, 9*4    ;; this hiccups 
    add edi, 8*4
    sub eax, 8
    jg a100

;;---------------

    pop edi
    pop esi


    ret
_map_xform02 endp
;====================================
;====================================
_map_xform proc public
;;
;;void map_xform(float xr[], float rr[])
;; map transform data to FFT energy bins
;;
;;
;; arguments
xr        textequ <dword ptr [esp+4*(1+npush)]>
rr        textequ <dword ptr [esp+4*(2+npush)]>
n         textequ <dword ptr [esp+4*(3+npush)]>


npush = 2
    push    esi
    push    edi
;;-----

    mov esi, xr
    mov edi, rr
    mov eax, n

ka = 0
kb = 7

a100:
    fld     dword ptr [esi+4*ka]
    fmul    st, st(0)
    fld     dword ptr [esi+4*ka+4]
    fmul    st, st(0)
    ;;fld     map_table[4*ka]  ;; map_table[0] == 1.0
    ;;fmulp   st(2), st
    fld     map_table[4*kb]
    fmul    st, st(1)
    faddp   st(2), st
    fxch
    fstp    dword ptr [edi+4*ka]
    ka = ka + 1
    kb = kb - 1
rept 6
    fld     dword ptr [esi+4*ka+4]
    fmul    st, st(0)
    fld     map_table[4*ka]
    fmulp   st(2), st
    fld     map_table[4*kb]
    fmul    st, st(1)
    faddp   st(2), st
    fxch
    fstp    dword ptr [edi+4*ka]
    ka = ka + 1
    kb = kb - 1
endm
    fld     dword ptr [esi+4*ka+4]
    fmul    st, st(0)
    fld     map_table[4*ka]
    fmulp   st(2), st
    ;;fld     map_table[4*kb]   ;; map_table[0] == 1.0
    ;;fmul    st, st(1)
    faddp   st(1), st
    fstp    dword ptr [edi+4*ka]
    ka = ka + 1
    kb = kb - 1


    add esi, 9*4    ;; this hiccups 
    add edi, 8*4
    sub eax, 8
    jg a100

;;---------------

    pop edi
    pop esi


    ret
_map_xform endp
;====================================
;====================================
_ifnc_inverse_gsf_xfer proc public
;;
;;   do not call if ixmax == 0!
;;
;;
;;  inverse g for best xfer function
;;  int g = ifnc_inverse_gsf(int ix[], float x[], int n)
;;
;; arguments
ix        textequ <dword ptr [esp+4*(1+npush)]>
x         textequ <dword ptr [esp+4*(2+npush)]>
n         textequ <dword ptr [esp+4*(3+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>

    fld con_2
    fldz
    fldz
    mov esi, ix
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [edi]
    fmul st(0), st
    mov eax, [esi]
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:

    fmul st(0), st  ;; q*q 
    faddp   st(3), st
    faddp   st(1), st   ;;  sxx, sqq

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fdivrp  st(1), st   ;; use mbLoq twice?
    fyl2x               ;; use mbLog?
;;-----
    fistp    ix
    mov eax, ix
    add eax, 8    ;; add g-offset

    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    mov tmp, eax
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_inverse_gsf_xfer endp
;====================================
_ifnc_inverse_gsf_snr proc public
;;
;;   do not call if ixmax == 0!
;;
;;  inverse g for best snr
;;  int g = ifnc_inverse_gsf(int ix[], float x[], int n)
;;
;; arguments
ix        textequ <dword ptr [esp+4*(1+npush)]>
x         textequ <dword ptr [esp+4*(2+npush)]>
n         textequ <dword ptr [esp+4*(3+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>

    fld con_4
    fldz
    fldz
    mov esi, ix
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [edi]
    mov eax, [esi]
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:

    fmul st(1), st  ;; q*x
    fmul st(0), st  ;; q*q
    faddp   st(3), st
    faddp   st(1), st   ;;  sxq, sqq

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

    
;;-----
    fdivrp  st(1), st   ;; use mbLoq twice?
    fyl2x               ;; use mbLog?
;;-----
    fistp    ix
    mov eax, ix
    add eax,  8    ;; add g-offset

    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    mov tmp, eax
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_inverse_gsf_snr endp
;====================================
;====================================
_ifnc_inverse_gsf_snr2 proc public
;;
;;   do not call if ixmax == 0!
;;
;;  inverse g for best snr  g scaled by 8*1024
;;  int g = ifnc_inverse_gsf(int ix[], float x[], int n)
;;
;; arguments
ix        textequ <dword ptr [esp+4*(1+npush)]>
x         textequ <dword ptr [esp+4*(2+npush)]>
n         textequ <dword ptr [esp+4*(3+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>

;;    fld con_4
    fldz
    fldz
    mov esi, ix
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [edi]
    mov eax, [esi]
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:

    fmul st(1), st  ;; q*x
    fmul st(0), st  ;; q*q
    faddp   st(3), st
    faddp   st(1), st   ;;  sxq, sqq

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

;;-----
;;    fdivrp  st(1), st   ;; use mbLoq twice?
;;    fyl2x               ;; use mbLog?
;;    fistp    ix
;;    mov eax, ix
;;    add eax,  8    ;; add g-offset
;;-----
    fstp ix     ;; sxq
    fstp x      ;; sqq
    mbLogMacroC  x
    mov ecx, eax    
    mbLogMacroC  ix
    sub eax, ecx    
    imul  eax, 109      ;; result scaled by 8*1024

;    add eax, (8 shl 13) + (1 shl 12)
;    shr eax, 13

    add eax, (8 shl 13)   ;; g_offset scaled by 8*1024

    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    mov tmp, eax
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_inverse_gsf_snr2 endp
;====================================
;====================================
_ifnc_inverse_gsf_xfer2 proc public
;;
;;   do not call if ixmax == 0!
;;
;;  inverse g for best xfer  g scaled by 8*1024
;;  int g = ifnc_inverse_gsf(int ix[], float x[], int n)
;;
;; arguments
ix        textequ <dword ptr [esp+4*(1+npush)]>
x         textequ <dword ptr [esp+4*(2+npush)]>
n         textequ <dword ptr [esp+4*(3+npush)]>


npush = 5
    push    esi
    push    edi
    push    ebx
    push    ecx
    sub     esp, 4
;;-----
tmp       textequ <dword ptr [esp]>

;;    fld con_4
    fldz
    fldz
    mov esi, ix
    mov edi, x
    mov ecx, n
a100:
    fld dword ptr [edi]
    fmul st(0), st  ;; x*x
    mov eax, [esi]
    cmp eax, 256
    jge big_pow43

    fld _look_ix43[eax*4]
big_pow43_return:

    fmul st(0), st  ;; q*q
    faddp   st(3), st
    faddp   st(1), st   ;;  sxx, sqq

    add esi, 4
    add edi, 4

    dec ecx
    jg  a100

;;-----
    fstp ix     ;; sxx
    fstp x      ;; sqq
    mbLogMacroC  x
    mov ecx, eax    
    mbLogMacroC  ix
    sub eax, ecx    
    imul  eax, 54      ;; result scaled by 8*1024

    add eax, (8 shl 13)   ;; g_offset scaled by 8*1024

    add esp, 4
    pop ecx
    pop ebx
    pop edi
    pop esi


    ret

;;---------

big_pow43:
    mov tmp, eax
    ;; arg in tmp ([esp]), return will be in st(0)
    call _pow43ix
    jmp big_pow43_return

_ifnc_inverse_gsf_xfer2 endp
;====================================
;====================================
_TEXT ENDS
;====================================
          END
