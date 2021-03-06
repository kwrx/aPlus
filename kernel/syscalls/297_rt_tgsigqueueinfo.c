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
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/task.h>
#include <aplus/smp.h>
#include <aplus/hal.h>
#include <aplus/errno.h>



/***
 * Name:        rt_tgsigqueueinfo
 * Description: queue a signal and data
 * URL:         http://man7.org/linux/man-pages/man2/rt_tgsigqueueinfo.2.html
 *
 * Input Parameters:
 *  0: 0x129
 *  1: pid_t tgid
 *  2: pid_t pid
 *  3: int sig
 *  4: siginfo_t __user * uinfo
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */


SYSCALL(297, rt_tgsigqueueinfo,
long sys_rt_tgsigqueueinfo (pid_t tgid, pid_t tid, int sig, siginfo_t __user * uinfo) {
    
    if(unlikely(tid == 1 && current_task->tid != 1))
        return -EINVAL;

    if(unlikely(sig < 0))
        return -EINVAL;

    if(unlikely(sig > _NSIG - 1))
        return -EINVAL;

    if(unlikely(!uinfo))
        return -EINVAL;

    if(unlikely(!uio_check(uinfo, R_OK)))
        return -EFAULT;

    if(unlikely(tgid != current_task->tgid && uinfo->si_code >= 0))
        return -EPERM;



    int found = 0;
    
    cpu_foreach(cpu) {

        task_t* tmp;
        for(tmp = cpu->sched_queue; tmp; tmp = tmp->next) {

            if(tmp->tgid != tgid)
                continue;

            if(tid != -1 && tmp->tid != tid)    // FIXME: not sure...
                continue;

            if(tmp->sigqueue.size > tmp->rlimits[RLIMIT_SIGPENDING].rlim_cur)
                return -EAGAIN;

            if(!(current_task->euid == tmp->uid || current_task->uid == tmp->uid))
                return -EPERM;      


            found = 1;

            if(unlikely(sig == 0))
                continue;

            if(unlikely(tmp->sighand->action[sig].handler == SIG_IGN))
                continue;

            if(unlikely(tmp->sighand->action[sig].handler == SIG_ERR))
                continue;

        

            siginfo_t* siginfo = (siginfo_t*) kcalloc(1, sizeof(siginfo_t), GFP_KERNEL);

            memcpy(siginfo, uinfo, sizeof(siginfo_t));
            siginfo->si_signo = sig;
           

            //? Check if the signal is blocked
            if(unlikely(tmp->sighand->sigmask.__bits[sig / (sizeof(long) << 3)] & (1 << (sig % (sizeof(long) << 3))))) {

                if(tmp->sighand->action[sig].sa_flags & SA_NODEFER)
                    queue_enqueue(&current_task->sigqueue, siginfo, 0);
                else
                    queue_enqueue(&current_task->sigpending, siginfo, 0);
                    
            } else
                queue_enqueue(&current_task->sigqueue, siginfo, 0);


        }

    }



    if(unlikely(!found))
        return -ESRCH;
    

    return 0;

});
