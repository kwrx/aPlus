#ifndef _APLUS_IPC_H
#define _APLUS_IPC_H

#ifndef __ASSEMBLY__
#include <sys/cdefs.h>
#include <aplus.h>
#include <aplus/debug.h>
#include <time.h>


#if defined(DEBUG)
#define IPC_DEFAULT_TIMEOUT     10000   //? 10sec
#endif


#define FUTEX_WAIT		            0
#define FUTEX_WAKE		            1
#define FUTEX_FD		            2
#define FUTEX_REQUEUE		        3
#define FUTEX_CMP_REQUEUE	        4
#define FUTEX_WAKE_OP		        5
#define FUTEX_LOCK_PI		        6
#define FUTEX_UNLOCK_PI		        7
#define FUTEX_TRYLOCK_PI	        8
#define FUTEX_WAIT_BITSET	        9
#define FUTEX_WAKE_BITSET	        10
#define FUTEX_WAIT_REQUEUE_PI	    11
#define FUTEX_CMP_REQUEUE_PI	    12

#define FUTEX_PRIVATE_FLAG	        128
#define FUTEX_CLOCK_REALTIME	    256
#define FUTEX_CMD_MASK		        ~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)

#define FUTEX_WAITERS		        0x80000000
#define FUTEX_OWNER_DIED	        0x40000000
#define FUTEX_TID_MASK		        0x3FFFFFFF


#define FUTEX_WAIT_PRIVATE	            (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE	            (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)
#define FUTEX_REQUEUE_PRIVATE	        (FUTEX_REQUEUE | FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PRIVATE       (FUTEX_CMP_REQUEUE | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_OP_PRIVATE	        (FUTEX_WAKE_OP | FUTEX_PRIVATE_FLAG)
#define FUTEX_LOCK_PI_PRIVATE	        (FUTEX_LOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_UNLOCK_PI_PRIVATE	        (FUTEX_UNLOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_TRYLOCK_PI_PRIVATE        (FUTEX_TRYLOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_BITSET_PRIVATE	    (FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_BITSET_PRIVATE	    (FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_REQUEUE_PI_PRIVATE	(FUTEX_WAIT_REQUEUE_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PI_PRIVATE	(FUTEX_CMP_REQUEUE_PI | FUTEX_PRIVATE_FLAG)






#define SPINLOCK_INIT           { { 0L }, 0L }

typedef volatile struct {
    union {
        volatile char value;
        volatile long __padding;
    };
    volatile long flags;
} spinlock_t;


typedef struct {

    volatile uint32_t* address;
    uint32_t value;

    struct timespec timeout;

} futex_t;



#define SEMAPHORE_INIT          0L

typedef volatile long semaphore_t;



__BEGIN_DECLS

struct task;


void spinlock_init(spinlock_t*);
void spinlock_lock(spinlock_t*);
void spinlock_unlock(spinlock_t*);
int spinlock_trylock(spinlock_t*);

void sem_init(semaphore_t*, long);
void sem_wait(semaphore_t*);
void sem_post(semaphore_t*);
int sem_trywait(semaphore_t* s);

void futex_rt_lock(void);
void futex_rt_unlock(void);
void futex_wait(struct task*, uint32_t*, uint32_t, const struct timespec*);
size_t futex_wakeup(uint32_t*, size_t);
size_t futex_requeue(uint32_t*, uint32_t*, size_t);
int futex_expired(futex_t*);
void futex_wakeup_thread(struct task*, futex_t*);

__END_DECLS


#define __lock_break    \
    break


#define __lock(lk, fn...)                   \
    {                                       \
        spinlock_lock((lk));                \
        do { fn; } while(0);                \
        spinlock_unlock((lk));              \
    }

#define __lock_return(lk, type, fn...)      \
    {                                       \
        type __r;                           \
        spinlock_lock(lk);                  \
        __r = fn;                           \
        spinlock_unlock(lk);                \
        return __r;                         \
    }

#define __trylock(lk, fn...)                    \
    {                                           \
        if(unlikely(!spinlock_trylock((lk))))   \
            kpanicf("spinlock: DEADLOCK! %s in %s:%d <%s>", #lk, __FILE__, __LINE__, __func__); \
        do { fn; } while(0);                    \
        spinlock_unlock((lk));                  \
    }

#define __trylock_return(lk, type, fn...)       \
    {                                           \
        type __r;                               \
        if(unlikely(!spinlock_trylock((lk))))   \
            kpanicf("spinlock: DEADLOCK! %s in %s:%d <%s>", #lk, __FILE__, __LINE__, __func__); \
        __r = fn;                               \
        spinlock_unlock((lk));                  \
        return __r;                             \
    }


#define atomic_load(ptr)                    \
    __atomic_load_n(ptr, __ATOMIC_ACQUIRE)

#define atomic_store(ptr, val)              \
    __atomic_load_n(ptr, val, __ATOMIC_RELEASE)

#define atomic_exchange(ptr, val)           \
    __atomic_exchange(ptr, val, __ATOMIC_ACQ_REL)

#define atomic_thread_fence()               \
    __atomic_thread_fence(__ATOMIC_SEQ_CST)

#define atomic_signal_fence()               \
    __atomic_signal_fence(__ATOMIC_SEQ_CST)


#endif
#endif