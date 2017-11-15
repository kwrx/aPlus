#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/vfs.h>
#include <aplus/mm.h>
#include <aplus/task.h>
#include <aplus/timer.h>
#include <libc.h>

#include "ext2.h"



int ext2_read(inode_t* ino, void* buf, off_t pos, size_t size) {
    if(unlikely(!ino))
        return 0;
        
    if(unlikely(!buf))
        return 0;
        
    if(unlikely((off64_t) size > ino->size))
        size = ino->size;
        
    if(unlikely(pos > ino->size))
        pos = ino->size;
        
    if(unlikely(pos + size > ino->size))
        size = (off_t) (ino->size - pos);
        
    if(unlikely(!size))
        return 0;

    ext2_priv_t* priv = (ext2_priv_t*) ino->userdata;
    if(!priv->blockchain) {
        errno = EIO;
        return 0;
    }
    
    uint32_t sb = pos / priv->ext2->blocksize;
    uint32_t eb = (pos + size - 1) / priv->ext2->blocksize;
    off64_t off = 0;

    if(pos % priv->ext2->blocksize) {
        long p;
        p = priv->ext2->blocksize - (pos % priv->ext2->blocksize);
        p = p > size ? size : p;

        ext2_read_block(priv->ext2, priv->ext2->cache, priv->blockchain[sb]);
        memcpy(buf, (void*) ((uintptr_t) priv->ext2->cache + ((uintptr_t) pos % priv->ext2->blocksize)), p);
    
        off += p;
        sb++;
    }


    if(((pos + size) % priv->ext2->blocksize) && (sb <= eb)) {
        long p = (pos + size) % priv->ext2->blocksize;

        ext2_read_block(priv->ext2, priv->ext2->cache, priv->blockchain[eb]);
        memcpy((void*) ((uintptr_t) buf + size - p), priv->ext2->cache, p);
        eb--;
    }

    long i = eb - sb + 1;
    if(likely(i > 0)) {
        for(; i--; off += priv->ext2->blocksize)
            ext2_read_block(priv->ext2, (void*) ((uintptr_t) buf + (uintptr_t) off), priv->blockchain[sb++]);
    }


    pos += size;
    return size;
}