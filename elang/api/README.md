# API between components

* `MachineCodeBuilder` interface
* `SourceCodeRepository` interface
 * `SourceCodeLocation` object

## MachineCodeBuilder
VM implements `MachineCodeBuilder` interface and LIR code emitter consumes
this interface for emitting machine code from `Function`.

## SourceCodeRepository
Compiler and VM implement `SourceCodeRepository` for attaching source code
location information by `SourceCodeLocation` to HIR, LIR and machine code.
