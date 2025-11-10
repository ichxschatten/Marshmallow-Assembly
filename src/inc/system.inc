init_flatrm:
	push	sp
	pop	ax
	cmp	ax,sp
	jne	processor_error
	smsw	ax
	test	al,1
	jnz	pm_error
	pushf
	pop	ax
	or	ah,F0h
	push	ax
	popf
	pushf
	pop	ax
	test	ah,F0h
	jz	processor_error
	xor	eax,eax
	mov	ax,cs
	shl	eax,4
	add	eax,GDT
	mov	dword [cs:GDTR+2],eax
	cli
	lgdt	pword [cs:GDTR]
	mov	eax,cr0
	or	al,1
	mov	cr0,eax
	jmp	pm_start
    pm_start:
	mov	ax,1 shl 3
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	eax,cr0
	and	al,not 1
	mov	cr0,eax
	jmp	pm_end
    pm_end:
	xor	eax,eax
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	shl	eax,4
	mov	[program_base],eax
	sti
	ret

GDTR dw 2*8-1
     dd ?

GDT rw 4
    dw FFFFh,0,9200h,8Fh

init_memory:
	mov	ax,4300h
	int	2Fh
	cmp	al,80h
	je	xms_init
	mov	ah,88h
	int	15h
	or	ax,ax
	jz	no_extended_memory
	movzx	eax,ax
	shl	eax,10
	add	eax,100000h
	sub	eax,[program_base]
	mov	[memory_end],eax
	push	ds
	push	0
	pop	ds
	call	enable_a20
	call	test_a20
	jz	a20_ok
	pop	ds
	jmp	no_extended_memory
    a20_ok:
	lds	bx,dword [4*19h]
	mov	eax,100000h
	cmp	dword [bx+12h],'VDIS'
	jne	byte no_vdisk
	mov	eax,dword [bx+2Ch]
	add	eax,Fh
	and	eax,FFFFF0h
    no_vdisk:
	push	FFFFh
	pop	ds
	cmp	dword [13h],'VDIS'
	jne	byte vdisk_ok
	movzx	ebx,word [2Eh]
	shl	ebx,10
	cmp	eax,ebx
	ja	byte vdisk_ok
	mov	eax,ebx
    vdisk_ok:
	pop	ds
	sub	eax,[program_base]
	mov	[memory_start],eax
	jmp	extended_memory_ok

enable_a20:
	call	test_a20
	jz	a20_enabled
	in	al,92h
	or	al,2
	out	92h,al
	call	test_a20
	jz	a20_enabled
	call	kb_wait
	jnz	a20_enabled
	mov	al,0D1h
	out	64h,al
	call	kb_wait
	jnz	a20_enabled
	mov	al,0DFh
	out	60h,al
	call	kb_wait
    a20_enabled:
	ret
    kb_wait:
	xor	cx,cx
      .loop:
	in	al,64h
	test	al,2
	loopnz	.loop
	ret
    test_a20:
	mov	al,[0]
	mov	ah,al
	not	al
	xchg	al,[100000h]
	cmp	ah,[0]
	mov	[100000h],al
	ret

xms_init:
	push	es
	mov	ax,4310h
	int	2Fh
	mov	word [xms_call],bx
	mov	word [xms_call+2],es
	pop	es
	mov	ah,3
	call	far dword [xms_call]
	cmp	ax,1
	jne	no_extended_memory
	mov	ah,8
	xor	bl,bl
	call	far dword [xms_call]
	or	bl,bl
	jnz	no_extended_memory
	mov	dx,ax
	movzx	eax,ax
	shl	eax,10
	mov	[memory_end],eax
	mov	ah,9
	call	far dword [xms_call]
	mov	[xms_handle],dx
	cmp	ax,1
	jne	no_extended_memory
	mov	ah,Ch
	call	far dword [xms_call]
	cmp	ax,1
	jne	no_extended_memory
	shl	edx,16
	mov	dx,bx
	sub	edx,[program_base]
	mov	[memory_start],edx
	add	[memory_end],edx
	jmp	extended_memory_ok

no_extended_memory:
	xor	eax,eax
	mov	[memory_start],eax
extended_memory_ok:
	mov	ah,48h
	mov	bx,-1
	int	21h
	movzx	ecx,bx
	shl	ecx,4
	mov	ah,48h
	int	21h
	movzx	edi,ax
	shl	edi,4
	sub	edi,[program_base]
	mov	[conventional_memory],edi
	mov	[conventional_memory_end],edi
	add	[conventional_memory_end],ecx
	cmp	[memory_start],0
	jne	memory_ok
	shr	ecx,2
	add	edi,ecx
	mov	[memory_start],edi
	xchg	[conventional_memory_end],edi
	mov	[memory_end],edi
    memory_ok:
	ret

exit_program:
	cmp	[xms_handle],0
	je	byte .exit
	push	ax
	mov	ah,Dh
	mov	dx,[xms_handle]
	call	far dword [xms_call]
	mov	ah,Ah
	call	far dword [xms_call]
	pop	ax
      .exit:
	mov	ah,4Ch
	int	21h

xms_call dd ?
xms_handle dw ?

open:
	push	esi di
	mov	esi,edx
	mov	di,buffer
      .name:
	lods	byte [esi]
	stos	byte [di]
	or	al,al
	jnz	.name
	mov	dx,buffer
	mov	ax,3D00h
	int	21h
	mov	bx,ax
	pop	di esi
	ret
create:
	push	esi edi
	mov	esi,edx
	mov	di,buffer
      .name:
	lods	byte [esi]
	stos	byte [es:di]
	or	al,al
	jnz	.name
	mov	dx,buffer
	mov	ah,3Ch
	xor	cx,cx
	int	21h
	mov	bx,ax
	pop	edi esi
	ret
write:
	push	esi edi ebp
	mov	ebp,ecx
	mov	esi,edx
      .loop:
	mov	ecx,4000h
	sub	ebp,4000h
	jnc	.write
	add	ebp,4000h
	mov	cx,bp
	xor	ebp,ebp
      .write:
	push	cx
	mov	edi,buffer
	shr	cx,2
	rep	movs dword [edi],[esi]
	mov	cx,[esp]
	and	cx,11b
	rep	movs byte [edi],[esi]
	pop	cx
	mov	dx,buffer
	mov	ah,40h
	int	21h
	or	ebp,ebp
	jnz	.loop
	pop	ebp edi esi
	ret
read:
	push	esi edi ebp
	mov	ebp,ecx
	mov	edi,edx
      .loop:
	mov	ecx,4000h
	sub	ebp,4000h
	jnc	.read
	add	ebp,4000h
	mov	cx,bp
	xor	ebp,ebp
      .read:
	push	cx
	mov	dx,buffer
	mov	ah,3Fh
	int	21h
	mov	esi,buffer
	mov	cx,[esp]
	shr	cx,2
	rep	movs dword [edi],[esi]
	pop	cx
	and	cx,11b
	rep	movs byte [edi],[esi]
	or	ebp,ebp
	jnz	.loop
	pop	ebp edi esi
	ret
close:
	mov	ah,3Eh
	int	21h
	ret
lseek:
	mov	ah,42h
	mov	ecx,edx
	shr	ecx,16
	int	21h
	shl	edx,16
	mov	dx,ax
	mov	eax,edx
	ret
