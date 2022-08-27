# PL-EarlyC
Early C 3D Software Renderer Public Domain Source Release by EMMIR 2018-2022
==============================================================================



https://user-images.githubusercontent.com/109979235/187040347-88be4944-57c3-4c76-b37b-a5f1a0cef786.mp4



The code here is an early C implementation of PiSHi LE.
PiSHi LE is a subset of the integer-only 3D graphics library used
in King's Crook, a software rendered game.

## General Info:

This code is written for an early C compiler for the Motorola 68000 processor.
(an old version of the Portable C Compiler from around 1981).
Along with an assembler and linker from the same time period.
-- Side note: even though the Portable C Compiler supports the void type,
structure assignment, and enums, I don't use them in this code.

I modified the compiler a bit to add special handling for inline ASM and
interrupt service routines.
I also stripped any support for floats or doubles from it.

The assembled code runs on a modified Motorola 68000 emulator which I
also stripped of floating point support.

The program interacts with simulated hardware components through
memory mapped I/O.

Hardware components I'm simulating are a clock, keyboard, terminal
and a VDC (Video Device Controller) -- a simple 256/true color video card.

Unlike PL3D-KC, this release is not as general as it utilizes 16-bit
operations where necessary to improve speed since the 68000 does not support
hardware 32-bit math operations.

## Purpose:

Seeing how C looked back in its early days.
Learning about the interesting features of early C.

Main things to notice are:
- K&R function declarations
- A function declaration was only necessary when its return type was not int
  (I kept unneeded decls so the reader could easily see what functions exist)
- B-style initialization support (e.g  x 0;)
- Old assignment operator (e.g  =<< instead of <<=, or =+ instead of +=)
- Heavy use of implied-integer type whenever a type is not explicitly written.
- Auto (local) variables must be declared at top of scope.
- No void type.
- Six character external symbol limit (for linking).
- No fancy preprocessing, only #define and #include
- Octal literals with 8 and 9 were legal and were interpreted as 010 and 011 respectively.
- Struct member names are global for all structs.
- You can use an arrow operator -> to access first member of a struct
  on (for example) an integer to access memory at that location.
   ex:
   ```
   struct { int iaddr; };  ... 0720->iaddr = 0;
   ```
   The above is the same as
   ```
   *((int *)0720) = 0;
   ```
   
## Feature List:

- N-gon rendering (not limited to triangles)
- Depth (Z) buffering
- Flat polygon filling
- Affine texture mapped polygon filling
- Near plane clipping
- Viewport clipping
- Back face culling
- Immediate mode interface
- Matrix stack for transformations
- Code to generate a box
- Indexed or true color rendering

## External Resources:

https://spin0r.wordpress.com/2014/11/21/kr-c/

https://news.ycombinator.com/item?id=10165007


YouTube channel and Discord server for my game King's Crook:

YouTube: https://www.youtube.com/c/LMP88

Discord: https://discord.gg/hdYctSmyQJ
