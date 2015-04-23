#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>

#include <msgpuck.h>
#include "test.h"
#include "sp.h"

#define header() note("*** %s: prep ***", __func__)
#define footer() note("*** %s: done ***", __func__)

enum mp_test_type {
	MPT_NIL = 0,
	MPT_UINT,
	MPT_INT,
	MPT_STR,
	MPT_BIN,
	MPT_ARRAY,
	MPT_MAP,
	MPT_BOOL,
	MPT_FLOAT,
	MPT_DOUBLE,
	MPT_EXT,
	MPT_ARRAY_BEGIN,
	MPT_ARRAY_VALUE,
	MPT_ARRAY_END,
	MPT_MAP_BEGIN,
	MPT_MAP_KEY,
	MPT_MAP_VALUE,
	MPT_MAP_END,
	MPT_QUERY_BEGIN,
	MPT_QUERY_END,
};

/*
static int
test_(void)
{
	plan();
	header();
	footer();
	return check_plan();
}
*/

struct sp_test_ctx {
	enum mp_test_type stack[16384];
	int depth;
};

int sp_test_nil(void *ctx) {
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_NIL;
	return 0;
}

int sp_test_uint(void *ctx, uint64_t val) {
	SP_UNUSED(val);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_UINT;
	return 0;
}

int sp_test_int(void *ctx, int64_t val) {
	SP_UNUSED(val);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_INT;
	return 0;
}

int sp_test_str(void *ctx, char *str, uint32_t str_len) {
	SP_UNUSED(str);
	SP_UNUSED(str_len);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_STR;
	return 0;
}

int sp_test_bin(void *ctx, char *str, uint32_t str_len) {
	SP_UNUSED(str);
	SP_UNUSED(str_len);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_BIN;
	return 0;
}

int sp_test_array_begin(void *ctx) {
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_ARRAY_BEGIN;
	return 0;
}

int sp_test_array_value(void *ctx, enum mp_type type) {
	SP_UNUSED(type);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_ARRAY_VALUE;
	return 0;
}

int sp_test_array_end(void *ctx) {
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_ARRAY_END;
	return 0;
}

int sp_test_map_begin(void *ctx) {
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_MAP_BEGIN;
	return 0;
}

int sp_test_map_key(void *ctx, enum mp_type type) {
	SP_UNUSED(type);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_MAP_KEY;
	return 0;
}

int sp_test_map_value(void *ctx, enum mp_type type) {
	SP_UNUSED(type);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_MAP_VALUE;
	return 0;
}

int sp_test_map_end(void *ctx) {
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_MAP_END;
	return 0;
}

int sp_test_bool(void *ctx, char val) {
	SP_UNUSED(val);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_BOOL;
	return 0;
}

int sp_test_float(void *ctx, float val) {
	SP_UNUSED(val);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_FLOAT;
	return 0;
}

int sp_test_double(void *ctx, double val) {
	SP_UNUSED(val);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_DOUBLE;
	return 0;
}

int sp_test_ext(void *ctx, int8_t type, char *str, int32_t str_len) {
	SP_UNUSED(type);
	SP_UNUSED(str);
	SP_UNUSED(str_len);
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_FLOAT;
	return 0;
}

int sp_test_query_begin(void *ctx) {
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_QUERY_BEGIN;
	return 0;
}
int sp_test_query_end(void *ctx) {
	struct sp_test_ctx *t = (struct sp_test_ctx *)ctx;
	t->stack[t->depth++] = MPT_QUERY_END;
	return 0;
}

static struct sp_callbacks_t cb_plain = {
	NULL, /* nil */
	NULL, /* uint */
	NULL, /* int*/
	NULL, /* str */
	NULL, /* bin */
	NULL, /* array_begin */
	NULL, /* array_val */
	NULL, /* array_end */
	NULL, /* map_begin*/
	NULL, /* map_key */
	NULL, /* map_val */
	NULL, /* map_end */
	NULL, /* bool */
	NULL, /* float */
	NULL, /* double */
	NULL, /* ext */
	NULL, /* query_begin */
	NULL  /* query_end */
};

#define SET_TEST_CB(to, name) (to)->sp_##name = sp_test_##name
#define UNSET_TEST_CB(to, name) (to)->sp_##name = NULL

#define SET_TEST_CANCEL_CB(HDL, to, name) do {	\
	(to)->sp_##name = sp_test_cancel_##name;\
	sp_options_set((HDL), sp_set_cb, (to));	\
} while (0)

int sp_test_cancel_str(void *ctx, char *val, uint32_t val_len) {
	SP_UNUSED(ctx);
	SP_UNUSED(val);
	SP_UNUSED(val_len);
	return 1;
}

int sp_test_cancel_map_begin(void *ctx) {
	SP_UNUSED(ctx);
	return 1;
}

static int
test_plain(void)
{
	plan(5);
	header();
	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "[%d {%s%d%s%d}]",
			1000, "hello", 1, "hello", 1, "world", 2);
	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	sp_options_set(hdl, sp_set_cb, &cb_plain);
	sp_feed(hdl, data, end);
	enum sp_status retval = 0;
	retval = sp_process(hdl);
	is(ctx.depth, 0, "Simple test");
	is(hdl->state, sp_state_end, "OK state test");
	is(retval, sp_status_ok, "OK status test");

	sp_reset(hdl);
	sp_feed(hdl, NULL, 5);
	retval = sp_process(hdl);
	is(hdl->state, sp_state_error_garbage, "Garbage test");
	is(retval, sp_status_error, "ERR status test");

	sp_free(hdl);

	footer();
	return check_plan();
}

