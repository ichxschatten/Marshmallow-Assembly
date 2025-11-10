dd FFFFFFFFh
dw A000h
dw strategy
dw interrupt
db 'DEVICE  '

strategy:
	mov	word [cs:request_header],bx
	mov	word [cs:request_header+2],es
	retf

interrupt:
	pusha
	push	ds es
	push	cs
	pop	ds
	mov	bx,word [request_header]
	mov	es,word [request_header+2]
	mov	al,byte [es:bx+2]
	or	al,al
	jz	initialize_device
	cmp	al,4
	je	device_input
    interrupt_done:
	mov	word [es:bx+3],100h
	pop	es ds
	popa
	retf

initialize_device:
	mov	word [es:bx+0Eh],device_end
	mov	word [es:bx+10h],cs
	jmp	interrupt_done

device_input:
	push	es
	mov	cx,[es:bx+12h]
	mov	di,[es:bx+0Eh]
	mov	es,[es:bx+10h]
	mov	al,1Ah
	rep	stosb
	pop	es
	jmp	interrupt_done

request_header dd ?

device_end:
