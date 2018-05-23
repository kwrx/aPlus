#include <aplus.h>
#include <aplus/module.h>
#include <aplus/vfs.h>
#include <libc.h>

MODULE_NAME("char/full");
MODULE_DEPS("");
MODULE_AUTHOR("Antonino Natale");
MODULE_LICENSE("GPL");


static int full_write(struct inode* inode, void* buf, off_t pos, size_t size) {
    errno = ENOSPC;
    return 0;
}

int init(void) {
    inode_t* ino;
    if(unlikely((ino = vfs_mkdev("full", -1, S_IFCHR | 0222)) == NULL))
        return -1;


    ino->write = full_write;
    return 0;
}



int dnit(void) {
    sys_unlink("/dev/full");
    return 0;
}