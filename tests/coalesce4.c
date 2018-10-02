/* check for coalesce free space */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main() {
   assert(Mem_Init(4096) == 0);
   void * ptr[7];
Mem_Dump();
   ptr[0] = Mem_Alloc(500);
Mem_Dump();
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(500);
   assert(ptr[1] != NULL);
Mem_Dump();
   ptr[2] = Mem_Alloc(500);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(500);
   assert(ptr[3] != NULL);
Mem_Dump();
   ptr[4] = Mem_Alloc(500);
   assert(ptr[4] != NULL);
Mem_Dump();
   ptr[5] = Mem_Alloc(500);
   assert(ptr[5] != NULL);
Mem_Dump();
   ptr[6] = Mem_Alloc(500);
   assert(ptr[6] != NULL);
Mem_Dump();
   while (Mem_Alloc(500) != NULL)
     ;
Mem_Dump();
   assert(Mem_Free(ptr[1]) == 0);
   Mem_Dump();
   assert(Mem_Free(ptr[5]) == 0);
      Mem_Dump();
   assert(Mem_Free(ptr[2]) == 0);
      Mem_Dump();
   assert(Mem_Free(ptr[4]) == 0);
      Mem_Dump();
   assert(Mem_Free(ptr[3]) == 0);
      Mem_Dump();
   ptr[2] = Mem_Alloc(2500);
   assert(ptr[2] != NULL);
      Mem_Dump();
   exit(0);
}
