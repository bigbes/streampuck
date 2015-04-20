#ifndef __MP_ADDITIONS_H__
#define __MP_ADDITIONS_H__

int
mp_singlecheck(const char *data, const char *end);

uint32_t
mp_sizeof_extl(uint32_t len);

size_t
mp_sizeof_ext(uint32_t len);

uint32_t
mp_decode_extl(const char **data);

uint8_t
mp_decode_ext(const char **data, const char **val, uint32_t *val_len);

char *
mp_encode_extl(char *data, uint32_t len);

char *
mp_encode_ext(char *data, uint8_t type, char *val, uint32_t val_len);

#endif /* __MP_ADDITIONS_H__ */
