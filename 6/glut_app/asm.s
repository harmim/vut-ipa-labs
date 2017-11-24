;#IPA - lab6 AVX

;#Author: Tomas Goldmann
;#STUDENT LOGIN: igoldmann

[BITS 64]

GLOBAL BouncingBall_ASM
EXPORT BouncingBall_ASM


SECTION .data

SPHERES_COUNT EQU 4096
G DD 8.90
constant_0_5 DD 0.50
constant_n160 DD -160.0
constant_n160vec DD -160.0,-160.0,-160.0,-160.0,-160.0,-160.0,-160.0,-160.0


SECTION .text

%define spheres.x 0
%define spheres.y SPHERES_COUNT*4
%define spheres.z SPHERES_COUNT*2*4
%define spheres.r SPHERES_COUNT*3*4
%define spheres.v SPHERES_COUNT*4*4
%define spheres.q SPHERES_COUNT*5*4


BouncingBall_ASM:

	PUSH RBP
	MOV RBP, RSP
	; and spl,0x0f
	SUB RSP, 16


	; pointer - rsi
	MOV RSI, RCX
	
	; counter - rcx
	MOV RCX, RDX

	; dt - rbp-8 a rbx
	;mov rbx, r8
	MOVSS [RBP - 8], XMM2


	; dt*G => YMM5
	VBROADCASTSS YMM7, [RBP - 8]
	VBROADCASTSS YMM6, [REL G]
	VMULPS YMM5, YMM6, YMM7

	; 0.50 => YMM3
	VBROADCASTSS YMM3, [REL constant_0_5]

nav:
	LEA RDX, [REL RSI + 4 * RCX - 32]
	VMOVAPS YMM0, [REL RDX + spheres.v]

	; v_old => YMM1
	VMOVAPS YMM1, YMM0

	; speed_new => YMM2
	VMOVAPS YMM2, YMM0
	VADDPS YMM2, YMM2, YMM5

	; y_new YMM4
	VBROADCASTSS YMM7, [RBP - 8]
	VMOVAPS YMM4, [REL RDX + spheres.y]
	VMULPS YMM1, YMM1, YMM7
	VSUBPS YMM4, YMM4, YMM1
	VMOVAPS YMM6, YMM5
	VMULPS YMM6, YMM6, YMM7
	VMULPS YMM6, YMM6, YMM3
	VADDPS YMM4, YMM4, YMM6


	; if (y_new < -160.0)
	VMOVUPS YMM6, [REL constant_n160]
	VCMPLTPS YMM0, YMM4, YMM6
	VCMPGEPS YMM7, YMM4, YMM6
	VANDPS YMM4, YMM7, YMM4
	VMASKMOVPS YMM0, YMM0, [REL constant_n160vec]
	VORPS YMM4, YMM4, YMM0

	; set _spheres->y[i]
	VMOVAPS [REL RDX + spheres.y], YMM4

	; _spheres->v[i]
	VMASKMOVPS YMM6, YMM0, [REL RDX + spheres.q]
	VMULPS YMM6, YMM6, YMM2
	VANDPS YMM2, YMM2, YMM7
	VADDPS YMM2, YMM2, YMM6
	VMOVAPS [REL RDX + spheres.v], YMM2

	SUB RCX, 8
	CMP RCX, 0
	JG nav

	MOV RSP, RBP
	POP RBP

RET