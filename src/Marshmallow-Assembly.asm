	org	100h
	use16

start:

	mov	ah,4Ah
	mov	bx,1010h
	int	21h
	mov	dx,_logo
	mov	ah,9
	int	21h

	cld

	call	init_flatrm
	call	init_memory

	call	get_params
	cmp	[params],0
	je	information
	lea	eax,[params+1]
	mov	[input_file],eax
	movzx	ecx,byte [eax-1]
	add	eax,ecx
	cmp	byte [eax],0
	je	information
	inc	eax
	mov	[output_file],eax

	mov	ebx,46Ch
	sub	ebx,[program_base]
	mov	eax,[ebx]
	mov	[start_time],eax

	call	preprocessor
	call	parser
	call	assembler
	call	formatter

	movzx	eax,[current_pass]
	inc	al
	call	display_number
	mov	ah,9
	mov	dx,_passes_suffix
	int	21h
	mov	ebx,46Ch
	sub	ebx,[program_base]
	mov	eax,[ebx]
	sub	eax,[start_time]
	mov	ebx,100
	mul	ebx
	mov	ebx,182
	div	ebx
	or	eax,eax
	jz	display_bytes_count
	xor	edx,edx
	mov	ebx,10
	div	ebx
	push	edx
	call	display_number
	mov	ah,2
	mov	dl,'.'
	int	21h
	pop	eax
	call	display_number
	mov	ah,9
	mov	dx,_seconds_suffix
	int	21h
      display_bytes_count:
	mov	eax,[written_size]
	call	display_number
	mov	ah,9
	mov	dx,_bytes_suffix
	int	21h
	xor	al,al
	jmp	exit_program

information:
	mov	dx,_usage
	mov	ah,9
	int	21h
	mov	al,1
	jmp	exit_program

get_params:
	mov	si,81h
	mov	di,params
    find_param:
	lodsb
	cmp	al,20h
	je	find_param
	cmp	al,0Dh
	je	all_params
	or	al,al
	jz	all_params
	inc	di
	mov	bx,di
    copy_param:
	stosb
	lodsb
	cmp	al,20h
	je	param_end
	cmp	al,0Dh
	je	param_end
	or	al,al
	jz	param_end
	jmp	copy_param
    param_end:
	dec	si
	xor	al,al
	stosb
	mov	ax,di
	sub	ax,bx
	mov	[bx-1],al
	jmp	find_param
    all_params:
	xor	al,al
	stosb
	ret

include 'system.inc'
include 'errors.inc'

include 'expressions.inc'
include 'preprocessor.inc'
include 'parser.inc'
include 'assembler.inc'
include 'formats.inc'
include 'tables.inc'

_copyright db 'Copyright (c) 2025, Help From the Void Independent Systems (HFtV-IS)',24h

_logo db 'Marshmallow-Assembly development version 0.0.0.0',0Dh,0Ah,24h
_usage db 'usage: Marshmallow-Assembly source output',0Dh,0Ah,24h

_passes_suffix db ' passes, ',24h
_seconds_suffix db ' seconds, ',24h
_bytes_suffix db ' bytes.',0Dh,0Ah,24h

program_base dd ?

conventional_memory dd ?
conventional_memory_end dd ?
memory_start dd ?
memory_end dd ?

input_file dd ?
output_file dd ?

source_start dd ?
code_start dd ?
code_size dd ?
real_code_size dd ?

start_time dd ?
written_size dd ?

params rb 100h
buffer rb 4000h
