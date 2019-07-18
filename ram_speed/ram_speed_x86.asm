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

global _read_sse

;void __stdcall read_sse(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+04] PIXEL_YC       *src
;  [esp+08] uint32_t        size
;  [esp+12] uint32_t        count_n
;)

    _read_sse:
        push ebp
        push edi
        push esi
        push ebx
; @+16

        mov edi, 128
        mov esi, [esp+16+12]; count_n
        mov eax, [esp+16+04]; src
        mov ebp, [esp+16+08]; size
        shr ebp, 7
        align 16
    .OUTER_LOOP:
        mov ebx, eax; src
        mov edx, ebx
        add edx, 64
        mov ecx, ebp
    .INNER_LOOP:
        movaps xmm0, [ebx];
        movaps xmm1, [ebx+16];
        movaps xmm2, [ebx+32];
        movaps xmm3, [ebx+48];
        add ebx, edi;
        movaps xmm4, [edx];
        movaps xmm5, [edx+16];
        movaps xmm6, [edx+32];
        movaps xmm7, [edx+48];
        add edx, edi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        pop ebx
        pop esi
        pop edi
        pop ebp

        ret




global _read_avx

;void __stdcall read_avx(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+04] PIXEL_YC       *src
;  [esp+08] uint32_t        size
;  [esp+12] uint32_t        count_n
;)

    _read_avx:
        push ebp
        push edi
        push esi
        push ebx
; @+16

        mov edi, 256
        mov esi, [esp+16+12]; count_n
        mov eax, [esp+16+04]; src
        mov ebp, [esp+16+08]; size
        shr ebp, 8
        align 16
    .OUTER_LOOP:
        mov ebx, eax; src
        mov edx, ebx
        add edx, 128
        mov ecx, ebp
    .INNER_LOOP:
        vmovaps ymm0, [ebx]
        vmovaps ymm1, [ebx+32]
        vmovaps ymm2, [ebx+64]
        vmovaps ymm3, [ebx+96]
        add ebx, edi
        vmovaps ymm4, [edx]
        vmovaps ymm5, [edx+32]
        vmovaps ymm6, [edx+64]
        vmovaps ymm7, [edx+96]
        add edx, edi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

        pop ebx
        pop esi
        pop edi
        pop ebp

        ret


global _read_avx512

;void __stdcall read_avx512(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+04] PIXEL_YC       *src
;  [esp+08] uint32_t        size
;  [esp+12] uint32_t        count_n
;)

    _read_avx512:
        push ebp
        push edi
        push esi
        push ebx
; @+16

        mov edi, 512
        mov esi, [esp+16+12]; count_n
        mov eax, [esp+16+04]; src
        mov ebp, [esp+16+08]; size
        shr ebp, 9
        align 16
    .OUTER_LOOP:
        mov ebx, eax; src
        mov edx, ebx
        add edx, 256
        mov ecx, ebp
    .INNER_LOOP:
        vmovdqa32 zmm0, [ebx]
        vmovdqa32 zmm1, [ebx+64]
        vmovdqa32 zmm2, [ebx+128]
        vmovdqa32 zmm3, [ebx+192]
        add ebx, edi
        vmovdqa32 zmm4, [edx]
        vmovdqa32 zmm5, [edx+64]
        vmovdqa32 zmm6, [edx+128]
        vmovdqa32 zmm7, [edx+192]
        add edx, edi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

        pop ebx
        pop esi
        pop edi
        pop ebp

        ret

global _write_sse

;void __stdcall write_sse(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+04] PIXEL_YC       *src
;  [esp+08] uint32_t        size
;  [esp+12] uint32_t        count_n
;)

    _write_sse:
        push ebp
        push edi
        push esi
        push ebx
; @+16

        mov edi, 128
        mov esi, [esp+16+12]; count_n
        mov eax, [esp+16+04]; src
        mov ebp, [esp+16+08]; size
        shr ebp, 7
        align 16
    .OUTER_LOOP:
        mov ebx, eax; src
        mov edx, ebx
        add edx, 64
        mov ecx, ebp
    .INNER_LOOP:
        movaps [ebx],    xmm0
        movaps [ebx+16], xmm0
        movaps [ebx+32], xmm0
        movaps [ebx+48], xmm0
        add ebx, edi
        movaps [edx],    xmm0
        movaps [edx+16], xmm0
        movaps [edx+32], xmm0
        movaps [edx+48], xmm0
        add edx, edi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        pop ebx
        pop esi
        pop edi
        pop ebp

        ret



global _write_avx

;void __stdcall write_avx(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+04] PIXEL_YC       *src
;  [esp+08] uint32_t        size
;  [esp+12] uint32_t        count_n
;)

_write_avx:
        push ebp
        push edi
        push esi
        push ebx
; @+16

        mov edi, 256
        mov esi, [esp+16+12]; count_n
        mov eax, [esp+16+04]; src
        mov ebp, [esp+16+08]; size
        shr ebp, 8
        align 16
    .OUTER_LOOP:
        mov ebx, eax; src
        mov edx, ebx
        add edx, 128
        mov ecx, ebp
    .INNER_LOOP:
        vmovaps [ebx],    ymm0
        vmovaps [ebx+32], ymm0
        vmovaps [ebx+64], ymm0
        vmovaps [eax+96], ymm0
        add ebx, edi
        vmovaps [edx],    ymm0
        vmovaps [edx+32], ymm0
        vmovaps [edx+64], ymm0
        vmovaps [edx+96], ymm0
        add edx, edi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

        pop ebx
        pop esi
        pop edi
        pop ebp

        ret

global _write_avx512

;void __stdcall write_avx512(uint8_t *src, uint32_t size, uint32_t count_n) (
;  [esp+04] PIXEL_YC       *src
;  [esp+08] uint32_t        size
;  [esp+12] uint32_t        count_n
;)

_write_avx512:
        push ebp
        push edi
        push esi
        push ebx
; @+16

        mov edi, 512
        mov esi, [esp+16+12]; count_n
        mov eax, [esp+16+04]; src
        mov ebp, [esp+16+08]; size
        shr ebp, 9
        align 16
    .OUTER_LOOP:
        mov ebx, eax; src
        mov edx, ebx
        add edx, 256
        mov ecx, ebp
    .INNER_LOOP:
        vmovdqa32 [ebx],     zmm0
        vmovdqa32 [ebx+64],  zmm0
        vmovdqa32 [ebx+128], zmm0
        vmovdqa32 [eax+192], zmm0
        add ebx, edi
        vmovdqa32 [edx],     zmm0
        vmovdqa32 [edx+64],  zmm0
        vmovdqa32 [edx+128], zmm0
        vmovdqa32 [edx+192], zmm0
        add edx, edi
        dec ecx
        jnz .INNER_LOOP

        dec esi
        jnz .OUTER_LOOP

        vzeroupper

        pop ebx
        pop esi
        pop edi
        pop ebp

        ret



global _ram_latency_test

;void __stdcall ram_latency_test(void *src) (
;  [esp+04] void       *src
;)

    _ram_latency_test:
        mov ecx, [esp+00+04]; src
        xor edx, edx
        align 16
    .LOOP:
        mov edx, [ecx + edx*4]
        test edx, edx
        jg .LOOP


        ret