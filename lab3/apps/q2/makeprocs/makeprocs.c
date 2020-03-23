#include "usertraps.h"
#include "misc.h"

#include "radeon.h"

void main (int argc, char *argv[])
{
  int numprocs, i;               // Used to store number of processes to create
  int n_s2, n_s, n_co, n_o2, n_so4;

  //int i;                          // Loop index variable

  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  mbox_t m_co, m_s2, m_s, m_o2, m_so4;

  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char m_co_str[10];
  char m_s2_str[10];
  char m_s_str[10];
  char m_o2_str[10];
  char m_so4_str[10];


  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }



  // Convert string from ascii command line argument to integer number
  n_s2 = dstrtol(argv[1], NULL, 10); // user input for number of S2
  Printf("Number of s2 molecules: %d\n", n_s2);
  n_co = dstrtol(argv[2], NULL, 10); // user input for number of CO
  Printf("Number of CO molecules: %d\n", n_co);
  n_s = n_s2 * 2;
  n_o2 = (n_co >= 4) ? n_co / 2 : 0;
  if (n_s == 0 || n_co == 0){
    n_so4 = 0;
  }
  else {
    n_so4 = (n_s >= n_o2/2) ? n_o2/2 : n_s;
  }
  Printf("Number of SO4 molecules: %d\n", n_so4);

  numprocs = (n_s2*2) + n_co + (n_co / 4) + n_so4;
  Printf("Number of Processes: %d\n", numprocs);

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
  if ((m_co = mbox_create()) == SYNC_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((m_s2 = mbox_create()) == SYNC_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((m_o2 = mbox_create()) == SYNC_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((m_s = mbox_create()) == SYNC_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((m_so4 = mbox_create()) == SYNC_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  if (mbox_open(m_co) == SYNC_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(m_s2) == SYNC_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(m_o2) == SYNC_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(m_s) == SYNC_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(m_so4) == SYNC_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }


  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
//  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);

  //Set up command-line arguments for semaphores that we are passing over
  ditoa(m_co, m_co_str);
  ditoa(m_s, m_s_str);
  ditoa(m_o2, m_o2_str);
  ditoa(m_s2, m_s2_str);
  ditoa(m_so4, m_so4_str);


  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.

    for(i=0; i<numprocs; i++) {
        if(i < n_s2)
        {
            process_create(FILENAME_INJS2, 0, 1, s_procs_completed_str, m_s2_str, NULL);
            Printf("makeprocs (%d): Process %d created\n", getpid(), i);
        }
        if(i < n_co)
        {
            process_create(FILENAME_INJCO, 0, 1, s_procs_completed_str, m_co_str, NULL);
            Printf("makeprocs (%d): Process %d created\n", getpid(), i);
        }
        if(i < n_s2){
            process_create(FILENAME_R2, 0, 1, s_procs_completed_str, m_s2_str, m_s_str, NULL);
            Printf("makeprocs (%d): Process %d created\n", getpid(), i);
        }
        if(i < (n_co/4)){
            process_create(FILENAME_R1, 0, 1, s_procs_completed_str, m_co_str, m_o2_str, NULL);
            Printf("makeprocs (%d): Process %d created\n", getpid(), i);
        }
        if(i < n_so4){
            process_create(FILENAME_R3, 0, 1, s_procs_completed_str, m_s_str, m_o2_str, m_so4_str, NULL);
            Printf("makeprocs (%d): Process %d created\n", getpid(), i);
        }

    }
  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(m_co) == SYNC_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(m_s2) == SYNC_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(m_o2) == SYNC_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(m_s) == SYNC_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(m_so4) == SYNC_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  /*
  Printf("%d H2O's left over. %d H2's left over. %d O2's left over. %d SO2's left over.",r_h2o,r_h2,r_o2,r_so2);
  Printf("%d H2SO4's created.\n", n_h2so4);
  */
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());
}

