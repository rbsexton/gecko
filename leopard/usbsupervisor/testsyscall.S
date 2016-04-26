.text
.syntax unified

.thumb_func     @  Make sure the linker puts it in the table.
.global PutChar

PutChar:
	svc #2
	bx lr

