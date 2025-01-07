Utility elf2exe takes an ELF binary compiled for RP/M and modifies the code
in .plt section for proper linking by dynamic loader at run time.

# x86_64
Example of .plt section on x86_64 (or amd64) architecture:
```
Disassembly of section .plt:

0000000000001000 <.plt>:
    1000:       ff 35 ea 2f 00 00       push   0x2fea(%rip)         # 3ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
    1006:       ff 25 ec 2f 00 00       jmp    *0x2fec(%rip)        # 3ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
    100c:       0f 1f 40 00             nopl   0x0(%rax)
    1010:       f3 0f 1e fa             endbr64
    1014:       68 00 00 00 00          push   $0x0
    1019:       e9 e2 ff ff ff          jmp    1000 <rpm_puts@plt-0x20>
    101e:       66 90                   xchg   %ax,%ax

Disassembly of section .plt.sec:

0000000000001020 <rpm_puts@plt>:
    1020:       f3 0f 1e fa             endbr64
    1024:       ff 25 d6 2f 00 00       jmp    *0x2fd6(%rip)        # 4000 <rpm_puts>
    102a:       66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)
```

# arm64
Example of .plt section on arm64 architecture:
```
0000000000000220 <.plt>:
 220:   a9bf7bf0        stp     x16, x30, [sp, #-16]!
 224:   f00000f0        adrp    x16, 1f000 <main+0x1edb0>
 228:   f947fe11        ldr     x17, [x16, #4088]
 22c:   913fe210        add     x16, x16, #0xff8
 230:   d61f0220        br      x17
 234:   d503201f        nop
 238:   d503201f        nop
 23c:   d503201f        nop

0000000000000240 <rpm_puts@plt>:
 240:   90000110        adrp    x16, 20000 <rpm_puts>
 244:   f9400211        ldr     x17, [x16]
 248:   91000210        add     x16, x16, #0x0
 24c:   d61f0220        br      x17
```

# arm32
Example of .plt section on arm32 architecture:
```
Disassembly of section .plt:

00000160 <.plt>:
 160:   e52de004        push    {lr}             @ (str lr, [sp, #-4]!)
 164:   e59fe004        ldr     lr, [pc, #4]     @ 170 <.plt+0x10>
 168:   e08fe00e        add     lr, pc, lr
 16c:   e5bef008        ldr     pc, [lr, #8]!
 170:   00001e90        .word   0x00001e90

00000174 <rpm_puts@plt>:
 174:   e28fc600        add     ip, pc, #0, 12
 178:   e28cca01        add     ip, ip, #4096    @ 0x1000
 17c:   e5bcfe90        ldr     pc, [ip, #3728]! @ 0xe90
```
