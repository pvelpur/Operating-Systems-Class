//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
// num_pages = 2MB / 4KB = 512, 512 / 32 = 16 because each int represents 32 pages
static uint32 freemap[16];
static uint32 pagestart;
static int nfreepages;
static int freemapmax;

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
    int i;
    int curpage;
    //int maxpage = MemoryGetSize() / MEM_PAGESIZE;
    //MEM_MAX_PAGES (?)
    dbprintf ('m', "MemoryModuleInit, Start\n");


    // pagestart = first page since last os page
    pagestart = (lastosaddress + MEM_PAGESIZE - 4) / MEM_PAGESIZE;

    //freemapmax = max index for freemap array
    freemapmax = (MEM_MAX_PAGES + 31) / 32;

    dbprintf ('m', "Map has %d entries, memory size is 0x%x.\n", freemapmax, MEM_MAX_PAGES);

    //nfreepages = how many free pages available in the system not including os pages
    nfreepages = 0; // CHANGE THIS (?)

    // set every entry in freemap to 0
    for(i = 0; i < freemapmax; i++) {
        freemap[i] = 0;
    }

   // for all free pages
   //       MemorySetFreemap()
    for(curpage = pagestart; curpage < MEM_MAX_PAGES; curpage++){
        nfreepages+=1;
        MemorySetFreemap(curpage, 1);
    }
    dbprintf('m', "Initialized %d free pages.\n", nfreepages);
}

//MemorySetFreemap(int pagenum, int b)
void MemorySetFreemap(int pagenum, int b){
    uint32 index = pagenum / 32;
    uint32 position = pagenum % 32;
    freemap[index] = (freemap[index] & invert(1 << position)) | (b << position);
    dbprintf ('m', "MemorySetFreemap, Set freemap entry %d to 0x%x.\n", index, freemap[index]);
}

//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
    int pagenum = addr / MEM_PAGESIZE;
    int offset = addr % MEM_PAGESIZE;

    dbprintf ('m', "Enter MemoryTranslateUserToSystem\n");

    //Check given address is less than the max vaddress
    if(addr >= MEM_MAX_VIRTUAL_ADDRESS) {
        return MEM_FAIL;
    }

    // Get page table entry in page table and check that it is valid
    if((pcb->pagetable[pagenum] & MEM_PTE_VALID) == 0){
        //Save address to the currentSavedFrame
        pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
        MemoryPageFaultHandler(pcb);
    }

    return (pcb->pagetable[pagenum] & MEM_PTE_MASK) + offset;
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference.
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
    int newallocpage;
    int fault_addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
    int user_stack_address = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER];
    int faultPage = fault_addr / MEM_PAGESIZE;
    int userPage = user_stack_address / MEM_PAGESIZE;

    dbprintf ('m', "MemoryPageFaultHandler, Start\n");


    if(faultPage < userPage) {
        printf("Segfault\n");
        ProcessKill();
        return MEM_FAIL;
    }
    else {
        newallocpage = MemoryAllocPage(); //returns allocated page number
        pcb->pagetable[faultPage] = MemorySetupPte(newallocpage); //Returns PTE and stores
        pcb->npages += 1;
    }

    return MEM_SUCCESS;
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) {
    int	mapnum = 0;
    uint32		bitnum;
    uint32	v;

    if (nfreepages == 0) {
      dbprintf('m', "MemoryAllocPage: no available pages\n");
      return MEM_FAIL;
    }
    dbprintf ('m', "Allocating memory, starting with page %d\n", mapnum);
    while (freemap[mapnum] == 0) {
      mapnum += 1;
      if (mapnum >= freemapmax) {
        mapnum = 0;
      }
    }
    v = freemap[mapnum];
    for (bitnum = 0; (v & (1 << bitnum)) == 0; bitnum++) {
    }
    freemap[mapnum] &= invert(1 << bitnum);
    v = (mapnum * 32) + bitnum;
    dbprintf ('m', "Allocated memory, from map %d, page %d, map=0x%x.\n",
  	    v, mapnum, freemap[mapnum]);
    nfreepages -= 1;
    return v;
}


