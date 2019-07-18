; -----------------------------------------------------------------------------------------
; ram_speed by rigaya
; -----------------------------------------------------------------------------------------
; The MIT License
;
; Copyright (c) 2014-2017 rigaya
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
;
; --------------------------------------------------------------------------------------------

section .code
    align 16

section .text

global read_sse

;void __stdcall read_sse(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [rcx] PIXEL_YC       *src
;  [rdx] uint32_t        size
;  [r8]  uint32_t        count_n
;)

read_sse:
        push rbx

%ifdef LINUX
        mov r9,  rdi
        mov eax, esi
        mov rsi, rdx
%else
        push rdi
        push rsi
        mov r9,  rcx
        mov eax, edx
        mov rsi, r8
%endif
        mov edi, 128
        shr eax, 7
        align 16
    .OUTER_LOOP:
        mov rbx, r9
        mov rdx, rbx
        add rdx, 64
        mov ecx, eax
    .INNER_LOOP:
        movaps xmm0, [rbx];
        movaps xmm1, [rbx+16];
        movaps xmm2, [rbx+32];
        movaps xmm3, [rbx+48];
        add rbx, rdi;
        movaps xmm4, [rdx];
        movaps xmm5, [rdx+16];
        movaps xmm6, [rdx+32];
        movaps xmm7, [rdx+48];
        add rdx, rdi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

%ifndef LINUX
        pop rsi
        pop rdi
%endif
        pop rbx

        ret



global read_avx

;void __stdcall read_avx(uint8_t *src, uint32_t size, uint32_t count_n) (
;  Win  Linux
;  [rcx][rdi] PIXEL_YC       *src
;  [rdx][rsi] uint32_t        size
;  [r8] [rdx] uint32_t        count_n
;)

read_avx:
        push rbx

%ifdef LINUX
        mov r9,  rdi
        mov eax, esi
        mov rsi, rdx
%else
        push rdi
        push rsi
        mov r9,  rcx
        mov eax, edx
        mov rsi, r8
%endif
        mov edi, 256
        shr eax, 8
        align 16
    .OUTER_LOOP:
        mov rbx, r9
        mov rdx, rbx
        add rdx, 128
        mov ecx, eax
    .INNER_LOOP:
        vmovaps ymm0, [rbx];
        vmovaps ymm1, [rbx+32];
        vmovaps ymm2, [rbx+64];
        vmovaps ymm3, [rbx+96];
        add rbx, rdi
        vmovaps ymm4, [rdx];
        vmovaps ymm5, [rdx+32];
        vmovaps ymm6, [rdx+64];
        vmovaps ymm7, [rdx+96];
        add rdx, rdi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

%ifndef LINUX
        pop rsi
        pop rdi
%endif
        pop rbx

        ret


global read_avx512

;void __stdcall read_avx512(uint8_t *src, uint32_t size, uint32_t count_n) (
;  Win  Linux
;  [rcx][rdi] PIXEL_YC       *src
;  [rdx][rsi] uint32_t        size
;  [r8] [rdx] uint32_t        count_n
;)

read_avx512:
        push rbx

%ifdef LINUX
        mov r9,  rdi
        mov eax, esi
        mov rsi, rdx
%else
        push rdi
        push rsi
        mov r9,  rcx
        mov eax, edx
        mov rsi, r8
%endif
        mov edi, 512
        shr eax, 9
        align 16
    .OUTER_LOOP:
        mov rbx, r9
        mov rdx, rbx
        add rdx, 256
        mov ecx, eax
    .INNER_LOOP:
        vmovdqa32 zmm0, [rbx];
        vmovdqa32 zmm1, [rbx+64];
        vmovdqa32 zmm2, [rbx+128];
        vmovdqa32 zmm3, [rbx+192];
        add rbx, rdi
        vmovdqa32 zmm4, [rdx];
        vmovdqa32 zmm5, [rdx+64];
        vmovdqa32 zmm6, [rdx+128];
        vmovdqa32 zmm7, [rdx+192];
        add rdx, rdi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

