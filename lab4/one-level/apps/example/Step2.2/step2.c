#include "usertraps.h"
#include "misc.h"
#include "os/memory_constants.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int *x;

  if (argc != 2) {
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n");
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  Printf("Step 2.2 (%d): Testing Memory access out of bounds (> MAX vaddr)\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  x = (MEM_MAX_VIRTUAL_ADDRESS + 1);
  Printf("Step2.2 (%d): Attempted to access Memory Location: %d\n", getpid(), x);
  Printf("Step2.2 (%d): Accessing Memory Location: %d\n", getpid(), *x);
  Printf("Step2.2 (%d): Done!\n", getpid());
}
