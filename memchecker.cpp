/** \file
 * \brief debug-mode memory allocator implementation.
 *
 * \license WTFPL v2
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
*/

//#define FORCE_DISABLE_DEBUG_MEMORY 1

#if !defined(DMEMALLOC_IMPLEMENT)
#include "memchecker.h"
#endif

#include <string.h>

#include <assert.h>
#include <cstdio>
#include <cstdlib>


//#define PRINT_DFREE
#define DANGEROUS_GARBAGE_AFTER_FREE 1

#define nullptr NULL
#define noexcept

#ifdef __cplusplus
extern "C" {
#endif

int plop() {
    return 1;
}

int dMemAllocEnabled(){
#if defined(FORCE_DEBUG_MEMORY)
    return (FORCE_DEBUG_MEMORY!=0);
#else
    return 0;
#endif
}

#ifdef __cplusplus
}
#endif


#if defined(FORCE_DEBUG_MEMORY) && FORCE_DEBUG_MEMORY && !(defined(FORCE_DISABLE_DEBUG_MEMORY) && FORCE_DISABLE_DEBUG_MEMORY)

#define PTHREAD_MUTEX

#if defined(CPP_ELEVEEN_MUTEX)
#include <mutex>
static std::mutex memoryMutex ;

#define LOCK_DMEMORY() std::lock_guard<std::mutex> guard(memoryMutex);
#define UNLOCK_DMEMORY()
#endif

#if defined(PTHREAD_MUTEX)
#include <pthread.h>

pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;


#define LOCK_DMEMORY()   pthread_mutex_lock  ( &memoryMutex );
#define UNLOCK_DMEMORY() pthread_mutex_unlock( &memoryMutex );
#endif

#define fPtrLess(pLeft,pRight) ((pLeft) < pRight) )
#define fPtrGrtr(pLeft,pRight) ((pLeft) > pRight) )
#define fPtrEqual(pLeft,pRight) ((pLeft) == (pRight) )
#define fPtrLessEq(pLeft,pRight) ((pLeft) <=  (pRight) )
#define fPtrGrtrEq(pLeft,pRight) ((pLeft) >=  (pRight) )


static int bGarbagAlloc = 0xBE;
static int bGarbage = 0xCA;
/* current amount of bytes allocated. */
static size_t gsCurrentlyAllocated=0;


/* maximal ever memory usage seen */
static size_t gsMaxAllocatedMemorySeen=0;



/** indicates the context
*/
typedef enum {
    /** C context*/
    PLAIN_C,
    /** C++ context*/
    PLAIN_CXX
} allocation_source_t;

static size_t nbNewCounter = 0;
static size_t nbFreeCounter = 0;

/** memory block log entry. */
typedef struct blocinfo_s {
    /** next entry */
    struct blocinfo_s *pbiNext;
    /** memory block allocated*/
    char * pb;
    /** size of the allocated memory block*/
    size_t size;
    /** context of the memory block */
    allocation_source_t source;
} blockinfo;

/* head of the memory blocks log.*/
static blockinfo *pbiHead=nullptr;






/* return the log entry that contain pb.
\param[in] pb an address that is in one allocated memory block.
\return the corresponding log entry.

\warning If pb dosen't match, it asserts.
         This function is NOT threadsafe.
*/
static blockinfo * pbiGetBlockInfo(void * pb) {
    blockinfo * pbi;

    pbi =pbiHead;

    for (pbi = pbiHead; pbi != nullptr; pbi = pbi->pbiNext) {
        void * pbStart = pbi->pb;
        void * pbEnd = pbi->pb + pbi->size -1;

        if ( fPtrGrtrEq(pb,pbStart) && fPtrLessEq(pb, pbEnd)) {
            break;
        }
    }

    /* make sure that pb has been previously allocated.
     * otherwise it means that you try to find unallocated memory.
     * (double delete of the same memory ?)
     */
    assert(pbi !=nullptr);

    return (pbi);
}

