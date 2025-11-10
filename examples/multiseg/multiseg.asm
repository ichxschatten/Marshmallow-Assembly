format MZ

entry main:start
stack 100h

segment main

  start:
	mov	ax,data
	mov	ds,ax

	mov	dx,hello
	call	extra:write_text

	mov	ax,4C00h
	int	21h

segment data

  hello db 'Hello world!',24h

segment extra

  write_text:
	mov	ah,9
	int	21h
	retf
