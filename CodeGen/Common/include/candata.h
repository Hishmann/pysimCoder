#ifndef CANDATA_H
#define CANDATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
CANopen CiA 301 data type mapping.

CANopen type   Type ID   C type     Put / Get helper suffix   Width
-----------------------------------------------------------------
INTEGER8       0x0002    int8_t     i8                         8
INTEGER16      0x0003    int16_t    i16                       16
INTEGER32      0x0004    int32_t    i32                       32
INTEGER64      0x0015    int64_t    i64                       64

UNSIGNED8      0x0005    uint8_t    u8                         8
UNSIGNED16     0x0006    uint16_t   u16                       16
UNSIGNED32     0x0007    uint32_t   u32                       32
UNSIGNED64     0x001B    uint64_t   u64                       64

REAL32         0x0008    float      float                    32
REAL64         0x0011    double     double                   64

Generic helpers:
    can_put_bits() / can_get_bits():
        unsigned fields of 1..64 bits, stored in uint64_t.
    can_put_signed_bits() / can_get_signed_bits():
        signed fields of 1..64 bits, stored in int64_t.

Intel bit layout: bit 0 is the LSB of data[0]. Offsets and widths
are measured in bits; length is the actual payload size in bytes (0-64).
Fields may cross byte boundaries.

Integer widths range from 1 to the capacity of the selected C type.
Signed fields use two's-complement encoding and are sign-extended on read.
Writes reject values that do not fit; unrelated bits are preserved.

Floats occupy 32 bits and doubles occupy 64 bits. They require IEEE-754
object layouts with the same byte ordering as uint32_t and uint64_t.

Functions return false for NULL pointers, invalid fields, or values that
do not fit. Failed calls leave the destination unchanged.

Example: CANopen INTEGER16 at payload byte offset 2:
    can_put_i16(data, length, 16, 16, value);
    can_get_i16(data, length, 16, 16, &value);
*/

/* CANopen CiA 301 data type IDs for the types supported by candata.h. */
typedef enum {
    CANOPEN_INTEGER8   = 0x0002,
    CANOPEN_INTEGER16  = 0x0003,
    CANOPEN_INTEGER32  = 0x0004,
    CANOPEN_UNSIGNED8  = 0x0005,
    CANOPEN_UNSIGNED16 = 0x0006,
    CANOPEN_UNSIGNED32 = 0x0007,
    CANOPEN_REAL32     = 0x0008,
    CANOPEN_REAL64     = 0x0011,
    CANOPEN_INTEGER64  = 0x0015,
    CANOPEN_UNSIGNED64 = 0x001B 
} canopen_data_type_t;


bool can_put_from_double(int8_t *data, size_t length, unsigned offset, unsigned width, double value, canopen_data_type_t datatype);
bool can_get_as_double(const int8_t *data, size_t length, unsigned offset, unsigned width, double *value, canopen_data_type_t datatype);

/* Generic unsigned and signed fields, with widths from 1 to 64 bits. */
bool can_put_bits(int8_t *data, size_t length, unsigned offset, unsigned width, uint64_t value);
bool can_get_bits(const int8_t *data, size_t length, unsigned offset, unsigned width, uint64_t *value);
bool can_put_signed_bits(int8_t *data, size_t length, unsigned offset, unsigned width, int64_t value);
bool can_get_signed_bits(const int8_t *data, size_t length, unsigned offset, unsigned width, int64_t *value);

/* Typed integer fields with flexible widths. */
bool can_put_u8(int8_t *data, size_t length, unsigned offset, unsigned width, uint8_t value);
bool can_get_u8(const int8_t *data, size_t length, unsigned offset, unsigned width, uint8_t *value);
bool can_put_u16(int8_t *data, size_t length, unsigned offset, unsigned width, uint16_t value);
bool can_get_u16(const int8_t *data, size_t length, unsigned offset, unsigned width, uint16_t *value);
bool can_put_u32(int8_t *data, size_t length, unsigned offset, unsigned width, uint32_t value);
bool can_get_u32(const int8_t *data, size_t length, unsigned offset, unsigned width, uint32_t *value);
bool can_put_u64(int8_t *data, size_t length, unsigned offset, unsigned width, uint64_t value);
bool can_get_u64(const int8_t *data, size_t length, unsigned offset, unsigned width, uint64_t *value);
bool can_put_i8(int8_t *data, size_t length, unsigned offset, unsigned width, int8_t value);
bool can_get_i8(const int8_t *data, size_t length, unsigned offset, unsigned width, int8_t *value);
bool can_put_i16(int8_t *data, size_t length, unsigned offset, unsigned width, int16_t value);
bool can_get_i16(const int8_t *data, size_t length, unsigned offset, unsigned width, int16_t *value);
bool can_put_i32(int8_t *data, size_t length, unsigned offset, unsigned width, int32_t value);
bool can_get_i32(const int8_t *data, size_t length, unsigned offset, unsigned width, int32_t *value);
bool can_put_i64(int8_t *data, size_t length, unsigned offset, unsigned width, int64_t value);
bool can_get_i64(const int8_t *data, size_t length, unsigned offset, unsigned width, int64_t *value);

/* Works for Fixed-width IEEE-754 fields. */
bool can_put_float(int8_t *data, size_t length, unsigned offset, float value);
bool can_get_float(const int8_t *data, size_t length, unsigned offset, float *value);
bool can_put_double(int8_t *data, size_t length, unsigned offset, double value);
bool can_get_double(const int8_t *data, size_t length, unsigned offset, double *value);

#ifdef __cplusplus
}
#endif

#endif /* CANDATA_H */
