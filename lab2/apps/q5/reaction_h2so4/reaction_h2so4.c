#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{

  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  sem_t s_so2;
  sem_t s_h2;
  sem_t s_o2;

  int h2so4_count;
  int i;

  if (argc != 6) {
    Printf("Usage: "); Printf(argv[0]); Printf(" Error in reaction h2o\n");
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  s_so2 = dstrtol(argv[2], NULL, 10);
  s_h2 = dstrtol(argv[3], NULL, 10);
  s_o2 = dstrtol(argv[4], NULL, 10);
  h2so4_count = dstrtol(argv[5], NULL, 10);

  for (i = 0; i < h2so4_count; i++){
      if((sem_wait(s_h2)) != SYNC_SUCCESS ) {
        Printf("H2O sem wait fail, pid: %d\n", getpid());
      }
      if((sem_wait(s_o2)) != SYNC_SUCCESS ) {
        Printf("H2O sem wait fail, pid: %d\n", getpid());
      }
      if((sem_wait(s_so2)) != SYNC_SUCCESS ) {
        Printf("H2 sem signal fail, pid: %d\n", getpid());
      }



      Printf("(%d) H2 + O2 + SO2 -> H2SO4 reacted, PID: %d\n",(i+1), getpid());

  }

  // Signal the semaphore to tell the original process that we're done
  //Printf("PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
