[BITS 32]

; Export DllMain, so LoadLibrary in other module, could call this
GLOBAL getY
EXPORT getY
GLOBAL getX
EXPORT getX
GLOBAL update
EXPORT update


SECTION .data

balXmin DD 0.0
balYmin DD 0.0
balXmax DD 0.0
balYmax DD 0.0

ballY DD 0.000
ballX DD 0.000
stepY DD 0.1
stepX DD 0.2


SECTION .text

update:
	ENTER 4, 0
	PUSH EDX

	%define ballYMax [EBP + 20]
	%define ballXMax [EBP + 16]
	%define ballYMin [EBP + 12]
	%define ballXMin [EBP + 8]

	MOV EDX, ballXMin
	MOV [balXmin], EDX
	MOV EDX, ballYMin
	MOV [balYmin], EDX
	MOV EDX, ballXMax
	MOV [balXmax], EDX
	MOV EDX, ballYMax
	MOV [balYmax], EDX

	POP EDX
	LEAVE
	RET 0


getY:
	ENTER 0, 0

	FINIT
                         ; ST0      ST1      ST2
	FLD DWORD [ballY]    ; ballY
	FADD DWORD [stepY]
	FST DWORD [ballY]

	FLD DWORD [balYmax]  ; balYmax  ballY
	FCOMI ST1
	JA .min
	JMP .continue

.min:
	FLD DWORD [balYmin]  ; balYmin  balYmax   ballY
	FCOMI ST2
	JB .ret

.continue:
	FST DWORD [ballY]
	FLD DWORD [stepY]
	FCHS
	FST DWORD [stepY]

.ret:
	FLD DWORD [ballY]

	LEAVE
	RET 0


getX:
	ENTER 0, 0

	FINIT
                         ; ST0      ST1      ST2
	FLD DWORD [ballX]    ; ballX
	FADD DWORD [stepX]
	FST DWORD [ballX]

	FLD DWORD [balXmax]  ; balXmax  ballX
	FCOMI ST1
	JA .min
	JMP .continue

.min:
	FLD DWORD [balXmin]  ; balXmin  balXmax   ballX
	FCOMI ST2
	JB .ret

.continue:
	FST DWORD [ballX]
	FLD DWORD [stepX]
	FCHS
	FST DWORD [stepX]

.ret:
	FLD DWORD [ballX]

	LEAVE
	RET 0
