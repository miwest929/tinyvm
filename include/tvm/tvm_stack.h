#ifndef TVM_STACK_H_
#define TVM_STACK_H_

#define MIN_STACK_SIZE (2 * 1024 * 1024) /* 2 MB */

#include "tvm_memory.h"

/* Initialize our stack by setting the base pointer and stack pointer */

static inline void tvm_stack_create(struct tvm_mem *mem, size_t size)
{
	mem->registers[EBP].i32_ptr =
		((int32_t *)mem->mem_space) + (size / sizeof(int32_t));
	mem->registers[ESP].i32_ptr = mem->registers[EBP].i32_ptr;
}

static inline void tvm_stack_push(struct tvm_mem *mem, int *item)
{
	mem->registers[ESP].i32_ptr -= 1;
	*mem->registers[ESP].i32_ptr = *item;
}

static inline void tvm_stack_pop(struct tvm_mem *mem, int *dest)
{
	*dest = *mem->registers[ESP].i32_ptr;
	mem->registers[ESP].i32_ptr += 1;
}

// Duplicate the top of the stack
static inline void tvm_stack_dup(struct tvm_mem *mem)
{
    int* item = mem->registers[ESP].i32_ptr;
	tvm_stack_push(mem, item);
}

#endif
