extern ExitProcess:PROC
public myFunc
public TestMethod

.data
sum DWORD 0
buf db 0e9h, 42h, 42h, 42h, 42h, 42h, 42h, 42h, 42h

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

GetTeb PROC
	xor rax, rax
	mov rax, gs:[30h]
	push rdx
	xor rdx, rdx
	lea rdx, byte ptr [buf]
	xor rdx, rdx
	pop rdx
	ret
GetTeb ENDP

GetRIP PROC
	mov rax, [rsp]
	ret
GetRIP ENDP

END