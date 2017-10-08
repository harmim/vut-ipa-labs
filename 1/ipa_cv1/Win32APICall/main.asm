[bits 32]
	
	; Import some function
	extern _LoadLibraryA@4
	extern _ExitProcess@4
	extern _FreeLibrary@4
	extern _WriteFile@20
	extern _GetStdHandle@4

	global  _main

	
section .data
	
	dllName				db "IPA_DLL.dll",0

	
		message:
    db      'Hello, World', 10
message_end:
	
section .bss
	
	hInstDll				resb 4
	
section .text


_main:

	; # Terminate program, quit
	push dword 2
	call _ExitProcess@4