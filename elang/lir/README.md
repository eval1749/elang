# LIR - Low-level Intermediate Representation

LIR is most likely target machine assemble code with virtual registers and
'phi' instruction.

LIR are implemented with following classes with:
 * `Instruction`
 * `Literal`
  * `BasicBlock`
  * `Function`
  * `Float32Literal`
  * `Float64Literal`
  * `Int32Literal`
  * `Int64Literal`
  * `StringLiteral`
 * `Value`
  * `Register`
  * `FloatRegiser`
  * `Immediate` (29-bit signed integer)
  * `VirtualRegister`

Unlike HIR, LIR has no types. But, registers are classified into general
purpose register to hold integer and address, and floating-pointer register
to hold floating pointer number. Literals are also classified integer, floating
pointer number, string, and address of function and basic block.

These classes provide getters. `Factory` class provides functions for creating
instances of them and `Editor` class provides functions for changing and
deleting.

# Basic Block
Successors of basic block are embedded in input operand list in the last
instruction.
