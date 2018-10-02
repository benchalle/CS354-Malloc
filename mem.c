////////////////////////////////////////////////////////////////////////////////
// Main File:        mem.c
// This File:        mem.c
// Other Files:      none
// Semester:         CS 354 Spring 2018
//
// Author:           Benjamin Challe
// Email:            bchalle@wisc.edu
// CS Login:         challe
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"

/*
 * This structure serves as the header for each allocated and free block
 * It also serves as the footer for each free block
 * The blocks are ordered in the increasing order of addresses 
 */
typedef struct blk_hdr {                         
        int size_status;
  
    /*
    * Size of the block is always a multiple of 8
    * => last two bits are always zero - can be used to store other information
    *
    * LSB -> Least Significant Bit (Last Bit)
    * SLB -> Second Last Bit 
    * LSB = 0 => free block
    * LSB = 1 => allocated/busy block
    * SLB = 0 => previous block is free
    * SLB = 1 => previous block is allocated/busy
    * 
    * When used as the footer the last two bits should be zero
    */

    /*
    * Examples:
    * 
    * For a busy block with a payload of 20 bytes (i.e. 20 bytes data + an additional 4 bytes for header)
    * Header:
    * If the previous block is allocated, size_status should be set to 27
    * If the previous block is free, size_status should be set to 25
    * 
    * For a free block of size 24 bytes (including 4 bytes for header + 4 bytes for footer)
    * Header:
    * If the previous block is allocated, size_status should be set to 26
    * If the previous block is free, size_status should be set to 24
    * Footer:
    * size_status should be 24
    * 
    */
} blk_hdr;

/* Global variable - This will always point to the first block
 * i.e. the block with the lowest address */
blk_hdr *first_blk = NULL;

/*
 * Note: 
 *  The end of the available memory can be determined using end_mark
 *  The size_status of end_mark has a value of 1
 *
 */

/* 
 * Function for allocating 'size' bytes
 * Returns address of allocated block on success 
 * Returns NULL on failure 
 * Here is what this function should accomplish 
 * - Check for sanity of size - Return NULL when appropriate 
 * - Round up size to a multiple of 8 
 * - Traverse the list of blocks and allocate the best free block which can accommodate the requested size 
 * - Also, when allocating a block - split it into two blocks
 * Tips: Be careful with pointer arithmetic 
 */
void* Mem_Alloc(int size) {   


                   
  if(size <= 0){ // make sure user gives valid size
    return NULL;
  }
  
  size = size +4; //add header
  if(size%8 !=0){ //check to see if you have to add padding
	size = size + 8 - (size%8); //make size the correct size to fit in double word alignment
 }
  blk_hdr* ptr = NULL; //create a current pointer
  blk_hdr* bf = NULL; //create a bestfit pointer
  ptr = first_blk;  //set the pointer to the beginning of the blocks

   
	while(ptr->size_status != 1){ //while not at end
 
   int blocksize = ptr->size_status - (ptr->size_status % 8); //find the actual size of the best fit block
   if(ptr->size_status % 2 == 0){ //if the ptr is pointing to a free block
   
	  	if(blocksize >= size ){ //see if the block can hold the correct size
                   
        if(size == blocksize){ //if size is exact save and exit loop
          bf = ptr;  //set the best fit block
          break; //found the best fitting block so exist the loop
        }
        
        if(bf == NULL){ //if first block tested
          bf = ptr; //set the block to first free block
        }else if(blocksize < bf->size_status){ //if there is a better fitting block
            bf = ptr; //change the bf pointer
			}
    }
	
  }
  	ptr = (blk_hdr*)((void*)ptr + blocksize); //go to the block
  }

if(bf == NULL){ //check to see if you found a best fitting block
  return NULL; //return null if you couldnt find a block with requested size
}
 int bfsize = bf->size_status; //get the size of the best fitting block
 ptr = bf;   //set the pointer to the best fitting block
//spliting the block
  if((bf->size_status - (bf->size_status % 8)) != size){ //see if there is room for another 
    ptr = (blk_hdr*)((void*) ptr + size); //increment pointer to the next blockk
    blk_hdr* newblock = ptr; //create a new block header for the split
   newblock->size_status = (bfsize - bfsize%8 - size); //set the size of the new blcok
    ptr = (blk_hdr*)((void*) ptr + newblock->size_status -4); //get to footer
    ptr->size_status = newblock->size_status;  //set the footer size
    newblock->size_status +=2; //set the status of the new block
    if(bfsize & (1<<1)){ //check to see if the status of the size_sttus of the best block needs to be updated
      bf->size_status = size+3; //update status
      }else {
          bf->size_status = size +1; //update status
      }
    
  }else{
   
    ptr = (blk_hdr*)((void*)ptr + size); // increment to the next block
   if(ptr->size_status != 1){ //if not the end of the block chain
     ptr->size_status += 2; //save new status
   }
    bf->size_status += 1; 
    //adjust the best fitting block to say it is allocated
  }
  bf = (blk_hdr*)((void*) bf +4); //set the return pointer to point at the payload
   // Your code goes in here
    return bf;
}


