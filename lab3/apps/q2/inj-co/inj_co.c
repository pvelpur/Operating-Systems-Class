#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{

  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t m_co;

  if (argc != 3) {
    Printf("Error inject CO: "); Printf(argv[0]);
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  m_co = dstrtol(argv[2], NULL, 10);

  if(mbox_open(m_co) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Inject CO (%d)", getpid());
    Exit();
  }
  if(mbox_send(m_co, 2, (void *)"CO") != MBOX_SUCCESS) {
    Printf("Failed to send to mailbox: Inject CO (%d)", getpid());
    Exit();
  }
  Printf("Injected CO molecule into atmosphere! PID:(%d) \n", getpid());

  if(mbox_close(m_co) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox: Inject CO (%d)", getpid());
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
