#define MP_SOURCE 1
#include "msgpuck/msgpuck.h"
#include "streampuck.h"
#include "mp_additions.h"

#include <stdio.h>

enum mp_type
sp_istate_curtype(struct sp_istate_t *ist) {
	assert (ist);
	if (ist->depth == 0) return MP_NIL;
	return ist->state[ist->depth - 1].type;
}

uint32_t
sp_istate_curleft(struct sp_istate_t *ist) {
	assert (ist);
	if (ist->depth == 0) return 0;
	return ist->state[ist->depth - 1].left;
}

int
sp_istate_pop(struct sp_istate_t *ist) {
	assert (ist);
	if (ist->depth == 0) return -1;
	if (ist->state[ist->depth - 1].left == 1) {
		ist->depth--;
		return 0;
	}
	return --ist->state[ist->depth - 1].left;
}

int
sp_istate_push(struct sp_istate_t *ist, enum mp_type type, uint32_t size) {
	assert(ist); assert(type == MP_MAP || type == MP_ARRAY); assert(size > 0);
	if (ist->depth == SP_MAX_DEPTH - 1) return -1;
	ist->state[ist->depth++] = (struct sp_istate_stack_t){size,type};
	return 0;
}

void sp_free(struct sp_handle_t *h) {
	if (h) {
		if (h->cb) SP_FREE((void *)h->cb);
		if (h->istate) SP_FREE((void *)h->istate);
		SP_FREE((void *)h);
	}
}

struct sp_handle_t *
sp_alloc() {
	struct sp_handle_t *h = SP_MALLOC(sizeof(struct sp_handle_t));
	memset(h, 0, sizeof(struct sp_handle_t));
	if (!h) goto cleanup;
	h->cb = SP_MALLOC(sizeof(struct sp_callbacks_t));
	if (!h->cb) goto cleanup;
	h->istate = SP_MALLOC(sizeof(struct sp_istate_t));
	if (!h->istate) goto cleanup;
	return h;
cleanup:
	sp_free(h);
	return NULL;
}

void sp_feed(struct sp_handle_t *h, char *buffer, size_t enlarge) {
	assert(h); assert(enlarge > 0); assert (buffer || h->buf);
	if (buffer) h->buf = h->buf_pos = h->buf_end = buffer;
	h->buf_end += enlarge;
}

void sp_reset(struct sp_handle_t *h) {
	assert(h);
	h->buf_pos = h->buf;
	h->istate->depth = 0;
}

void sp_refeed(struct sp_handle_t *h, char *buffer, size_t size) {
	assert(h); assert(h->buf);
	size_t ppos = h->buf_pos - h->buf;
	assert(buffer && size >= ppos);
	h->buf = h->buf_pos = h->buf_end = buffer;
	h->buf_pos += ppos;
	h->buf_end += size;
}

#define CB_APPLY(FROM, TO, TYPE) do {				\
		if (FROM->sp_##TYPE)				\
			TO->sp_##TYPE = FROM->sp_##TYPE;	\
		else						\
			TO->sp_##TYPE = sp_default_cb_##TYPE;	\
	} while (0)

void
sp_options_unset(struct sp_handle_t *h, enum sp_options opt) {
	assert(h);
	switch (opt) {
	case (sp_validate_before):
		h->opt &= ~(sp_validate_before);
		break;
	case (sp_allow_garbage):
		h->opt &= ~(sp_allow_garbage);
		break;
	case (sp_allow_multiple):
		h->opt &= ~(sp_allow_multiple);
		break;
	case (sp_set_mem_ctx):
		break;
	case (sp_set_cb):
		if (h->cb) SP_FREE((void *)h->cb);
		h->cb = NULL;
		break;
	case (sp_set_cb_ctx):
		h->cb_ctx = NULL;
		break;
	}
}

