#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{

  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t m_s2;

  if (argc != 3) {
    Printf("Error inject S2: "); Printf(argv[0]);
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  m_s2 = dstrtol(argv[2], NULL, 10);

  if(mbox_open(m_s2) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Inject S2 (%d)", getpid());
    Exit();
  }
  if(mbox_send(m_s2, 2, (void *)"S2") != MBOX_SUCCESS) {
    Printf("Failed to send to mailbox: Inject S2 (%d)", getpid());
    Exit();
  }
  Printf("(%d) Created a S2 molecule\n", getpid());

  if(mbox_close(m_s2) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox: Inject S2 (%d)", getpid());
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