/* 
 * Function for freeing up a previously allocated block 
 * Argument - ptr: Address of the block to be freed up 
 * Returns 0 on success 
 * Returns -1 on failure 
 * Here is what this function should accomplish 
 * - Return -1 if ptr is NULL
 * - Return -1 if ptr is not 8 byte aligned or if the block is already freed
 * - Mark the block as free 
 * - Coalesce if one or both of the immediate neighbours are free 
 */
int Mem_Free(void *ptr) {     
  if(ptr == NULL){ //if invalid pointer return -1
  return -1;
  } 
  blk_hdr* curr = NULL; //create pointer that points at the current block
  curr = ptr;   //set it equal to incoming pointer
  curr = (blk_hdr*)((void*) curr -4);  //go to header  
   
  
  if(curr->size_status %8 == 0 || curr->size_status %8 == 2){ //if the pointer size is  already free return -1
      return -1;
  }
  blk_hdr *before = first_blk; //create a variable that points to the block before the ptr
  blk_hdr *after = first_blk; //create a variable that points to the block after the ptr
  int before_is_found = 0; //condition if a block before the pointer is found
  int beforecoalesce =1; // remembers the status number of the before block
  int aftercoalesce = 1;  // remembers the status number of the after block
  int after_is_found =0; //condition if a block after the pointer is found
  if(before != ptr){ //if the block isnt the first in the list
    while(before_is_found ==0){ //while the before block isnt found
      int blocksize = before->size_status - (before->size_status % 8); //find the size of the block to traverse
        if((blk_hdr*)((void*)before + blocksize) == curr){ //if the block after before pointer is the ptr block 
          if(before->size_status % 8 ==0){ // if the block is free
          beforecoalesce =0; //save the status
          }else if( before->size_status % 8 ==2){ //if the block is free
          beforecoalesce = 2; //save the status
          }
          before_is_found =1; //exist the loop
        }else if(((blk_hdr*)((void*)before + blocksize))->size_status == 1){ //check to see if the next block would be the end of the chain
          before_is_found = 1; //exit the loop if it is
          }else{
          before = (blk_hdr*)((void*) before + blocksize); //traverse the list of blocks
        }
        
  
    }
  }

  while(after_is_found ==0){ //while the after block isnt found
  int blocksize = after->size_status - (after->size_status % 8); //find the size of the block to traverse
  if(after == curr && ((blk_hdr*)((void*)after + blocksize))->size_status != 1){  // if the after block is the block of ptr and the the block after the ptr isnt the endmark
      after = (blk_hdr*)((void*) after +blocksize); // traverse to the block after the ptr block
          if(after->size_status % 8 ==0){ //if the block is free
          aftercoalesce =0; //save the status
          }else if( after->size_status % 8 == 2){ //if the block is free
          aftercoalesce = 2; //save the status
          }
      after_is_found =1; //exist the loop
  }else if(((blk_hdr*)((void*)after + blocksize))->size_status == 1){ //otherwise if incrementing the the after block pointer is endmark (the ptr block is the last block in the list
      after_is_found = 1; //exist the loop
      
  }else{ 
	after = (blk_hdr*)((void*) after +blocksize); //traverse the list of blocks
	}  
  
  }

  if((beforecoalesce ==0 || beforecoalesce ==2 )&& (aftercoalesce ==0 || aftercoalesce ==2)){ //if both the after and before blocks are free
    //coalesce all 3 blocks
    int ptrsize = (curr->size_status - (curr->size_status % 8)); //get the size of the ptr block
    int beforesize = (before->size_status - (before->size_status % 8)); //get the size of the before block
    int aftersize = (after->size_status - (after->size_status % 8)); //get the size of the after block
    before->size_status = beforesize + ptrsize +  aftersize + beforecoalesce; //set the new size in the before block and add its previous status
    before = (blk_hdr*)((void*) before +beforesize -4); //get to footer
    before->size_status = beforesize + ptrsize + aftersize; // update the footer
  }else if((beforecoalesce ==0 || beforecoalesce ==2 )&& aftercoalesce ==1){ //if there is a free block before ptr but not a free block after
  //coalesce before and pointer
    int ptrsize = (curr->size_status - (curr->size_status % 8)); //get the size of the ptr block
    int beforesize = (before->size_status - (before->size_status % 8)); //get the size of the before block
    int totalsize = beforesize +ptrsize; //get the total size of the block
  //get the size of the before block
    before->size_status = totalsize + beforecoalesce;  //set the new size in the before block and add its previous status
     if(((blk_hdr*)((void*)before + totalsize))->size_status != 1){  //if the next block is at the end of the chain
  after = before; //set before to after
  after = ((blk_hdr*)((void*)after + totalsize)); //increment to desired block
  after->size_status -=2; //update the status
  }
    before = (blk_hdr*)((void*) before +beforesize -4); //go the the footer of the next block
    before->size_status = beforesize + ptrsize; //update the footer size
  }else if(beforecoalesce ==1 && (aftercoalesce ==0 || aftercoalesce ==2)){ //if there is not a free block before but there is a free block after
   //coalesce ptr and after blocks
   int aftersize = (after->size_status - (after->size_status % 8)); //get the size of the after block
   int ptrsize = (curr->size_status - (curr->size_status % 8)); //get actual size of block
   curr->size_status = ptrsize + aftersize + aftercoalesce; //add the block afters size to the ptr size keeping the status the same
   curr = (blk_hdr*)((void*) curr + aftersize -4); //go to footer
    curr->size_status = aftersize + ptrsize; //update the footer
  }else if(beforecoalesce ==1 && aftercoalesce ==1){ //if there are no blocks to coalesce
  //coalsece none just free ptr
  curr->size_status = curr->size_status -1; //update the size status

  int ptrsize = (curr->size_status - (curr->size_status % 8)); // set the status of ptr block to free
  if(((blk_hdr*)((void*)curr + ptrsize))->size_status != 1){ //if the block is at the end of the chain
  after = curr; //set the pointer
  after = ((blk_hdr*)((void*)after + ptrsize)); //increment to next block
  after->size_status -=2; //update the status size
  }
       curr = (blk_hdr*)((void*) curr + ptrsize -4); //go to footer
     curr->size_status = ptrsize + ptrsize; //update footer size
  
 }
    // Your code goes in here 
    return 0;

}

