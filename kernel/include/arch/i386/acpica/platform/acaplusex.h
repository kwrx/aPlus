#ifndef __ACAPLUSEX_H__
#define __ACAPLUSEX_H__


#ifndef ACPI_USE_NATIVE_DIVIDE

#ifndef ACPI_DIV_64_BY_32
#define ACPI_DIV_64_BY_32(n_hi, n_lo, d32, q32, r32) \
    do { \
        UINT64 (__n) = ((UINT64) n_hi) << 32 | (n_lo); \
        (r32) = do_div ((__n), (d32)); \
        (q32) = (UINT32) (__n); \
    } while (0)
#endif

#ifndef ACPI_SHIFT_RIGHT_64
#define ACPI_SHIFT_RIGHT_64(n_hi, n_lo) \
    do { \
        (n_lo) >>= 1; \
        (n_lo) |= (((n_hi) & 1) << 31); \
        (n_hi) >>= 1; \
    } while (0)
#endif

#endif


#endif /* __ACAPLUSEX_H__ */
