format MZ

	use16

	mov	ah,16h
	mov	al,86h
	int	2Fh
	or	al,ah
	jz	byte start
	mov	ax,4CFFh
	int	21h

	use32

  start:
	mov	ah,9
	mov	edx,hello
	int	21h

	mov	ax,4C00h
	int	21h

  hello db 'Hello from protected mode!',0Dh,0Ah,24h
