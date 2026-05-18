# 8-Bit Assembler & Virtual Machine

A semester-long systems programming project: a two-stage pipeline written in C that **assembles** a custom assembly language into packed machine code and **executes** it inside a software-emulated virtual machine.

---

## Overview

The project is split into two tightly coupled components:

| Component | Role |
|---|---|
| **Assembler** | Parses `.asm` source files, resolves operands, and encodes each instruction into a packed 16-bit word stored in a 150-word virtual memory array |
| **Virtual Machine** | Fetches, decodes, and executes the machine code word-by-word, maintaining four general-purpose registers, a flag register, and a downward-growing call stack |

After execution the full address space is printed in both decimal and hex so you can inspect every memory location.

---

## Architecture

### Memory Model

- **150 16-bit words** form the entire address space (addresses `0`–`149`)
- Program code and program data coexist in the same flat address space
- The **call stack** grows downward from address `149`; the program counter starts at `0`

### Registers

| Register | Purpose |
|---|---|
| `AX` | Primary accumulator; also the I/O register (`GET`/`PUT`) |
| `BX` | Base/index register; used for indirect and offset memory addressing |
| `CX` | Count register |
| `DX` | Data / general-purpose register |
| **Flag** | Three-valued comparison result: `-1` (less), `0` (equal), `1` (greater) |

### Instruction Encoding

Every instruction is packed into a single **16-bit word** using three bit fields:

```
Bits:  7  6  5  |  4  3  |  2  1  0
       CMD (3)  |  OP2   |  OP3 (3)
```

| Field | Bits | Values |
|---|---|---|
| `CMD` | 7–5 | Opcode family (8 possible commands) |
| `OP2` | 4–3 | Second operand type (4 values) |
| `OP3` | 2–0 | Third operand type (8 values: registers, memory modes, constants) |

When an operand is a literal (a constant or a direct address), its value is stored in the **next memory word**, making instructions either 1 or 2 words long.

---

## Instruction Set

### One-Operand Instructions

| Mnemonic | Description |
|---|---|
| `HALT` | Stop execution |
| `GET` | Read an integer from stdin into `AX` |
| `PUT` | Print `AX` to stdout |
| `RET` | Return from a function call: restore all registers and jump to the return address |

### Jump Instructions (conditional / unconditional)

All jumps take a single address operand.

| Mnemonic | Condition | Flag value |
|---|---|---|
| `JMP [addr]` | Unconditional | — |
| `JE  [addr]` | Equal | `0` |
| `JNE [addr]` | Not Equal | `≠ 0` |
| `JB  [addr]` | Below (less than) | `-1` |
| `JBE [addr]` | Below or Equal | `≤ 0` |
| `JA  [addr]` | Above (greater than) | `1` |
| `JAE [addr]` | Above or Equal | `≥ 0` |

### Two-Operand Instructions

| Mnemonic | Operation |
|---|---|
| `MOV  dest, src` | Copy `src` → `dest` (register–register or register–memory) |
| `ADD  dest, src` | `dest += src` |
| `SUB  dest, src` | `dest -= src` |
| `CMP  op1, op2`  | Set flag: `-1` if `op1 < op2`, `0` if equal, `1` if `op1 > op2` |

### Function Call

```asm
FUN [funcAddr] numParams param1 param2 ...
```

`FUN` pushes the return address and saves all four registers and the flag onto the stack, then jumps to `funcAddr`. Inside the function `[BX+offset]` addressing gives access to the parameters. `RET` restores everything and resumes after the `FUN` instruction.

---

## Addressing Modes

| Syntax | Code | Meaning |
|---|---|---|
| `AX` / `BX` / `CX` / `DX` | `0`–`3` | Register direct |
| `[BX]` | `4` | Memory at address stored in `BX` |
| `[BX+n]` | `5` | Memory at `BX + n` (base + offset) |
| `[addr]` | `6` | Memory at literal 16-bit address |
| `n` | `7` | Immediate constant |

---

## Example Programs

All examples live in `ASMFiles/`.

### Multiplication via repeated addition (`JackWiegman_ASMPart6.asm`)

Two values are pre-loaded at the end of the address space. The program multiplies them by adding one value to an accumulator in a counted loop, then outputs intermediate totals each iteration.

