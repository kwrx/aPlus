#include <aplus.h>
#include <aplus/module.h>
#include <aplus/vfs.h>
#include <aplus/mm.h>
#include <libc.h>


MODULE_NAME("fs/ext2");
MODULE_DEPS("");
MODULE_AUTHOR("Antonino Natale");
MODULE_LICENSE("GPL");




int init(void) {
	if(vfs_fsys_register("ext2", ext2_mount) != E_OK)
		return E_ERR;

	return E_OK;
}



int dnit(void) {
	return E_OK;
}