uint32 MemorySetupPte (uint32 page) {
  dbprintf('m', "Enter MemorySetupPte\n");
  return ((page * MEM_PAGESIZE) | MEM_PTE_VALID);
}


void MemoryFreePage(uint32 page)
{
  MemorySetFreemap (page, 1);
  nfreepages += 1;
  dbprintf ('m',"Freed page 0x%x, %d remaining.\n", page, nfreepages);
}

void MemoryFreePte(uint32 pte) {
    MemoryFreePage ((pte & MEM_PTE_MASK) / MEM_PAGESIZE);
}

//Functions to compile the OS
void *malloc(PCB *pcb, int memsize) {
    int searchRes;
    int splitRes;
    uint32 virtualAddr, physAddr;

    dbprintf('m', "Start Memory Malloc for size %d\n", memsize);

    if ((memsize <= 0 || memsize > MEM_PAGESIZE))
    {
        return NULL;
    }

    // Next, find suitable memory slot
    searchRes = MemoryFindSlot(&pcb->heapArr[0], memsize);
    if(searchRes != MEM_FAIL){
        //we found a match so return virtual address
        virtualAddr = searchRes + pcb->heapArea;
        physAddr = MemoryTranslateUserToSystem(pcb, virtualAddr);
        printf("Created a heap block of size %d bytes: virtual address 0x%x, physical address 0x%x \n", memsize, virtualAddr, physAddr);
        return (void *)(pcb->heapArea + searchRes);
    }
    else {
        // need to split the memory and continue the search
        splitRes = MemorySplitHeap(&(pcb->heapArr[0]), pcb, memsize);
        if(splitRes != MEM_FAIL){
            return (void *)(pcb->heapArea + splitRes);
        }
        else {
            printf("Could not find a block of memory of necessary size to allocate\n");
        }
    }

    return NULL;
}

int MemoryFindSlot(Node * node, int memsize) {
    int block; // Value for the min block size needed to allocate
    int val;

    block = 32; // min block that can be allocted (order 0) is 32
    while(block < memsize){
        block = block * 2;
    }

    if(node == NULL){ return MEM_FAIL;}
    // if it's a leaf node (no left or right node, and inuse is 0, then we can return it
    if((node->left == NULL) && (node->right == NULL) && (node->inuse ==0))
    {
        //If we find a match
        if(node->Blocksize == block){
            node->inuse = 1;
            printf("Allocated the block: order= %d, addr=%d, requested mem size = %d, block size = %d\n", node->order, node->offset, memsize, block);
            return node->offset;
        }
        else{
            return MEM_FAIL;
        }
    }

    // If leaf node is not found, need to recursively traverse until we can find something
    val = MemoryFindSlot(node -> left, memsize);
    if( val != MEM_FAIL){
        return val;
    }
    else{
        return MemoryFindSlot(node -> right, memsize);
    }

}

