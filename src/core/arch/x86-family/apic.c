/*                                                                      
 * GPL3 License                                                         
 *                                                                      
 * Author(s):                                                              
 *      Antonino Natale <antonio.natale97@hotmail.com>                  
 *                                                                      
 *                                                                      
 * Copyright (c) 2013-2019 Antonino Natale                              
 *                                                                      
 * This file is part of aPlus.                                          
 *                                                                      
 * aPlus is free software: you can redistribute it and/or modify        
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or    
 * (at your option) any later version.                                  
 *                                                                      
 * aPlus is distributed in the hope that it will be useful,             
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        
 * GNU General Public License for more details.                         
 *                                                                      
 * You should have received a copy of the GNU General Public License    
 * along with aPlus.  If not, see <http://www.gnu.org/licenses/>.       
 */                                                                     
                                                                      
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <aplus.h>
#include <aplus/multiboot.h>
#include <aplus/debug.h>
#include <aplus/memory.h>
#include <aplus/ipc.h>

#include <hal/cpu.h>
#include <hal/interrupt.h>
#include <hal/vmm.h>

#include <arch/x86/cpu.h>
#include <arch/x86/asm.h>
#include <arch/x86/intr.h>
#include <arch/x86/acpi.h>
#include <arch/x86/apic.h>



extern ioapic_t ioapic[];
static int x2apic;


__percpu
void apic_enable(void) {

    uint64_t msr = x86_rdmsr (
        X86_APIC_BASE_MSR
    );

    x86_wrmsr (
        X86_APIC_BASE_MSR, 
        X86_APIC_BASE_ADDR | X86_APIC_MSR_EN | (x2apic ? X86_APIC_MSR_EXTD : 0) | (msr & 0x3FF)
    );


    if (x2apic) {

        x86_wrmsr(X86_X2APIC_REG_LVT_TIMER, (1 << 16));
        x86_wrmsr(X86_X2APIC_REG_LVT_THERMAL, (1 << 16));
        x86_wrmsr(X86_X2APIC_REG_LVT_PERFMON, (1 << 16));
        x86_wrmsr(X86_X2APIC_REG_LVT_LINT0, (1 << 16));
        x86_wrmsr(X86_X2APIC_REG_LVT_LINT1, (1 << 16));
        
        x86_wrmsr(X86_X2APIC_REG_TASK_PRIO, 0);
        x86_wrmsr(X86_X2APIC_REG_SPURIOUS, 0x1FF);

    } else {

        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_LVT_TIMER, (1 << 16));
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_LVT_THERMAL, (1 << 16));
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_LVT_PERFMON, (1 << 16));
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_LVT_LINT0, (1 << 16));
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_LVT_LINT1, (1 << 16));
        
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_DFR, 0xFFFFFFFF);
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_TASK_PRIO, 0);
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_SPURIOUS, 0x1FF);

    }

    
    uint32_t sd = 1193180 / CLOCKS_PER_SEC;
    outb(0x61, inb(0x61) & ~2);
    outb(0x43, 0x80 | 0x30);


    if (x2apic) {

        x86_wrmsr(X86_X2APIC_REG_TMR_DIV, 3);
        x86_wrmsr(X86_X2APIC_REG_TMR_ICNT, 0xFFFFFFFF);

    } else {
        
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_TMR_DIV, 3);
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_TMR_ICNT, 0xFFFFFFFF);
    
    }


    outb(0x42, sd & 0xFF);
    outb(0x42, (sd >> 8) & 0xFF);
    
    uint8_t cb = inb(0x61);
    outb(0x61, cb & ~1);
    outb(0x61, cb | 1);

    while(!(inb(0x61) & 0x20))
        ;
    

    uint32_t ticks = 0xFFFFFFFF;
    
    if(x2apic)
        ticks -= x86_rdmsr(X86_X2APIC_REG_TMR_CCNT);
    else
        ticks -= mmio_r32(X86_APIC_BASE_ADDR + X86_APIC_REG_TMR_CCNT);

    


    if (x2apic) {
        
        x86_wrmsr(X86_X2APIC_REG_LVT_TIMER, (1 << 17) | 32);
        x86_wrmsr(X86_X2APIC_REG_TMR_DIV, 3);
        x86_wrmsr(X86_X2APIC_REG_TMR_ICNT, ticks);

    } else {

        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_LVT_TIMER, (1 << 17) | 32);
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_TMR_DIV, 3);
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_TMR_ICNT, ticks);

    }

    kprintf("x86-apic: Local APIC #%d initialized [base(%p), x2apic(%d)]\n", apic_get_id(), X86_APIC_BASE_ADDR, x2apic);

}


void apic_eoi(void) {

    if (x2apic)
        x86_wrmsr(X86_X2APIC_REG_EOI, 0);
    else
        mmio_w32(X86_APIC_BASE_ADDR + X86_APIC_REG_EOI, 0);

}