static int
test_cancel(void)
{
	plan(6);
	header();

	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "[%d {%s%d%s%d}]",
			1000, "hello", 1, "hello", 1, "world", 2);
	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	SET_TEST_CANCEL_CB(hdl, &cb_plain, str);

	sp_feed(hdl, data, end);
	enum sp_status retval = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_cancel, "Cancel state test");
	is(retval, sp_status_cancel, "Cancel status test");
	is(hdl->istate->depth, 2, "Cancel depth test");

	UNSET_TEST_CB(&cb_plain, str);
	SET_TEST_CANCEL_CB(hdl, &cb_plain, map_begin);
	sp_reset(hdl);
	retval = sp_process(hdl);

	is(hdl->state, sp_state_cancel, "Cancel map state test");
	is(retval, sp_status_cancel, "Cancel map status test");
	is(hdl->istate->depth, 1, "Cancel map depth test");

	UNSET_TEST_CB(&cb_plain, map_begin);

	sp_free(hdl);

	footer();
	return check_plan();
}

static int
test_intime(void)
{
	plan(28);
	header();

	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "[%d {%s%d%s%d}]",
			1000, "hello", 1, "hello", 1, "world", 2);
	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	SET_TEST_CB(&cb_plain, uint);
	SET_TEST_CB(&cb_plain, str);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_feed(hdl, data, end-1);
	enum sp_status retval = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_error_need_more, "Validate intime failed test"
	   " (state)");
	is(retval, sp_status_error, "Validate intime failed test (status)");
	is(ctx.depth, 4, "Validate intime failed test (depth)");

	sp_reset(hdl);
	ctx.depth = 0;
	sp_feed(hdl, NULL, 1);
	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Validate intime success test (state)");
	is(retval, sp_status_ok, "Validate intime success test (status)");
	is(ctx.depth, 5, "Validate intime success test (depth)");

	sp_reset(hdl);
	sp_feed(hdl, data, 1);
	ctx.depth = 0;

	size_t pos = 1;
	while(pos < end - 1) {
		pos++;
		sp_feed(hdl, NULL, 1);
		retval = sp_process(hdl);
		is(retval, sp_status_error, "+1 test status");
	}

	sp_feed(hdl, NULL, 1);
	retval = sp_process(hdl);
	is(ctx.depth, 5, "final +1 test depth");
	is(retval, sp_status_ok, "final +1 test status");

	UNSET_TEST_CB(&cb_plain, str);
	UNSET_TEST_CB(&cb_plain, uint);

	SET_TEST_CB(&cb_plain, query_begin);
	SET_TEST_CB(&cb_plain, query_end);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_reset(hdl);
	sp_feed(hdl, data, end);
	ctx.depth = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Validate intime query_begin/end test"
	   " (state)");
	is(retval, sp_status_ok, "Validate intime query_begin/end test"
	   " (status)");
	is(ctx.depth, 0, "Validate intime query_begin/end test (depth)");

	UNSET_TEST_CB(&cb_plain, query_begin);
	UNSET_TEST_CB(&cb_plain, query_end);

	sp_free(hdl);

	footer();
	return check_plan();
}

static int
test_before(void)
{
	plan(45);
	header();

	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "[%d {%s%d%s%d}]",
			1000, "hello", 1, "hello", 1, "world", 2);
	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	SET_TEST_CB(&cb_plain, uint);
	SET_TEST_CB(&cb_plain, str);
	sp_options_set(hdl, sp_set_cb, &cb_plain);
	sp_options_set(hdl, sp_validate_before, NULL);

	sp_feed(hdl, data, end-1);
	enum sp_status retval = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_error_need_more, "Validate before failed test"
	   " (state)");
	is(retval, sp_status_error, "Validate before failed test (status)");
	is(ctx.depth, 0, "Validate before failed test (depth)");

	sp_reset(hdl);
	sp_feed(hdl, NULL, 1);
	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Validate before success test (state)");
	is(retval, sp_status_ok, "Validate before success test (status)");
	is(ctx.depth, 5, "Validate before success test (depth)");

	sp_reset(hdl);
	sp_feed(hdl, data, 1);
	ctx.depth = 0;

	size_t pos = 1;
	while(pos < end - 1) {
		pos++;
		sp_feed(hdl, NULL, 1);
		retval = sp_process(hdl);
		is(ctx.depth, 0, "+1 test depth");
		is(retval, sp_status_error, "+1 test status");
	}
	sp_feed(hdl, NULL, 1);
	retval = sp_process(hdl);
	is(ctx.depth, 5, "final +1 test depth");
	is(retval, sp_status_ok, "final +1 test status");

	UNSET_TEST_CB(&cb_plain, str);
	UNSET_TEST_CB(&cb_plain, uint);

	sp_reset(hdl);
	sp_feed(hdl, data, 1);
	ctx.depth = 0;

	SET_TEST_CB(&cb_plain, query_begin);
	SET_TEST_CB(&cb_plain, query_end);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_reset(hdl);
	sp_feed(hdl, data, end);
	ctx.depth = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Validate before query_begin/end test"
	   " (state)");
	is(retval, sp_status_ok, "Validate before query_begin/end test"
	   " (status)");
	is(ctx.depth, 0, "Validate before query_begin/end test (depth)");

	UNSET_TEST_CB(&cb_plain, query_begin);
	UNSET_TEST_CB(&cb_plain, query_end);

	sp_free(hdl);

	footer();
	return check_plan();
}

