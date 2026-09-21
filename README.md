# D5asm Assembler

An assembler for [DOT-5 fantasy console](https://github.com/Vglin21/dot-5).

## Getting Started

### Prerequisites
- A C compiler toolchain
- CMake (3.16+)

### Building for Windows/Linux
```sh
# Clone the repository
git clone https://github.com/Vglin21/d5asm
cd d5asm

# Configure and compile the assembler
cmake -B build
cmake --build build
```

### Assembling a Program
- **NOTE:** The repository includes example assembly code in `examples/`.

```sh
# Assemble the example source code
./bin/d5asm examples/pong.asm examples/pong.d5
```

## Architecture & Memory Map

| Address Range | Description   |
| :-----------: | :-----------: |
| `$00` - `$07` | RAM           |
| `$08` - `$FF` | Cartridge ROM |

### RAM Adresses

| Address       | Description                                                                  |
| :-----------: | ---------------------------------------------------------------------------- |
| `$00 bit 0`   | Set to `1` when console enters vblank state, must be cleared manually        |
| `$00 bit 1`   | `1` if `right` button is pressed, otherwise `0`                              |
| `$00 bit 2`   | `1` if `left` button is pressed, otherwise `0`                               |
| `$00 bit 3`   | `1` if `down` button is pressed, otherwise `0`                               |
| `$00 bit 4`   | `1` if `up` button is pressed, otherwise `0`                                 |
| `$01` - `$05` | Pixel positions, first 4 bits are X coordinate, last 4 bits are Y coordinate |
| `$06` - `$07` | Unused/Reserved                                                              |

## CPU Specifications

### Registers

- **8-bit Accumulator Register**
- **8-bit Program Counter (PC)**
- **Zero Flag**
  
### Instruction Set Architecture (ISA)

- **NOTE:** DOT-5 CPU uses only a nibble (first 4 bits of a byte) for it's instructions, so it only has 16 instructions total.  

| Instruction       | Full Name           | Bytecode | Function                                                                    | Zero Flag Updates? |
| :---------------: | :-----------------: | :------: | --------------------------------------------------------------------------- | :----------------: |
| `INC`             | Increment           | `$00`    | Increments accumulator                                                      | Yes                |
| `DEC`             | Decrement           | `$01`    | Decrements accumulator                                                      | Yes                |
| `LDA` (Immediate) | Load Accumulator    | `$02`    | Loads immediate value into an accumulator                                   | Yes                |
| `LDA` (Zero Page) | Load Accumulator    | `$03`    | Loads value from memory into an accumulator                                 | Yes                |
| `STA`             | Store Accumulator   | `$04`    | Stores accumulator into a memory slot                                       | No                 |
| `JMP`             | Jump                | `$05`    | Sets program counter to an immediate value                                  | No                 |
| `BEQ`             | Branch If Equal     | `$06`    | If zero flag is `true` then an immediate value is added to program counter  | No                 |
| `BNE`             | Branch If Not Equal | `$07`    | If zero flag is `false` then an immediate value is added to program counter | No                 |
| `ADD` (Immediate) | Add (duh)           | `$08`    | Adds an immediate value to accumulator                                      | Yes                |
| `ADD` (Zero Page) | Add                 | `$09`    | Adds a value from memory to accumulator                                     | Yes                |
| `SUB` (Immediate) | Subtract            | `$0A`    | Subtract an immediate value from accumulator                                | Yes                |
| `SUB` (Zero Page) | Subtract            | `$0B`    | Subtract a value from memory from accumulator                               | Yes                |
| `AND` (Immediate) | Bitwise AND         | `$0C`    | Performs bitwise AND on accumulator with an immediate value                 | Yes                |
| `AND` (Zero Page) | Bitwise AND         | `$0D`    | Performs bitwise AND on accumulator with a value from memory                | Yes                |
| `ORA` (Immediate) | Bitwise OR          | `$0E`    | Performs bitwise OR on accumulator with an immediate value                  | Yes                |
| `ORA` (Zero Page) | Bitwise OR          | `$0F`    | Performs bitwise OR on accumulator with a value from memory                 | Yes                |
