#ifndef DMEMALLOCATOR_H
#define DMEMALLOCATOR_H
#include <string.h>
/** \file
 * \brief debug-mode memory allocator.
 *
 * \license WTFPL v2
 * \mainpage
 *
 *  \section intro What is that
 *
 * This module reimplement the c++ new and delete operator
 * to get predictable garbage valies after new and delete.
 *
 * A list of allocated chunks is also maintained, and assert will
 * fire-up if trying to deallocate invalid (ie non allocated) blocks.
 *
 * Also assert fires in case of invalid cross alloc/free (a c++ new followed by a
 * C dfree() call).
 *
 * Allocation and deallocation are thread-safe (internally protected by mutex).
 *
 * Main ideas are taken from the awesome book "writing solid code", and then enhanced
 * to handle both C and C++ allocs.
 *
 * Defining \c DMEMALLOC_IMPLEMENT allow to only include "dmemalloc.h". DO it only once !
 *
 * \section usage Usage
 * The following flags disable this module:
 *    - \c  NDEBUG (in that case, we are in \c RELEASE mode
 * Flags that Enables the module in any cases:
 *    - \c FORCE_DEBUG_MEMORY
 *
 * \section license License
\verbatim
        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
\endverbatim
 *
 *
 *
 **/
#if defined(DMEMALLOC_IMPLEMENT) && DMEMALLOC_IMPLEMENT
#include "dmemalloc.cpp"
#endif

int plop() ;


#ifdef NDEBUG
/* release mode*/
#else
/* debug mode.
 * Force usage of the module
 */
#define FORCE_DEBUG_MEMORY 1
#endif


/** get the number of allocations performed*/
size_t getNbOfNew();

/** get the number of de-allocations performed*/
size_t getNbOfFree();

#ifdef __cplusplus
extern "C" {
#endif
/** \c malloc() like function. */
void* dmalloc(size_t sz );
/** \c free() like function.*/
void dfree( void* ptr );

/** check is pointer has been allocated.
 * @retval false if not valid
 */
int IsValidPointer(void * pv);

/**
 * @brief get the maximum amount of memory ever allocated.
 * @return the max amount of memory that have been ever allocated.
 *
 * \sa getAllocatedMemory()
 */
size_t getMemoryAllocatedPeak() ;


/** indicates if memory logging is enabled.
@return non-zero if enabled
@retval 0 if disabled
*/
int dMemAllocEnabled();

/**
 * @brief get current allocated amount of memory.
 * @return  return the amount of memory allocated in bytes.
 *
 * \sa getMemoryAllocatedPeak()
 */
size_t getAllocatedMemory();



#ifdef __cplusplus
}
#endif

#endif // DMEMALLOCATOR_H