static int
test_garbage(void)
{
	plan(9);
	header();

	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "[%d {%s%d%s%d}]",
			1000, "hello", 1, "hello", 1, "world", 2);
	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	SET_TEST_CB(&cb_plain, uint);
	SET_TEST_CB(&cb_plain, str);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_feed(hdl, data, end+10);
	enum sp_status retval = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_error_garbage, "Check for garbage without flag"
	   " (state)");
	is(retval, sp_status_error, "Check for garbage without flag (status)");
	is(ctx.depth, 5, "Check for garbage without flag (depth)");

	sp_reset(hdl);
	ctx.depth = 0;

	sp_options_set(hdl, sp_allow_garbage, NULL);

	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Check for garbage with flag (state)");
	is(retval, sp_status_ok, "Check for garbage with flag (status)");
	is(ctx.depth, 5, "Check for garbage with flag (status)");

	UNSET_TEST_CB(&cb_plain, str);
	UNSET_TEST_CB(&cb_plain, uint);

	SET_TEST_CB(&cb_plain, query_begin);
	SET_TEST_CB(&cb_plain, query_end);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_reset(hdl);
	sp_feed(hdl, data, end);
	ctx.depth = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Garbage and query_begin/end test"
	   " (state)");
	is(retval, sp_status_ok, "Garbage and query_begin/end test (status)");
	is(ctx.depth, 0, "Garbage query_begin/end test (depth)");

	UNSET_TEST_CB(&cb_plain, query_begin);
	UNSET_TEST_CB(&cb_plain, query_end);

	sp_free(hdl);

	footer();
	return check_plan();
}

static int
test_multiple(void)
{
	plan(9);
	header();

	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "%d[%d {%s%d%s%d}]{%s%s%d%s}",
			102,
			1000, "hello", 1, "hello", 1,
			"world","hello", 100500, "blabla");
	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	SET_TEST_CB(&cb_plain, uint);
	SET_TEST_CB(&cb_plain, str);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_feed(hdl, data, end);
	enum sp_status retval = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_error_garbage, "Check for multiple without"
	   " flag (state)");
	is(retval, sp_status_error, "Check for multiple without flag"
	   " (status)");
	is(ctx.depth, 1, "Check for multiple without flag (depth)");

	sp_reset(hdl);
	ctx.depth = 0;

	sp_options_set(hdl, sp_allow_multiple, NULL);

	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Check for multiple with flag (state)");
	is(retval, sp_status_ok, "Check for multiple with flag (status)");
	is(ctx.depth, 10, "Check for multiple with flag (status)");

	UNSET_TEST_CB(&cb_plain, str);
	UNSET_TEST_CB(&cb_plain, uint);

	SET_TEST_CB(&cb_plain, query_begin);
	SET_TEST_CB(&cb_plain, query_end);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_reset(hdl);
	sp_feed(hdl, data, end);
	ctx.depth = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Multiple fields and query_begin/end test"
	   " (state)");
	is(retval, sp_status_ok, "Multiple fields and query_begin/end test"
	   " (status)");
	is(ctx.depth, 6, "Multiple fields and query_begin/end test (depth)");

	UNSET_TEST_CB(&cb_plain, query_begin);
	UNSET_TEST_CB(&cb_plain, query_end);

	sp_free(hdl);

	footer();
	return check_plan();
}

static int
test_cb(void)
{
	plan(3);
	header();

	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "[%d%d%f%lf%b{%sNIL}]",
			1, -1, 1.2, 1.4, 1, "hello");
	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	SET_TEST_CB(&cb_plain, nil);
	SET_TEST_CB(&cb_plain, uint);
	SET_TEST_CB(&cb_plain, int);
	SET_TEST_CB(&cb_plain, str);
	SET_TEST_CB(&cb_plain, float);
	SET_TEST_CB(&cb_plain, double);
	SET_TEST_CB(&cb_plain, bool);
	SET_TEST_CB(&cb_plain, array_begin);
	SET_TEST_CB(&cb_plain, array_value);
	SET_TEST_CB(&cb_plain, array_end);
	SET_TEST_CB(&cb_plain, map_begin);
	SET_TEST_CB(&cb_plain, map_key);
	SET_TEST_CB(&cb_plain, map_value);
	SET_TEST_CB(&cb_plain, map_end);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_feed(hdl, data, end);
	enum sp_status retval = 0;
	retval = sp_process(hdl);

	is(hdl->state, sp_state_end, "Check for all available cb (state)");
	is(retval, sp_status_ok, "Check for all available cb (status)");
	is(ctx.depth, 19, "Check for all available cb (depth)");

	UNSET_TEST_CB(&cb_plain, nil);
	UNSET_TEST_CB(&cb_plain, uint);
	UNSET_TEST_CB(&cb_plain, int);
	UNSET_TEST_CB(&cb_plain, str);
	UNSET_TEST_CB(&cb_plain, float);
	UNSET_TEST_CB(&cb_plain, double);
	UNSET_TEST_CB(&cb_plain, bool);
	UNSET_TEST_CB(&cb_plain, array_begin);
	UNSET_TEST_CB(&cb_plain, array_value);
	UNSET_TEST_CB(&cb_plain, array_end);
	UNSET_TEST_CB(&cb_plain, map_begin);
	UNSET_TEST_CB(&cb_plain, map_key);
	UNSET_TEST_CB(&cb_plain, map_value);
	UNSET_TEST_CB(&cb_plain, map_end);

	sp_free(hdl);

	footer();
	return check_plan();
}

