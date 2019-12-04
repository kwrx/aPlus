#ifndef _APLUS_X86_64_ASM_H
#define _APLUS_X86_64_ASM_H


#define KERNEL_CS               0x08
#define KERNEL_DS               0x10


#define KERNEL_HIGH_AREA        0xFFFFFFFF80000000
#define KERNEL_STACK_AREA       0xFFFFFFFFC0000000
#define KERNEL_STACK_SIZE       0x0000000000200000
#define KERNEL_HEAP_AREA        0xFFFF800000000000


#define V2P(virt)      \
    ((virt) - KERNEL_HIGH_AREA)

#endif