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

#define _GNU_SOURCE
#include <sched.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/mm.h>
#include <aplus/smp.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        clone
 * Description: create a child process
 * URL:         http://man7.org/linux/man-pages/man2/clone.2.html
 *
 * Input Parameters:
 *  0: 0x38
 *  1: long (*fn) (void*)
 *  2: void __user * stack
 *  3: int flags
 *  4: void __user * arg
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */


SYSCALL(56, clone,
long sys_clone (int (*fn)(void*), void __user * stack, unsigned long flags, void* arg) {

    if(unlikely(!fn))
        return -EINVAL;

    if(unlikely(!ptr_check(fn, R_OK | W_OK | X_OK)))
        return -EFAULT;

    if(stack)
        if(unlikely(!ptr_check(stack, R_OK | W_OK)))
            return -EFAULT;



    task_t* child = NULL;
    

    __lock(&current_task->lock, {
    
        
        child = (task_t*) kcalloc(sizeof(task_t), 1, GFP_KERNEL);

        child->tid = sched_nextpid();
        child->uid = current_task->uid;
        child->gid = current_task->gid;
        child->sid = current_task->sid;
        child->euid = current_task->euid;
        child->egid = current_task->egid;

        child->pgid = current_task->pgid;
        child->tgid = current_task->tid;

        child->argv = current_task->argv;
        child->environ = current_task->environ;

        child->status = TASK_STATUS_READY;
        child->priority = current_task->priority;
        child->affinity = current_task->affinity;

        child->umask = 022;
        child->parent = current_task;

        spinlock_init(&child->lock);


        memcpy(&child->context.fpu, &current_task->context.fpu, sizeof(current_task->context.fpu));
        memcpy(&child->rlimits, &current_task->rlimits, sizeof(struct rlimit) * RLIM_NLIMITS);
        memcpy(&child->exit, &current_task->exit, sizeof(current_task->exit));


        if(likely(stack))
            child->context.frame = arch_task_init_frame(stack, fn, arg);
        else
            child->context.frame = current_task->context.frame;


        #define __is(bit) \
            ((flags) & (bit))


        if(__is(CLONE_IO))
            kpanic("sys_clone: CLONE_IO not yet supported");

        if(__is(CLONE_NEWCGROUP))
            kpanic("sys_clone: CLONE_NEWCGROUP not yet supported");

        if(__is(CLONE_NEWIPC))
            kpanic("sys_clone: CLONE_NEWIPC not yet supported");

        if(__is(CLONE_NEWNET))
            kpanic("sys_clone: CLONE_NEWNET not yet supported");

        if(__is(CLONE_NEWNS))
            kpanic("sys_clone: CLONE_NEWNS not yet supported");

        if(__is(CLONE_NEWPID))
            kpanic("sys_clone: CLONE_NEWPID not yet supported");

        if(__is(CLONE_NEWUSER))
            kpanic("sys_clone: CLONE_NEWUSER not yet supported");

        if(__is(CLONE_NEWUTS))
            kpanic("sys_clone: CLONE_NEWUTS not yet supported");

        if(__is(CLONE_PTRACE))
            kpanic("sys_clone: CLONE_PTRACE not yet supported");

        if(__is(CLONE_SYSVSEM))
            kpanic("sys_clone: CLONE_SYSVSEM not yet supported");

        if(__is(CLONE_UNTRACED))
            kpanic("sys_clone: CLONE_UNTRACED not yet supported");
       
        if(__is(CLONE_PARENT_SETTID))
            kpanic("sys_clone: CLONE_PARENT_SETTID not yet supported");
        
        if(__is(CLONE_CHILD_CLEARTID) || __is(CLONE_CHILD_SETTID))
            kpanic("sys_clone: CLONE_CHILD_SETTID/CLEARTID not yet supported");

        if(__is(CLONE_SETTLS)) /* TODO */
            kpanic("sys_clone: CLONE_SETTLS not yet supported");

        if(__is(CLONE_SIGHAND)) /* TODO */
            kpanic("sys_clone: CLONE_NEWUTS not yet supported");


        if(__is(CLONE_THREAD))
            child->tgid = current_task->tgid;
        
        if(__is(CLONE_PARENT))
            child->parent = current_task->parent;

        if(__is(CLONE_FILES))
            memcpy(&child->fd, &current_task->fd, sizeof(fd_t) * TASK_NFD);
            
        if(__is(CLONE_FS)) {
            child->root = current_task->root;
            child->cwd = current_task->cwd;
            child->exe = current_task->exe;
            child->umask = current_task->umask;
        }

        if(__is(CLONE_VM))
            ; // TODO: aspace_fork(&child->aspace, current_task->aspace);
        else
            aspace_clone(&child->aspace, current_task->aspace);



        DEBUG_ASSERT(child->aspace);
        DEBUG_ASSERT(child->context.frame);
        
        sched_enqueue(child);

    });


    if(__is(CLONE_VFORK))
        ; /* TODO */


    return child->tid;
});
