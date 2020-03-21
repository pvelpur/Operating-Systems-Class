#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{

  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t m_s2, m_s;
  char message[2];

  if (argc != 4) {
    Printf("Error Split S2: "); Printf(argv[0]);
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  m_s2 = dstrtol(argv[2], NULL, 10);
  m_s = dstrtol(argv[3], NULL, 10);

  if(mbox_open(m_s2) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Split S2 (%d)", getpid());
    Exit();
  }

  //Receive 1 S2 molecule
  if(mbox_recv(m_s2, 2, (void *)&message) != 2) {
    Printf("Failed to receive to mailbox 1: Split S2 (%d)", getpid());
    Exit();
  }

  if(mbox_close(m_s2) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox: Split S2 (%d)", getpid());
    Exit();
  }

  // Open s mailbox and create 2 S molecules
  if(mbox_open(m_s) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Split S2 (%d)", getpid());
    Exit();
  }
  if(mbox_send(m_s, 1, (void *)"S") != MBOX_SUCCESS){
    Printf("Failed to send to mailbox: Split S2 (%d)", getpid());
    Exit();
  }
  Printf("(%d) Created S Molecule\n", getpid());
  if(mbox_send(m_s, 1, (void *)"S") != MBOX_SUCCESS){
    Printf("Failed to send to mailbox: Split S2 (%d)", getpid());
    Exit();
  }
  Printf("(%d) Created S Molecule\n", getpid());
  if(mbox_close(m_s) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox: Split S2 (%d)", getpid());
    Exit();
  }
  // Signal the semaphore to tell the original process that we're done
  Printf("PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
