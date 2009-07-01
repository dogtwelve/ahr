// used in debug (windows only) to check memory allocations/free, and evaluate memory requirement

#ifdef _DEBUG
#include <stdlib.h>
#include <memory>
#include <assert.h>

typedef struct
{
  void *Addr;
  int U8Size;
  short Tag;
} TMemBlock;

int MemAllocMax;                       // max reached allocated size
int MemAllocCurrent;                   // current allocated size

#define MAX_MEMB 4096
TMemBlock MemBlockList[MAX_MEMB];

void MemAllocInit(void)
{
  memset(MemBlockList, 0, MAX_MEMB*sizeof(TMemBlock));
  MemAllocMax = 0;
  MemAllocCurrent = 0;
}

void *Malloc(int U8Size)
{
  for (int i=0; i<MAX_MEMB; i++)
    if (!MemBlockList[i].Addr)
    {
      MemBlockList[i].Addr = malloc(U8Size);
      assert(MemBlockList[i].Addr);

      MemBlockList[i].U8Size = U8Size;
      MemAllocCurrent += U8Size;

      if (MemAllocCurrent > MemAllocMax)
        MemAllocMax = MemAllocCurrent;

      return MemBlockList[i].Addr;
    }
  assert(0);                           // not enough entry, increase MAX_MEMB
  return NULL;
}

void *Realloc(void *Ptr, int U8Size)
{
  for (int i=0; i<MAX_MEMB; i++)
    if (MemBlockList[i].Addr == Ptr)
    {
      MemAllocCurrent -= MemBlockList[i].U8Size;
      assert(MemAllocCurrent >= 0);
      
      void *NewPtr = realloc(Ptr, U8Size);
      if (!U8Size)
      {
        MemBlockList[i].Addr = NULL;
        MemBlockList[i].U8Size = 0;
        return NewPtr;
      }

      MemBlockList[i].Addr = NewPtr;
      assert(MemBlockList[i].Addr);

      MemBlockList[i].U8Size = U8Size;
      MemAllocCurrent += U8Size;

      if (MemAllocCurrent > MemAllocMax)
        MemAllocMax = MemAllocCurrent;

      return MemBlockList[i].Addr;
    }
  assert(0);                           // entry not found, or increase MAX_MEMB
  return NULL;
}

void Free(void *Ptr)
{
  if (!Ptr)
    return;

  for (int i=0; i<MAX_MEMB; i++)
    if (MemBlockList[i].Addr == Ptr)
    {
      MemAllocCurrent -= MemBlockList[i].U8Size;
      assert(MemAllocCurrent >= 0);
      
      MemBlockList[i].Addr = NULL;
      MemBlockList[i].U8Size = 0;
      return;
    }
  assert(0);                           // entry not found
}


// ----------------------------------------------
// alloc / free  checks

int MemAllocTag;                       // allocated size when TagCurrentAlloc called

// flag current allocated block
void TagCurrentAlloc(void)
{
  MemAllocTag = MemAllocCurrent;

  for (int i=0; i<MAX_MEMB; i++)
    if (MemBlockList[i].Addr)
      MemBlockList[i].Tag = 1;
    else
      MemBlockList[i].Tag = 0;
}

// check that there is not unfreed block since TagCurrentAlloc called
void CheckAllocTag(void)
{
  assert(MemAllocCurrent == MemAllocTag);

  for (int i=0; i<MAX_MEMB; i++)
    if ((MemBlockList[i].Addr) && (!MemBlockList[i].Tag))
    {
      int Size = MemBlockList[i].U8Size;
      assert(0);                       // not freed memory block
    }
}

#endif