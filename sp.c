#define MP_SOURCE 1
#include <msgpuck.h>
#include "sp.h"
#include "mp_custom.h"

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
		return ist->depth--;
	}
	return --ist->state[ist->depth - 1].left;
}

int
sp_istate_growstack(struct sp_istate_t *ist) {
	struct sp_istate_stack_t *s = SP_REALLOC(ist->state, ist->max_depth + SP_DEPTH_STEP);
	if (!s) return -1;
	ist->state = s;
	ist->max_depth += SP_DEPTH_STEP;
	return 0;
}

int
sp_istate_push(struct sp_istate_t *ist, enum mp_type type, uint32_t size) {
	assert(ist); assert(type == MP_MAP || type == MP_ARRAY); assert(size > 0);
	if (ist->depth == SP_DEPTH_MAX - 1) return -1;
	if (ist->depth == ist->max_depth - 1) {
		if (sp_istate_growstack(ist))
			return -2;
	}
	ist->state[ist->depth++] = (struct sp_istate_stack_t){size,type};
	return 0;
}

void sp_free(struct sp_handle_t *h) {
	if (h) {
		if (h->cb) SP_FREE((void *)h->cb);
		if (h->istate) {
			if (h->istate->state)
				SP_FREE((void *)h->istate->state);
			SP_FREE((void *)h->istate);
		}
		SP_FREE((void *)h);
	}
}

struct sp_handle_t *
sp_alloc() {
	struct sp_handle_t *h = SP_MALLOC(sizeof(struct sp_handle_t));
	memset(h, 0, sizeof(struct sp_handle_t));
	if (!h) goto cleanup;
	h->state = sp_state_start;
	h->cb = SP_MALLOC(sizeof(struct sp_callbacks_t));
	if (!h->cb) goto cleanup;
	h->istate = SP_MALLOC(sizeof(struct sp_istate_t));
	struct sp_callbacks_t cb = {
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL
	};
	sp_options_set(h, sp_set_cb, &cb);
	if (!h->istate) goto cleanup;
	h->istate->state = SP_MALLOC(sizeof(struct sp_istate_stack_t) * SP_DEPTH_STEP);
	h->istate->max_depth = SP_DEPTH_STEP;
	if (!h->istate->state) goto cleanup;
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
	h->state = sp_state_start;
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

int
sp_options_unset(struct sp_handle_t *h, enum sp_options opt) {
	struct sp_callbacks_t cb = {0};
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
	case (sp_set_mem):
		sp_mem_init((sp_allocator_t *)realloc);
		break;
	case (sp_set_cb):
		cb = (struct sp_callbacks_t ){
			NULL, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL
		};
		sp_options_set(h, sp_set_cb, &cb);
		break;
	case (sp_set_cb_ctx):
		h->cb_ctx = NULL;
		break;
	default:
		return -1;
	}
	return 0;
}

int
sp_options_set(struct sp_handle_t *h, enum sp_options opt, void *val) {
	assert(h);
	struct sp_callbacks_t *cb = NULL;
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
	case (sp_set_mem):
		sp_mem_init((sp_allocator_t *)val);
		break;
	case (sp_set_cb):
		cb = (struct sp_callbacks_t *)val;
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
	default:
		return -1;
	}
	return 0;
}

#undef CB_APPLY

