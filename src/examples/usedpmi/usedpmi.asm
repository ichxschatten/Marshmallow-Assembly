format MZ
heap 0

segment loader use16

	push	cs
	pop	ds

	mov	ax,1687h
	int	2Fh
	or	ax,ax
	jnz	error
	test	bl,1
	jz	error
	mov	word [mode_switch],di
	mov	word [mode_switch+2],es
	mov	bx,si
	mov	ah,48h
	int	21h
	jc	error
	mov	es,ax
	mov	ax,1
	call	far [mode_switch]
	jc	error

	mov	cx,1
	xor	ax,ax
	int	31h
	mov	si,ax
	xor	ax,ax
	int	31h
	mov	di,ax
	mov	dx,cs
	lar	cx,dx
	shr	cx,8
	or	cx,C000h
	mov	bx,si
	mov	ax,9
	int	31h
	mov	dx,ds
	lar	cx,dx
	shr	cx,8
	or	cx,C000h
	mov	bx,di
	int	31h
	mov	ecx,main
	shl	ecx,4
	mov	dx,cx
	shr	ecx,16
	mov	ax,7
	int	31h
	mov	bx,si
	int	31h
	mov	cx,FFFFh
	mov	dx,FFFFh
	mov	ax,8
	int	31h
	mov	bx,di
	int	31h

	mov	ds,di
	mov	es,di
	mov	fs,di
	mov	gs,di
	push	si
	push	start
	retf

    error:
	mov	ax,4CFFh
	int	21h

  mode_switch dd ?

segment main use32

  start:
	mov	esi,hello
    .loop:
	lodsb
	or	al,al
	jz	.done
	mov	dl,al
	mov	ah,2
	int	21h
	jmp	.loop
    .done:

	mov	ax,4C00h
	int	21h

  hello db 'Hello from protected mode!',0Dh,0Ah,0
