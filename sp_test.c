#include <msgpuck.h>

#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "sp.h"

int sp_cb_printf_uint(void *ctx, uint64_t val){
	SP_UNUSED(ctx);
	printf("uint val: %zd\n", val);
	return 0;
}
int sp_cb_printf_str(void *ctx, const char *str, uint32_t str_len){
	SP_UNUSED(ctx);
	printf("str val: %.*s\n", str_len, str);
	return 0;
}

int main() {
	char data[1024]; memset(data, 0, sizeof(data));
	size_t end = mp_format(data, 1024, "[%d {%s%d%s%d}]",
			1000, "hello", 1, "hello", 1, "world", 2);
	struct sp_handle_t *hdl = sp_alloc();
	sp_options_set(hdl, sp_validate_before, NULL);
	struct sp_callbacks_t cb = {
		NULL, /* nil */
		sp_cb_printf_uint, /* uint */
		NULL, /* int*/
		sp_cb_printf_str, /* str */
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
	sp_options_set(hdl, sp_set_cb, &cb);
	sp_feed(hdl, data, 1);
	while (sp_process(hdl) == sp_status_error) {
		sp_feed(hdl, NULL, 1);
	}
	sp_reset(hdl);
	size_t sz = end / 2;
	sp_feed(hdl, data, sz);
	assert(sp_process(hdl) == sp_status_error);
	printf("error\n");
	sp_feed(hdl, NULL, end - sz);
	assert(sp_process(hdl) == sp_status_ok);
	sp_free(hdl);
}
