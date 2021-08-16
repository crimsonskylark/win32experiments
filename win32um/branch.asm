extern ExitProcess:PROC
public myFunc
public TestMethod

.data
sum DWORD 0

.code
myFunc PROC
	mov eax, 25
	mov ebx, 50
	add ebx, ebx
	mov sum, eax

	xor ecx, ecx
	ret
myFunc ENDP

TestMethod PROC
	xor rdx, rdx
	mov rdx, gs:[rdx+60h]
	mov rdx, [rdx+18h]
	mov rdx, [rdx+20h]
	mov rax, rdx
	ret
TestMethod ENDP
END