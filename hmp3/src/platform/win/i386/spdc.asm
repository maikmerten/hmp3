; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: spdc.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
        
;=========================================
;  SpdC.ASM
;   from spd.asm, modified for Mp3Enc Class
;
;  MPEG Layer III encode
;     acountic model, spreading function
;   flat 32, masm 6.0
;
; old layer II spd.asm not valid for Layer III at 32kHz
;  have N=2, rewritten for Layer III 11/16/98
;
;==========================================
.386P
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT

;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'


_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
;;==========================================================
_spd    PROC PUBLIC 
;;
;;void spd(float e[2][64], float ec[2][64], SPD_CNTL c[65], float w[])
;;
;; arguments
e_addr         textequ <dword ptr [esp+4*(1+npush)]>
ec_addr        textequ <dword ptr [esp+4*(2+npush)]>
spd_cntl_arg   textequ <dword ptr [esp+4*(3+npush)]>
w_arg          textequ <dword ptr [esp+4*(4+npush)]>

npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;
w           textequ <dword ptr [ebx]>
spd_cntl    textequ <dword ptr [edx]>
;;
;;
    mov esi, e_addr
    mov edi, ec_addr
    mov ebx, w_arg         ;; w[k]
    mov edx, spd_cntl_arg

    mov cl, byte ptr spd_cntl[8*64]     ;;  npart

a100:
    mov eax, spd_cntl[4]   ;; offset
    mov ch,  byte ptr spd_cntl[0]      ;; count
    fldz
    fldz
    add edx, 8
a200:
    fld w
    fld dword ptr [esi+4*eax]    
    fmul    st, st(1)
    fld dword ptr [esi+4*eax][4*64]    
    fmulp   st(2), st
    faddp   st(3), st
    faddp   st(1), st
    inc eax
    add ebx, 4
    dec ch
    jg  a200
;;-----
    fstp    dword ptr [edi][4*64]
    fstp    dword ptr [edi]
    add edi, 4
    dec cl
    jg  a100
;
    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
;;
_spd           ENDP
;====================================
_TEXT ENDS
;;==================================
          END
