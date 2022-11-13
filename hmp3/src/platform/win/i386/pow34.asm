; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: pow34.asm,v 1.2 2005/08/09 20:43:45 karll Exp $ 
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
        
;-----------------------------------------
;  pow34.ASM
;
;   compute fabs(y)**0.75 
;   max error relative aprox 0.5e-4 
;
;-------------------------------------------
.386P
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT
;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

;
ALIGN 4

;; table generation by testpow.c
e34 label dword
dd  000000000h, 0103504F3h, 0109837F0h, 011000000h
dd  0115744FDh, 011B504F3h, 0121837F0h, 012800000h
dd  012D744FDh, 0133504F3h, 0139837F0h, 014000000h
dd  0145744FDh, 014B504F3h, 0151837F0h, 015800000h
dd  015D744FDh, 0163504F3h, 0169837F0h, 017000000h
dd  0175744FDh, 017B504F3h, 0181837F0h, 018800000h
dd  018D744FDh, 0193504F3h, 0199837F0h, 01A000000h
dd  01A5744FDh, 01AB504F3h, 01B1837F0h, 01B800000h
dd  01BD744FDh, 01C3504F3h, 01C9837F0h, 01D000000h
dd  01D5744FDh, 01DB504F3h, 01E1837F0h, 01E800000h
dd  01ED744FDh, 01F3504F3h, 01F9837F0h, 020000000h
dd  0205744FDh, 020B504F3h, 0211837F0h, 021800000h
dd  021D744FDh, 0223504F3h, 0229837F0h, 023000000h
dd  0235744FDh, 023B504F3h, 0241837F0h, 024800000h
dd  024D744FDh, 0253504F3h, 0259837F0h, 026000000h
dd  0265744FDh, 026B504F3h, 0271837F0h, 027800000h
dd  027D744FDh, 0283504F3h, 0289837F0h, 029000000h
dd  0295744FDh, 029B504F3h, 02A1837F0h, 02A800000h
dd  02AD744FDh, 02B3504F3h, 02B9837F0h, 02C000000h
dd  02C5744FDh, 02CB504F3h, 02D1837F0h, 02D800000h
dd  02DD744FDh, 02E3504F3h, 02E9837F0h, 02F000000h
dd  02F5744FDh, 02FB504F3h, 0301837F0h, 030800000h
dd  030D744FDh, 0313504F3h, 0319837F0h, 032000000h
dd  0325744FDh, 032B504F3h, 0331837F0h, 033800000h
dd  033D744FDh, 0343504F3h, 0349837F0h, 035000000h
dd  0355744FDh, 035B504F3h, 0361837F0h, 036800000h
dd  036D744FDh, 0373504F3h, 0379837F0h, 038000000h
dd  0385744FDh, 038B504F3h, 0391837F0h, 039800000h
dd  039D744FDh, 03A3504F3h, 03A9837F0h, 03B000000h
dd  03B5744FDh, 03BB504F3h, 03C1837F0h, 03C800000h
dd  03CD744FDh, 03D3504F3h, 03D9837F0h, 03E000000h
dd  03E5744FDh, 03EB504F3h, 03F1837F0h, 03F800000h
dd  03FD744FDh, 0403504F3h, 0409837F0h, 041000000h
dd  0415744FDh, 041B504F3h, 0421837F0h, 042800000h
dd  042D744FDh, 0433504F3h, 0439837F0h, 044000000h
dd  0445744FDh, 044B504F3h, 0451837F0h, 045800000h
dd  045D744FDh, 0463504F3h, 0469837F0h, 047000000h
dd  0475744FDh, 047B504F3h, 0481837F0h, 048800000h
dd  048D744FDh, 0493504F3h, 0499837F0h, 04A000000h
dd  04A5744FDh, 04AB504F3h, 04B1837F0h, 04B800000h
dd  04BD744FDh, 04C3504F3h, 04C9837F0h, 04D000000h
dd  04D5744FDh, 04DB504F3h, 04E1837F0h, 04E800000h
dd  04ED744FDh, 04F3504F3h, 04F9837F0h, 050000000h
dd  0505744FDh, 050B504F3h, 0511837F0h, 051800000h
dd  051D744FDh, 0523504F3h, 0529837F0h, 053000000h
dd  0535744FDh, 053B504F3h, 0541837F0h, 054800000h
dd  054D744FDh, 0553504F3h, 0559837F0h, 056000000h
dd  0565744FDh, 056B504F3h, 0571837F0h, 057800000h
dd  057D744FDh, 0583504F3h, 0589837F0h, 059000000h
dd  0595744FDh, 059B504F3h, 05A1837F0h, 05A800000h
dd  05AD744FDh, 05B3504F3h, 05B9837F0h, 05C000000h
dd  05C5744FDh, 05CB504F3h, 05D1837F0h, 05D800000h
dd  05DD744FDh, 05E3504F3h, 05E9837F0h, 05F000000h
dd  05F5744FDh, 05FB504F3h, 0601837F0h, 060800000h
dd  060D744FDh, 0613504F3h, 0619837F0h, 062000000h
dd  0625744FDh, 062B504F3h, 0631837F0h, 063800000h
dd  063D744FDh, 0643504F3h, 0649837F0h, 065000000h
dd  0655744FDh, 065B504F3h, 0661837F0h, 066800000h
dd  066D744FDh, 0673504F3h, 0679837F0h, 068000000h
dd  0685744FDh, 068B504F3h, 0691837F0h, 069800000h
dd  069D744FDh, 06A3504F3h, 06A9837F0h, 06B000000h
dd  06B5744FDh, 06BB504F3h, 06C1837F0h, 06C800000h
dd  06CD744FDh, 06D3504F3h, 06D9837F0h, 06E000000h
dd  06E5744FDh, 06EB504F3h, 06F1837F0h, 07F800000h