/*
 * Function used to initialize the memory allocator
 * Not intended to be called more than once by a program
 * Argument - sizeOfRegion: Specifies the size of the chunk which needs to be allocated
 * Returns 0 on success and -1 on failure 
 */
int Mem_Init(int sizeOfRegion) {                         
    int pagesize;
    int padsize;
    int fd;
    int alloc_size;
    void* space_ptr;
    blk_hdr* end_mark;
    static int allocated_once = 0;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: Mem_Init has allocated space during a previous call\n");
        return -1;
    }
    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, 
                    fd, 0);
    if (MAP_FAILED == space_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
     allocated_once = 1;

    // for double word alignement and end mark
    alloc_size -= 8;

    // To begin with there is only one big free block
    // initialize heap so that first block meets 
    // double word alignement requirement
    first_blk = (blk_hdr*) space_ptr + 1;
    end_mark = (blk_hdr*)((void*)first_blk + alloc_size);
  
    // Setting up the header
    first_blk->size_status = alloc_size;

    // Marking the previous block as busy
    first_blk->size_status += 2;

    // Setting up the end mark and marking it as busy
    end_mark->size_status = 1;

    // Setting up the footer
    blk_hdr *footer = (blk_hdr*) ((char*)first_blk + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
}

/* 
 * Function to be used for debugging 
 * Prints out a list of all the blocks along with the following information i
 * for each block 
 * No.      : serial number of the block 
 * Status   : free/busy 
 * Prev     : status of previous block free/busy
 * t_Begin  : address of the first byte in the block (this is where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block (as stored in the block header) (including the header/footer)
 */ 
void Mem_Dump() {                        
    int counter;
    char status[5];
    char p_status[5];
    char *t_begin = NULL;
    char *t_end = NULL;
    int t_size;

    blk_hdr *current = first_blk;
    counter = 1;

    int busy_size = 0;
    int free_size = 0;
    int is_busy = -1;

    fprintf(stdout, "************************************Block list***\
                    ********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, "-------------------------------------------------\
                    --------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => busy block
            strcpy(status, "Busy");
            is_busy = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "Free");
            is_busy = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "Busy");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "Free");
        }

        if (is_busy) 
            busy_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blk_hdr*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, "---------------------------------------------------\
                    ------------------------------\n");
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fprintf(stdout, "Total busy size = %d\n", busy_size);
    fprintf(stdout, "Total free size = %d\n", free_size);
    fprintf(stdout, "Total size = %d\n", busy_size + free_size);
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fflush(stdout);

    return;
}