int MemorySplitHeap(Node * node, PCB * pcb, int memsize) {
    int val;
    int slot;
    //int slotFound;
    Node* left;
    Node* right;

    if(node == NULL){ return MEM_FAIL;}
    if((node->left == NULL) && (node->right == NULL) && (node->inuse ==0))
    {
        if(memsize <= (node->Blocksize / 2)) {
            if(node->order == 0){
                return MEM_FAIL;
            }
            else{
                //Allocate the left child
                left = &(pcb->heapArr[(2*node->index) + 1]);
                left->parent = node;
                //left and right should already be null from the initialization
                left->Blocksize = node->Blocksize / 2;
                left->order = node->order - 1;
                left->offset = node->offset;
                printf("Created a left child node (order = %d, addr=%d, size = %d) of parent (order = %d, addr=%d, size = %d)\n", left->order, left->offset, left->Blocksize, node->order, node->offset, node->Blocksize);
                //Allocate the right child
                right = &(pcb->heapArr[(2*node->index) + 2]);
                right->parent = node;
                right->Blocksize = node->Blocksize / 2;
                right->order = node->order - 1;
                right->offset = node-> offset + right->Blocksize;
                printf("Created a right child node (order = %d, addr=%d, size = %d) of parent (order = %d, addr=%d, size = %d)\n", right->order, right->offset, right->Blocksize, node->order,  node->offset, node->Blocksize);

                //Set the left and right child of the parent
                node->left = left;
                node->right = right;
            }
        }
    }

    slot = MemoryFindSlot(&(pcb->heapArr[0]), memsize);
    if(slot != MEM_FAIL){
        //printf("Memory Split heap complete and memory slot found");
        return slot;
    }
    else {
        //printf("nodeLeftSize: %d\n", (node->left)->Blocksize);
        val = MemorySplitHeap(node->left, pcb, memsize);
        if(val != MEM_FAIL) {
            return val;
        }
        else {
            return MemorySplitHeap(node->right, pcb, memsize);
        }
    }
}

int mfree(PCB* pcb, void* ptr){
    Node * node;
    int addr_offset;
    int i, size;

    //printf("FREEING MEMORY!\n");

    if(ptr == NULL) {
        return MEM_FAIL;
    }
    //check if ptr is outside of the heap pages (?)
    if(((int)ptr >= (5*MEM_PAGESIZE)) || ((int)ptr < (4*MEM_PAGESIZE))) {
        return MEM_FAIL;
    }

    // Get the offset bits from the address
    addr_offset = ((int)ptr & (MEM_ADDRESS_OFFSET_MASK));
    for(i= MEM_NUM_NODES-1 ; i >= 0; i--){
        if(pcb->heapArr[i].offset == addr_offset){
            node = &(pcb->heapArr[i]);
            size = pcb->heapArr[i].Blocksize;
            break;
        }
    }
    //printf("\n");
    //printf("O: %d, A: %d, Size: %d\n", node->order, node->offset, node->Blocksize);
    //printf("\n");
    MemoryCoalescing(node);
    printf("Freed the block: order = %d, addr = %d, size = %d\n", node->order, node->offset, size);
    return size;
}

void MemoryCoalescing(Node * node){
    Node * parent;

    if(node == NULL) {return;}

    node->inuse = 0;
    node->left = NULL;
    node->right = NULL;

    //printf("MemoryCoalescing function START!\n");

    // We have reached the root node
    if(node->parent == NULL) {
        return;
    }
    else{
        //We are at a leaf node
        //check if we are the left node or the right node
        parent = node->parent;
        if(parent->left == node){
            // Check if the right node is in use or not and that its a leaf node
            if(parent->right->inuse == 0 && (parent->right->left == NULL && parent->right->right == NULL)) {
                printf("Coalesced buddy nodes (order = %d, addr = %d, size = %d) & (order = %d, addr = %d, size = %d)\n", node->order , node->offset, node->Blocksize, parent->right->order, parent->right->offset, parent->right->Blocksize);
                printf("into the parent node (order = %d, addr = %d, size=%d)\n", parent->order, parent->offset, parent->Blocksize);
                MemoryCoalescing(parent);
            }
        }
        else {
            // Check if the right node is in use or not
            if(parent->left->inuse == 0 && (parent->left->left == NULL && parent->left->right == NULL)) {
                printf("Coalesced buddy nodes (order = %d, addr = %d, size = %d) & (order = %d, addr = %d, size = %d)\n", node->order , node->offset, node->Blocksize, parent->left->order, parent->left->offset, parent->left->Blocksize);
                printf("into the parent node (order = %d, addr = %d, size=%d)\n", parent->order, parent->offset, parent->Blocksize);
                MemoryCoalescing(parent);
            }
        }

    }

}

