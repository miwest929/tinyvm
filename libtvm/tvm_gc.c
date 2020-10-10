#include <tvm/tvm_gc.h>

void tvm_gc_run(struct tvm_mem *m)
{
    // Initially, mark every obj as unreachable
    struct tvm_objs *curr = m->objs;
    while (curr->next) { curr->reachable = false; curr = curr->next; }
    // loop through every local variable and mark which obj in m->objs is reachable
    for(int i = 0; i < 256; i++) {
       struct tvm_single_local_var currLocal = m->local_vars.values[i];
       if (currLocal.isInUse && currLocal.variable.refValue) {
         struct tvm_objs *found = find_obj_by_ref(m, currLocal.variable.refValue);

         if (!found && currLocal.variable.refValue) {
            printf("ERROR: local var #%i is in use and has ref = %p but was not found in objs\n", i, (void*)currLocal.variable.refValue);
         }

         if (found) {
             printf("ref %p reachable\n", (void*)found->ref);
             found->reachable = true;
         }
       }
    }

    struct tvm_objs *current = m->objs;
    struct tvm_objs *prev = NULL;
    int index = 0;
    printf("garabage collecting all objs\n");
    while (curr) {
        // if current obj is not reachable by any local variable then free it
        if (!current->reachable) {
           printf("INFO: obj #%i is unreachable. It's being dellocated.\n", index);
           if (prev) {
             prev->next = current->next;
           }
           free(current->ref);
        }

        prev = current;
        curr = current->next;
        index += 1;
    }
}

struct tvm_objs* find_obj_by_ref(struct tvm_mem *m, int* ref)
{
    struct tvm_objs *curr = m->objs;
    while (curr) {
        curr->reachable = false;
        if (curr->ref == ref) {
            curr->reachable = true;
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}
