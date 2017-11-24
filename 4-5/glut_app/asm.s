;#IPA - lab5 solution
;#Author: Tomas Goldmann
;#STUDENT LOGIN: igoldmann
[bits 32]

SPHERES_COUNT equ 512

section .data


global _optim_function
export _optim_function


section .text
	%define spheres.x 0
	%define spheres.y SPHERES_COUNT*4
	%define spheres.z SPHERES_COUNT*2*4
	%define spheres.r SPHERES_COUNT*3*4
	%define spheres.m SPHERES_COUNT*4*4
	%define spheres.vector_x SPHERES_COUNT*5*4
	%define spheres.vector_y SPHERES_COUNT*6*4
	%define spheres.vector_z SPHERES_COUNT*7*4
	%define spheres.color SPHERES_COUNT*8*4
	
_optim_function:
	push ebp
	mov ebp, esp
	mov esi, [ebp+8]
	mov ecx, [ebp+12]
	mov ebx, [ebp+16]
	pushad
	mov edi, esp
	and esp,0xFFFFFFF0

	%define norm_x esp-16
	%define norm_y esp-32
	%define norm_z esp-48

	%define diff_x esp-16
	%define diff_y esp-32
	
	%define diff_z esp-48
	%define a1_dot esp-64
	%define a2_dot esp-80
	%define dt esp-96
	mov [dt],ebx
	;inner cycle edx/4
	

cycle_outer:
	
	movss xmm0, [esi+spheres.x+ecx*4-4]
	movss xmm1, [esi+spheres.y + ecx*4-4]
	movss xmm2, [esi+spheres.z + ecx*4-4]
	movss xmm6, [esi+spheres.r + ecx*4-4]
	
	shufps xmm0,xmm0,0
	shufps xmm1,xmm1,0
	shufps xmm2,xmm2,0
	shufps xmm6,xmm6,0


	lea edx,[esi+ecx*4-4]
