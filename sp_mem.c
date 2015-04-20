#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sp_mem.h"

static void *(*_sp_realloc)(void *ptr, size_t size) =
	(void *(*)(void*, size_t))realloc;

void *sp_mem_init(sp_allocator_t alloc) {
	void *ptr = _sp_realloc;
	if (alloc)
		_sp_realloc = alloc;
	return ptr;
}

void *sp_mem_alloc(size_t size) {
	return _sp_realloc(NULL, size);
}

void *sp_mem_realloc(void *ptr, size_t size) {
	return _sp_realloc(ptr, size);
}

char *sp_mem_dup(char *sz) {
	size_t len = strlen(sz);
	char *szp = sp_mem_alloc(len + 1);
	if (szp == NULL)
		return NULL;
	memcpy(szp, sz, len + 1);
	return szp;
}

void sp_mem_free(void *ptr) {
	_sp_realloc(ptr, 0);
}


