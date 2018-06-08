/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2018 Antonino Natale
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


#include <aplus.h>
#include <aplus/syscall.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <aplus/mm.h>
#include <aplus/debug.h>
#include <libc.h>

SYSCALL(155, sched_getparam,
int sys_sched_getparam(pid_t pid, struct sched_param* param) {
    if(unlikely(!param)) {
        errno = EINVAL;
        return -1;
    }
    
    if(pid == 0) {
        param->sched_priority = current_task->priority;
        return 0;
    }

    task_t* tmp;
    for(tmp = task_queue; tmp; tmp = tmp->next) {
        if(tmp->pid != pid)
            continue;

        param->sched_priority = tmp->priority;
        return 0;
    }

    errno = ESRCH;
    return -1;
});