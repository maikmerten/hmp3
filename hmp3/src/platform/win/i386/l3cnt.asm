; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: l3cnt.asm,v 1.1 2005/07/13 17:22:23 rggammon Exp $ 
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
; L3cnt.asm   
;
;  Layer 3 bitallo bit counting routines
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

table_quad  label dword
   dw   1 ,   4         ;; table a,  table b
   dw   4+1,  4+1
   dw   4+1,  4+1
   dw   5+2,  4+2
   dw   4+1,  4+1
   dw   6+2,  4+2
   dw   5+2,  4+2
   dw   6+3,  4+3
   dw   4+1,  4+1
   dw   5+2,  4+2
   dw   5+2,  4+2
   dw   6+3,  4+3
   dw   5+2,  4+2
   dw   6+3,  4+3
   dw   6+3,  4+3
   dw   6+4,  4+4




_DATA ENDS
;=========================================================
;=========================================================
_TEXT SEGMENT para PUBLIC USE32 'CODE'
;;==========================================================
;====================================
_count_bits_quadab proc public
;;
;;  eax/edx = count_bits_quadab(int ix[], int nquads)
;;  count both table at same time
;;  eax = tablea, edx = tableb
;;
;; arguments
ix         textequ <dword ptr [esp+4*(1+npush)]>
nquad      textequ <dword ptr [esp+4*(2+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    mov esi, ix
    mov edi, nquad
    xor eax, eax
    test edi, edi
    jle  exit

a100:
    mov ebx, [esi+0*4]
    mov ecx, [esi+1*4]
    lea ebx, [ecx + ebx*2]

    mov edx, [esi+2*4]
    mov ecx, [esi+3*4]
    lea edx, [ecx + edx*2]

    lea ebx, [edx + ebx*4]

    mov ebx, table_quad[ebx*4]
    add eax, ebx

    add esi, 4*4
    dec edi
    jg  a100


;;---------------
exit:
    mov edx, eax
    and eax, 0FFFFh
    shr edx, 16

    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
_count_bits_quadab endp
;====================================
_CountBits0 proc public

;; null case, ixmax = 0

    xor eax, eax
    xor edx, edx
    ret

_CountBits0 endp
;====================================
_CountBits1 proc public
;;
;;  eax/edx = CountBits(void *table, int ix[], int n)
;;  count both table at same time
;;  eax = bits, edx = index
;;
;; Case1  int table[][2]  two tables packed into 32 bits
;;
;; arguments
table      textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
n          textequ <dword ptr [esp+4*(3+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    mov edx, n
    mov esi, ix
    mov edi, table
    xor ecx, ecx
    test edx, edx
    jle  exit

a100:
    mov eax, [esi+0*4]
    mov ebx, [esi+1*4]
    lea ebx, [ebx + eax*2]
    mov ebx, [edi+ebx*4]
    add ecx, ebx
    add esi, 2*4
    sub edx, 2      ;; pairs
    jg  a100

;;---------------
    mov eax, ecx
    shr ecx, 16
    and eax, 0FFFFh
    xor ebx, ebx    ;; index

;;jmp exit2
;; find min bits and index
    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    dec ebx
    and ebx, edx
    inc ebx

exit2:
    mov edx, ebx

    pop ecx
    pop ebx
    pop edi
    pop esi
    ret

;;------- null count
exit:
    xor eax, eax
    xor ebx, ebx
    jmp exit2

_CountBits1 endp
;====================================
;====================================
_CountBits2 proc public
;;
;;  eax/edx = CountBits(void *table, int ix[], int n)
;;  count both table at same time
;;  eax = bits, edx = index
;;
;; Case2  int table[][4]  two tables packed into 32 bits
;;
;; arguments
table      textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
n          textequ <dword ptr [esp+4*(3+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    mov edx, n
    mov esi, ix
    mov edi, table
    xor ecx, ecx
    test edx, edx
    jle  exit

a100:
    mov eax, [esi+0*4]
    mov ebx, [esi+1*4]
    lea ebx, [ebx + eax*4]
    mov ebx, [edi+ebx*4]
    add ecx, ebx
    add esi, 2*4
    sub edx, 2      ;; pairs
    jg  a100


;;---------------
    mov eax, ecx
    shr ecx, 16
    and eax, 0FFFFh
    xor ebx, ebx    ;; index

;;jmp exit2
;; find min bits and index
    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    dec ebx
    and ebx, edx
    inc ebx

exit2:
    mov edx, ebx

    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
;;------- null count
exit:
    xor eax, eax
    xor ebx, ebx
    jmp exit2

_CountBits2 endp
;====================================
_CountBits3 proc public
;;
;;  eax/edx = CountBits(void *table, int ix[], int n)
;;  count four tables at same time
;;  eax = bits, edx = index
;;
;; Case3  int table[][8][2]  wide table, 4 tables in 64 bits
;;
;; arguments
table      textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
n          textequ <dword ptr [esp+4*(3+npush)]>


npush = 5
    push    ebp
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    mov edx, n
    mov esi, ix
    mov edi, table
    xor ecx, ecx
    xor ebp, ebp
    test edx, edx
    jle  exit

a100:
    mov eax, [esi+0*4]
    mov ebx, [esi+1*4]
    lea ebx, [ebx + eax*8]

    mov eax, [edi+ebx*8]
    add ecx, eax
    mov eax, [edi+ebx*8+4]
    add ebp, eax
    add esi, 2*4
    sub edx, 2      ;; pairs
    jg  a100


;;---------------
    mov eax, ecx
    shr ecx, 16
    and eax, 0FFFFh
    xor ebx, ebx    ;; index

;;jmp exit2
;; find min bits and index
    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    dec ebx
    and ebx, edx
    inc ebx
;;--
    mov ecx, ebp
    shr ebp, 16
    and ecx, 0FFFFh

    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    sub ebx, 2
    and ebx, edx
    add ebx, 2
;;--
;;jmp exit2
    sub eax, ebp
    cdq
    and eax, edx
    add eax, ebp

    sub ebx, 3
    and ebx, edx
    add ebx, 3

exit2:
    mov edx, ebx

    pop ecx
    pop ebx
    pop edi
    pop esi
    pop ebp

    ret

;;------- null count
exit:
    xor eax, eax
    xor ebx, ebx
    jmp exit2

_CountBits3 endp
;====================================
;====================================
_CountBits4 proc public
;;
;;  eax/edx = CountBits(void *table, int ix[], int n)
;;  count both table at same time
;;  eax = bits, edx = index
;;
;; Case4  int table[][16]  two tables packed into 32 bits
;;
;; arguments
table      textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
n          textequ <dword ptr [esp+4*(3+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    mov edx, n
    mov esi, ix
    mov edi, table
    xor ecx, ecx
    test edx, edx
    jle  exit

a100:
    mov eax, [esi+0*4]
    shl eax, 4
    mov ebx, [esi+1*4]
    add ebx, eax
    mov ebx, [edi+ebx*4]
    add ecx, ebx
    add esi, 2*4
    sub edx, 2      ;; pairs
    jg  a100


;;---------------
    mov eax, ecx
    shr ecx, 16
    and eax, 0FFFFh
    xor ebx, ebx    ;; index

;;jmp exit2
;; find min bits and index
    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    dec ebx
    and ebx, edx
    inc ebx


exit2:
    mov edx, ebx

    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
;;------- null count
exit:
    xor eax, eax
    xor ebx, ebx
    xor edx, edx
    jmp exit2

_CountBits4 endp
;====================================
;====================================
;====================================
_CountBits5 proc public
;;
;;  eax/edx = CountBits(void *table, int ix[], int n)
;;  count both table at same time
;;  eax = bits, edx = index
;;
;; Case5  int table[][16]  linbits, two tables packed into 32 bits
;;
;; arguments
table      textequ <dword ptr [esp+4*(1+npush)]>
ix         textequ <dword ptr [esp+4*(2+npush)]>
n          textequ <dword ptr [esp+4*(3+npush)]>


npush = 5
    push    ebp
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    mov ebp, n
    mov esi, ix
    mov edi, table
    xor ecx, ecx
    test ebp, ebp
    jle  exit

a100:
    mov eax, [esi+0*4]
    sub eax, 15
    cdq
    and eax, edx
    add eax, 15

    mov ebx, [esi+1*4]
    sub ebx, 15
    sbb edx, edx
    and ebx, edx
    add ebx, 15

    shl eax, 4
    add ebx, eax
    mov ebx, [edi+ebx*4]
    add ecx, ebx
    add esi, 2*4
    sub ebp, 2      ;; pairs
    jg  a100


;;---------------
    mov eax, ecx
    shr ecx, 16
    and eax, 0FFFFh
    xor ebx, ebx    ;;  index

;;jmp exit2
;; find min bits and index
    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    dec ebx
    and ebx, edx
    inc ebx

exit2:
    mov edx, ebx

    pop ecx
    pop ebx
    pop edi
    pop esi
    pop ebp

    ret
;;------- null count
exit:
    xor eax, eax
    xor ebx, ebx
    xor edx, edx
    jmp exit2

_CountBits5 endp
;====================================
;====================================
_CountBitsQuad proc public
;;
;;  eax/edx = CountBitsQuad(int ix[], int nquads)
;;  count both table at same time
;;  eax = bits edx = index 
;;
;; arguments
ix         textequ <dword ptr [esp+4*(1+npush)]>
nquad      textequ <dword ptr [esp+4*(2+npush)]>


npush = 4
    push    esi
    push    edi
    push    ebx
    push    ecx
;;-----

    mov esi, ix
    mov edi, nquad
    xor eax, eax
    test edi, edi
    jle  exit

a100:
    mov ebx, [esi+0*4]
    mov ecx, [esi+1*4]
    lea ebx, [ecx + ebx*2]

    mov edx, [esi+2*4]
    mov ecx, [esi+3*4]
    lea edx, [ecx + edx*2]

    lea ebx, [edx + ebx*4]

    mov ebx, table_quad[ebx*4]
    add eax, ebx

    add esi, 4*4
    dec edi
    jg  a100


;;---------------
    mov ecx, eax
    and eax, 0FFFFh
    shr ecx, 16
    xor ebx, ebx        ;; index


;;mov eax, ecx    ;; test test
;;mov ebx, 1      ;; tezt test
;;jmp exit2       ;; test test
;; find min bits and index
    sub eax, ecx
    cdq
    and eax, edx
    add eax, ecx

    dec ebx
    and ebx, edx
    inc ebx

exit2:
    mov edx, ebx

    pop ecx
    pop ebx
    pop edi
    pop esi

    ret
;;------- null count
exit:
    xor eax, eax
    xor ebx, ebx
    jmp exit2

_CountBitsQuad endp
;====================================
_vect_ixmax_ba proc public
;;
;;
;; arguments
ix         textequ <dword ptr [esp+4*(1+npush)]>
n          textequ <dword ptr [esp+4*(2+npush)]>


npush = 3
    push    esi
    push    ebx
    push    ecx


    mov esi, ix
    mov ecx, n
    xor eax, eax
    test ecx, ecx
    jle exit
a100:
    mov ebx, [esi]
    mov edx, -1
    sub eax, ebx
    adc edx, 0
    and eax, edx
    add eax, ebx

    add esi, 4

    dec ecx
    jg  a100

;;--------

exit:
    pop ecx
    pop ebx
    pop esi

    ret
_vect_ixmax_ba endp
;====================================
_TEXT ENDS
;====================================
          END