#ifdef __cplusplus
extern "C" {
#endif

int  IsValidPointer(void * pb) {
  LOCK_DMEMORY();
  blockinfo * pbi;
  pbi =pbiHead;

  for (pbi = pbiHead; pbi != nullptr; pbi = pbi->pbiNext) {
    void * pbStart = pbi->pb;
    void * pbEnd = pbi->pb + pbi->size -1;

    if ( fPtrGrtrEq(pb,pbStart) && fPtrLessEq(pb, pbEnd)) {
      break;
    }
  }


  UNLOCK_DMEMORY();

  return pbi!=nullptr;
}


/** create a new log entry.
 * \param[in] pbNew the memory that the log should record. It must be NOT null.
 * \param sizeNew the size of the memory behind pbNew. Must be NOT zero.
 * \param source the allocation context.
 * \warning This function is NOT thread safe.
 *
 */
bool fCreateBlockInfo(void *pbNew, size_t sizeNew,allocation_source_t source){
    blockinfo * pbi;

    assert(pbNew !=nullptr && sizeNew != 0);

    pbi = static_cast<blockinfo *> (malloc(sizeof(blockinfo)));
    if (pbi !=nullptr) {
        pbi->pb = static_cast<char*>(pbNew);
        pbi->size = sizeNew;
        pbi->pbiNext = pbiHead;
        pbi->source = source;
        pbiHead = pbi;
    }
    return (pbi !=nullptr);

}

/** delete the log entry that exactly points to pbToFree.
 * \param[in] pbToFree the memory block pointed by the log-entry to delete.
 * pbToFree MUST be in the log entry. Otherwise, they will be one assert.
 * \warning  This function is NOT thread safe.
 */
void FreeBlockInfo(void *  pbToFree){
    blockinfo * pbi, *pbiPrev;

    pbiPrev = nullptr;

    for (pbi=pbiHead; pbi !=nullptr;pbi = pbi->pbiNext) {
        if (fPtrEqual(pbi->pb,pbToFree)) {
            if (pbiPrev == nullptr) {
                pbiHead = pbi->pbiNext;
            } else {
                pbiPrev->pbiNext = pbi->pbiNext;
            }
            break;
        }
        pbiPrev = pbi;
    }

    assert(pbi !=nullptr);

    memset(pbi,bGarbage,sizeof(blockinfo));
    free(pbi);

}

/** returns the size of the memory allocated to the block containing pb.
 * \param[in] pb a pointer inside a previously allocated memory block.
 *
 * \warning this function is NOT thread safe.
 * \c pb MUST be inside an allocated memory block, otherwise the assert fails.
 */
size_t sizeOfBlock(void * pb){
    blockinfo * pbi;
    pbi = pbiGetBlockInfo(pb);
    assert(pb==pbi->pb);
    return (pbi->size);
}

/**
 * \warning this function is NOT thread safe.
 */
bool fValidPointer(void * pv, size_t size){
    blockinfo * pbi;

    void * pb = static_cast<void * > (pv);

    assert(pv!=nullptr && size != 0);

    pbi = pbiGetBlockInfo(pb);
    assert(fPtrLessEq(static_cast<char*>(pb)+size,pbi->pb+ pbi->size));
    return true;
}




size_t getAllocatedMemory() {
#ifdef NDEBUG
    return 0;
#endif
    size_t retval;
    LOCK_DMEMORY();
    retval = gsCurrentlyAllocated;
    UNLOCK_DMEMORY();
    return retval;
}

#ifdef __cplusplus
}
#endif


void* operator new[]( std::size_t count ) {
    LOCK_DMEMORY();
    void * ptr =  std::malloc(count);
    assert(ptr);
    memset(ptr,bGarbagAlloc,count);
    if (!fCreateBlockInfo(ptr,count,PLAIN_CXX)) {
        std::free(ptr);
        ptr =nullptr;
    }
    ++nbNewCounter;
    gsCurrentlyAllocated+=count;

    UNLOCK_DMEMORY();
    return ptr;
}

/** replacement of new operator
 */
void* operator new(std::size_t sz) {
    LOCK_DMEMORY();
    void * ptr =  std::malloc(sz);
    assert(ptr);
    memset(ptr,bGarbagAlloc,sz);
    if (!fCreateBlockInfo(ptr,sz,PLAIN_CXX)) {
        std::free(ptr);
        ptr =nullptr;
    }
    ++nbNewCounter;
    gsCurrentlyAllocated+=sz;
    if (gsCurrentlyAllocated > gsMaxAllocatedMemorySeen) {
        gsMaxAllocatedMemorySeen = gsCurrentlyAllocated;
    }
#if defined(PRINT_NEW)
    printf("new(%p  %ld);\n",
        ptr,
        sz
    );
#endif

    UNLOCK_DMEMORY();
    return ptr;
}


