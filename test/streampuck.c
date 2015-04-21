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

	is(hdl->state, sp_state_error_need_more, "Validate intime failed test (state)");
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

	is(hdl->state, sp_state_end, "Validate intime query_begin/end test (state)");
	is(retval, sp_status_ok, "Validate intime query_begin/end test (status)");
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

	is(hdl->state, sp_state_error_need_more, "Validate before failed test (state)");
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

	is(hdl->state, sp_state_end, "Validate before query_begin/end test (state)");
	is(retval, sp_status_ok, "Validate before query_begin/end test (status)");
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

	is(hdl->state, sp_state_error_garbage, "Check for garbage without flag (state)");
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

	is(hdl->state, sp_state_end, "Garbage and query_begin/end test (state)");
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

	is(hdl->state, sp_state_error_garbage, "Check for multiple without flag (state)");
	is(retval, sp_status_error, "Check for multiple without flag (status)");
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

	is(hdl->state, sp_state_end, "Multiple fields and query_begin/end test (state)");
	is(retval, sp_status_ok, "Multiple fields and query_begin/end test (status)");
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
	is(sp_options_set(hdl, sp_allow_garbage, NULL), 0, "Set good value (garb)");
	is(sp_options_set(hdl, sp_allow_multiple, NULL), -1, "Set bad config value (multiple)");
	is(sp_options_unset(hdl, sp_allow_garbage), 0, "Unset good value (garb)");
	is(sp_options_set(hdl, sp_allow_multiple, NULL), 0, "Set good value (multiple)");
	is(sp_options_set(hdl, sp_allow_garbage, NULL), -1, "Set bad config value (garb)");
	is(sp_options_unset(hdl, sp_allow_multiple), 0, "Unset good value (garb)");
	ok(hdl->opt == 0, "Check options later");
	is(sp_options_set(hdl, sp_set_mem, NULL), 0, "Set memory allocator function to NULL");
	is(sp_options_unset(hdl, sp_set_mem), 0, "Set memory allocator function to realloc");

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

	return check_plan();
}
