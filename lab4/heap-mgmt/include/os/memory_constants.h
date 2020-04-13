#ifndef	_memory_constants_h_
#define	_memory_constants_h_

//------------------------------------------------
// #define's that you are given:
//------------------------------------------------

// We can read this address in I/O space to figure out how much memory
// is available on the system.
#define	DLX_MEMSIZE_ADDRESS	0xffff0000

// Return values for success and failure of functions
#define MEM_SUCCESS 1
#define MEM_FAIL -1

//--------------------------------------------------------
// Put your constant definitions related to memory here.
// Be sure to prepend any constant names with "MEM_" so 
// that the grader knows they are defined in this file.

//--------------------------------------------------------
// bit position of least significant bit of the level 1 page number field in a virtual address
#define MEM_L1FIELD_FIRST_BITNUM 12 //because last 3 BYTES correspond to Page offset bits

// Maximum allowable address in the virtual address space. Not 4-byte-aligned address, but rather
// the actual maximum address (it should end with 0xF).
#define MEM_MAX_VIRTUAL_ADDRESS 0xFFFFF

#define MEM_PTE_READONLY 0x4
#define MEM_PTE_DIRTY 0x2
#define MEM_PTE_VALID 0x1

#define MEM_PAGESIZE (0x1 << MEM_L1FIELD_FIRST_BITNUM)
#define MEM_L1TABLE_SIZE ((MEM_MAX_VIRTUAL_ADDRESS + 1) >> MEM_L1FIELD_FIRST_BITNUM)
#define MEM_ADDRESS_OFFSET_MASK (MEM_PAGESIZE - 1)
// use the maximum physical memory size of 2 MB (MEM_MAX_SIZE)
#define MEM_MAX_PHYS_MEM 0x200000
#define MEM_MAX_PAGES (MEM_MAX_PHYS_MEM / MEM_PAGESIZE)
#define MEM_PTE_MASK (~(MEM_PTE_DIRTY | MEM_PTE_VALID | MEM_PTE_READONLY))


#endif	// _memory_constants_h_
