#include <tvm/tvm_memory.h>

#include <stdlib.h>
#include <string.h>

#define NUM_REGISTERS 17

struct tvm_mem *tvm_mem_create(size_t size)
{
	struct tvm_mem *m =
		(struct tvm_mem *)calloc(1, sizeof(struct tvm_mem));

	m->registers = calloc(NUM_REGISTERS, sizeof(union tvm_reg_u));

	m->mem_space_size = size;
	m->mem_space = (int *)calloc(size, 1);

    // initially there no local variables in use
	m->local_vars->count = 0;

	m->objs = NULL; // tvm_objs

	return m;
}

void tvm_mem_destroy(struct tvm_mem *m)
{
	free(m->mem_space);
	free(m->registers);
	free(m);
}

void* tvm_mem_allocate(size_t size)
{
   return calloc(size, sizeof(int));
}

union tvm_local_var_value_type tvm_mem_get_local_var_value(struct tvm_mem *m, uint localIndex)
{
  return m->local_vars->values[localIndex];
}

void tvm_mem_set_local_var_value(struct tvm_mem *m, uint localIndex, union tvm_local_var_value_type value)
{
  m->local_vars->values[localIndex] = value;
}