enum sp_status
sp_process(struct sp_handle_t *h) {
	const char *data = h->buf_pos;
	while (1) {
		int status = 0;
		if (sp_st_isstart(h) && sp_opt_cmultiple(h)) {
			status = h->cb->sp_query_begin(h->cb_ctx);
			if (status) goto cancel;
		}
		/* Validate begin */
		if (mp_unlikely(h->buf_pos >= h->buf_end)) {
			goto needmore;
		}
		enum mp_type type = mp_typeof(*data);
		if (sp_opt_cvalidate(h)) {
			if (sp_st_isstart(h) || sp_st_isneedmore(h)) {
				const char *datachk = data;
				if (mp_unlikely(mp_check(&datachk, h->buf_end)))
					goto needmore;
			}
		} else {
			if (mp_unlikely(mp_singlecheck(data, h->buf_end)))
				goto needmore;
		}
		/* Validate end */
		h->state = sp_state_process;
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
		uint8_t ext_type = 0;
		uint32_t val_len = 0;
		const char *val = NULL;
		int push_state = 0;
		switch (type) {
		case MP_NIL:
			data += 1;
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
			push_state = sp_istate_push(h->istate, MP_ARRAY, val_len);
			if (push_state == -1) {
				h->state = sp_state_error_depth_exceeded;
				return sp_status_error;
			} else if (push_state == -2) {
				h->state = sp_state_error_not_enough_mem;
				return sp_status_error;
			}
			h->buf_pos = data;
			continue;
			break;
		case MP_MAP:
			val_len = mp_decode_map(&data);
			status = h->cb->sp_map_begin(h->cb_ctx, val_len);
			if (status) goto cancel;
			push_state = sp_istate_push(h->istate, MP_MAP, val_len * 2);
			if (push_state == -1) {
				h->state = sp_state_error_depth_exceeded;
				return sp_status_error;
			} else if (push_state == -2) {
				h->state = sp_state_error_not_enough_mem;
				return sp_status_error;
			}
			h->buf_pos = data;
			continue;
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
		default:
			mp_unreachable();
		}
		h->buf_pos = data;
		if (status) goto cancel;

		while (sp_istate_curleft(h->istate) == 1) {
			switch (sp_istate_curtype(h->istate)) {
			case (MP_MAP):
				status = h->cb->sp_map_end(h->cb_ctx);
				break;
			case (MP_ARRAY):
				status = h->cb->sp_array_end(h->cb_ctx);
				break;
			default:
				mp_unreachable();
			}
			if (status) goto cancel;
			sp_istate_pop(h->istate);
		}
		if (sp_istate_pop(h->istate) != -1)
			continue;
		h->state = sp_state_end;
		if (sp_opt_cmultiple(h)) {
			status = h->cb->sp_query_end(h->cb_ctx);
			if (status) goto cancel;
		}
		if (!sp_is_end(h)) {
			if (sp_opt_cmultiple(h)) {
				h->state = sp_state_start;
				continue;
			} else if (sp_opt_cgarbage(h)) {
				return sp_status_ok;
			} else {
				h->state = sp_state_error_garbage;
				return sp_status_error;
			}
		} else {
			return sp_status_ok;
		}
	}
cancel:
	h->state = sp_state_cancel;
	return sp_status_cancel;
needmore:
	h->state = sp_state_error_need_more;
	return sp_status_error;
}


int
sp_default_cb_nil (void *ctx) {
	SP_UNUSED(ctx);
	return 0;
}

int
sp_default_cb_uint (void *ctx, uint64_t val) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	return 0;
}

int
sp_default_cb_int (void *ctx, int64_t val) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	return 0;
}

int
sp_default_cb_str (void *ctx, const char *val, uint32_t val_len) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	SP_UNUSED(val_len);
	return 0;
}

int
sp_default_cb_bin (void *ctx, const char *val, uint32_t val_len) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	SP_UNUSED(val_len);
	return 0;
}

int
sp_default_cb_array_begin (void *ctx, uint32_t len) {
	SP_UNUSED(ctx);
	SP_UNUSED(len);
	return 0;
}

int
sp_default_cb_array_value (void *ctx, enum mp_type type) {
	SP_UNUSED(ctx);
	SP_UNUSED(type);
	return 0;
}

int
sp_default_cb_array_end (void *ctx) {
	SP_UNUSED(ctx);
	return 0;
}

int
sp_default_cb_map_begin (void *ctx, uint32_t len) {
	SP_UNUSED(ctx);
	SP_UNUSED(len);
	return 0;
}

int
sp_default_cb_map_key (void *ctx, enum mp_type type) {
	SP_UNUSED(ctx);
	SP_UNUSED(type);
	return 0;
}

int
sp_default_cb_map_value (void *ctx, enum mp_type type) {
	SP_UNUSED(ctx);
	SP_UNUSED(type);
	return 0;
}

int
sp_default_cb_map_end (void *ctx) {
	SP_UNUSED(ctx);
	return 0;
}

int
sp_default_cb_bool (void *ctx, char val) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	return 0;
}

int
sp_default_cb_float (void *ctx, float val) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	return 0;
}

int
sp_default_cb_double (void *ctx, double val) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	return 0;
}

int
sp_default_cb_ext (void *ctx, uint8_t type, const char *val, uint32_t val_len) {
	SP_UNUSED(ctx);
	SP_UNUSED(type);
	SP_UNUSED(val);
	SP_UNUSED(val_len);
	return 0;
}

int
sp_default_cb_query_begin (void *ctx) {
	SP_UNUSED(ctx);
	return 0;
}

int
sp_default_cb_query_end (void *ctx) {
	SP_UNUSED(ctx);
	return 0;
}
