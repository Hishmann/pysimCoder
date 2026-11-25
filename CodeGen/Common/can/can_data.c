#include <candata.h>

#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>

_Static_assert(CHAR_BIT == 8, "Requires 8-bit bytes");
_Static_assert(sizeof(float) == 4, "Requires binary32-compatible float");
_Static_assert(sizeof(double) == 8, "Requires binary64-compatible double");

static inline bool can_field_valid(size_t length, unsigned offset, unsigned width) {
  return length <= 64 &&
          width >= 1 && width <= 64 &&
          width <= length * 8 &&
          offset <= length * 8 - width;
}

/* width must be in [1, 64]. */
static inline uint64_t can_mask(unsigned width) {
  return UINT64_MAX >> (64 - width);
}

/* Write an unsigned field, preserving all unrelated bits.  */
bool can_put_bits(int8_t *data, size_t length, unsigned offset, unsigned width, uint64_t value) {
  
  if (!data || !can_field_valid(length, offset, width)) // Check if offset and width fit within length constraints
    return false;
  
  if (value > can_mask(width)) // Reject values that do not fit.
    return false;

  for (unsigned i = 0; i < width; ++i) {
    unsigned position = offset + i;
    unsigned index = position / 8;
    unsigned shift = position % 8;

    uint8_t byte = (uint8_t)data[index];
    uint8_t mask = (uint8_t)(1u << shift);

    if ((value >> i) & UINT64_C(1))
			byte = (uint8_t)(byte | mask);
    else
			byte = (uint8_t)(byte & (uint8_t)~mask);

    memcpy(&data[index], &byte, sizeof byte);
  }

  return true;
}

bool can_get_bits(const int8_t *data, size_t length, unsigned offset, unsigned width, uint64_t *value) {
	if (!data || !value || !can_field_valid(length, offset, width))
		return false;

	uint64_t result = 0;

	for (unsigned i = 0; i < width; ++i) {
		unsigned position = offset + i;
		uint8_t byte = (uint8_t)data[position / 8];
		uint64_t bit = (byte >> (position % 8)) & 1u;

		result |= bit << i;
	}

	*value = result;
	return true;
}

/* Signed fields use two's-complement encoding. */
bool can_put_signed_bits(int8_t *data, size_t length, unsigned offset, unsigned width, int64_t value) {
	if (!data || !can_field_valid(length, offset, width))
		return false;

	if (width < 64) {
		int64_t limit = INT64_C(1) << (width - 1); // width = 4 => limit = 8

		if (value < -limit || value > limit - 1) // value < -8 || value > 7
			return false;
	}

	return can_put_bits(data, length, offset, width, (uint64_t)value & can_mask(width));
}

bool can_get_signed_bits(const int8_t *data, size_t length, unsigned offset, unsigned width, int64_t *value) {
	uint64_t raw; // eg. raw = 0b1110 (-2)

	if (!data || !value || !can_get_bits(data, length, offset, width, &raw))
		return false;

	uint64_t sign = UINT64_C(1) << (width - 1); // width = 4 => sign = 0b1000

	if (raw & sign) 
		// If negative int 
		*value = -INT64_C(1) - (int64_t)(can_mask(width) ^ raw); // -1 - (0b1111 ^ 0b1110) = -1 - (0b0001) = -1 - 1 = -2
	else
		// If positive int
		*value = (int64_t)raw;

	return true;
}

/* Typed integer functions with flexible field widths. */

#define CAN_UNSIGNED_FUNCTIONS(SUFFIX, TYPE, TYPE_BITS)                   \
bool can_put_##SUFFIX(int8_t *data, size_t length, unsigned offset, unsigned width, TYPE value)   \
{                                                                      \
	if (!data || width == 0 || width > TYPE_BITS)                                \
		return false;                                                  \
	return can_put_bits(data, length, offset, width, (uint64_t)value);   \
}                                                                      \
bool can_get_##SUFFIX(const int8_t *data, size_t length, unsigned offset,unsigned width, TYPE *value)  \
{                                                                      \
	uint64_t raw;                                                      \
	if (!value || width == 0 || width > TYPE_BITS)                      \
		return false;                                                  \
	if (!can_get_bits(data, length, offset, width, &raw))                \
		return false;                                                  \
	*value = (TYPE)raw;                                                 \
	return true;                                                       \
}

#define CAN_SIGNED_FUNCTIONS(SUFFIX, TYPE, TYPE_BITS)                     \
bool can_put_##SUFFIX(int8_t *data, size_t length, unsigned offset, unsigned width, TYPE value) \
{                                                                      \
	if (!data || width == 0 || width > TYPE_BITS)                                \
		return false;                                                  \
	return can_put_signed_bits(data, length, offset, width, (int64_t)value); \
}                                                                      \
bool can_get_##SUFFIX(const int8_t *data, size_t length, unsigned offset, unsigned width, TYPE *value) \
{                                                                      \
	int64_t raw;                                                       \
	if (!value || width == 0 || width > TYPE_BITS)                      \
		return false;                                                  \
	if (!can_get_signed_bits(data, length, offset, width, &raw))         \
		return false;                                                  \
	*value = (TYPE)raw;                                                 \
	return true;                                                       \
}

CAN_UNSIGNED_FUNCTIONS(u8,  uint8_t,   8)
CAN_UNSIGNED_FUNCTIONS(u16, uint16_t, 16)
CAN_UNSIGNED_FUNCTIONS(u32, uint32_t, 32)
CAN_UNSIGNED_FUNCTIONS(u64, uint64_t, 64)

