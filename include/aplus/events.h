#ifndef _APLUS_EVENTS_H
#define _APLUS_EVENTS_H

#include <aplus/base.h>
#include <aplus/input.h>
#include <stdint.h>
#include <sys/ioctl.h>


#define EV_SYN                  0x00
#define EV_KEY                  0x01
#define EV_REL                  0x02
#define EV_ABS                  0x03
#define EV_MSC                  0x04
#define EV_SW                   0x05
#define EV_LED                  0x11
#define EV_SND                  0x12
#define EV_REP                  0x14
#define EV_FF                   0x15
#define EV_PWR                  0x16
#define EV_FF_STATUS            0x17
#define EV_MAX                  0x1f
#define EV_CNT                  (EV_MAX + 1)

#define EC_SYN                  (1 << 0x00)
#define EC_KEY                  (1 << 0x01)
#define EC_REL                  (1 << 0x02)
#define EC_ABS                  (1 << 0x03)
#define EC_MSC                  (1 << 0x04)
#define EC_SW                   (1 << 0x05)
#define EC_LED                  (1 << 0x11)
#define EC_SND                  (1 << 0x12)
#define EC_REP                  (1 << 0x14)
#define EC_FF                   (1 << 0x15)
#define EC_PWR                  (1 << 0x16)
#define EC_FF_STATUS            (1 << 0x17)

#define EVIOGID                 _IOR('e', 1, long[2])
#define EVIOGNAME               _IOR('e', 2, char[64 + sizeof(evid_t)])
#define EVIOGCAPS               _IOR('e', 3, evid_t)
#define EVIOGSTATUS             _IOR('e', 4, evid_t)
#define EVIOSSTATUS             _IOW('e', 5, long[2])
#define EVIOSEXCL               _IOW('e', 6, long[2])

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t vkey_t;
typedef int16_t vaxis_t;
typedef uint16_t evid_t;

typedef struct {
    evid_t ev_devid;
    uint8_t ev_type;
    
    union {
        struct {
            vkey_t vkey;
            uint8_t down;
        } ev_key;

        struct {
            vaxis_t x;
            vaxis_t y;
            vaxis_t z;
        } ev_rel;

        struct {
            vaxis_t x;
            vaxis_t y;
            vaxis_t z;
        } ev_abs;

        struct {
            uint16_t raw_type;
            uint64_t raw_data;
        } ev_msc;

        struct {
            int status;
        } ev_sw;

        struct {
            int16_t ledno;
            uint8_t on;
        } ev_led;

        struct {
            int status;
        } ev_pwr;

        char ev_args[1];
    };
} __attribute__((packed)) event_t;

typedef struct {
    evid_t ed_id;
    char ed_name[64];

    int ed_caps;
    int ed_enabled;
    int ed_exclusive;
} event_device_t;




/* Kernel */
evid_t sys_events_device_add(char* name, int caps);
int sys_events_device_remove(evid_t id);
int sys_events_device_set_enabled(evid_t id, int enabled);
int sys_events_device_set_caps(evid_t id, int caps);
int sys_events_device_set_exclusive(evid_t id, int evno);
int sys_events_raise(event_t* e);


#define sys_events_raise_EV_KEY(id, k, d)       \
    {                                           \
        event_t e;                              \
        e.ev_devid = id;                        \
        e.ev_type = EV_KEY;                     \
        e.ev_key.vkey = k;                      \
        e.ev_key.down = d;                      \
        sys_events_raise(&e);                   \
    }

#define sys_events_raise_EV_REL(id, dx, dy, dz) \
    {                                           \
        event_t e;                              \
        e.ev_devid = id;                        \
        e.ev_type = EV_REL;                     \
        e.ev_rel.x = dx;                        \
        e.ev_rel.y = dy;                        \
        e.ev_rel.z = dz;                        \
        sys_events_raise(&e);                   \
    }

#define sys_events_raise_EV_ABS(id, x, y, z)    \
    {                                           \
        event_t e;                              \
        e.ev_devid = id;                        \
        e.ev_type = EV_ABS;                     \
        e.ev_abs.x = x;                         \
        e.ev_abs.y = y;                         \
        e.ev_abs.z = z;                         \
        sys_events_raise(&e);                   \
    }

#define sys_events_raise_EV_PWR(id, s)          \
    {                                           \
        event_t e;                              \
        e.ev_devid = id;                        \
        e.ev_type = EV_PWR;                     \
        e.ev_pwr.status = s;                    \
        sys_events_raise(&e);                   \
    }

#ifdef __cplusplus
}
#endif

#endif