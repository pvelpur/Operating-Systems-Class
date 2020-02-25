#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{

  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  sem_t s_so4;
  int so4_count;
  int i;

  if (argc != 4) {
    Printf("Usage: "); Printf(argv[0]); Printf("<handle_to_page_mapped_semaphore> <semaphore> <count>\n");
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  s_so4 = dstrtol(argv[2], NULL, 10);
  so4_count = dstrtol(argv[3], NULL, 10);

  for (i = 0; i < so4_count; i++){
      sem_signal(s_so4);
      Printf("SO4 injected into Radeon atmosphere, PID: %d\n", getpid());

  }

  // Now print a message to show that everything worked
//  Printf("spawn_me: This is one of the %d instances you created.  ", mc->numprocs);
//  Printf("spawn_me: Missile code is: %c\n", mc->really_important_char);
//  Printf("spawn_me: My PID is %d\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  //Printf("PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
