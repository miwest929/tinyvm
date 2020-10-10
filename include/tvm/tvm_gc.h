#ifndef TVM_GC_H_
#define TVM_GC_H_

#include <tvm/tvm_memory.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void tvm_gc_run(struct tvm_mem *m);

struct tvm_objs* find_obj_by_ref(struct tvm_mem *m, int* ref);

#endif
