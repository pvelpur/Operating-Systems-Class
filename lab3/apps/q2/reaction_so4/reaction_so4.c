#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{

  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t m_s;
  mbox_t m_o2;
  mbox_t m_so4;
  char s_msg[1];
  char o2_msg[2];

  if (argc != 5) {
    Printf("Usage: "); Printf(argv[0]); Printf(" Error in reaction so4\n");
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  m_s = dstrtol(argv[2], NULL, 10);
  m_o2 = dstrtol(argv[3], NULL, 10);
  m_so4 = dstrtol(argv[4], NULL, 10);

  // Open Mailboxes
  if(mbox_open(m_s) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Make SO4 (%d)", getpid());
    Exit();
  }
  if(mbox_open(m_o2) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Make SO4 (%d)", getpid());
    Exit();
  }
  if(mbox_open(m_so4) != MBOX_SUCCESS){
    Printf("Failed to open mailbox: Make SO4 (%d)", getpid());
    Exit();
  }

  //Receive relevant molecules (1 S and 2 O2)
  if(mbox_recv(m_s, 1, (void *)&s_msg) != 1) {
    Printf("Failed to receive to mailbox 1: Make SO4 (%d)", getpid());
    Exit();
  }
  if(mbox_recv(m_o2, 2, (void *)&o2_msg) != 2) {
    Printf("Failed to receive to mailbox 1: Make SO4 (%d)", getpid());
    Exit();
  }
  if(mbox_recv(m_o2, 2, (void *)&o2_msg) != 2) {
    Printf("Failed to receive to mailbox 1: Make SO4 (%d)", getpid());
    Exit();
  }

  //Send to SO4
  if(mbox_send(m_so4, 3, (void *)"SO4") != MBOX_SUCCESS){
    Printf("Failed to send to mailbox: Make SO4 (%d)", getpid());
    Exit();
  }
  Printf("(%d) Created SO4 Molecule\n", getpid());

  //Close
  if(mbox_close(m_s) != MBOX_SUCCESS){
    Printf("Failed to close mailbox: Make SO4 (%d)", getpid());
    Exit();
  }
  if(mbox_close(m_o2) != MBOX_SUCCESS){
    Printf("Failed to close mailbox: Make SO4 (%d)", getpid());
    Exit();
  }
  if(mbox_close(m_so4) != MBOX_SUCCESS){
    Printf("Failed to close mailbox: Make SO4 (%d)", getpid());
    Exit();
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
