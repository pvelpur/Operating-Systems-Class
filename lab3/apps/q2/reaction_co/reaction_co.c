#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{

  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t m_co, m_o2;
  char message[2];

  if (argc != 4) {
    Printf("Error Split CO: "); Printf(argv[0]);
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  m_co = dstrtol(argv[2], NULL, 10);
  m_o2 = dstrtol(argv[3], NULL, 10);

  if(mbox_open(m_co) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Split CO (%d)", getpid());
    Exit();
  }

  //Receive 4 CO molecules
  if(mbox_recv(m_co, 2, (void *)&message) != 2) {
    Printf("Failed to receive to mailbox 1: Split CO (%d)", getpid());
    Exit();
  }
  if(mbox_recv(m_co, 2, (void *)&message) != 2) {
    Printf("Failed to receive to mailbox 2: Split CO (%d)", getpid());
    Exit();
  }
  if(mbox_recv(m_co, 2, (void *)&message) != 2) {
    Printf("Failed to receive to mailbox 3: Split CO (%d)", getpid());
    Exit();
  }
  if(mbox_recv(m_co, 2, (void *)&message) != 2) {
    Printf("Failed to receive to mailbox 4: Split CO (%d)", getpid());
    Exit();
  }

  if(mbox_close(m_co) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox: Split CO (%d)", getpid());
    Exit();
  }

  // Open o2 mailbox and create 2 O2
  if(mbox_open(m_o2) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Split CO (%d)", getpid());
    Exit();
  }
  if(mbox_send(m_o2, 2, (void *)"O2") != MBOX_SUCCESS){
    Printf("Failed to send to mailbox: Split CO (%d)", getpid());
    Exit();
  }
  //Printf("4CO2 -> 2O2 + 2C2 PID: (%d)\n", getpid());
  if(mbox_send(m_o2, 2, (void *)"O2") != MBOX_SUCCESS){
    Printf("Failed to send to mailbox: Split CO (%d)", getpid());
    Exit();
  }
  Printf("4CO2 -> 2O2 + 2C2 PID: (%d)\n", getpid());
  if(mbox_close(m_o2) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox: Split CO (%d)", getpid());
    Exit();
  }
  // Signal the semaphore to tell the original process that we're done
  Printf("PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
