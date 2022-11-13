; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: epartc.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
;  EpartC.ASM
;
;  from Epart.asm modified for Mp3Enc Class
;  
;  MPEG Layer II encode
;     acountic model II, partition
;  masm 6.0 flat 32
; mod bigger calc partition sized for Layer III
;
;-------------------------------------------
.486P
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT
;=========================================================
;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'


_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;=========================================================
;------------------------------------
_epart proc public
;;
;;void epart(float rr[], float pmeas[], float e[][64], int nsum[68])
;;
;; arguments
rr       textequ <dword ptr [esp+4*(1+npush)]>
pmeas    textequ <dword ptr [esp+4*(2+npush)]>
eout     textequ <dword ptr [esp+4*(3+npush)]>
nsum_arg textequ <dword ptr [esp+4*(4+npush)]>

npush = (4)
    push    esi
    push    edi
    push    ebx
    push    ecx
;;
nsum    textequ <[edx]>

	mov       edx, nsum_arg
	mov       ebx, rr
	mov       esi, pmeas
	mov       edi, eout
;;
;; --- computed pred region
	mov ch, byte ptr nsum[4*64]
	mov al, byte ptr nsum[4*65]
	add	ch, al
    mov ah, byte ptr nsum[4*66]     ;; save for default pred region
A_100:
	fldz
	fldz
	mov       cl, byte ptr nsum
A_150:
	fld       dword ptr [ebx]   ;; rr
	fadd      st(1), st         ;; sum rr
	fmul      dword ptr [esi]   ;; pmeas
	add       ebx, 4
	add       esi, 4
	dec       cl
	faddp     st(2), st           ;; sum p*rr
	jne       a_150

	add       edx, 4
	fstp      dword ptr [edi]
	fstp      dword ptr [edi+4*64]
	add       edi, 4
	dec       ch
    jne       a_100


;; --- default pred region
	test      ah, ah
	jle       done
	fld       dword ptr [esi]   ;; default pmeas
A_200:
	fldz
	mov       cl, byte ptr nsum[edx]
A_250:
	fadd      dword ptr [ebx]   ;; rr
	add       ebx, 4
	dec       cl
    jne       a_250
	add       edx, 4
	fst       dword ptr [edi]
	fmul      st, st(1)
	fstp      dword ptr [edi+4*64]
	add       edi, 4
	dec       ah
    jne       a_200
	fstp      st(0)        ;; pop stack
done:
;; zero fill
	mov       ecx, 4*64
	sub       ecx, edx
	jle       exit
	xor       eax, eax
A_300:
	mov       dword ptr [edi], eax
	mov       dword ptr [edi+4*64], eax
	add       edi, 4
	sub       ecx, 4
	jg        a_300
;;
exit:
    pop ecx
    pop ebx
    pop edi
    pop esi

	ret
;;
_epart           ENDP
;----------------------------------
;====================================
;====================================
_TEXT ENDS
;====================================
          END
