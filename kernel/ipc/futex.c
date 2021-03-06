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
#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/memory.h>
#include <aplus/smp.h>
#include <aplus/task.h>
#include <aplus/hal.h>



static spinlock_t rt_lock = SPINLOCK_INIT;


void futex_rt_lock() {
    spinlock_lock(&rt_lock);
}

void futex_rt_unlock() {
    spinlock_unlock(&rt_lock);
}


void futex_wait(task_t* task, uint32_t* kaddr, uint32_t value, const struct timespec* utime) {

    DEBUG_ASSERT(task);
    DEBUG_ASSERT(kaddr);


#if defined(DEBUG) && DEBUG_LEVEL >= 3
    kprintf("futex: futex_wait() pid(%d) kaddr(%p) value(%p)\n", task->tid, kaddr, value);
#endif


    futex_t* futex = (futex_t*) kcalloc(1, sizeof(futex_t), GFP_KERNEL);

    futex->address = kaddr;
    futex->value = value;
    
    if(utime) {

        memcpy(&futex->timeout, utime, sizeof(struct timespec));

        futex->timeout.tv_sec  += arch_timer_generic_getms() / 1000ULL;
        futex->timeout.tv_nsec += arch_timer_generic_getns() % 1000000000ULL; 

    }


    __lock(&task->lock,
        list_push(task->futexes, futex));

    thread_suspend(task);


    arch_task_context_set(task, ARCH_TASK_CONTEXT_RETVAL, 0L);

}


size_t futex_wakeup(uint32_t* kaddr, size_t max) {

    DEBUG_ASSERT(kaddr);

#if defined(DEBUG) && DEBUG_LEVEL >= 3
    kprintf("futex: futex_wakeup() pid(%d) kaddr(%p) max(%p)\n", current_task->tid, kaddr, max);
#endif


    size_t wok = 0;

    task_t* tmp;
    for(tmp = current_cpu->sched_queue; tmp && max; tmp = tmp->next) {

        __lock(&tmp->lock, {

            list_each(tmp->futexes, i) {

                if(likely(i->address != kaddr))
                    continue;


#if defined(DEBUG) && DEBUG_LEVEL >= 4
                kprintf("futex: woke up pid(%d) kaddr(%p)\n", tmp->tid, kaddr);
#endif

                list_remove(tmp->futexes, i);
                futex_wakeup_thread(tmp, i);

                max--; wok++;
                break;

            }

        });

    }

    return wok;

}


size_t futex_requeue(uint32_t* kaddr, uint32_t* kaddr2, size_t max) {

    DEBUG_ASSERT(kaddr);
    DEBUG_ASSERT(kaddr2);

#if defined(DEBUG) && DEBUG_LEVEL >= 3
    kprintf("futex: futex_requeue() pid(%d) kaddr(%p) kaddr2(%p) max(%p)\n", current_task->tid, kaddr, kaddr2, max);
#endif


    size_t req = 0;

    task_t* tmp;
    for(tmp = current_cpu->sched_queue; tmp && max; tmp = tmp->next) {

        __lock(&tmp->lock, {

            list_each(tmp->futexes, i) {

                if(likely(i->address != kaddr))
                    continue;


#if defined(DEBUG) && DEBUG_LEVEL >= 4
                kprintf("futex: requeue pid(%d) from kaddr(%p) to kaddr2(%p)\n", tmp->tid, kaddr, kaddr2);
#endif

                i->address = kaddr2;

                max--; req++;
                break;

            }

        });

    }

    return req;

}



int futex_expired(futex_t* futex) {

    DEBUG_ASSERT(futex);
    DEBUG_ASSERT(futex->address);

    if(*futex->address == futex->value)
        return 0;

    if(futex->timeout.tv_sec + futex->timeout.tv_nsec == 0)
        return 0;

    return (arch_timer_generic_getns() > (futex->timeout.tv_sec * 1000000000ULL + futex->timeout.tv_nsec));

}


void futex_wakeup_thread(task_t* task, futex_t* futex) {

    futex->address =  0;
    futex->value   = ~0;

    thread_wake(task);

}