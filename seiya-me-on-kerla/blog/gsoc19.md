---
title: GSoC'19 - Improve LLVM binary utilities
date: Aug 22, 2019
lang: en
---

## Abstract
**[Proposal is here.](https://docs.google.com/document/d/1HpgtcUt02NG-2cRf6eSwGMi2fSZWOSE9vt0JPACG-VM/edit?usp=sharing)**

LLVM includes binary utilities equivalent to GNU binutils. Basic 
functionalities are implemented but there are incomplete ones such as 
Mach-O support. This project aims to support those missing functionalities 
and improve usability those who crave for an alternative to GNU binutils.
            
The goal of this project is twofold:

- llvm-objcopy: Improve Mach-O support
- llvm-objdump: More human-friendly disassembly

## Status
I've implemented all functionalities proposed in the proposal but majority 
of them are under review. For more details, see patches listed below.

## Acknowledgements
I would like to express my special thanks to James Henderson and Jordan 
Rupprecht for mentoring me to become a part of the LLVM community. With 
their awesome support, I learned and experienced a lot of precious things
throughout the GSoC and it was really (really!) fun.

## Authored Patches in reviews.llvm.org
- [[llvm-objcopy][MachO] Implement -Obinary](https://reviews.llvm.org/D66409)
- [[llvm-objcopy][MachO] Implement --dump-section](https://reviews.llvm.org/D66408)
- [[llvm-objcopy][MachO] Implement -Ibinary](https://reviews.llvm.org/D66407)
- [[llvm-objcopy][MachO] Implement --add-section](https://reviews.llvm.org/D66283)
- [[llvm-objcopy][MachO] Implement --remove-section](https://reviews.llvm.org/D66282)
- [[llvm-objcopy][MachO] Implement --strip-all](https://reviews.llvm.org/D66281)
- [[llvm-objcopy][MachO] Support indirect symbol table](https://reviews.llvm.org/D66280)
- [[llvm-readobj][MachO] Fix section type printing](https://reviews.llvm.org/D66075)
- [[yaml2obj/obj2yaml][MachO] Allow setting custom section data](https://reviews.llvm.org/D65799)
- [[llvm-objcopy][MachO] Implement --only-section](https://reviews.llvm.org/D65541)
- [[llvm-objcopy][MachO] Fix method names. NFC.](https://reviews.llvm.org/D65540)
- [[llvm-objcopy][MachO] Implement a layout algorithm for executables](https://reviews.llvm.org/D65539)
- [[llvm-objdump] Implement highlighting](https://reviews.llvm.org/D65191)
- [[X86] X86ATTInstPrinter: replace markup with startMarkup/endMarkup](https://reviews.llvm.org/D65190)
- [[MC] Support returning marked up ranges in the disassembly](https://reviews.llvm.org/D65189)
- [[MC] Support returning a structured rich disassembly output](https://reviews.llvm.org/D65188)
- [[llvm-objdump][NFC] Make the PrettyPrinter::printInst() output buffered](https://reviews.llvm.org/D64969)
- [[llvm-objdump] Implement syntax highlighting](https://reviews.llvm.org/D64311)
- [[llvm-objcopy][NFC] Refactor output target parsing v2](https://reviews.llvm.org/D64170)
- [[MC] Add MCInstrAnalysis::evaluateMemoryOperandAddress](https://reviews.llvm.org/D63847)
- [[llvm-objcopy][MachO] Fix strict-aliasing warning. NFCI](https://reviews.llvm.org/D63699)
- [[llvm-objcopy][MachO] Support load commands used in executables/shared libraries](https://reviews.llvm.org/D63395)
- [[llvm-objcopy][MachO] Rebuild the symbol/string table in the writer](https://reviews.llvm.org/D63309)
- [[llvm-objcopy] Fix sparc target endianness](https://reviews.llvm.org/D63251)
- [[llvm-objcopy][NFC] Refactor output target parsing](https://reviews.llvm.org/D63239)
- [[llvm-objcopy] Add elf32-sparc and elf32-sparcel target](https://reviews.llvm.org/D63238)
- [[llvm-objcopy] Recognize the output file format other than ELF](https://reviews.llvm.org/D63184)
- [[yaml2obj][MachO] Don't fill dummy data for virtual sections](https://reviews.llvm.org/D62991)
- [[llvm-objcopy][MachO] Recompute and update offset/size fields in the writer](https://reviews.llvm.org/D62652)
- [[llvm-objcopy][MachO] Print an error message on use of unsupported options](https://reviews.llvm.org/D62578)


## Patches that are not yet uploaded to Phabricator
Those patches are ready to upload but I'd like to focus on already sent 
patches.
            
- [[llvm-objdump] Print symbol names referenced by memory operands](https://github.com/llvm/llvm-project/commit/572f934dd09556b331b951471af5681ce311b3b9)
- [[llvm-objdump] Print string literals](https://github.com/llvm/llvm-project/commit/9885e7fd72c954600d15e3d1f7ae77cbc5d89f6f)
- [[llvm-objdump] Highlight relocations](https://github.com/llvm/llvm-project/commit/41e2ea6162142100cb063daadccacb56327788f8)
- [[llvm-objcopy][MachO] Implement --set-section-flags](https://github.com/llvm/llvm-project/commit/5eea6180a4faa2c20f9932eafafcd78a704b834c)
- [[llvm-objcopy][MachO] Implement --rename-section](https://github.com/llvm/llvm-project/commit/aa89503aae1287b61577db03ac36ecafc94e4cc1)