static int
test_configure() {
	plan(21);
	header();

	struct sp_handle_t *hdl = sp_alloc();
	ok(hdl != NULL, "Check allocated handler");
	ok(hdl->cb != NULL, "Check allocated callbacks");
	ok(hdl->state == sp_state_start, "Check initial state");
	ok(hdl->istate != NULL, "Check allocated istate");
	ok(hdl->istate->state != NULL, "Check allocated istate's stack");
	ok(hdl->istate->depth == 0, "Check istate's initial depth");
	ok(hdl->istate->max_depth == SP_DEPTH_STEP, "Check istate's initial max_depth");
	ok(hdl->cb_ctx == NULL, "Check initial ctx");
	ok(hdl->opt == 0, "Check initial options");
	is(hdl->buf, NULL, "Check initial buffer pos");
	is(hdl->buf_pos, NULL, "Check initial buffer pos");
	is(hdl->buf_end, NULL, "Check initial buffer pos");
	is(sp_options_set(hdl, sp_allow_garbage, NULL), 0, "Set good value"
	   " (garbage)");
	is(sp_options_set(hdl, sp_allow_multiple, NULL), -1, "Set bad config"
	   " value (multiple)");
	is(sp_options_unset(hdl, sp_allow_garbage), 0, "Unset good value"
	   " (garbage)");
	is(sp_options_set(hdl, sp_allow_multiple, NULL), 0, "Set good value"
	   " (multiple)");
	is(sp_options_set(hdl, sp_allow_garbage, NULL), -1, "Set bad config"
	   " value (garbage)");
	is(sp_options_unset(hdl, sp_allow_multiple), 0, "Unset good value"
	   " (garbage)");
	ok(hdl->opt == 0, "Check options later");
	is(sp_options_set(hdl, sp_set_mem, NULL), 0, "Set memory allocator"
	   " function to NULL");
	is(sp_options_unset(hdl, sp_set_mem), 0, "Set memory allocator"
	   " function to realloc");

	footer();
	return check_plan();
}