int
sp_options_set(struct sp_handle_t *h, enum sp_options opt, void *val) {
	switch (opt) {
	case (sp_validate_before):
		h->opt |= opt;
		break;
	case (sp_allow_garbage):
		if (h->opt & sp_allow_multiple) return -1;
		h->opt |= opt;
		break;
	case (sp_allow_multiple):
		if (h->opt & sp_allow_garbage) return -1;
		h->opt |= opt;
		break;
	case (sp_set_mem_ctx):
		if (!val) return -1;
		break;
	case (sp_set_cb):
		if (!h->cb) h->cb = SP_MALLOC(sizeof(struct sp_callbacks_t));
		if (!h->cb) return -1;
		struct sp_callbacks_t *cb = (struct sp_callbacks_t *)val;
		CB_APPLY(cb, h->cb, nil);
		CB_APPLY(cb, h->cb, uint);
		CB_APPLY(cb, h->cb, int);
		CB_APPLY(cb, h->cb, str);
		CB_APPLY(cb, h->cb, bin);
		CB_APPLY(cb, h->cb, array_begin);
		CB_APPLY(cb, h->cb, array_value);
		CB_APPLY(cb, h->cb, array_end);
		CB_APPLY(cb, h->cb, map_begin);
		CB_APPLY(cb, h->cb, map_key);
		CB_APPLY(cb, h->cb, map_value);
		CB_APPLY(cb, h->cb, map_end);
		CB_APPLY(cb, h->cb, bool);
		CB_APPLY(cb, h->cb, float);
		CB_APPLY(cb, h->cb, double);
		CB_APPLY(cb, h->cb, ext);
		CB_APPLY(cb, h->cb, query_begin);
		CB_APPLY(cb, h->cb, query_end);
		break;
	case (sp_set_cb_ctx):
		if (!val) return -1;
		h->cb_ctx = val;
		break;
	}
}

enum sp_status
sp_process(struct sp_handle_t *h) {
	const char *data = h->buf_pos;
	while (1) {
		int status = 0;
		if (h->state & sp_state_start) {
			status = h->cb->sp_query_begin(h->cb_ctx);
			if (status) goto cancel;
		}
		if (mp_unlikely(h->buf_pos >= h->buf_end)) {
			h->state = sp_state_needmore;
			return sp_status_error;
		}
		if ((h->opt & sp_validate_before) && ((h->state == sp_state_start) ||
					(h->state == sp_state_needmore))) {
			const char *datachk = data;
			if (mp_check(&datachk, h->buf_end)) {
				h->state = sp_state_needmore;
				return sp_status_error;
			}
		}
		enum mp_type type = mp_typeof(*data);
		if (!(h->opt & sp_validate_before) &&
				mp_unlikely(mp_singlecheck(data, h->buf_end))) {
			h->state = sp_state_needmore;
			return sp_status_error;
		}
		if (sp_istate_curtype(h->istate) == MP_ARRAY) {
			status = h->cb->sp_array_value(h->cb_ctx, type);
			if (status) goto cancel;
		} else if (sp_istate_curtype(h->istate) == MP_MAP) {
			if (sp_istate_curleft(h->istate) % 2 == 0)
				status = h->cb->sp_map_key(h->cb_ctx, type);
			else
				status = h->cb->sp_map_value(h->cb_ctx, type);
			if (status) goto cancel;
		}
		h->state = sp_state_process;
		uint8_t ext_type = 0;
		uint32_t val_len = 0;
		const char *val = NULL;
		switch (type) {
		case MP_NIL:
			status = h->cb->sp_nil(h->cb_ctx);
			break;
		case MP_UINT:
			status = h->cb->sp_uint(h->cb_ctx, mp_decode_uint(&data));
			break;
		case MP_INT:
			status = h->cb->sp_int(h->cb_ctx, mp_decode_int(&data));
			break;
		case MP_STR:
			val_len = 0;
			val = mp_decode_str(&data, &val_len);
			status = h->cb->sp_str(h->cb_ctx, val, val_len);
			break;
		case MP_BIN:
			val_len = 0;
			val = mp_decode_bin(&data, &val_len);
			status = h->cb->sp_bin(h->cb_ctx, val, val_len);
			break;
		case MP_ARRAY:
			val_len = mp_decode_array(&data);
			status = h->cb->sp_array_begin(h->cb_ctx, val_len);
			if (status) goto cancel;
			sp_istate_push(h->istate, MP_ARRAY, val_len);
			break;
		case MP_MAP:
			val_len = mp_decode_map(&data);
			status = h->cb->sp_map_begin(h->cb_ctx, val_len);
			if (status) goto cancel;
			sp_istate_push(h->istate, MP_MAP, val_len * 2);
			break;
		case MP_BOOL:
			status = h->cb->sp_bool(h->cb_ctx, mp_decode_bool(&data));
			break;
		case MP_FLOAT:
			status = h->cb->sp_float(h->cb_ctx, mp_decode_float(&data));
			break;
		case MP_DOUBLE:
			status = h->cb->sp_double(h->cb_ctx, mp_decode_double(&data));
			break;
		case MP_EXT:
			ext_type = mp_decode_ext(&data, &val, &val_len);
			status = h->cb->sp_ext(h->cb_ctx, ext_type, val, val_len);
			break;
		}
		if (status) goto cancel;
		h->buf_pos = data;
		if (sp_istate_pop(h->istate) == -1) {
			if (h->buf_end != h->buf_pos) {
				if (h->opt & sp_allow_garbage) {
					h->state = sp_state_end;
					status = h->cb->sp_query_end(h->cb_ctx);
					if (status) goto cancel;
					return sp_status_ok;
				} else if (h->opt & sp_allow_multiple) {
					h->state = sp_state_start;
					continue;
				} else {
					h->state = sp_state_garbage;
					return sp_status_error;
				}
			} else {
				h->state = sp_state_end;
				status = h->cb->sp_query_end(h->cb_ctx);
				if (status) goto cancel;
				return sp_status_ok;
			}
		}
	}
cancel:
	h->state = sp_state_cancel;
	return sp_status_cancel;
}


