#ifndef __SP_ALLOC_H__
#define __SP_ALLOC_H__

typedef void *(sp_allocator_t)(void *ptr, size_t size);

void *sp_mem_init(sp_allocator_t alloc);
void *sp_mem_alloc(size_t size);
void *sp_mem_realloc(void *ptr, size_t size);
char *sp_mem_dup(char *sz);
void sp_mem_free(void *ptr);


#define SP_MALLOC(sz)       sp_mem_alloc(sz)
#define SP_FREE(ptr)        sp_mem_free(ptr)
#define SP_REALLOC(ptr, sz) sp_mem_realloc(ptr, sz)

#endif /* __SP_ALLOC_H__ */