static int
test_big() {
	plan(4);
	header();

	// strings: 26
	// map: 6 [kv - 15]
	// array: 1 [v - 2]
	const char msgpack_example1[] = "\x81\xa8\x67\x6c\x6f\x73\x73\x61\x72\x79\x82\xa8"
		"\x47\x6c\x6f\x73\x73\x44\x69\x76\x82\xa9\x47\x6c\x6f\x73\x73\x4c\x69\x73\x74"
		"\x81\xaa\x47\x6c\x6f\x73\x73\x45\x6e\x74\x72\x79\x87\xa8\x47\x6c\x6f\x73\x73"
		"\x44\x65\x66\x82\xac\x47\x6c\x6f\x73\x73\x53\x65\x65\x41\x6c\x73\x6f\x92\xa3"
		"\x47\x4d\x4c\xa3\x58\x4d\x4c\xa4\x70\x61\x72\x61\xda\x00\x48\x41\x20\x6d\x65"
		"\x74\x61\x2d\x6d\x61\x72\x6b\x75\x70\x20\x6c\x61\x6e\x67\x75\x61\x67\x65\x2c"
		"\x20\x75\x73\x65\x64\x20\x74\x6f\x20\x63\x72\x65\x61\x74\x65\x20\x6d\x61\x72"
		"\x6b\x75\x70\x20\x6c\x61\x6e\x67\x75\x61\x67\x65\x73\x20\x73\x75\x63\x68\x20"
		"\x61\x73\x20\x44\x6f\x63\x42\x6f\x6f\x6b\x2e\xa8\x47\x6c\x6f\x73\x73\x53\x65"
		"\x65\xa6\x6d\x61\x72\x6b\x75\x70\xa7\x41\x63\x72\x6f\x6e\x79\x6d\xa4\x53\x47"
		"\x4d\x4c\xa9\x47\x6c\x6f\x73\x73\x54\x65\x72\x6d\xda\x00\x24\x53\x74\x61\x6e"
		"\x64\x61\x72\x64\x20\x47\x65\x6e\x65\x72\x61\x6c\x69\x7a\x65\x64\x20\x4d\x61"
		"\x72\x6b\x75\x70\x20\x4c\x61\x6e\x67\x75\x61\x67\x65\xa6\x41\x62\x62\x72\x65"
		"\x76\xad\x49\x53\x4f\x20\x38\x38\x37\x39\x3a\x31\x39\x38\x36\xa6\x53\x6f\x72"
		"\x74\x41\x73\xa4\x53\x47\x4d\x4c\xa2\x49\x44\xa4\x53\x47\x4d\x4c\xa5\x74\x69"
		"\x74\x6c\x65\xa1\x53\xa5\x74\x69\x74\x6c\x65\xb0\x65\x78\x61\x6d\x70\x6c\x65"
		"\x20\x67\x6c\x6f\x73\x73\x61\x72\x79";
	// int: 18
	// string: 134
	// map: 12 [kv - 81]
	// array: 1 [v - 5]
	const char msgpack_example2[] = "\x81\xa7\x77\x65\x62\x2d\x61\x70\x70\x83\xaf\x73"
		"\x65\x72\x76\x6c\x65\x74\x2d\x6d\x61\x70\x70\x69\x6e\x67\x85\xaa\x63\x6f\x66"
		"\x61\x78\x54\x6f\x6f\x6c\x73\xa8\x2f\x74\x6f\x6f\x6c\x73\x2f\x2a\xa8\x63\x6f"
		"\x66\x61\x78\x43\x44\x53\xa1\x2f\xab\x66\x69\x6c\x65\x53\x65\x72\x76\x6c\x65"
		"\x74\xa9\x2f\x73\x74\x61\x74\x69\x63\x2f\x2a\xaa\x63\x6f\x66\x61\x78\x41\x64"
		"\x6d\x69\x6e\xa8\x2f\x61\x64\x6d\x69\x6e\x2f\x2a\xaa\x63\x6f\x66\x61\x78\x45"
		"\x6d\x61\x69\x6c\xb3\x2f\x63\x6f\x66\x61\x78\x75\x74\x69\x6c\x2f\x61\x65\x6d"
		"\x61\x69\x6c\x2f\x2a\xa6\x74\x61\x67\x6c\x69\x62\x82\xaf\x74\x61\x67\x6c\x69"
		"\x62\x2d\x6c\x6f\x63\x61\x74\x69\x6f\x6e\xb7\x2f\x57\x45\x42\x2d\x49\x4e\x46"
		"\x2f\x74\x6c\x64\x73\x2f\x63\x6f\x66\x61\x78\x2e\x74\x6c\x64\xaa\x74\x61\x67"
		"\x6c\x69\x62\x2d\x75\x72\x69\xa9\x63\x6f\x66\x61\x78\x2e\x74\x6c\x64\xa7\x73"
		"\x65\x72\x76\x6c\x65\x74\x95\x83\xac\x73\x65\x72\x76\x6c\x65\x74\x2d\x6e\x61"
		"\x6d\x65\xa8\x63\x6f\x66\x61\x78\x43\x44\x53\xaa\x69\x6e\x69\x74\x2d\x70\x61"
		"\x72\x61\x6d\xde\x00\x2a\xaf\x63\x61\x63\x68\x65\x50\x61\x67\x65\x73\x53\x74"
		"\x6f\x72\x65\x64\xb8\x73\x65\x61\x72\x63\x68\x45\x6e\x67\x69\x6e\x65\x4c\x69"
		"\x73\x74\x54\x65\x6d\x70\x6c\x61\x74\x65\xb8\x66\x6f\x72\x53\x65\x61\x72\x63"
		"\x68\x45\x6e\x67\x69\x6e\x65\x73\x4c\x69\x73\x74\x2e\x68\x74\x6d\xb9\x63\x6f"
		"\x6e\x66\x69\x67\x47\x6c\x6f\x73\x73\x61\x72\x79\x3a\x61\x64\x6d\x69\x6e\x45"
		"\x6d\x61\x69\x6c\xad\x6b\x73\x6d\x40\x70\x6f\x62\x6f\x78\x2e\x63\x6f\x6d\xac"
		"\x6d\x61\x78\x55\x72\x6c\x4c\x65\x6e\x67\x74\x68\xcd\x01\xf4\xb2\x64\x61\x74"
		"\x61\x53\x74\x6f\x72\x65\x54\x65\x73\x74\x51\x75\x65\x72\x79\xda\x00\x22\x53"
		"\x45\x54\x20\x4e\x4f\x43\x4f\x55\x4e\x54\x20\x4f\x4e\x3b\x73\x65\x6c\x65\x63"
		"\x74\x20\x74\x65\x73\x74\x3d\x27\x74\x65\x73\x74\x27\x3b\xb3\x64\x65\x66\x61"
		"\x75\x6c\x74\x46\x69\x6c\x65\x54\x65\x6d\x70\x6c\x61\x74\x65\xb3\x61\x72\x74"
		"\x69\x63\x6c\x65\x54\x65\x6d\x70\x6c\x61\x74\x65\x2e\x68\x74\x6d\xb0\x64\x61"
		"\x74\x61\x53\x74\x6f\x72\x65\x4c\x6f\x67\x46\x69\x6c\x65\xda\x00\x24\x2f\x75"
		"\x73\x72\x2f\x6c\x6f\x63\x61\x6c\x2f\x74\x6f\x6d\x63\x61\x74\x2f\x6c\x6f\x67"
		"\x73\x2f\x64\x61\x74\x61\x73\x74\x6f\x72\x65\x2e\x6c\x6f\x67\xb3\x74\x65\x6d"
		"\x70\x6c\x61\x74\x65\x4c\x6f\x61\x64\x65\x72\x43\x6c\x61\x73\x73\xbd\x6f\x72"
		"\x67\x2e\x63\x6f\x66\x61\x78\x2e\x46\x69\x6c\x65\x73\x54\x65\x6d\x70\x6c\x61"
		"\x74\x65\x4c\x6f\x61\x64\x65\x72\xae\x64\x61\x74\x61\x53\x74\x6f\x72\x65\x43"
		"\x6c\x61\x73\x73\xb6\x6f\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e\x53\x71\x6c\x44"
		"\x61\x74\x61\x53\x74\x6f\x72\x65\xb0\x72\x65\x64\x69\x72\x65\x63\x74\x69\x6f"
		"\x6e\x43\x6c\x61\x73\x73\xb8\x6f\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e\x53\x71"
		"\x6c\x52\x65\x64\x69\x72\x65\x63\x74\x69\x6f\x6e\xb4\x74\x65\x6d\x70\x6c\x61"
		"\x74\x65\x4f\x76\x65\x72\x72\x69\x64\x65\x50\x61\x74\x68\xa0\xb3\x63\x61\x63"
		"\x68\x65\x54\x65\x6d\x70\x6c\x61\x74\x65\x73\x53\x74\x6f\x72\x65\x32\xac\x64"
		"\x61\x74\x61\x53\x74\x6f\x72\x65\x55\x72\x6c\xda\x00\x3b\x6a\x64\x62\x63\x3a"
		"\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74\x3a\x73\x71\x6c\x73\x65\x72\x76\x65\x72"
		"\x3a\x2f\x2f\x4c\x4f\x43\x41\x4c\x48\x4f\x53\x54\x3a\x31\x34\x33\x33\x3b\x44"
		"\x61\x74\x61\x62\x61\x73\x65\x4e\x61\x6d\x65\x3d\x67\x6f\x6f\x6e\xb8\x73\x65"
		"\x61\x72\x63\x68\x45\x6e\x67\x69\x6e\x65\x46\x69\x6c\x65\x54\x65\x6d\x70\x6c"
		"\x61\x74\x65\xb4\x66\x6f\x72\x53\x65\x61\x72\x63\x68\x45\x6e\x67\x69\x6e\x65"
		"\x73\x2e\x68\x74\x6d\xaf\x63\x61\x63\x68\x65\x50\x61\x67\x65\x73\x54\x72\x61"
		"\x63\x6b\xcc\xc8\xb5\x63\x61\x63\x68\x65\x50\x61\x63\x6b\x61\x67\x65\x54\x61"
		"\x67\x73\x53\x74\x6f\x72\x65\xcc\xc8\xad\x64\x61\x74\x61\x53\x74\x6f\x72\x65"
		"\x4e\x61\x6d\x65\xa5\x63\x6f\x66\x61\x78\xb1\x64\x61\x74\x61\x53\x74\x6f\x72"
		"\x65\x50\x61\x73\x73\x77\x6f\x72\x64\xb2\x64\x61\x74\x61\x53\x74\x6f\x72\x65"
		"\x54\x65\x73\x74\x51\x75\x65\x72\x79\xa6\x75\x73\x65\x4a\x53\x50\xc2\xb3\x64"
		"\x65\x66\x61\x75\x6c\x74\x4c\x69\x73\x74\x54\x65\x6d\x70\x6c\x61\x74\x65\xb0"
		"\x6c\x69\x73\x74\x54\x65\x6d\x70\x6c\x61\x74\x65\x2e\x68\x74\x6d\xb8\x63\x6f"
		"\x6e\x66\x69\x67\x47\x6c\x6f\x73\x73\x61\x72\x79\x3a\x70\x6f\x77\x65\x72\x65"
		"\x64\x42\x79\xa5\x43\x6f\x66\x61\x78\xad\x64\x61\x74\x61\x53\x74\x6f\x72\x65"
		"\x55\x73\x65\x72\xa2\x73\x61\xaf\x6a\x73\x70\x4c\x69\x73\x74\x54\x65\x6d\x70"
		"\x6c\x61\x74\x65\xb0\x6c\x69\x73\x74\x54\x65\x6d\x70\x6c\x61\x74\x65\x2e\x6a"
		"\x73\x70\xaf\x6a\x73\x70\x46\x69\x6c\x65\x54\x65\x6d\x70\x6c\x61\x74\x65\xb3"
		"\x61\x72\x74\x69\x63\x6c\x65\x54\x65\x6d\x70\x6c\x61\x74\x65\x2e\x6a\x73\x70"
		"\xb1\x64\x61\x74\x61\x53\x74\x6f\x72\x65\x4d\x61\x78\x43\x6f\x6e\x6e\x73\x64"
		"\xb3\x63\x61\x63\x68\x65\x50\x61\x67\x65\x73\x44\x69\x72\x74\x79\x52\x65\x61"
		"\x64\x0a\xb1\x63\x61\x63\x68\x65\x50\x61\x67\x65\x73\x52\x65\x66\x72\x65\x73"
		"\x68\x0a\xb3\x63\x61\x63\x68\x65\x54\x65\x6d\x70\x6c\x61\x74\x65\x73\x54\x72"
		"\x61\x63\x6b\x64\xb7\x64\x61\x74\x61\x53\x74\x6f\x72\x65\x43\x6f\x6e\x6e\x55"
		"\x73\x61\x67\x65\x4c\x69\x6d\x69\x74\x64\xbd\x63\x6f\x6e\x66\x69\x67\x47\x6c"
		"\x6f\x73\x73\x61\x72\x79\x3a\x69\x6e\x73\x74\x61\x6c\x6c\x61\x74\x69\x6f\x6e"
		"\x41\x74\xb0\x50\x68\x69\x6c\x61\x64\x65\x6c\x70\x68\x69\x61\x2c\x20\x50\x41"
		"\xb4\x73\x65\x61\x72\x63\x68\x45\x6e\x67\x69\x6e\x65\x52\x6f\x62\x6f\x74\x73"
		"\x44\x62\xb1\x57\x45\x42\x2d\x49\x4e\x46\x2f\x72\x6f\x62\x6f\x74\x73\x2e\x64"
		"\x62\xb6\x74\x65\x6d\x70\x6c\x61\x74\x65\x50\x72\x6f\x63\x65\x73\x73\x6f\x72"
		"\x43\x6c\x61\x73\x73\xb9\x6f\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e\x57\x79\x73"
		"\x69\x77\x79\x67\x54\x65\x6d\x70\x6c\x61\x74\x65\xb7\x63\x61\x63\x68\x65\x50"
		"\x61\x63\x6b\x61\x67\x65\x54\x61\x67\x73\x52\x65\x66\x72\x65\x73\x68\x3c\xb9"
		"\x63\x6f\x6e\x66\x69\x67\x47\x6c\x6f\x73\x73\x61\x72\x79\x3a\x73\x74\x61\x74"
		"\x69\x63\x50\x61\x74\x68\xaf\x2f\x63\x6f\x6e\x74\x65\x6e\x74\x2f\x73\x74\x61"
		"\x74\x69\x63\xac\x74\x65\x6d\x70\x6c\x61\x74\x65\x50\x61\x74\x68\xa9\x74\x65"
		"\x6d\x70\x6c\x61\x74\x65\x73\xac\x75\x73\x65\x44\x61\x74\x61\x53\x74\x6f\x72"
		"\x65\xc3\xb5\x63\x61\x63\x68\x65\x54\x65\x6d\x70\x6c\x61\x74\x65\x73\x52\x65"
		"\x66\x72\x65\x73\x68\x0f\xaf\x64\x61\x74\x61\x53\x74\x6f\x72\x65\x44\x72\x69"
		"\x76\x65\x72\xda\x00\x2c\x63\x6f\x6d\x2e\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74"
		"\x2e\x6a\x64\x62\x63\x2e\x73\x71\x6c\x73\x65\x72\x76\x65\x72\x2e\x53\x51\x4c"
		"\x53\x65\x72\x76\x65\x72\x44\x72\x69\x76\x65\x72\xbc\x63\x6f\x6e\x66\x69\x67"
		"\x47\x6c\x6f\x73\x73\x61\x72\x79\x3a\x70\x6f\x77\x65\x72\x65\x64\x42\x79\x49"
		"\x63\x6f\x6e\xb1\x2f\x69\x6d\x61\x67\x65\x73\x2f\x63\x6f\x66\x61\x78\x2e\x67"
		"\x69\x66\xb5\x63\x61\x63\x68\x65\x50\x61\x63\x6b\x61\x67\x65\x54\x61\x67\x73"
		"\x54\x72\x61\x63\x6b\xcc\xc8\xb1\x64\x61\x74\x61\x53\x74\x6f\x72\x65\x4c\x6f"
		"\x67\x4c\x65\x76\x65\x6c\xa5\x64\x65\x62\x75\x67\xb2\x64\x61\x74\x61\x53\x74"
		"\x6f\x72\x65\x49\x6e\x69\x74\x43\x6f\x6e\x6e\x73\x0a\xad\x73\x65\x72\x76\x6c"
		"\x65\x74\x2d\x63\x6c\x61\x73\x73\xb8\x6f\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e"
		"\x63\x64\x73\x2e\x43\x44\x53\x53\x65\x72\x76\x6c\x65\x74\x83\xac\x73\x65\x72"
		"\x76\x6c\x65\x74\x2d\x6e\x61\x6d\x65\xaa\x63\x6f\x66\x61\x78\x45\x6d\x61\x69"
		"\x6c\xaa\x69\x6e\x69\x74\x2d\x70\x61\x72\x61\x6d\x82\xb0\x6d\x61\x69\x6c\x48"
		"\x6f\x73\x74\x4f\x76\x65\x72\x72\x69\x64\x65\xa5\x6d\x61\x69\x6c\x32\xa8\x6d"
		"\x61\x69\x6c\x48\x6f\x73\x74\xa5\x6d\x61\x69\x6c\x31\xad\x73\x65\x72\x76\x6c"
		"\x65\x74\x2d\x63\x6c\x61\x73\x73\xba\x6f\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e"
		"\x63\x64\x73\x2e\x45\x6d\x61\x69\x6c\x53\x65\x72\x76\x6c\x65\x74\x82\xac\x73"
		"\x65\x72\x76\x6c\x65\x74\x2d\x6e\x61\x6d\x65\xaa\x63\x6f\x66\x61\x78\x41\x64"
		"\x6d\x69\x6e\xad\x73\x65\x72\x76\x6c\x65\x74\x2d\x63\x6c\x61\x73\x73\xba\x6f"
		"\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e\x63\x64\x73\x2e\x41\x64\x6d\x69\x6e\x53"
		"\x65\x72\x76\x6c\x65\x74\x82\xac\x73\x65\x72\x76\x6c\x65\x74\x2d\x6e\x61\x6d"
		"\x65\xab\x66\x69\x6c\x65\x53\x65\x72\x76\x6c\x65\x74\xad\x73\x65\x72\x76\x6c"
		"\x65\x74\x2d\x63\x6c\x61\x73\x73\xb9\x6f\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e"
		"\x63\x64\x73\x2e\x46\x69\x6c\x65\x53\x65\x72\x76\x6c\x65\x74\x83\xac\x73\x65"
		"\x72\x76\x6c\x65\x74\x2d\x6e\x61\x6d\x65\xaa\x63\x6f\x66\x61\x78\x54\x6f\x6f"
		"\x6c\x73\xaa\x69\x6e\x69\x74\x2d\x70\x61\x72\x61\x6d\x8d\xab\x6c\x6f\x67\x4c"
		"\x6f\x63\x61\x74\x69\x6f\x6e\xda\x00\x25\x2f\x75\x73\x72\x2f\x6c\x6f\x63\x61"
		"\x6c\x2f\x74\x6f\x6d\x63\x61\x74\x2f\x6c\x6f\x67\x73\x2f\x43\x6f\x66\x61\x78"
		"\x54\x6f\x6f\x6c\x73\x2e\x6c\x6f\x67\xb2\x66\x69\x6c\x65\x54\x72\x61\x6e\x73"
		"\x66\x65\x72\x46\x6f\x6c\x64\x65\x72\xda\x00\x34\x2f\x75\x73\x72\x2f\x6c\x6f"
		"\x63\x61\x6c\x2f\x74\x6f\x6d\x63\x61\x74\x2f\x77\x65\x62\x61\x70\x70\x73\x2f"
		"\x63\x6f\x6e\x74\x65\x6e\x74\x2f\x66\x69\x6c\x65\x54\x72\x61\x6e\x73\x66\x65"
		"\x72\x46\x6f\x6c\x64\x65\x72\xa3\x6c\x6f\x67\x01\xa7\x64\x61\x74\x61\x4c\x6f"
		"\x67\x01\xaf\x64\x61\x74\x61\x4c\x6f\x67\x4c\x6f\x63\x61\x74\x69\x6f\x6e\xda"
		"\x00\x22\x2f\x75\x73\x72\x2f\x6c\x6f\x63\x61\x6c\x2f\x74\x6f\x6d\x63\x61\x74"
		"\x2f\x6c\x6f\x67\x73\x2f\x64\x61\x74\x61\x4c\x6f\x67\x2e\x6c\x6f\x67\xac\x61"
		"\x64\x6d\x69\x6e\x47\x72\x6f\x75\x70\x49\x44\x04\xad\x6c\x6f\x6f\x6b\x49\x6e"
		"\x43\x6f\x6e\x74\x65\x78\x74\x01\xaf\x72\x65\x6d\x6f\x76\x65\x50\x61\x67\x65"
		"\x43\x61\x63\x68\x65\xda\x00\x25\x2f\x63\x6f\x6e\x74\x65\x6e\x74\x2f\x61\x64"
		"\x6d\x69\x6e\x2f\x72\x65\x6d\x6f\x76\x65\x3f\x63\x61\x63\x68\x65\x3d\x70\x61"
		"\x67\x65\x73\x26\x69\x64\x3d\xb3\x72\x65\x6d\x6f\x76\x65\x54\x65\x6d\x70\x6c"
		"\x61\x74\x65\x43\x61\x63\x68\x65\xda\x00\x29\x2f\x63\x6f\x6e\x74\x65\x6e\x74"
		"\x2f\x61\x64\x6d\x69\x6e\x2f\x72\x65\x6d\x6f\x76\x65\x3f\x63\x61\x63\x68\x65"
		"\x3d\x74\x65\x6d\x70\x6c\x61\x74\x65\x73\x26\x69\x64\x3d\xaa\x6c\x6f\x67\x4d"
		"\x61\x78\x53\x69\x7a\x65\xa0\xae\x64\x61\x74\x61\x4c\x6f\x67\x4d\x61\x78\x53"
		"\x69\x7a\x65\xa0\xaa\x62\x65\x74\x61\x53\x65\x72\x76\x65\x72\xc3\xac\x74\x65"
		"\x6d\x70\x6c\x61\x74\x65\x50\x61\x74\x68\xaf\x74\x6f\x6f\x6c\x73\x74\x65\x6d"
		"\x70\x6c\x61\x74\x65\x73\x2f\xad\x73\x65\x72\x76\x6c\x65\x74\x2d\x63\x6c\x61"
		"\x73\x73\xbf\x6f\x72\x67\x2e\x63\x6f\x66\x61\x78\x2e\x63\x6d\x73\x2e\x43\x6f"
		"\x66\x61\x78\x54\x6f\x6f\x6c\x73\x53\x65\x72\x76\x6c\x65\x74";

	char *ex1 = msgpack_example1;
	size_t msgpack_example1_size = 307;
	char *ex1_end = ex1 + 307;
	is(sizeof(msgpack_example1), msgpack_example1_size, "Check example1"
	   " size");
	is(mp_check(&ex1, ex1_end), 0, "Check example1 correctness");
	ex1 = msgpack_example1;

	char *ex2 = msgpack_example2;
	size_t msgpack_example2_size = 2403;
	char *ex2_end = ex2 + 2403;
	is(sizeof(msgpack_example2), msgpack_example2_size, "Check example2"
	   " size");
	is(mp_check(&ex2, ex2_end), 0, "Check example2 correctness");
	ex2 = msgpack_example2;

	struct sp_handle_t *hdl = sp_alloc();
	struct sp_test_ctx ctx; memset(&ctx, 0, sizeof(struct sp_test_ctx));
	sp_options_set(hdl, sp_set_cb_ctx, &ctx);
	SET_TEST_CB(&cb_plain, uint);
	SET_TEST_CB(&cb_plain, str);
	SET_TEST_CB(&cb_plain, map_begin);
	SET_TEST_CB(&cb_plain, map_key);
	SET_TEST_CB(&cb_plain, map_value);
	SET_TEST_CB(&cb_plain, map_end);
	SET_TEST_CB(&cb_plain, array_begin);
	SET_TEST_CB(&cb_plain, array_value);
	SET_TEST_CB(&cb_plain, array_end);
	sp_options_set(hdl, sp_set_cb, &cb_plain);

	sp_feed(hdl, ex1, msgpack_example1_size);
	enum sp_status retval = 0;
	retval = sp_process(hdl);
	is(ctx.depth, 74, "First big query");
	is(hdl->state, sp_state_end, "OK state test");
	is(retval, sp_status_ok, "OK status test");

	footer();
	return check_plan();
}

int main() {
	plan(8);

	test_configure();
	test_plain();
	test_cancel();
	test_intime();
	test_before();
	test_garbage();
	test_multiple();
	test_cb();
	test_big();

	return check_plan();
}
