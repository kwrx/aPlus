#ifndef _APLUS_VIRTIO_H
#define _APLUS_VIRTIO_H

#include <aplus.h>
#include <aplus/ipc.h>
#include <libc.h>

#define VIRTIO_IO_DEVICE_FEATURES                   0x00
#define VIRTIO_IO_GUEST_FEATURES                    0x04
#define VIRTIO_IO_QUEUE_ADDRESS                     0x08
#define VIRTIO_IO_QUEUE_SIZE                        0x0C
#define VIRTIO_IO_QUEUE_SELECT                      0x0E
#define VIRTIO_IO_QUEUE_NOTIFY                      0x10
#define VIRTIO_IO_DEVICE_STATUS                     0x12
#define VIRTIO_IO_ISR_STATUS                        0x13

#define VIRTIO_STATUS_ACK                           0x01
#define VIRTIO_STATUS_LOADED                        0x02
#define VIRTIO_STATUS_READY                         0x04
#define VIRTIO_STATUS_FEATURES_OK                   0x08
#define VIRTIO_STATUS_ERROR                         0x40
#define VIRTIO_STATUS_FAILED                        0x80

#define VIRTIO_DESC_FLAG_NEXT                       0x01 
#define VIRTIO_DESC_FLAG_WRITE_ONLY                 0x02 
#define VIRTIO_DESC_FLAG_INDIRECT                   0x04


#define VIRTIO_TYPE_NETWORK                         0x01
#define VIRTIO_TYPE_BLOCK                           0x02
#define VIRTIO_TYPE_CONSOLE                         0x03
#define VIRTIO_TYPE_ENTROPY                         0x04
#define VIRTIO_TYPE_MEMORY                          0x05
#define VIRTIO_TYPE_MMIO                            0x06
#define VIRTIO_TYPE_RPMSG                           0x07
#define VIRTIO_TYPE_SCSI                            0x08
#define VIRTIO_TYPE_9P                              0x09
#define VIRTIO_TYPE_WLAN                            0x0A




typedef struct {
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t next;
} vqueue_buffer_t;

typedef struct {
    uint16_t flags;
    uint16_t index;
    uint16_t rings[];
} vqueue_available_t;

typedef struct {
    uint32_t index;
    uint32_t length;
} vqueue_used_item_t;

typedef struct {
    uint16_t flags;
    uint16_t index;
    vqueue_used_item_t rings[];
} vqueue_used_t;

typedef struct {
    uint16_t size;
    union {
        vqueue_buffer_t* buffers;
        uintptr_t address;
        uint64_t value;
    };

    vqueue_available_t* available;
    vqueue_used_t* used;
    uint16_t used_index;
    uint16_t last_available_index;
    uint8_t* buffer;
    uint32_t chunk_size;
    uint16_t next_buffer;
    uint64_t lock;
} vqueue_t;


typedef struct {
    uint8_t* buffer;
    uint64_t size;
    uint8_t flags;
    uint8_t copy;
} vqueue_buffer_info_t;

typedef struct {
    int pci;
    int irq;
    int type;
    uintptr_t io;

    vqueue_t vqueue[16];
    void* userdata;
} virtio_device_t;


#define DISABLE_FEATURE(v,feature)  \
    v &= ~(1 << feature)

#define ENABLE_FEATURE(v,feature)   \
    v |= (1 << feature)

#define HAS_FEATURE(v,feature)      \
    (v & (1 << feature))




#ifdef __APLUS_MODULE__
#include <aplus/base.h>
#include <aplus/utils/list.h>

list(virtio_device_t*, virtio_devices);

void vqueue_enable_interrupts(vqueue_t* v);
void vqueue_disable_interrupts(vqueue_t* v);

int virtio_init_device(virtio_device_t* dev, void (*negotiate) (uint32_t* features));
int virtio_send_buffer(virtio_device_t* dev, uint16_t index, vqueue_buffer_info_t* buf, uint64_t size);


uint8_t virtio_r8(uint16_t);
uint16_t virtio_r16(uint16_t);
uint32_t virtio_r32(uint16_t);
void virtio_w8(uint16_t, uint8_t);
void virtio_w16(uint16_t, uint16_t);
void virtio_w32(uint16_t, uint32_t);
#endif
#endif