ab  label dword
dd   03E82F5BAh, 03F3E8806h,   03E88E2EAh, 03F3BBE0Eh
dd   03E8EB69Bh, 03F392714h,   03E94763Fh, 03F36BB71h
dd   03E9A231Eh, 03F34764Bh,   03E9FBE5Ah, 03F32538Ah
dd   03EA548FAh, 03F304FADh,   03EAAC3E8h, 03F2E67B3h
dd   03EB02FFBh, 03F2C9902h,   03EB58DF2h, 03F2AE156h
dd   03EBADE80h, 03F293EB5h,   03EC02244h, 03F27AF5Eh
dd   03EC559D4h, 03F2631C7h,   03ECA85B7h, 03F24C491h
dd   03ECFA66Dh, 03F236683h,   03ED3DD93h, 03F225006h

tmp dd 0

_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================

;loopx macro counter, loop_label
;	dec	counter
;	jne	loop_label
;endm
_fpow34  PROC PUBLIC 
;;  float fpow34(float x)
;;
;; arguments
x     textequ <dword ptr [esp+4*(1+npush)]>
npush = 0
    ;push ecx
    mov eax, [x]
    ;lea ecx, [e34]

    mov edx, eax
    and eax, 7FFFFFh

    shr edx, 19
    or  eax, (127 shl 23)

    mov x, eax
    mov eax, edx

    and edx, 15
    shr eax, 23-19

    fld     [x]
    fmul    [edx*8+4+e34+(ab-e34)]
    and eax, 255
    fadd    [edx*8+e34+(ab-e34)]
    fmul    [eax*4+e34]
    pop ecx

    ret
_fpow34           ENDP

_vect_fpow34  PROC PUBLIC 
;;  vect_fpow34(float x[], float y[], int n)
;;USES ESI EDI EBX ECX EBP
;; arguments
x          textequ <dword ptr [esp+4*(1+npush)]>
y          textequ <dword ptr [esp+4*(2+npush)]>
n          textequ <dword ptr [esp+4*(3+npush)]>
npush = 5

    push esi
    push edi
    push ebx
    push ecx

    push    ebp
    mov esi, x
    mov edi, y
    mov ebp, n
    test ebp, ebp
    jle exit
;;-----
    mov eax, [esi]
    add esi, 4
    mov ecx, eax
    and eax, 7FFFFFh
    shr ecx, 19
    or  eax, (127 shl 23)
    mov tmp, eax
    mov edx, ecx
    and ecx, 15
    shr edx, 23-19
    mov ebx, ecx
    and edx, 255
    dec ebp
    je  a800
;;-----------
a100:
  fld     tmp
  fmul    ab[ebx*8+4]
    mov eax, [esi]
    add esi, 4
    mov ecx, eax
    and eax, 7FFFFFh
  fadd    ab[ebx*8]
    or  eax, (127 shl 23)
        ;;mov ebx, [edi]  ;; dummy read
    shr ecx, 19
    mov tmp, eax
  fmul    e34[edx*4]
    mov edx, ecx
    and ecx, 15
    shr edx, 23-19
    add     edi, 4
    mov ebx, ecx
    and edx, 255
  fstp    dword ptr [edi-4]
;;
    dec ebp
    jne a100
a800:
  fld     tmp
  fmul    ab[ebx*8+4]
  fadd    ab[ebx*8]
  fmul    e34[edx*4]
  fstp    dword ptr [edi]


exit:
    pop ebp
    pop ecx
    pop ebx
    pop edi
    pop esi
	ret
;;
_vect_fpow34           ENDP

_vect_fmax  PROC PUBLIC 
;;  float vect_fmax(float xin[], int nvect)
;;  max(x[i]) xmax >=0.0  ( compare done with integer alu)
;;USES ESI EDI EBX ECX
;; arguments
xin        textequ <dword ptr [esp+4*(1+npush)]>
nvect      textequ <dword ptr [esp+4*(2+npush)]>
npush = 4

;;
    push esi
    push edi
    push ebx
    push ecx

    mov esi, xin
    mov ecx, nvect
    xor edx, edx
    test ecx, ecx
    jle a800
a100:
    mov eax, [esi]
    add esi, 4
    cmp eax, edx
    jg  a200
    dec ecx
    jne a100
    jmp a800

a200:
    mov edx, eax
    dec ecx
    jne a100


a800:
    mov tmp, edx
    fld tmp         ;; return result on fpu stack
exit:
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret
;;
_vect_fmax           ENDP

;====================================
;====================================
_TEXT ENDS
;====================================
          END
