#ifndef _APLUS_X86_APIC_H
#define _APLUS_X86_APIC_H

#ifndef __ASSEMBLY__
#include <sys/cdefs.h>
#include <aplus.h>
#include <aplus/debug.h>


#define X86_APIC_BASE_MSR                       0x01B
#define X86_APIC_BASE_ADDR                      0xFEE00000

#define X86_APIC_MSR_EN                         (1 << 11)
#define X86_APIC_MSR_EXTD                       (1 << 10)
#define X86_APIC_MSR_BSP                        (1 << 8)



#define X86_APIC_REG_ID                         0x0020
#define X86_APIC_REG_VERSION                    0x0030
#define X86_APIC_REG_TASK_PRIO                  0x0080
#define X86_APIC_REG_PROC_PRIO                  0x00A0
#define X86_APIC_REG_EOI                        0x00B0
#define X86_APIC_REG_LDR                        0x00D0
#define X86_APIC_REG_DFR                        0x00E0
#define X86_APIC_REG_SPURIOUS                   0x00F0

#define X86_APIC_REG_ICR_LO                     0x0300
#define X86_APIC_REG_ICR_HI                     0x0310

#define X86_APIC_REG_LVT_TIMER                  0x0320
#define X86_APIC_REG_LVT_THERMAL                0x0330
#define X86_APIC_REG_LVT_PERFMON                0x0340
#define X86_APIC_REG_LVT_LINT0                  0x0350
#define X86_APIC_REG_LVT_LINT1                  0x0360
#define X86_APIC_REG_LVT_ERR                    0x0370

#define X86_APIC_REG_TMR_ICNT                   0x0380
#define X86_APIC_REG_TMR_CCNT                   0x0390
#define X86_APIC_REG_TMR_DIV                    0x03E0


#define X86_X2APIC_REG_ID                       0x0802
#define X86_X2APIC_REG_VERSION                  0x0803
#define X86_X2APIC_REG_TASK_PRIO                0x0808
#define X86_X2APIC_REG_PROC_PRIO                0x080A
#define X86_X2APIC_REG_EOI                      0x080B
#define X86_X2APIC_REG_LDR                      0x080D
#define X86_X2APIC_REG_SPURIOUS                 0x080F

#define X86_X2APIC_REG_ICR                      0x0830

#define X86_X2APIC_REG_LVT_TIMER                0x0832
#define X86_X2APIC_REG_LVT_THERMAL              0x0833
#define X86_X2APIC_REG_LVT_PERFMON              0x0834
#define X86_X2APIC_REG_LVT_LINT0                0x0835
#define X86_X2APIC_REG_LVT_LINT1                0x0836
#define X86_X2APIC_REG_LVT_ERR                  0x0837

#define X86_X2APIC_REG_TMR_ICNT                 0x0838
#define X86_X2APIC_REG_TMR_CCNT                 0x0839
#define X86_X2APIC_REG_TMR_DIV                  0x083E
#define X86_X2APIC_REG_SELF_IPI                 0x083F

#define X86_X2APIC_LOGICAL_ID(id)               \
    (((id & 0xFFFF0) << 16) | (1 << (id & 0x0000F)))



#define X86_IOAPIC_IOAPICID                     0x00
#define X86_IOAPIC_IOAPICVER                    0x01
#define X86_IOAPIC_IOAPICARB                    0x02
#define X86_IOAPIC_IOAPICREDTBL(n)              (0x10 + 2 * n)
#define X86_IOAPIC_MAX                          0x10


typedef struct {
    uintptr_t address;
    uint32_t gsi_base;
    uint32_t gsi_max;
    spinlock_t lock;
} __packed ioapic_t;


__BEGIN_DECLS

void apic_init(void);
void apic_enable(void);
void apic_eoi(void);
uint32_t apic_get_id(void);
int apic_is_x2apic(void);


void ioapic_enable(void);
void ioapic_map_irq(uint8_t, uint8_t, uint8_t);
void ioapic_unmap_irq(uint8_t);

__END_DECLS

#endif
#endif