%ifndef LINUX
        pop rsi
        pop rdi
%endif
        pop rbx

        ret

global write_sse

;void __stdcall _write_sse(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+08] PIXEL_YC       *src
;  [esp+16] uint32_t        size
;  [esp+20] uint32_t        count_n
;)

write_sse:
        push rbx

%ifdef LINUX
        mov r9,  rdi
        mov eax, esi
        mov rsi, rdx
%else
        push rdi
        push rsi
        mov r9,  rcx
        mov eax, edx
        mov rsi, r8
%endif
        mov edi, 128
        shr eax, 7
        align 16
    .OUTER_LOOP:
        mov rbx, r9
        mov rdx, rbx
        add rdx, 64
        mov ecx, eax
    .INNER_LOOP:
        movaps [rbx],    xmm0
        movaps [rbx+16], xmm0
        movaps [rbx+32], xmm0
        movaps [rbx+48], xmm0
        add rbx, rdi
        movaps [rdx],    xmm0
        movaps [rdx+16], xmm0
        movaps [rdx+32], xmm0
        movaps [rdx+48], xmm0
        add rdx, rdi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

%ifndef LINUX
        pop rsi
        pop rdi
%endif
        pop rbx
        ret




global write_avx

;void __stdcall _write_avx(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+08] PIXEL_YC       *src
;  [esp+16] uint32_t        size
;  [esp+20] uint32_t        count_n
;)

write_avx:
        push rbx

%ifdef LINUX
        mov r9,  rdi
        mov eax, esi
        mov rsi, rdx
%else
        push rdi
        push rsi
        mov r9,  rcx
        mov eax, edx
        mov rsi, r8
%endif
        mov edi, 256
        shr eax, 8
        align 16
    .OUTER_LOOP:
        mov rbx, r9
        mov rdx, rbx
        add rdx, 128
        mov ecx, eax
    .INNER_LOOP:
        vmovaps [rbx],    ymm0
        vmovaps [rbx+32], ymm0
        vmovaps [rbx+64], ymm0
        vmovaps [rbx+96], ymm0
        add rbx, rdi
        vmovaps [rdx],    ymm0
        vmovaps [rdx+32], ymm0
        vmovaps [rdx+64], ymm0
        vmovaps [rdx+96], ymm0
        add rdx, rdi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

%ifndef LINUX
        pop rsi
        pop rdi
%endif
        pop rbx

        ret

global write_avx512

;void __stdcall _write_avx512(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+08] PIXEL_YC       *src
;  [esp+16] uint32_t        size
;  [esp+20] uint32_t        count_n
;)

write_avx512:
        push rbx

%ifdef LINUX
        mov r9,  rdi
        mov eax, esi
        mov rsi, rdx
%else
        push rdi
        push rsi
        mov r9,  rcx
        mov eax, edx
        mov rsi, r8
%endif
        mov edi, 512
        shr eax, 9
        align 16
    .OUTER_LOOP:
        mov rbx, r9
        mov rdx, rbx
        add rdx, 256
        mov ecx, eax
    .INNER_LOOP:
        vmovdqa32 [rbx],     zmm0
        vmovdqa32 [rbx+64],  zmm0
        vmovdqa32 [rbx+128], zmm0
        vmovdqa32 [rbx+192], zmm0
        add rbx, rdi
        vmovdqa32 [rdx],     zmm0
        vmovdqa32 [rdx+64],  zmm0
        vmovdqa32 [rdx+128], zmm0
        vmovdqa32 [rdx+192], zmm0
        add rdx, rdi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

%ifndef LINUX
        pop rsi
        pop rdi
%endif
        pop rbx

        ret

global ram_latency_test

;void __stdcall ram_latency_test(uint8_t *src) (
;  [rcx] PIXEL_YC       *src
;)

ram_latency_test:

%ifdef LINUX
        mov rcx, rdi
%endif
        xor rdx, rdx
        align 16
    .LOOP:
        mov edx, [rcx + rdx*4]
        test edx, edx
        jg .LOOP

        ret