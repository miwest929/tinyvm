#ifndef TVM_H_
#define TVM_H_

#include <stddef.h>
#include <stdio.h> 
#include <stdlib.h>

#include "tvm_file.h"
#include "tvm_preprocessor.h"
#include "tvm_stack.h"

#include "tvm_memory.h"
#include "tvm_program.h"
#include "tvm_tokens.h"
#include "tvm_gc.h"

struct tvm_ctx {
	struct tvm_prog *prog;
	struct tvm_mem *mem;
};

struct tvm_ctx *tvm_vm_create();
void tvm_vm_destroy(struct tvm_ctx *vm);

int tvm_vm_interpret(struct tvm_ctx *vm, char *filename);
void tvm_vm_run(struct tvm_ctx *vm);

static inline void tvm_step(struct tvm_ctx *vm, int *instr_idx)
{
	int **args = vm->prog->args[*instr_idx];

	switch (vm->prog->instr[*instr_idx]) {
/* nop   */	case 0x0:  break;
/* int   */	case 0x1:  /* unimplemented */ break;
/* mov   */	case 0x2:  *args[0] = *args[1]; break;
/* push  */	case 0x3:  tvm_stack_push(vm->mem, args[0]); break;
/* pop   */	case 0x4:  tvm_stack_pop(vm->mem, args[0]); break;
/* pushf */	case 0x5:  tvm_stack_push(vm->mem, &vm->mem->FLAGS); break;
/* popf  */	case 0x6:  tvm_stack_pop(vm->mem, args[0]); break;
/* inc   */	case 0x7:  ++(*args[0]); break;
/* dec   */	case 0x8:  --(*args[0]); break;
/* add   */	case 0x9:  *args[0] += *args[1]; break;
/* sub   */	case 0xA:  *args[0] -= *args[1]; break;
/* mul   */	case 0xB:  *args[0] *= *args[1]; break;
/* div   */	case 0xC:  *args[0] /= *args[1]; break;
/* mod   */	case 0xD:  vm->mem->remainder = *args[0] % *args[1]; break;
/* rem   */	case 0xE:  *args[0] = vm->mem->remainder; break;
/* not   */	case 0xF:  *args[0] = ~(*args[0]); break;
/* xor   */	case 0x10:  *args[0] ^= *args[1];  break;
/* or    */	case 0x11: *args[0] |= *args[1];   break;
/* and   */	case 0x12: *args[0] &= *args[1];   break;
/* shl   */	case 0x13: *args[0] <<= *args[1];  break;
/* shr   */	case 0x14: *args[0] >>= *args[1];  break;
/* cmp   */	case 0x15: vm->mem->FLAGS =
				((*args[0] == *args[1]) | (*args[0] > *args[1]) << 1);
				break;
/* call	 */	case 0x17: tvm_stack_push(vm->mem, instr_idx);
/* jmp	 */	case 0x16: *instr_idx = *args[0] - 1; break;
/* ret   */	case 0x18: tvm_stack_pop(vm->mem, instr_idx);
				break;
/* je    */	case 0x19:
				*instr_idx = (vm->mem->FLAGS & 0x1)
					? *args[0] - 1 : *instr_idx;
				break;
/* jne   */	case 0x1A:
				*instr_idx = (!(vm->mem->FLAGS & 0x1))
					? *args[0] - 1 : *instr_idx;
				break;
/* jg    */	case 0x1B:
				*instr_idx = (vm->mem->FLAGS & 0x2)
					? *args[0] - 1 : *instr_idx;
				break;
/* jge   */	case 0x1C:
				*instr_idx = (vm->mem->FLAGS & 0x3)
					? *args[0] - 1 : *instr_idx;
				break;
/* jl    */	case 0x1D:
				*instr_idx = (!(vm->mem->FLAGS & 0x3))
					? *args[0] - 1 : *instr_idx;
				break;
/* jle   */	case 0x1E:
				*instr_idx = (!(vm->mem->FLAGS & 0x2))
					? *args[0] - 1 : *instr_idx;
				break;
/* prn   */	case 0x1F: 
                printf("%i\n", *args[0]);
				break;
/* newarray */ case 0x20:
                   // Array size must be stored on the top of stack. It'll be popped.
				   {
					    int arraySize = 0;
						tvm_stack_pop(vm->mem, &arraySize);
						// dynamically allocate an array of that size (type is always INT)
						int* arrayRef = tvm_mem_allocate(vm->mem, arraySize);
						// push reference to the new array on to the stack
						vm->mem->registers[ESP].i32_ptr -= 1;
	                    vm->mem->registers[ESP].i32_ptr = arrayRef;
				   }

				   // the VM itself needs to keep track of the array reference so it can deallocate it later on
				   // Eventually, this will be handled by a Garbage Collector
                   break;
/* astore */    case 0x21:
                    {
                   // store array reference into local variable. Index of local variable is at the top of stack
					    int localVarIndex = 0;
						tvm_stack_pop(vm->mem, &localVarIndex);

						// pop without dereferencing
						int* aref = vm->mem->registers[ESP].i32_ptr;
						vm->mem->registers[ESP].i32_ptr += 1;

                        union tvm_local_var_value_type local_var_value;
						local_var_value.refValue = aref;
						tvm_mem_set_local_var_value(vm->mem, localVarIndex, local_var_value);	
					}

					break;
/* aload */		case 0x22:
                   // load an array reference onto the stack from a local variable whose index is at top of the stack
				   {
						union tvm_local_var_value_type local_var_value = tvm_mem_get_local_var_value(vm->mem, *args[0]);
						tvm_stack_push(vm->mem, local_var_value.refValue);
				   }
                   break;		   
/* iastore */	case 0x23:
                   // store value into specific index of an array
				   // Expected stack: TOP -> [value, index, arrayRef]
				   {
                     int value, index;
					 int* aref;
					 tvm_stack_pop(vm->mem, &value);
					 tvm_stack_pop(vm->mem, &index);

					 // pop without dereferencing
                     aref = vm->mem->registers[ESP].i32_ptr;
					 vm->mem->registers[ESP].i32_ptr += 1;
					 aref[index] = value;	 
				   }
				   break;
/* dup */       case 0x24:
                   // duplicate the top of the stack
				   tvm_stack_dup(vm->mem);
                   break;
/* iaload */	case 0x25:
                   {
					 int index;
					 int* aref;
					 tvm_stack_pop(vm->mem, &index);

					 // pop without dereferencing
                     aref = vm->mem->registers[ESP].i32_ptr;
					 vm->mem->registers[ESP].i32_ptr += 1;
					 tvm_stack_push(vm->mem, &aref[index]);
				   }
				   break;
/* iconst */    case 0x26:
					{
						// push the given argument (const value) onto the stack
						tvm_stack_push(vm->mem, args[0]);
					}
					break;
/* istore */    case 0x27:
					{
						// push the given argument (const value) onto the stack
						int value;
						tvm_stack_pop(vm->mem, &value);
						union tvm_local_var_value_type localValue = {.value = value};
						tvm_mem_set_local_var_value(vm->mem, *args[0], localValue);
					}
					break;
/* iload */     case 0x28:
					{
						int value = GET_VALUE(tvm_mem_get_local_var_value(vm->mem, *args[0]));
						//printf("value to be pushed onto stack = %i", value);
						tvm_stack_push(vm->mem, &value);
					}
					break;
/* icmp   */	case 0x29: 
					{
						int value1, value2;
						tvm_stack_pop(vm->mem, &value1);
						tvm_stack_pop(vm->mem, &value2);
						vm->mem->FLAGS = ((value1 == value2) | (value1 > value2) << 1);
					}
					break;
/* iprn   */	case 0x2A:
                    {
						int value;
						tvm_stack_pop(vm->mem, &value);
						printf("%i\n", value);
					}
					break;
/* iinc   */	case 0x2B:
                    {
						union tvm_local_var_value_type localValue = tvm_mem_get_local_var_value(vm->mem, *args[0]);
						localValue.value += *args[1];
						tvm_mem_set_local_var_value(vm->mem, *args[0], localValue);
					}
					break;
/* arraylength */ case 0x2C:
					{
						int dummyValue = 0;
						tvm_stack_pop(vm->mem, &dummyValue);
						int arrLength = 6;
						tvm_stack_push(vm->mem, &arrLength);
					}
					break;
/* rungc */		case 0x2D:
				    {
                        tvm_gc_run(vm->mem);      
					}
					break;
	};
}

#endif
