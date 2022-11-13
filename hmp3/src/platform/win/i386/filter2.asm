; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: filter2.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
        
;  filter2.asm
;
; from filter.asm, modified for Mp3Enc Class
;
;	MASM 6.0
;     flat 32 bit 
;
;  MPEG Layer I/II/III encode
;     input filters (no 8 bit convert)
;		
;-------------------------------------------------
.486P
OPTION PROC:PRIVATE
OPTION CASEMAP:NOTPUBLIC
ASSUME ds:FLAT, cs:FLAT, ss:FLAT
;=========================================================
;;OVERLAP = (480+288)
;; additional 576 3/27/00
;;OVERLAP = ((2192+576)-1152)
;; additional 1152 3/27/00
OVERLAP = ((2192+1152)-1152)

;=========================================================
;=========================================================
_DATA SEGMENT PARA PUBLIC USE32 'DATA'

   ALIGN 4
;
con1    dd  44.1        ;; 0.001*44100

filter_table label dword
    dd    filter_mono0,    filter_dual0
    dd    filter_mono_dc,  filter_dual_dc

_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;=========================================================
_filter2_init proc public
;;
;;        samprate:DWORD,
;;        filter_select:DWORD,
;;        monodual:DWORD,
;;        select_out_addr:DWORD
;;
;;void filter2_init(int samprate, int filter_select, 
;;                 int monodual, FILTER2_CONTROL *fc2)
;;typedef struct {
;;    int select;
;;    float alpha;
;;    float d, d2;
;;} FILTER2_CONTROL;
;;

;; arguments
samprate       textequ <dword ptr [esp+4*(1+npush)]>
filter_select  textequ <dword ptr [esp+4*(2+npush)]>
monodual       textequ <dword ptr [esp+4*(3+npush)]>
fc2_addr       textequ <dword ptr [esp+4*(4+npush)]>
;;
npush = (4)
    push    esi
    push    edi
    push    ebx
    push    ecx
;;
select  textequ <dword ptr [esi][0]>
alpha   textequ <dword ptr [esi][4]>
dc      textequ <dword ptr [esi][2*4]>
dc2     textequ <dword ptr [esi][3*4]>
;;
    mov esi, fc2_addr
;;
;; alpha = (float)(0.001*44100.0/samprate);  /* set dc filter coef */
    fld     con1
    fild    samprate
    fdiv
    fstp    alpha
;;
	mov       dword ptr dc, 0
	mov       dword ptr dc2, 0
;;
    mov eax, filter_select
    cmp eax, 0
    jge  a1
    mov eax, 1
a1:
    cmp eax, 1
    jle a2
    mov eax, 1
a2:
;
;
    mov edx, monodual
    lea eax, [edx + eax*2]
    mov select, eax
;;
exit:
    pop ecx
    pop ebx
    pop edi
    pop esi
	ret

_filter2_init        ENDP
;----------------------------------------
SHIFT_BUF MACRO
          LOCAL a_100
;;
;; shift buffers - with overlap > 1152 need to start at hi mem
    add edi, 4*(OVERLAP - 4)
	mov       ecx, OVERLAP/4
a_100:
	mov       eax, [edi]
	mov       edx, [edi+4]
	mov       [4*1152][edi],   eax
	mov       [4*1152][edi+4], edx
	mov       eax, [edi+2*4]
	mov       edx, [edi+3*4]
	mov       [4*1152][edi+2*4],   eax
	mov       [4*1152][edi+3*4], edx
	sub       edi, 4*4
	dec       ecx
    jne       a_100
ENDM
;=============================================
_filter2 proc public
;;
;;void filter2(short pcm[], float *buf1, float *buf2,
;;             FILTER2_CONTROL *fc2)
;;
;;typedef struct {
;;    int select;
;;    float alpha;
;;    float d, d2;
;;} FILTER2_CONTROL;
;;

;; arguments
pcm_addr       textequ <dword ptr [esp+4*(1+npush)]>
buf1_addr      textequ <dword ptr [esp+4*(2+npush)]>
buf2_addr      textequ <dword ptr [esp+4*(3+npush)]>
fc2_addr       textequ <dword ptr [esp+4*(4+npush)]>
;;
npush = 5
    push    ebp
    push    esi
    push    edi
    push    ebx
    push    ecx
;;
select  textequ <dword ptr [ebp][0]>
alpha   textequ <dword ptr [ebp][4]>
dc      textequ <dword ptr [ebp][2*4]>
dc2     textequ <dword ptr [ebp][3*4]>
;;
    mov ebp, fc2_addr
    mov eax, select
    jmp filter_table[eax*4]
