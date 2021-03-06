#ifndef __STREAMPUCK_H__
#define __STREAMPUCK_H__

#if !defined(SP_LOG) && !defined(NDEBUG)
#  define SP_LOG(lvl, info, fmt, ...) fprintf(stderr, "["lvl"] \
		(%s:%d"info"): "fmt"\n" \
		__FILE__, __LINE__, ##__VA_ARGS__)
#  define clean_errno() (errno == 0 ? "None" : strerror(errnro))
#else
#  define SP_LOG(lvl, info, fmt, ...)
#  define clean_errno()
#endif
#define SP_LOG_ERROR(fmt, ...) SP_LOG("ERROR", " errno:%d:%s", fmt, \
		errno, clean_errno(), ##__VA_ARGS__)
#define SP_LOG_WARN(fmt, ...) SP_LOG("WARN", " errno:%d:%s", fmt, \
		errno, clean_errno(), ##__VA_ARGS__)
#define SP_LOG_INFO(fmt, ...) SP_LOG("INFO", "", fmt, ##__VA_ARGS__)

#define SP_UNUSED(arg) (void )(arg)

#include "sp_mem.h"
#include "mp_custom.h"

struct sp_callbacks_t {
	int (*sp_nil)         (void *ctx);
	int (*sp_uint)        (void *ctx, uint64_t val);
	int (*sp_int)         (void *ctx, int64_t val);
	int (*sp_str)         (void *ctx, const char *val, uint32_t val_len);
	int (*sp_bin)         (void *ctx, const char *val, uint32_t val_len);
	/* Callback for array begin */
	int (*sp_array_begin) (void *ctx, uint32_t len);
	/* Callback called befor array value */
	int (*sp_array_value) (void *ctx, enum mp_type type);
	/* Callback for array end */
	int (*sp_array_end)   (void *ctx);
	/* Callback for map begin */
	int (*sp_map_begin)   (void *ctx, uint32_t len);
	/* Callback called befor map key */
	int (*sp_map_key)     (void *ctx, enum mp_type type);
	/* Callback called befor map value */
	int (*sp_map_value)   (void *ctx, enum mp_type type);
	/* Callback for map end */
	int (*sp_map_end)     (void *ctx);
	int (*sp_bool)        (void *ctx, char val);
	int (*sp_float)       (void *ctx, float val);
	int (*sp_double)      (void *ctx, double val);
	int (*sp_ext)         (void *ctx, uint8_t type, const char *val, uint32_t val_len);
	/* Callback for begin of top-level element */
	/* for sp_allow_multiple */
	int (*sp_query_begin) (void *ctx);
	/* Callback for end of top-level element */
	/* for sp_allow_multiple */
	int (*sp_query_end)   (void *ctx);
};

enum sp_state {
	sp_state_start = 1,
	sp_state_process,
	sp_state_end,
	sp_state_cancel,
	sp_state_error_garbage,
	sp_state_error_need_more,
	sp_state_error_depth_exceeded,
	sp_state_error_not_enough_mem,
};

enum sp_status {
	sp_status_ok,
	sp_status_cancel,
	sp_status_error
};

enum sp_options {
	sp_validate_before = 0x01,
	sp_allow_garbage   = 0x02,
	sp_allow_multiple  = 0x04,
	sp_set_cb          = 0x08,
	sp_set_cb_ctx      = 0x10,
	sp_set_mem         = 0x20,
};

struct sp_istate_stack_t {
	uint32_t     left;
		/**
		 * MP_MAP
		 * MP_ARRAY
		 */
	enum mp_type type;
};

#define SP_DEPTH_MAX 128
#define SP_DEPTH_STEP 16

struct sp_istate_t {
	struct sp_istate_stack_t *state;
	uint16_t depth;
	uint16_t max_depth;
};

struct sp_handle_t {
	struct sp_callbacks_t *cb;
	enum sp_state state;
	void *cb_ctx;
	struct sp_istate_t *istate;
	enum sp_options opt;
	const char *buf;
	const char *buf_pos;
	const char *buf_end;
};

struct sp_handle_t *sp_alloc();
void   sp_free(struct sp_handle_t *h);
void   sp_feed(struct sp_handle_t *h, char *buffer, size_t enlarge);
void   sp_reset(struct sp_handle_t *h);
void   sp_refeed(struct sp_handle_t *h, char *buffer, size_t size);
int    sp_options_set(struct sp_handle_t *h, enum sp_options opt, void *val);
int    sp_options_unset(struct sp_handle_t *h, enum sp_options opt);
enum sp_status sp_process(struct sp_handle_t *h);

#define sp_opt_cgarbage(hdl)  ((hdl)->opt & sp_allow_garbage)
#define sp_opt_cmultiple(hdl) ((hdl)->opt & sp_allow_multiple)
#define sp_opt_cvalidate(hdl) ((hdl)->opt & sp_validate_before)

#define sp_st_isstart(hdl)    ((hdl)->state & sp_state_start)
#define sp_st_isneedmore(hdl) ((hdl)->state & sp_state_error_need_more)

#define sp_is_end(hdl)	      ((hdl)->buf_end == (hdl)->buf_pos)

int sp_default_cb_nil (void *ctx);
int sp_default_cb_uint (void *ctx, uint64_t val);
int sp_default_cb_int (void *ctx, int64_t val);
int sp_default_cb_str (void *ctx, const char *val, uint32_t val_len);
int sp_default_cb_bin (void *ctx, const char *val, uint32_t val_len);
int sp_default_cb_array_begin (void *ctx, uint32_t len);
int sp_default_cb_array_value (void *ctx, enum mp_type type);
int sp_default_cb_array_end (void *ctx);
int sp_default_cb_map_begin (void *ctx, uint32_t len);
int sp_default_cb_map_key (void *ctx, enum mp_type type);
int sp_default_cb_map_value (void *ctx, enum mp_type type);
int sp_default_cb_map_end (void *ctx);
int sp_default_cb_bool (void *ctx, char val);
int sp_default_cb_float (void *ctx, float val);
int sp_default_cb_double (void *ctx, double val);
int sp_default_cb_ext (void *ctx, uint8_t type, const char *val, uint32_t val_len);
int sp_default_cb_query_begin (void *ctx);
int sp_default_cb_query_end (void *ctx);

#endif /* __STREAMPUCK_H__ */
