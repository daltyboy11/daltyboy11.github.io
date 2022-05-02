---
layout: post
title: EVM Puzzles
---

The [EVM puzzles](https://github.com/fvictorio/evm-puzzles) test
your understanding of EVM bytecode. I went down the rabbit hole
to understand contract bytecode and here I present to you my
solutions. A helpful source for understanding how opcodes interact
with the stack can be found [here](https://www.evm.codes/).

# Puzzle 1
The lines

```
CALLVALUE
JUMP
```

To jump to the destination we need `CALLVALUE = 0x08`.

# Puzzle 2
We want to choose a call value and code size such that
`CODESIZE - CALLVALUE == 0x06`. The codesize is 10 bytes
so we need the call value to be 4. 

# Puzzle 3
To jump to line 4 the call data size must be 4 bytes. Any
4 byte payload will do, like `0x00000000`.

# Puzzle 4
For this challenge we want the XOR of two arguments to jump to the
desired line. It will be helpful to look at `CALLVALUE` and `CODESIZE`,
and the line's binary representations.

The jump destination is line `0A`, or `1010` (base 2). The code size is 12
bytes, or `1100` (base 2). We want to choose a value `x` such that `x ^ 1100 == 1010`.
The answer is `6`.

# Puzzle 5
For this challenge I found it helpful to visualize the stack after each instruction.

```
CALLVALUE       => stack = [CALLVALUE]
DUP1            => stack = [CALLVALUE, CALLVALUE]
MUL             => stack = [CALLVALUE**2]
PUSH2 0100      => stack = [0x0100, CALLVALUE**2]
EQ              => ?
```

What do we have on the stack after `EQ`? Well, that depends on `CALLVALUE`.
Recall the `EQ` opcode consumes two items from the stack and produces one.
If the two items are equal it will push `0x01` onto the stack, otherwise it
will push `0x00` onto the stack. In our case we want it to push `0x01` so we
must set `CALLVALUE = sqrt(0x0100) = 16 (base 10)`

# Puzzle 6
The `CALLDATALOAD` opcode takes element `x` off the stack and reads the
32 byte word in calldata starting at offset `x`. In this challenge we're
reading from offset `0x00`. We want the first 32 bytes of call data to be
the jump destination: `0x000000000000000000000000000000000000000000000000000000000000000a`

# Puzzle 7
This challenge uses the calldata to deploy a contract with the `CREATE`
opcode. We essentially need to write creation + runtime contract bytecode
where the deployment bytecode is only a single byte in length.

## Contract creation bytecode
A quick primer on contract creation bytecode: the bytecode loads the runtime
bytecode into memory and **returns it**. We could write something like this

```
#################
Creation bytecode
#################
00 PUSH1 0x01   (0x6001)
02 PUSH1 0x0c   (0x600c)
04 PUSH1 0x00   (0x6000)
06 CODECOPY     (0x39)
07 PUSH1 0x01   (0x6001)
09 PUSH1 0x00   (0x6000)
0b RETURN       (0xF3)
################
Runtime bytecode
################ 
0c INVALID      (0xFE)
```

This code copies 1 byte of code to memory starting at line `0c`
and returns that byte. If we sent this bytecode with a contract
creation transaction then it would create a contract whose
bytecode is just the single byte `0xFE`. This is exactly what
we need to solve the puzzle.

As a bytecode string the answer is: `0x6001601160003960016000F3FE`.

# Puzzle 8
This is another creation bytecode puzzle. The code calls our
newly created contract and jumps only if the `CALL` opcode 
returns 0. Let's look at our opcode reference to figure out how
to make `CALL` return 0: if the call is successful it returns 1,
otherwise it returns 0. Therefore our newly created contract should
revert. All we have to do is change the runtime bytecode from our
previous solutions to be `REVERT`.

```
#################
Creation bytecode
#################
00 PUSH1 0x01   (0x6001)
02 PUSH1 0x0c   (0x600c)
04 PUSH1 0x00   (0x6000)
06 CODECOPY     (0x39)
07 PUSH1 0x01   (0x6001)
09 PUSH1 0x00   (0x6000)
0b RETURN       (0xF3)
################
Runtime bytecode
################ 
0c REVERT       (0xFD)
```

As a bytecode string the answer is: `0x6001600c60003960016000F3FD`.

# Puzzle 9
For the first jump we need `0x03 < CALLDATASIZE` and for the second
jump we need `CALLVALUE * CALLDATASIZE = 0x08`. To satisfy the
constraints we'll choose a 4 byte long calldata, e.g. 0x00000000
and a value of 2.

# Puzzle 10
For the first jump we need `CODESIZE > CALLVALUE`. For the second
jump we need `CALLDATASIZE % 3 == 0` and `CALLVALUE + 0x0a = 0x1b`.
A call value of 15 and calldata 0x000000 will satisfy the constraints.