;--------------------------------------------
filter_dual0::
;;
;;
	mov       edi, buf1_addr
	mov       ebx, buf2_addr
	mov       esi, pcm_addr
	sub       ebx, edi       ;; address buf2 relative to buf1
;; shift buffers - with overlap > 1152 need to start at hi mem
    add edi, 4*(OVERLAP - 2)
	mov       ecx, OVERLAP/2
D_50:
	mov       eax, [edi]
	mov       edx, [edi+4]
	mov       [4*1152][edi],   eax
	mov       [4*1152][edi+4], edx
	mov       eax, [edi+ebx]
	mov       edx, [edi+ebx+4]
	mov       [4*1152][edi+ebx],   eax
	mov       [4*1152][edi+ebx+4], edx
	sub       edi, 2*4
	dec       ecx
    jne       d_50
	;;
	add       edi, 4*(1152+2)
	mov       ecx, 1152
D_100:
	sub       edi, 4
	fild      word ptr [esi]      ;; left
	fstp      dword ptr [edi]
	fild      word ptr [esi+2]    ;; right
	fstp      dword ptr [edi+ebx]
	add       esi, 2*2
	dec       ecx
	jne       d_100
;---------------------------------------
exit:
    pop ecx
    pop ebx
    pop edi
    pop esi
    pop ebp
	ret

;================================================
filter_mono0::
;;
;;
	mov       esi, pcm_addr
	mov       edi, buf1_addr
;; shift buffer
	SHIFT_BUF
;;
	add       edi, 4*(1152+4);
	mov       ecx, 1152/2
a_100:
	sub       edi, 2*4
	fild      word ptr [esi]
	fstp      dword ptr [edi+4]
	fild      word ptr [esi+2]
	fstp      dword ptr [edi]
	add       esi, 2*2
	dec       ecx
	jne       a_100
	;;
    jmp exit
;---------------------------------------------
filter_mono_dc::
;;
;;
	mov       esi, pcm_addr
	mov       edi, buf1_addr
;; shift buffer
          SHIFT_BUF
;;
	add       edi, 4*(1152+4);
	mov       ecx, 1152
	fld       alpha
	fld       dc
b_100:
	fild      word ptr [esi]
	sub       edi, 4
	add       esi, 2
	fsub      st, st(1)
	fst       dword ptr [edi]
	fmul      st, st(2)
	dec       ecx
	fadd
	jne       b_100
;;
	fstp      dc         ;; save dc for next time
	fstp      st(0)      ;; pop alpha
;;
    jmp exit

;--------------------------------------------
filter_dual_dc::
;;
;;
;;
	mov       edi, buf1_addr
	mov       ebx, buf2_addr
	mov       esi, pcm_addr
	sub       ebx, edi       ;; address buf2 relative to buf1
;; shift buffers - with overlap > 1152 need to start at hi mem
    add edi, 4*(OVERLAP - 2)
	mov       ecx, OVERLAP/2
E_50:
	mov       eax, [edi]
	mov       edx, [edi+4]
	mov       [4*1152][edi],   eax
	mov       [4*1152][edi+4], edx
	mov       eax, [edi+ebx]
	mov       edx, [edi+ebx+4]
	mov       [4*1152][edi+ebx],   eax
	mov       [4*1152][edi+ebx+4], edx
	sub       edi, 4*2
	dec       ecx
    jne       e_50
	;;
	add       edi, 4*(1152+2);
	mov       ecx, 1152
	fld       alpha
	fld       dc
	fld       dc2
E_100:
	sub       edi, 4
	fild      word ptr [esi]        ;; left
	fsub      st, st(2)
	fild      word ptr [esi+2]      ;; right
	fsub      st, st(2)
	fxch
	fst       dword ptr [edi]    ;; left
	fmul      st, st(4)
	fxch
	fst       dword ptr [edi+ebx] ;; right
	fmul      st, st(4)
	fxch
	faddp     st(3), st
	faddp     st(1), st
	lea       esi, [esi+2*2]
	dec       ecx
	jne       e_100
	fstp      dc2        ;; save dc for next time
	fstp      dc         ;; save dc for next time
	fstp      st(0)      ;; pop alpha
	;;
    jmp exit
;;
;=====================================================
_filter2 endp
;================================================
;=====================================================
_TEXT ENDS
;====================================
          END
