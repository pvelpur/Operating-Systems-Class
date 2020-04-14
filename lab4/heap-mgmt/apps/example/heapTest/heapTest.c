#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{

  int * arr1;
  int * arr2;
  int * arr3;
  int * arr4;
  int size;
  Printf("Test that the malloc and mfree functions work\n");
  Printf("Test 1: Normal malloc of a size\n");
  arr1 = (int *) malloc(512);

  Printf("Now test that we can keep allocating memory\n");
  arr2 = (int *) malloc(512);
  arr3 = (int *) malloc(128);
  arr4 = (int *) malloc(300);

  Printf("TESTING mFREE on the allocated memory blocks\n");
  size = mfree((void*)arr1);
  size = mfree((void*)arr2);
  size = mfree((void*)arr3);
  size = mfree((void*)arr4);

  Printf("TESTING attempting to malloc a value that is larger that 1 page (4kB) \n");
  arr1 = (int*)malloc(4200);
  arr2 = (int*)malloc(-3);
  if(arr1 == NULL && arr2 == NULL) {
    Printf("Test Passed, could not allocate size of 4200 and -3 \n");
  }

  Printf("HeapTest(%d): Done!\n", getpid());
}