CAN_SIGNED_FUNCTIONS(i8,  int8_t,   8)
CAN_SIGNED_FUNCTIONS(i16, int16_t, 16)
CAN_SIGNED_FUNCTIONS(i32, int32_t, 32)
CAN_SIGNED_FUNCTIONS(i64, int64_t, 64)

#undef CAN_UNSIGNED_FUNCTIONS
#undef CAN_SIGNED_FUNCTIONS


bool can_put_float(int8_t *data, size_t length, unsigned offset, float value)
{
	uint32_t bits;

	if (!data || !can_field_valid(length, offset, 32))
		return false;

	memcpy(&bits, &value, sizeof bits);
	return can_put_u32(data, length, offset, 32, bits);
}

bool can_get_float(const int8_t *data, size_t length, unsigned offset, float *value)
{
	uint32_t bits;

	if (!value || !can_get_u32(data, length, offset, 32, &bits))
		return false;

	memcpy(value, &bits, sizeof bits);
	return true;
}

bool can_put_double(int8_t *data, size_t length, unsigned offset, double value)
{
	uint64_t bits;
	
	if (!data || !can_field_valid(length, offset, 64))
		return false;

	memcpy(&bits, &value, sizeof bits);
	return can_put_u64(data, length, offset, 64, bits);
}

bool can_get_double(const int8_t *data, size_t length, unsigned offset, double *value)
{
	uint64_t bits;

	if (!value || !can_get_u64(data, length, offset, 64, &bits))
		return false;

	memcpy(value, &bits, sizeof bits);
	return true;
}


/* Call only with width in [1, 64]. */
static bool can_double_fits_integer(double value, unsigned width, bool is_signed) {
    if (!isfinite(value)) // Avoid NAN or INFINITY
        return false;

    if (is_signed) {
        double limit = (double)(UINT64_C(1) << (width - 1));
        return value >= -limit && value < limit; // -2^(w-1) <= vakue <= 2^(w-1)
    }

    double limit = width == 64
        ? 0x1p64 // Cannot shift by 64 so the value corresponds to 2^64
        : (double)(UINT64_C(1) << width);

    return value >= 0.0 && value < limit;
}

bool can_put_from_double(int8_t *data, size_t length, unsigned offset, unsigned width, double value, canopen_data_type_t datatype) {
    if (!data || !isfinite(value))
        return false;

#define PUT_INTEGER(TAG, TYPE, SUFFIX, MAX_BITS, IS_SIGNED)              \
    case TAG: {                                                        \
        if (width == 0 || width > MAX_BITS || !can_double_fits_integer(value, width, IS_SIGNED))          \
            return false;                                              \
        TYPE converted = (TYPE)value;                                  \
        return can_put_##SUFFIX(data, length, offset, width, converted);  \
    }

    switch (datatype) {
        PUT_INTEGER(CANOPEN_UNSIGNED8,  uint8_t,  u8,   8, false)
        PUT_INTEGER(CANOPEN_UNSIGNED16, uint16_t, u16, 16, false)
        PUT_INTEGER(CANOPEN_UNSIGNED32, uint32_t, u32, 32, false)
        PUT_INTEGER(CANOPEN_UNSIGNED64, uint64_t, u64, 64, false)

        PUT_INTEGER(CANOPEN_INTEGER8,  int8_t,  i8,   8, true)
        PUT_INTEGER(CANOPEN_INTEGER16, int16_t, i16, 16, true)
        PUT_INTEGER(CANOPEN_INTEGER32, int32_t, i32, 32, true)
        PUT_INTEGER(CANOPEN_INTEGER64, int64_t, i64, 64, true)

    case CANOPEN_REAL32:
        if (width != 32 || value < -FLT_MAX || value > FLT_MAX) return false;
        return can_put_float(data, length, offset, (float)value);

    case CANOPEN_REAL64:
        return width == 64 && can_put_double(data, length, offset, value);

    default:
        return false;
    }

#undef PUT_INTEGER
}

bool can_get_as_double(const int8_t *data, size_t length, unsigned offset, unsigned width, double *value, canopen_data_type_t datatype) {
    if (!data || !value)
        return false;

#define GET_VALUE(TAG, TYPE, SUFFIX)                                    \
    case TAG: {                                                        \
        TYPE decoded;                                                  \
        if (!can_get_##SUFFIX(data, length, offset, width, &decoded))  \
            return false;                                              \
        *value = (double)decoded;                                      \
        return true;                                                   \
    }

    switch (datatype) {
        GET_VALUE(CANOPEN_UNSIGNED8,  uint8_t,  u8)
        GET_VALUE(CANOPEN_UNSIGNED16, uint16_t, u16)
        GET_VALUE(CANOPEN_UNSIGNED32, uint32_t, u32)
        GET_VALUE(CANOPEN_UNSIGNED64, uint64_t, u64)

        GET_VALUE(CANOPEN_INTEGER8,  int8_t,  i8)
        GET_VALUE(CANOPEN_INTEGER16, int16_t, i16)
        GET_VALUE(CANOPEN_INTEGER32, int32_t, i32)
        GET_VALUE(CANOPEN_INTEGER64, int64_t, i64)

    case CANOPEN_REAL32: {
        float decoded;
        if (width != 32 || !can_get_float(data, length, offset, &decoded))
            return false;
        *value = (double)decoded;
        return true;
    }

    case CANOPEN_REAL64:
        return width == 64 && can_get_double(data, length, offset, value);

    default:
        return false;
    }

#undef GET_VALUE
}