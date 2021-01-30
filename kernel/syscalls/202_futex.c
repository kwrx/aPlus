/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2019 Antonino Natale
 * 
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
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/task.h>
#include <aplus/smp.h>
#include <aplus/hal.h>
#include <aplus/ipc.h>
#include <aplus/errno.h>




/***
 * Name:        futex
 * Description: fast user-space locking
 * URL:         http://man7.org/linux/man-pages/man2/futex.2.html
 *
 * Input Parameters:
 *  0: 0xca
 *  1: undefined
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */


SYSCALL(202, futex,
long sys_futex (uint32_t __user * uaddr, int op, uint32_t val, long __val2, uint32_t __user * uaddr2, uint32_t val3) {
    
    if(unlikely(!uaddr))
        return -EINVAL;

    if(unlikely(!uio_check(uaddr, R_OK)))
        return -EACCES;


    uint32_t* kaddr = uio_get_ptr(uaddr);


    switch(op & FUTEX_CMD_MASK) {

        case FUTEX_WAIT:

            {

                const struct timespec* utime = (const struct timespec*) __val2;

                if(unlikely(utime && !uio_check(utime, R_OK)))
                    return -EFAULT;

                if(__atomic_load_n(kaddr, __ATOMIC_RELAXED) != val)
                    return -EAGAIN;


                futex_wait(current_task, kaddr, val, utime);
                thread_postpone_resched(current_task);
                
                break;

            }
           


        case FUTEX_WAKE:
            return (long) futex_wakeup(kaddr, val);


        case FUTEX_FD:
            return -ENOSYS;


        case FUTEX_CMP_REQUEUE:

            if(__atomic_load_n(kaddr, __ATOMIC_RELAXED) != val3)
                return -EAGAIN;


        case FUTEX_REQUEUE:

            {

                futex_wakeup(kaddr, val);


                if(unlikely(!uaddr2))
                    return -EINVAL;

                if(unlikely(!uio_check(uaddr2, R_OK)))
                    return -EACCES;


                uint32_t* kaddr2 =  uio_get_ptr(uaddr2);

                return futex_requeue(kaddr, kaddr2, (uint32_t) __val2);

            }


        case FUTEX_WAKE_OP:
            return -ENOSYS;

        
        case FUTEX_LOCK_PI:
            
            {
                
                long e = 0L;
                futex_rt_lock();


                if(*kaddr == 0) 
                    *kaddr = (uint32_t) current_task->tid;
                
                else {

                    if(*kaddr == current_task->tid)
                        e = -EDEADLK;
                    
                    else {
                        
                        *kaddr |= FUTEX_WAITERS;


                        futex_wait(current_task, kaddr, *kaddr, NULL);
                        thread_postpone_resched(current_task);

                    }
                }


                futex_rt_unlock();
                return e;
            }

         
        case FUTEX_TRYLOCK_PI:

            {
                
                long e = 0L;
                futex_rt_lock();


                if(*kaddr == 0) 
                    *kaddr = (uint32_t) current_task->tid;
                
                else {

                    if(*kaddr == current_task->tid)
                        e = -EDEADLK;
                    
                    e = -1; // FIXME: Return valid error

                }


                futex_rt_unlock();
                return e;
            }


        case FUTEX_UNLOCK_PI:

            {
                
                futex_rt_lock();

                if(*kaddr != 0) 
                    *kaddr = (uint32_t) 0;

                futex_wakeup(kaddr, 1);
                futex_rt_unlock();
                break;

            }
            


        case FUTEX_WAIT_BITSET:
            return -ENOSYS;

        default:
            return -ENOSYS;

    }


    return 0L;

});