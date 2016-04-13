.equ ST_VALUE, 8
.equ ST_BUFFER, 12

.globl interger2string
.type interger2string, @function
interger2string:
	pushl %ebp
	movl %esp, %ebp
	# current char count
	movl $0, %ecx
	# current value 
	movl ST_VALUE(%ebp), %eax
	# base 10
	movl $10, %edi
conversion_loop:
	# division is actually performed on the combined %edx:%eax
	movl $0, %edx
	divl %edi
	addl $'0', %edx
	pushl %edx
	incl %ecx
	cmpl $0, %eax
	je end_conversion_loop
	jmp conversion_loop
end_conversion_loop:
	movl ST_BUFFER(%ebp), %edx
copy_reversing_loop:
	popl %eax
	movb %al, (%edx)
	decl %ecx
	incl %edx
	cmpl $0, %ecx
	je end_copy_reversing_loop
	jmp copy_reversing_loop
end_copy_reversing_loop:
	movb $0, (%edx)
	movl %ebp, %esp
	popl %ebp
	ret


