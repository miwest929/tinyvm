#ifndef TVM_MEMORY_H_
#define TVM_MEMORY_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MIN_MEMORY_SIZE (64 * 1024 * 1024) /* 64 MB */

#define EBP 0x7
#define ESP 0x6

#define GET_VALUE(v) (v.value)
#define GET_PTR_REF(v) (v.refValue)

typedef unsigned int uint;

union tvm_reg_u {
	int32_t i32;
	int32_t *i32_ptr;

	union {
		int16_t h;
		int16_t l;
	} i16;

};

union tvm_local_var_value_type {
	int value;
	int* refValue;
};


struct tvm_single_local_var {
    union tvm_local_var_value_type variable;
	bool isInUse;
};

struct tvm_objs {
  int* ref;
  uint count;
  bool reachable;
  struct tvm_objs* next;
};

// a indexed map into the local variables
// NOTE: The local var references get destroyed along with the stack frame
struct tvm_local_vars {
	struct tvm_single_local_var values[256];
	int count;
};

struct tvm_mem {
	/*
	 *	Similar to x86 FLAGS register
	 *
	 *	0x1	EQUAL
	 *	0x2	GREATER
	 *
	 */

	int FLAGS;
	int remainder;

	void *mem_space;
	int mem_space_size;

	union tvm_reg_u *registers;

	struct tvm_local_vars local_vars;
	struct tvm_objs *objs; // linked list of allocated objects
};

struct tvm_mem *tvm_mem_create(size_t size);
void tvm_mem_destroy(struct tvm_mem *mem);

union tvm_local_var_value_type tvm_mem_get_local_var_value(struct tvm_mem *m, uint localIndex);
void tvm_mem_set_local_var_value(struct tvm_mem *m, uint localIndex, union tvm_local_var_value_type value);
int* tvm_mem_allocate(struct tvm_mem *m, size_t size);
void tvm_register_obj(struct tvm_mem *m, int* ref, size_t size);

#endif
