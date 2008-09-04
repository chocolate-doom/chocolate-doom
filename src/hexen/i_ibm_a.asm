	.386
	.MODEL  small

.DATA



.CODE

IF 0
#define PEL_WRITE_ADR   0x3c8
#define PEL_READ_ADR    0x3c7
#define PEL_DATA                0x3c9
ENDIF

;================
;
; I_DivException
;
;================

PROC  	I_DivException_
PUBLIC 	I_DivException_
	mov	edx,03c9h
	mov	al,63
	out	dx,al

	mov	ebx,0ffffffh
	mov	eax,[ebx]
	retf
ENDP

;================
;
; I_SetDivException
;
;================

PROC  	I_SetDivException_
PUBLIC 	I_SetDivException_
	pusha

	mov	eax,0212h
	mov	ebx,0
	mov	ecx,cs
	mov	edx,OFFSET I_DivException_
	int 31h
	jnc	good

	popa
	mov	eax,0
	ret

good:
	popa
	mov	eax,1
	ret

ENDP


;================
;
; I_ReadJoystick
;
; Read the absolute joystick values
; returns false if not connected
;================

.data

_joystickx	dd	0
_joysticky	dd	0
PUBLIC	_joystickx, _joysticky

.code

PROC  	I_ReadJoystick_
PUBLIC 	I_ReadJoystick_
	pushad
	pushf					; state of interrupt flag
	cli

	mov		dx,0201h
	in		al,dx
	out		dx,al		; Clear the resistors

	mov		ah,1		; Get masks into registers
	mov		ch,2

	xor		esi,esi		; Clear count registers
	xor		edi,edi
	xor		ebx,ebx		; Clear high byte of bx for later

	mov		ebp,10000	; joystick is disconnected if value is this big

jloop:
	in		al,dx		; Get bits indicating whether all are finished

	dec		ebp			; Check bounding register
	jz		bad			; We have a silly value - abort

	mov		bl,al		; Duplicate the bits
	and		bl,ah		; Mask off useless bits (in [xb])
	add		esi,ebx		; Possibly increment count register
	mov		cl,bl		; Save for testing later

	mov		bl,al
	and		bl,ch		; [yb]
	add		edi,ebx

	add		cl,bl
	jnz		jloop 		; If both bits were 0, drop out

done:
	mov		[_joystickx],esi
	shr		edi,1		; because 2s were added
	mov		[_joysticky],edi

	popf			; restore interrupt flag
	popad
	mov	eax,1		; read was ok
	ret

bad:
	popf			; restore interrupt flag
	popad
	xor     eax, eax	; read was bad
	ret

ENDP


END

