;Lab3-solution
;Login studenta:igoldmann

[BITS 64]
DEFAULT REL 
GLOBAL _DllMainCRTStartup
EXPORT _DllMainCRTStartup

GLOBAL ipa_algorithm
EXPORT ipa_algorithm

ARRAY_SIZE equ 64

section .data
	array TIMES 64 dw 5
	constants db 50,50,50,50
section .text

_DllMainCRTStartup:
	push rbp
	mov rbp, rsp
	

	mov rax,1
	mov rsp, rbp
	pop rbp
	ret 
		
ipa_algorithm:
	push rbp
	mov rbp, rsp

	mov r13, rdx
	mov r12, rcx

	;Task 3 - modifikace obrazku
	;vypocet poctu kroku pro paralelizaci a pro dozpracovani pole
	mov rax,r8
	mul r9
	mov r9,3
	mul r9
	mov r10, rax
	shr rax,2
	shl rax,2
	sub r10, rax
	shr rax,2

	;uprava obrazových BGR dat
	xor rcx, rcx
	cycle:
		lea rbx, [r12 + rcx*4]
		movq mm0, [rel rbx]
		movq mm1, [rel constants]
		paddusb mm0,mm1
		
		lea rbx, [r13 + rcx*4]
		movq  [rel rbx], mm0

		add rcx,1
		cmp rcx,rax
	jne cycle


	;vypocteni efektini adresy
	lea r12,[r12+rax*4]
	lea r13,[r13+rax*4]

	;zpracování zarovnání
	cycle2:
		cmp r10,0
		je end1
		dec r10
		lea rbx, [r12+r10]
		mov cl,  [rel rbx]
		xor edx, edx
		add cl,50
		setnc dl
		dec dl
		or cl,dl

		lea rbx, [r13+r10]
		mov [rel rbx],cl
	jmp cycle2
end1:

	;Task 1 - secteni word prvku v poli sekvencne
	mov rsi,array
    mov rcx,ARRAY_SIZE
    mov rax,0
	cyklus:
		cmp rcx,0
		jz end
    
		dec rcx
		add ax,[rel rsi+rcx*2]
		jmp cyklus    
	end:   

	;Task 2 - secteni word prvku v poli paralelne, predpoklada se pole o nasobku 4 word
	xor rax, rax
	pxor mm0,mm0
    mov rcx, ARRAY_SIZE
cyklus2:
    cmp rcx,0
    jbe end2
    
    sub rcx,4
    paddw mm0,[rel rsi+rcx*2]
	
    jmp cyklus2    
end2:  
    movd edx,mm0
    mov ax,dx
    shr edx,16
    add ax,dx
    pxor mm1,mm1
    punpckhwd mm0,mm1
    movd edx,mm0
    add ax,dx
    packssdw mm0,mm0
    movd edx,mm0
    shr edx,16
    add ax,dx
	mov rsp, rbp
	pop rbp

ret 0

	