cycle_inner:
	sub edx,16
	;# x values
	movups xmm3,[edx+spheres.x]
	;# y values
	movups xmm4,[edx+spheres.y]
	;# z values
	movups xmm5,[edx+spheres.z]

	;#
	movups xmm7,xmm0
	subps xmm7,xmm3
	movups [diff_x],xmm7
	mulps xmm7,xmm7
	movups xmm3,xmm7

	movups xmm7,xmm1
	subps xmm7,xmm4
	movups [diff_y],xmm7
	mulps xmm7,xmm7
	addps xmm3,xmm7

	movups xmm7,xmm2
	subps xmm7,xmm5
	movups [diff_z],xmm7
	mulps xmm7,xmm7
	addps xmm3,xmm7

	

	;# xmm3=sqrt()
	sqrtps xmm3,xmm3
	movups xmm7,[edx+spheres.r]

	xorps xmm4,xmm4
	cmpneqps xmm4,xmm3

	;# xmm4=r1+r2
	addps xmm7,xmm6
	;# xmm7=mask
	cmpnleps xmm7, xmm3
	andps xmm7,xmm4

	movups xmm4,[diff_x]
	divps xmm4,xmm3
	movups [norm_x],xmm4

	movups xmm4,[diff_y]
	divps xmm4,xmm3
	movups [norm_y],xmm4

	movups xmm4,[diff_z]
	divps xmm4,xmm3
	movups [norm_z],xmm4


	movss xmm4, [esi+spheres.vector_x+ecx*4-4]
	shufps xmm4,xmm4,0
	mulps xmm4,[norm_x]
	movups xmm5,xmm4

	movss xmm4, [esi+spheres.vector_y+ecx*4-4]
	shufps xmm4,xmm4,0
	mulps xmm4,[norm_y]
	addps xmm5,xmm4

	movss xmm4, [esi+spheres.vector_z+ecx*4-4]
	shufps xmm4,xmm4,0
	mulps xmm4,[norm_z]
	addps xmm5,xmm4
	movups [a1_dot],xmm5


	movups xmm4, [edx+spheres.vector_x]
	mulps xmm4,[norm_x]
	movups xmm5,xmm4

	movups xmm4, [edx+spheres.vector_y]
	mulps xmm4,[norm_y]
	addps xmm5,xmm4

	movups xmm4, [edx+spheres.vector_z]
	mulps xmm4,[norm_z]
	addps xmm5,xmm4
	;movups [a2_dot],xmm5

	;double P =( 2.0*(a1_dot - a2_dot))/(spheres[i].m+ spheres[j].m);
	subps xmm5,[a1_dot]
	addps xmm5,xmm5
	movss xmm4, [esi+spheres.m+ecx*4-4]
	shufps xmm4,xmm4,0
	movups xmm3,[edx+spheres.m]
	addps xmm4,xmm3
	divps xmm5,xmm4

	;spheres[i].vectorX = spheres[i].vectorX - P*spheres[j].m* n_x_norm;
	;spheres[i].vectorY = spheres[i].vectorY - P*spheres[j].m* n_y_norm;
	;spheres[i].vectorZ = spheres[i].vectorZ - P*spheres[j].m* n_z_norm;
	;spheres[j].vectorX = spheres[j].vectorX + P*spheres[i].m* n_x_norm;
	;spheres[j].vectorY = spheres[j].vectorY + P*spheres[i].m* n_y_norm;
	;spheres[j].vectorZ = spheres[j].vectorZ + P*spheres[i].m* n_z_norm;

	;for spheres[i].vectorX
	;update_vector(m,norm,vector)
	%macro update_vector_i 4
		movups xmm3,[edx+%1]
		mulps xmm3,xmm5
		mulps xmm3,[%2]

		movss xmm4,[esi+%3+ecx*4-4]
		andps xmm3,xmm7
		haddps xmm3,xmm3
		haddps xmm3,xmm3
		
		addss xmm4,xmm3
		movss [esi+%3+ecx*4-4],xmm4
	%endmacro

	update_vector_i spheres.m,norm_x,spheres.vector_x,xmm0
	update_vector_i spheres.m,norm_y,spheres.vector_y,xmm1
	update_vector_i spheres.m,norm_z,spheres.vector_z,xmm2

	;for spheres[j].vectorX
	;update_vector(m,norm,vector)
	%macro update_vector_j 3

		movss xmm3,[esi+%1+ecx*4-4]
		shufps xmm3,xmm3,0
		mulps xmm3,xmm5
		mulps xmm3,[%2]

		movups xmm4,[edx+%3]
		andps xmm3,xmm7
		subps xmm4,xmm3
		;addps xmm4,xmm3
		movups [edx+%3],xmm4
	%endmacro

	update_vector_j spheres.m,norm_x,spheres.vector_x
	update_vector_j spheres.m,norm_y,spheres.vector_y
	update_vector_j spheres.m,norm_z,spheres.vector_z

	movmskps eax,xmm7

	mov ebx,2
	;colision comparsion
	test eax,1
	je next1
	mov [esi+spheres.color+ecx*4-4],ebx
	mov [edx+spheres.color],ebx
next1:
	test eax,2
	je next2
	mov [esi+spheres.color+ecx*4-4],ebx
	mov [edx+spheres.color+4],ebx
next2:
	test eax,4
	je next3
	mov [esi+spheres.color+ecx*4-4],ebx
	mov [edx+spheres.color+8],ebx
next3:
	test eax,8
	je next4
	mov [esi+spheres.color+ecx*4-4],ebx
	mov [edx+spheres.color+12],ebx
next4:


	cmp edx,esi
	ja cycle_inner
test:
	movss xmm3,[dt]
	movss xmm4,[esi+spheres.vector_x+ecx*4-4]
	mulss xmm4,xmm3
	addps xmm0,xmm4
	movss xmm4,[esi+spheres.vector_y+ecx*4-4]
	mulss xmm4,xmm3
	addps xmm1,xmm4
	movss xmm4,[esi+spheres.vector_z+ecx*4-4]
	mulss xmm4,xmm3
	addps xmm2,xmm4

	movss [esi+spheres.x+ecx*4-4],xmm0
	movss [esi+spheres.y+ecx*4-4],xmm1
	movss [esi+spheres.z+ecx*4-4],xmm2
	
	dec ecx
	cmp ecx,4
	jne cycle_outer

	mov esp, edi
	popad
	mov esp, ebp
	pop ebp

ret