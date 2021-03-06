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

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/memory.h>
#include <aplus/ipc.h>
#include <aplus/hal.h>

#include <arch/x86/asm.h>
#include <arch/x86/cpu.h>
#include <arch/x86/vmm.h>
#include <arch/x86/intr.h>




void pagefault_handle(interrupt_frame_t* frame, uintptr_t cr2) {


#if defined(DEBUG) && DEBUG_LEVEL >= 2

    #define PFE(reason)     \
        { kprintf("x86-pfe: FAULT! address(%p): %s\n", cr2, reason); goto pfe; }

#else

    #define PFE(reason)     \
        { goto pfe; }

#endif



    uintptr_t pm = x86_get_cr3();
    
    if(unlikely(!pm))
        PFE("no memory mapping");


    uintptr_t pagesize = X86_MMU_PAGESIZE;
    uintptr_t s = cr2;



    {

        x86_page_t* d;


#if defined(__x86_64__)

        /* CR3-L4 */ 
        {
            d = &((x86_page_t*) arch_vmm_p2v(pm, ARCH_VMM_AREA_HEAP)) [(s >> 39) & 0x1FF];
        }

        /* PML4-L3 */
        {
            if(*d == X86_MMU_CLEAR)
                PFE("PML4-L3 doesn't not exist");

            d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 30) & 0x1FF];
        }


        /* HUGE_1GB */
        if(!(*d & X86_MMU_PG_PS)) {

            /* PDP-L2 */
            {
                if(*d == X86_MMU_CLEAR)
                    PFE("PDP-L2 doesn't not exist");

                d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 21) & 0x1FF];
            }

            /* HUGE_2MB */
            if(!(*d & X86_MMU_PG_PS)) {

                /* PD-L1 */
                {
                    if(*d == X86_MMU_CLEAR)
                        PFE("PD-L1 doesn't not exist");

                    d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 12) & 0x1FF];
                }

            } else
                pagesize = X86_MMU_HUGE_2MB_PAGESIZE;

        } else
            pagesize = X86_MMU_HUGE_1GB_PAGESIZE;


#elif defined(__i386__)

        /* CR3-L2 */
        {
            d = &((x86_page_t*) arch_vmm_p2v(space->pm, ARCH_VMM_AREA_HEAP)) [(s >> 22) & 0x3FF];
        }


        /* HUGE_4MB */
        if(!(*d & X86_MMU_PG_PS)) {

            /* PD-L1 */
            {
                if(*d == X86_MMU_CLEAR)
                    PFE("PD-L1 doesn't not exist");

                d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 12) & 0x3FF];
            }

        } else
            pagesize = X86_MMU_HUGE_2MB_PAGESIZE;

#endif

        /* Page Table */
        {

            if(*d == X86_MMU_CLEAR)
                PFE("page not present");

            // TODO: implement X86_MMU_PG_AP_TP_MMAP
            if(!(*d & X86_MMU_PG_AP_TP_COW))
                PFE("page fault cannot be handled");



            // * Handle Copy on Write

            uintptr_t page = __alloc_page(pagesize, 0);

            if((*d & X86_MMU_ADDRESS_MASK) != 0) {

                memcpy (
                    (void*) arch_vmm_p2v(page, ARCH_VMM_AREA_HEAP),
                    (void*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP),
                    (size_t) pagesize
                );

                page |= X86_MMU_PG_RW;

            }
                
            *d = page | X86_MMU_PG_P 
                      | X86_MMU_PG_AP_PFB 
                      | X86_MMU_PG_AP_TP_PAGE
                      | ((*d & ~X86_MMU_ADDRESS_MASK) & ~(X86_MMU_PG_AP_TP_MASK));

        }

    }


    current_task->rusage.ru_majflt++;


#if defined(DEBUG) && DEBUG_LEVEL >= 4
    kprintf("x86-pfe: handled page fault! cs(%p), ip(%p), sp(%p), cr2(%p) cr3(%p) cpu(%d) pid(%d)\n", frame->cs, frame->ip, frame->sp, cr2, x86_get_cr3(), current_cpu->id, current_task ? current_task->tid : 0);
#endif

    return;


pfe:
    kpanicf("x86-pfe: PANIC! errno(%p), cs(%p), ip(%p), sp(%p), cr2(%p) cr3(%p) cpu(%d) pid(%d)\n", frame->errno, frame->cs, frame->ip, frame->sp, cr2, x86_get_cr3(), current_cpu->id, current_task ? current_task->tid : 0);

}