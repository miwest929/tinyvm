#include <tvm/tvm_memory.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h> 

#define NUM_REGISTERS 17

struct tvm_mem *tvm_mem_create(size_t size)
{
	struct tvm_mem *m =
		(struct tvm_mem *)calloc(1, sizeof(struct tvm_mem));

	m->registers = calloc(NUM_REGISTERS, sizeof(union tvm_reg_u));

	m->mem_space_size = size;
	m->mem_space = (int *)calloc(size, 1);

    // initially there no local variables in use
	m->local_vars.count = 0;

	m->objs = NULL;

	return m;
}

void tvm_mem_destroy(struct tvm_mem *m)
{
	free(m->mem_space);
	free(m->registers);
	free(m);

	struct tvm_objs *curr = m->objs;
	while (curr) {
		struct tvm_objs *tmp = curr;
		curr = curr->next;
		free(tmp);
	}
}

int* tvm_mem_allocate(struct tvm_mem *m, size_t size)
{
   int* ref = (int*)calloc(size, sizeof(int));

   if (!ref) {
	   printf("FATAL ERROR: calloc failed to allocate bytes");
	   return NULL;
   }

   tvm_register_obj(m, ref, size);
   return ref;
}

void tvm_register_obj(struct tvm_mem *m, int* ref, size_t size)
{
	struct tvm_objs *obj = (struct tvm_objs*)malloc(sizeof(struct tvm_objs));
	obj->ref = ref;
	obj->count = size;
	obj->reachable = true;
	obj->next = NULL;
	if (!m->objs) {
      m->objs = obj;
	} else {
      struct tvm_objs *curr = m->objs;
	  while (curr->next) { curr = curr->next; }
	  curr->next = obj;
	}
}

union tvm_local_var_value_type tvm_mem_get_local_var_value(struct tvm_mem *m, uint localIndex)
{
  struct tvm_single_local_var var = m->local_vars.values[localIndex];
  if (!var.isInUse) {
    printf("ERROR: Requested to get value for local var #%i but it doesn't exist.", localIndex);
    struct tvm_single_local_var emptyVar = {{.value = 0}, false};
	return emptyVar.variable;
  }

  return var.variable;
}

void tvm_mem_set_local_var_value(struct tvm_mem *m, uint localIndex, union tvm_local_var_value_type value)
{
  struct tvm_single_local_var var = m->local_vars.values[localIndex];
  if (!var.isInUse) {
	  m->local_vars.count += 1;
  }
  m->local_vars.values[localIndex].isInUse = true;
  m->local_vars.values[localIndex].variable = value;
}
