#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define PART1 "step1.dlx.obj"
#define PART2 "step2.dlx.obj"
#define PART3 "step3.dlx.obj"
#define PART4 "step4.dlx.obj"
#define PART5 "step5.dlx.obj"
#define PART6 "step6.dlx.obj"

void main (int argc, char *argv[])
{
  int which_step = 0;
  int num_procs = 0;
  int num_hello_world = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 2) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  //num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  //Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);

  which_step = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  switch(which_step) {
      case(0): num_procs = 1; break;
      case(1): num_procs = 1; break;
      case(2): num_procs = 1; break;
      case(3): num_procs = 1; break;
      case(4): num_procs = 1; break;
      case(5): num_procs = 1; break;
      case(6): num_procs = 30; break;
  }

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(-(num_procs-1))) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  if(which_step == 0){
    // Create Hello World processes
    num_hello_world = 5;
    Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
    for(i=0; i<num_hello_world; i++) {
      Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
      process_create(HELLO_WORLD, s_procs_completed_str, NULL);
      if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
      }
    }
  }
  if(which_step == 1) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating Hello World\n", getpid());
    process_create(PART1, s_procs_completed_str, NULL);
  }

  if(which_step == 2) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating Test for accessing memory beyond Max VAddr\n", getpid());
    process_create(PART2, s_procs_completed_str, NULL);
  }

  if(which_step == 3) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating Test for accessing Valid memory address but outside allocated pages\n", getpid());
    process_create(PART3, s_procs_completed_str, NULL);
  }

  if(which_step == 4) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating Test to cause user function call stack to grow larger than one page\n", getpid());
    process_create(PART4, s_procs_completed_str, NULL);
  }

  if(which_step == 5) {
    Printf("-------------------------------------------------------------------------------------\n");
    for(i=0; i<100; i++) {
      Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
      process_create(PART5, s_procs_completed_str, NULL);
      if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
      }
    }
  }

  if(which_step == 6) {
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating 30 Processes simultaneously as they loop through a large number \n", getpid());
    for(i = 0; i < 30; i++){
        process_create(PART6, s_procs_completed_str, NULL);
     }

  }

  if (!which_step==0 && !which_step==5 && (sem_wait(s_procs_completed) != SYNC_SUCCESS)) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }


  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