```asm
mov bx [18]        ; load multiplier
mov cx [19]        ; load multiplicand
mov ax 0           ; accumulator = 0
mov dx ax          ; loop counter = 0
cmp dx cx          ; counter >= multiplicand?
jae [17]           ; yes → done
add ax bx          ; accumulator += multiplier
add dx 1           ; counter++
jmp [8]
halt
```

### Input validation with a function call (`JackWiegman_ASMPart9.asm`)

Reads 10 numbers from the user (each must be in [30, 70]), prints a running count and running total, and outputs the final sum. Demonstrates `FUN`/`RET`, `[BX+offset]` parameter access, and conditional jumps.

### Category counting with array indexing (`Exam.asm`)

Reads integers until a negative value is entered, assigns each to a category (< 10, 10–30, > 30) by calling a function, increments a counter array in memory using `BX`-relative addressing, and prints the three bucket totals.

---

## Build & Run

### Prerequisites

- CMake ≥ 3.20
- GCC (Linux) or MSVC / MinGW (Windows)
- Ninja (recommended) or Make

### Linux — quick start

```bash
# Configure + build + run
bash run.sh
```

`run.sh` is equivalent to:

```bash
cmake --preset=linux-gcc-ninja
cmake --build --preset=linux-build
./out/linux/build/debug/Assembler
```

### Windows (MSVC)

```powershell
cmake --preset=win-msvc-vs2022
cmake --build --preset=win-build
.\out\win\build\debug\Assembler.exe
```

Other presets (`win-mingw-ninja`, `linux-clang-ninja`, etc.) are defined in `CMakePresets.json`.

---

## Usage

When the program starts you are prompted for an `.asm` file to run. Press **Enter** to use the default (`JackWiegman_ASMPart9.asm`) or type the name of any file in the working directory.

```
Enter ASM file name (default: JackWiegman_ASMPart9.asm): Exam.asm
```

The assembler loads and encodes the file, prints the initial memory dump, runs the VM, then prints the final memory state in decimal and hex.

---

## Project Structure

```
8BitAssembler/
├── src/
│   └── JackWiegman_AssemblerPart9.c   # Assembler + VM (~1 300 lines of C)
├── ASMFiles/
│   ├── JackWiegman_ASMPart5.asm       # Arithmetic & memory operations
│   ├── JackWiegman_ASMPart6.asm       # Multiplication loop
│   ├── JackWiegman_ASMPart7.asm       # Summing 10 user inputs
│   ├── JackWiegman_ASMPart8.asm       # Input validation with function calls
│   ├── JackWiegman_ASMPart9.asm       # Full function parameter passing
│   └── Exam.asm                       # Category counting with array indexing
├── CMakeLists.txt
├── CMakePresets.json
└── run.sh
```

The single source file is organized into clearly separated sections: constants & defines → data structures → assembler pipeline → VM execution engine → operand helpers → utilities.

---

## Assembly Language Syntax

```asm
; Semicolons begin comments — the rest of the line is ignored

; Numeric literals on their own line are written directly to memory
42

; Labels are noted in comments (the assembler uses absolute addresses)
;** [loopStart]: 5

; Instructions: mnemonic operand1 operand2
mov ax 0             ; AX = 0
mov bx [50]          ; BX = mem[50]
mov [bx+2] cx        ; mem[BX+2] = CX
add ax bx            ; AX += BX
cmp cx dx            ; flag = (CX compared to DX)
jae [37]             ; jump to address 37 if CX >= DX
fun [41] 1 [68]      ; call function at 41, pass 1 param at address 68
ret                  ; return from function
halt                 ; stop
```

---

## Key Design Decisions

- **Packed 16-bit instruction word** — fitting opcode, source, and destination encoding into a single 16-bit value keeps the memory footprint small and makes the decoder a few bit-mask operations.
- **Three-valued flag register** — a single flag that can be `-1 / 0 / 1` enables all six relational comparisons without needing multiple flag bits.
- **Stack-based function calls with full register save** — `FUN` pushes every register and the flag before jumping, so functions can freely use all registers and `RET` restores the caller's state exactly.
- **Unified address space** — code and data share the same 150-word array, matching how real embedded targets often work and simplifying the VM implementation.
