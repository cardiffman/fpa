# fpa
Functional Programming for Amateurs.

This project uses the fundamentals we learned from the Three-Instruction Machine to make a compiler 
that generates assembly code from functional programs.

The Three-Instruction Machine in its most fundamental guise works on closures, using a stack of closures, a frame containing some more closures, and three instructions:

- TAKE n
  Take n arguments from the stack and make a frame from them
- PUSH [item]
  Push the item on the stack
- ENTER [item]
  Enter the item

An [item] is a means of designating a closure to be entered or pushed. An item like 'ARG 0' refers to the 0th closure in the frame. All items that you can push or enter are closures. Some items have one of those . 

A closure contains a frame and a PC.

Some mechanical details of this for our x86-64 implementation are described in the rtl.s file. Further details:

A closure that represents a literal integer has the special PC value SELF and the integer is the frame pointer.

Lazy evaluation as always has two requirements: Defering evaluation of items until required, and not evaluating that which has been evaluated. A closure can be read to say, "evaluate factorial(5)" but at some point the expresssion is evaluated and the closure then hopefully says "it's 120". Defering evaluation is fun but it's better to transform expressions into memos of their values. This particular system uses markers to help with that. A marker goes on the stack to indicate what has not been evaluated.

The [item] notation above can be one of the following:
- A label
  The item is the current frame pointer and the given label. When a label is entered no effort is expended on changing the frame pointer.
- Argument N
  The item is the Nth element of the stack.
- A literal number
  This pushes or enters an integer closure as described above.

When entering a program function, or "supercombinator", a frame will be created by the Take instruction at the start of the function. So when a function calls a function, the caller will push a label located after the instruction that enters that combinator. That push will push the current frame.
