#include <msgpuck.h>

int
mp_singlecheck(const char *data, const char *end) {
	if (mp_unlikely(data >= end))
		return 1;
	uint8_t c = mp_load_u8(&data);
	int l = mp_parser_hint[c];
	if (mp_likely(l >= 0)) {
		if (mp_unlikely(data + l > end))
			return 1;
		return 0;
	} else if (mp_likely(l > MP_HINT)) {
		return 0;
	}
	uint32_t len;
	switch (l) {
	case MP_HINT_STR_8:
		/* MP_STR (8) */
		if (mp_unlikely(data + sizeof(uint8_t) > end))
			return 1;
		len = mp_load_u8(&data);
		break;
	case MP_HINT_STR_16:
		/* MP_STR (16) */
		if (mp_unlikely(data + sizeof(uint16_t) > end))
			return 1;
		len = mp_load_u16(&data);
		break;
	case MP_HINT_STR_32:
		/* MP_STR (32) */
		if (mp_unlikely(data + sizeof(uint32_t) > end))
			return 1;
		len = mp_load_u32(&data);
		break;
	case MP_HINT_ARRAY_16:
		/* MP_ARRAY (16) */
		if (mp_unlikely(data + sizeof(uint16_t) > end))
			return 1;
		break;
	case MP_HINT_ARRAY_32:
		/* MP_ARRAY (32) */
		if (mp_unlikely(data + sizeof(uint32_t) > end))
			return 1;
		break;
	case MP_HINT_MAP_16:
		/* MP_MAP (16) */
		if (mp_unlikely(data + sizeof(uint16_t) > end))
			return false;
		break;
	case MP_HINT_MAP_32:
		/* MP_MAP (32) */
		if (mp_unlikely(data + sizeof(uint32_t) > end))
			return 1;
		break;
	case MP_HINT_EXT_8:
		/* MP_EXT (8) */
		if (mp_unlikely(data + sizeof(uint8_t) + 1 > end))
			return 1;
		len = mp_load_u8(&data) + sizeof(uint8_t);
		break;
	case MP_HINT_EXT_16:
		/* MP_EXT (16) */
		if (mp_unlikely(data + sizeof(uint16_t) + 1 > end))
			return 1;
		len = mp_load_u16(&data) + sizeof(uint8_t);
		break;
	case MP_HINT_EXT_32:
		/* MP_EXT (32) */
		if (mp_unlikely(data + sizeof(uint32_t) + 1 > end))
			return 1;
		len = mp_load_u32(&data) + sizeof(uint8_t);
		break;
	default:
		mp_unreachable();
	}

	if (mp_unlikely(data + len > end))
		return 1;
	return 0;
}

uint32_t
mp_sizeof_extl(uint32_t len) {
	if (len == 1 || len == 2 || len == 4 || len == 8 || len == 16) {
		return 1;
	} else if (len <= UINT8_MAX) {
		return 1 + sizeof(uint8_t);
	} else if (len <= UINT16_MAX) {
		return 1 + sizeof(uint16_t);
	} else {
		return 1 + sizeof(uint32_t);
	}
}

size_t
mp_sizeof_ext(uint32_t len) {
	return mp_sizeof_extl(len) + len;
}

uint32_t
mp_decode_extl(const char **data) {
	uint8_t c = mp_load_u8(data);
	switch (c) {
	case 0xd4 ... 0xd8:
		return 1 << (c - 0xd4);
	case 0xc7:
		return mp_load_u8(data);
	case 0xc8:
		return mp_load_u16(data);
	case 0xc9:
		return mp_load_u32(data);
	default:
		mp_unreachable();
	}
}

/*
 * \brief Decode an ext format from MsgPack \a data
 * \param data - the pointer to a buffer
 * \param val - the pointer to save a pointer of byte array
 * \param val_len - the pointer to save a byte array length
 * \return type of ext format
 * \post *data = *data + mp_sizeof_ext(*val_len)
 */
uint8_t
mp_decode_ext(const char **data, const char **val, uint32_t *val_len) {
	assert(val != NULL); assert(val_len != NULL);
	*val_len = mp_decode_extl(data);
	uint8_t type = mp_load_u8(data);
	*val = *data;
	*data += *val_len;
	return type;
}

char *
mp_encode_extl(char *data, uint32_t len) {
	if (len == 1 || len == 2 || len == 4 || len == 8 || len == 16) {
		int depth = 0;
		for (; len > 1; ++depth) len = len<<1;
		return mp_store_u8(data, 0xd4 + depth);
	} else if (len <= UINT8_MAX) {
		data = mp_store_u8(data, 0xc7);
		return mp_store_u8(data, len);
	} else if (len <= UINT16_MAX) {
		data = mp_store_u8(data, 0xc8);
		return mp_store_u16(data, len);
	} else {
		data = mp_store_u8(data, 0xc9);
		return mp_store_u32(data, len);
	}
}

char *
mp_encode_ext(char *data, uint8_t type, char *val, uint32_t val_len) {
	data = mp_encode_extl(data, val_len);
	data = mp_store_u8(data, type);
	memcpy(data, val, val_len);
	return data + val_len;
}
