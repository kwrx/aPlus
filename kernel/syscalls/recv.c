#include <aplus.h>
#include <aplus/syscall.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <aplus/mm.h>
#include <aplus/debug.h>
#include <lwip/sockets.h>
#include <libc.h>

SYSCALL(80, recv,
int sys_recv(int fd, void* buffer, size_t len, int flags) {
#if CONFIG_NETWORK
    return lwip_recv(fd - TASK_FD_COUNT, buffer, len, flags);
#endif

    errno = ENOSYS;
    return -1;
});