// same code as for new
void* operator new(std::size_t sz, const std::nothrow_t& nothrow_constant) throw()
{
    LOCK_DMEMORY();
    void * ptr =  std::malloc(sz);
    assert(ptr);
    memset(ptr,bGarbagAlloc,sz);
    if (!fCreateBlockInfo(ptr,sz,PLAIN_CXX)) {
        std::free(ptr);
        ptr =nullptr;
    }
    ++nbNewCounter;
    gsCurrentlyAllocated+=sz;
    if (gsCurrentlyAllocated > gsMaxAllocatedMemorySeen) {
        gsMaxAllocatedMemorySeen = gsCurrentlyAllocated;
    }
#if defined(PRINT_NEW)
    printf("new nonThrow(%p  %ld);\n",
        ptr,
        sz
    );
#endif

    UNLOCK_DMEMORY();
    return ptr;
}



/** replacement of delete operator.
 */
void operator delete(void* ptr) noexcept
{
  if (ptr==nullptr) {
    /* deleting nullptr:
     *  3.7.4.2/3 says:
     *The value of the ﬁrst argument supplied to a deallocation function may be a null pointer value;
     * if so, and if the deallocation function is one supplied in the standard library,
     *  the call has no effect
     **/
    return;
  }

  LOCK_DMEMORY()

    blockinfo * pbi;
    pbi = pbiGetBlockInfo(ptr);
    assert(pbi->source == PLAIN_CXX);

    memset(ptr,bGarbage,pbi->size);
    gsCurrentlyAllocated-=pbi->size;



    FreeBlockInfo(ptr);
    std::free(ptr);
    ++nbFreeCounter;
    UNLOCK_DMEMORY();
}

void operator delete[]( void* ptr ) {
  if (ptr==nullptr) {
    return;
  }

    LOCK_DMEMORY()

    blockinfo * pbi;
    pbi = pbiGetBlockInfo(ptr);
    assert(pbi->source == PLAIN_CXX);

    memset(ptr,bGarbage,pbi->size);
    gsCurrentlyAllocated-=pbi->size;



    FreeBlockInfo(ptr);
    std::free(ptr);
    ++nbFreeCounter;
    UNLOCK_DMEMORY();
}

#ifdef __cplusplus
extern "C" {
#endif

void* dmalloc( size_t sz )
    {
        LOCK_DMEMORY();
        void * ptr =  std::malloc(sz);
        assert(ptr);
        memset(ptr,bGarbagAlloc,sz);
        if (!fCreateBlockInfo(ptr,sz,PLAIN_C)) {
            std::free(ptr);
            ptr =nullptr;
        }
        gsCurrentlyAllocated+=sz;
        if (gsCurrentlyAllocated > gsMaxAllocatedMemorySeen) {
            gsMaxAllocatedMemorySeen = gsCurrentlyAllocated;
        }
        ++nbNewCounter;
        UNLOCK_DMEMORY();
#if defined(PRINT_DALLOC)
        printf("dmalloc(%p  %ld);\n",
            ptr,
            sz
        );
#endif
        return ptr;
    }



void dfree( void* ptr ){

  if (ptr==nullptr) {
    /* free against NULL ptr is valid and does nothing. */
    return;
  }

    LOCK_DMEMORY();

    blockinfo * pbi;
    pbi = pbiGetBlockInfo(ptr);

    assert (pbi->source == PLAIN_C);
    memset(ptr,bGarbage,pbi->size);
    gsCurrentlyAllocated-=pbi->size;

#if defined(PRINT_DFREE)
    printf("dfree(%p  %ld) %x;\n",
        ptr,
        pbi->size,
           *(u_int32_t*) ptr
    );

#endif

    std::free(ptr);
#if defined(DANGEROUS_GARBAGE_AFTER_FREE) && DANGEROUS_GARBAGE_AFTER_FREE
   memset(ptr,bGarbage,pbi->size);
#endif

    ++nbFreeCounter;
    UNLOCK_DMEMORY();
}




size_t getNbOfNew() {
    size_t retval;
    LOCK_DMEMORY();
    retval =  nbNewCounter;
    UNLOCK_DMEMORY();
    return retval;
}



size_t getMemoryAllocatedPeak() {
    size_t retval;
    LOCK_DMEMORY();
    retval =  gsMaxAllocatedMemorySeen;
    UNLOCK_DMEMORY();
    return retval;
}



size_t getNbOfFree() {
    size_t retval;
    LOCK_DMEMORY();
    retval =  nbFreeCounter;
    UNLOCK_DMEMORY();
    return retval;
}

#ifdef __cplusplus
}
#endif

#else

/* release mode. */
size_t getNbOfNew() {
    return 0;
}
size_t getNbOfFree() {
    return 0;
}

extern "C" {
void* dmalloc( size_t sz )
{
    return malloc(sz);
}

void dfree( void* ptr ){
    free(ptr);
}

size_t getMemoryAllocatedPeak() {
    return 0;
}

}


#endif