int
sp_default_cb_nil (void *ctx) {
	(void *)ctx;
	return 0;
}

int
sp_default_cb_uint (void *ctx, uint64_t val) {
	(void *)ctx;
	(uint64_t )val;
	return 0;
}

int
sp_default_cb_int (void *ctx, int64_t val) {
	(void *)ctx;
	(int64_t )val;
	return 0;
}

int
sp_default_cb_str (void *ctx, const char *val, uint32_t val_len) {
	(void *)ctx;
	(const char *)val;
	(uint32_t )val_len;
	return 0;
}

int
sp_default_cb_bin (void *ctx, const char *val, uint32_t val_len) {
	(void *)ctx;
	(const char *)val;
	(uint32_t )val_len;
	return 0;
}

int
sp_default_cb_array_begin (void *ctx, uint32_t len) {
	(void *)ctx;
	(uint32_t )len;
	return 0;
}

int
sp_default_cb_array_value (void *ctx, enum mp_type type) {
	(void *)ctx;
	(enum mp_type )type;
	return 0;
}

int
sp_default_cb_array_end (void *ctx) {
	(void *)ctx;
	return 0;
}

int
sp_default_cb_map_begin (void *ctx, uint32_t len) {
	(void *)ctx;
	return 0;
}

int
sp_default_cb_map_key (void *ctx, enum mp_type type) {
	(void *)ctx;
	(enum mp_type )type;
	return 0;
}

int
sp_default_cb_map_value (void *ctx, enum mp_type type) {
	(void *)ctx;
	(enum mp_type )type;
	return 0;
}

int
sp_default_cb_map_end (void *ctx) {
	(void *)ctx;
	return 0;
}

int
sp_default_cb_bool (void *ctx, char val) {
	(void *)ctx;
	(char )val;
	return 0;
}

int
sp_default_cb_float (void *ctx, float val) {
	(void *)ctx;
	(float )val;
	return 0;
}

int
sp_default_cb_double (void *ctx, double val) {
	(void *)ctx;
	(double )val;
	return 0;
}

int
sp_default_cb_ext (void *ctx, uint8_t type, const char *val, uint32_t val_len) {
	(void *)ctx;
	(uint8_t )type;
	(const char *)val;
	(uint32_t )val_len;
	return 0;
}

int
sp_default_cb_query_begin (void *ctx) {
	(void *)ctx;
	return 0;
}

int
sp_default_cb_query_end (void *ctx) {
	(void *)ctx;
	return 0;
}
