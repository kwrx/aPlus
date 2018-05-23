#include <aplus.h>
#include <aplus/module.h>
#include <aplus/vfs.h>
#include <libc.h>

MODULE_NAME("char/urandom");
MODULE_DEPS("");
MODULE_AUTHOR("Antonino Natale");
MODULE_LICENSE("GPL");


static int urandom_read(struct inode* inode, void* buf, off_t pos, size_t size) {
    char* bc = (char*) buf;
    size_t i;
    for(i = 0; i < size; i++)
        *bc++ = rand() % 256;

    return size;
}

int init(void) {
    inode_t* ino;
    if(unlikely((ino = vfs_mkdev("urandom", -1, S_IFCHR | 0444)) == NULL))
        return -1;


    ino->read = urandom_read;
    return 0;
}



int dnit(void) {
    sys_unlink("/dev/urandom");
    return 0;
}