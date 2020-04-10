#include "usertraps.h"
#include "misc.h"
#include "os/memory_constants.h"

int recursion(int val){
    if(val == 0){
        return 0;
    }
    return (1 + recursion(val-1));

}

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int val = 3000;
  int x;

  if (argc != 2) {
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n");
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  Printf("Step 2.4 (%d): Testing User stack growing past 1 page\n", getpid());

  x = recursion(val);
  Printf("Step 2.4 (%d): Value after recursion: %d\n", getpid(), x);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Step2.4 (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("Step2.4 (%d): Done!\n", getpid());
}
