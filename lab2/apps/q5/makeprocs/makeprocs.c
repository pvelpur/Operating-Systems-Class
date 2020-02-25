#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

int minVal(int, int, int);

void main (int argc, char *argv[])
{
  int numprocs = 5;               // Used to store number of processes to create
  int n_h2o, n_so4,n_h2, n_o2, n_so2, n_h2so4;
  int r_h2o, r_h2, r_o2, r_so2;

  //int i;                          // Loop index variable

  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  sem_t s_h2o, s_so4, s_h2, s_o2, s_so2;
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char s_h2o_str[10];
  char s_so4_str[10];
  char s_h2_str[10];
  char s_o2_str[10];
  char s_so2_str[10];

  char n_h2o_str[10];
  char n_so4_str[10];
  char n_h2so4_str[10];

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }



  // Convert string from ascii command line argument to integer number
  n_h2o = dstrtol(argv[1], NULL, 10); // user input for number of H2O
  n_so4 = dstrtol(argv[2], NULL, 10);
  Printf("Creating %d H2Os and %d SO4s.\n", n_h2o, n_so4);
  n_h2 = (n_h2o- (n_h2o % 2));
  n_o2 = n_h2/2 + n_so4;
  n_so2 = n_so4;


  n_h2so4 = minVal(n_h2, n_o2, n_so4);
  r_h2o = n_h2o % 2;
  r_h2 = n_h2 - n_h2so4;
  r_o2 = n_o2 - n_h2so4;
  r_so2 = n_so2 - n_h2so4;


  // Create semaphore to not exit this process until all other processes
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((s_h2o = sem_create(0)) == SYNC_FAIL) {
      Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
      Exit();
    }

   if ((s_so4 = sem_create(0)) == SYNC_FAIL) {
       Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
       Exit();
     }
  if ((s_h2 = sem_create(0)) == SYNC_FAIL) {
      Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
      Exit();
    }
   if ((s_o2 = sem_create(0)) == SYNC_FAIL) {
       Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
       Exit();
     }
   if ((s_so2 = sem_create(0)) == SYNC_FAIL) {
       Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
       Exit();
     }


  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
//  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);

  //Set up command-line arguments for molecule count that we are passing over
  ditoa(n_h2o, n_h2o_str);
  ditoa(n_so4, n_so4_str);
  ditoa(n_h2so4, n_h2so4_str);
    //Set up command-line arguments for semaphores that we are passing over
  ditoa(s_h2o, s_h2o_str);
  ditoa(s_so4, s_so4_str);
  ditoa(s_h2, s_h2_str);
  ditoa(s_so2, s_so2_str);
  ditoa(s_o2, s_o2_str);


  //Make lock a string


  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
 /* for(i=0; i<numprocs; i++) {
    process_create(FILENAME_PRODUCER, h_mem_str, s_procs_completed_str, lock_str, NULL); // lock (?)
    process_create(FILENAME_CONSUMER, h_mem_str, s_procs_completed_str, lock_str, NULL); // lock (?)
    Printf("Process %d created\n", i);
  }*/

    process_create(FILENAME_INJH2O, s_procs_completed_str, s_h2o_str, n_h2o_str, NULL);
    process_create(FILENAME_INJSO4, s_procs_completed_str, s_so4_str, n_so4_str, NULL);
    process_create(FILENAME_R1, s_procs_completed_str, s_h2o_str, s_h2_str, s_o2_str, n_h2o_str, NULL);
    process_create(FILENAME_R2, s_procs_completed_str, s_so4_str, s_so2_str, s_o2_str, n_so4_str, NULL);
    process_create(FILENAME_R3, s_procs_completed_str, s_so2_str, s_h2_str, s_o2_str, n_h2so4_str, NULL);
  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("%d H2O's left over. %d H2's left over. %d O2's left over. %d SO2's left over.",r_h2o,r_h2,r_o2,r_so2);
  Printf("%d H2SO4's created.\n", n_h2so4);
  Printf("All other processes completed, exiting main process.\n");
}


int minVal(int a, int b, int c)
{
    if(a < b && a < c){
        return a;
    }
    else if(b< a && b < c){
        return b;
    }
    else {
        return c;
    }
}
