/** \file
 * \brief tests C interfacing
  */
#include "memchecker.h"

#include <assert.h>
#include <stdint.h>


int test_c_alloc_free() {
    uint32_t * p;
    size_t s1,s2,s3;

    s1 = getAllocatedMemory();
    p = dmalloc(sizeof(uint32_t));
    assert(p);
    s2 = getAllocatedMemory();
    dfree(p);
    s3 = getAllocatedMemory();

    return (s1 < s2) && (s3 < s2);
}
/**
  test correctnes of allocation and free garbage for C
 */
int test_calloc_garbage() {
     uint32_t* p =dmalloc(sizeof(uint32_t));
     assert(p);

     uint32_t alloc_val = *p;
     dfree(p);
     uint32_t free_val = *p;

     return (alloc_val == 0xBEBEBEBE) && (free_val == 0xCACACACA);

}
