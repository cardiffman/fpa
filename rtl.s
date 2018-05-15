#
# rtl.s
#
#  Created on: May 11, 2018
#      Author: menright
#
# Assembly code implemnting TIM take/push/enter
# rax is the stack pointer, pushing to stack increases rax
# rbx is the frame pointer, Arg 0 will be at rbx, Arg 1 at 8(rbx)
# Any 'non-linear' flow is done explicitly with enter, translated to some jmp
# Thus any need for control to 'return' needs to be done explicitly by pushing
# the desired destination using equivalent of push label, and best push closure
.data
stack: .org .+3000
frame: .org .+3000
 fptr: .long 0 #Initialize to &frame in start
              #Points to location where next frame will start.

.text
take:
# Thus taking n arguments (rcx) from stack to make a frame
# rsi = top closure on stack = rax-16
# rdi = new frame location = fptr(%rip)
	movq fptr(%rip),%rdi   # Point to first new frame entry
	lea	-16(%rax),%rsi # Point to top closure
	or	%rcx, %rcx
	jnz 1f
	ret
1:	mov %rdi, %rbx    # Want this frame pointer at end
2:  mov (%rsi),%rdx
	mov %rdx,(%rdi)
	add $8,%rdi
	add $8,%rsi
	mov (%rsi),%rdx
	mov %rdx,(%rdi)
	add $8,%rdi
	sub $24,%rsi
	sub $16,%rax
	sub	$1,%rcx
	jnz	2b
	add $16,%rax    # overshoots
	movq %rdi,fptr(%rip)
	ret
	
SELF:
	# We come here upon entering certain closures.
	# The intention is to end up with the entered closure transfered to the stack
	# and the closure that was on top gets popped and enterd.
	# The intention is to enter the closure on top of the stack, but replace it
	# with the SELF PC and value.
	# One implementation had this code:
	sub $16,%rax  # rax now points to the closure that should be entered
	              # Now just 'enter arg 0'
				  # but first
	mov (%rax),%rcx # future pc
	mov 8(%rax),%rdx # future frame pointer
	mov SELF(%rip),%rsi
	mov %rsi,(%rax)
	mov %rbx,8(%rax)
     mov %rdx,%rbx   #;; new frame pointer to frame ptr
     jmp *%rcx #;; enter arg0;

	ret