uint32_t apic_get_id(void) {

    if (x2apic)
        return (x86_rdmsr(X86_X2APIC_REG_ID) & 0xFFFFFFFF);
    else
        return (mmio_r32(X86_APIC_BASE_ADDR + X86_APIC_REG_ID) >> 24);

}

int apic_is_x2apic(void) {
    return x2apic;
}


void apic_init(void) {

    memset(ioapic, 0, sizeof(ioapic_t) * X86_IOAPIC_MAX);

    
    //* Disable PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);



    if(!(core->bsp.features & X86_CPU_FEATURES_APIC))
        kpanicf("x86-apic: APIC not supported!\n");


    acpi_sdt_t* sdt;
    if(acpi_find(&sdt, "APIC") != 0)
        kpanicf("x86-apic: APIC not found in ACPI tables\n");


    
    acpi_madt_t* madt;

    if(acpi_is_extended())
        madt = (acpi_madt_t*) &sdt->xtables;
    else
        madt = (acpi_madt_t*) &sdt->tables;

    DEBUG_ASSERT(madt);
    DEBUG_ASSERT(madt->lapic_address == X86_APIC_BASE_ADDR);



    uint8_t* p = madt->entries;

    for(int i = 0x2C; i < sdt->length; ) {

        switch(*p) {

            case X86_MADT_ENTRY_LAPIC:

                core->cpu.max_cores++;
                core->cpu.cores[p[2]].id = p[3];
                core->cpu.cores[p[2]].flags = 0ULL;
                
                if(*(uint32_t*) &p[4] & (1 << 0))
                    core->cpu.cores[p[2]].flags |= SMP_CPU_FLAGS_AVAILABLE;
                    
                break;

            case X86_MADT_ENTRY_IOAPIC:
                
                ioapic[p[2]].address = *(uint32_t*) &p[4];
                ioapic[p[2]].gsi_base = *(uint32_t*) &p[8];
                break;

            case X86_MADT_ENTRY_INTERRUPT:
                break;

            case X86_MADT_ENTRY_NMI:
                break;

            case X86_MADT_ENTRY_LAPIC64:

                kpanicf("x86-apic: X86_MADT_ENTRY_LAPIC64 not yet supported in x86-64\n");
                break;

            default:
                BUG_ON(0);
                break;

        }


#if defined(DEBUG)

        switch(*p) {

            case X86_MADT_ENTRY_LAPIC:
                    
                kprintf("x86-apic: X86_MADT_ENTRY_LAPIC: cpu (%d) id(%d) flags(%p)\n",
                    p[2],
                    p[3],
                    *(uint32_t*) &p[4]
                );
                break;

            case X86_MADT_ENTRY_IOAPIC:

                kprintf("x86-apic: X86_MADT_ENTRY_IOAPIC: id(%d) address(%p) gsi(%d)\n",
                    p[2],
                    *(uint32_t*) &p[4],
                    *(uint32_t*) &p[8]
                );
                break;

            case X86_MADT_ENTRY_INTERRUPT:

                kprintf("x86-apic: X86_MADT_ENTRY_INTERRUPT: bus(%d) irq(%d) gsi(%d) flags(%p)\n",
                    p[2],
                    p[3],
                    *(uint32_t*) &p[4],
                    *(uint16_t*) &p[8]
                );
                break;

            case X86_MADT_ENTRY_NMI:

                kprintf("x86-apic: X86_MADT_ENTRY_NMI: id(%d) flags(%p) lint(%d)\n",
                    p[2],
                    *(uint16_t*) &p[3],
                    p[5]
                );
                break;

            case X86_MADT_ENTRY_LAPIC64:
            
                kprintf("x86-apic: X86_MADT_ENTRY_LAPIC64: address(%p)\n",
                    *(uint64_t*) &p[4]
                );
                break;
            
            default:
                break;

        }

#endif

        i += p[1];
        p += p[1];

    }



    x2apic = 0;

#if !defined(CONFIG_X86_X2APIC_FORCE_DISABLED)
    if(core->bsp.features & X86_CPU_FEATURES_X2APIC)
        x2apic = 1;
#endif



    if(X86_APIC_BASE_ADDR < ((core->memory.phys_upper + core->memory.phys_lower) * 1024))
        pmm_claim_area (
            X86_APIC_BASE_ADDR,
            X86_APIC_BASE_ADDR + PML1_PAGESIZE
        );


    if(!x2apic) {
        
        arch_vmm_map (
            &core->bsp.address_space,
            X86_APIC_BASE_ADDR,
            X86_APIC_BASE_ADDR,
            PML1_PAGESIZE,
            
            ARCH_VMM_MAP_RDWR       |
            ARCH_VMM_MAP_UNCACHED   |
            ARCH_VMM_MAP_NOEXEC     |
            ARCH_VMM_MAP_FIXED
        );

    }


    ioapic_enable();
    apic_enable();


    __asm__ __volatile__("sti");

}