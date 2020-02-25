#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  circular_buf *circ_buf;        // Used to access missile codes in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  lock_t lock;
  int i; //loop control
  const char str [11] = "Hello World";

  if (argc != 4) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <handle_to_lock>\n");
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  lock = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((circ_buf = (circular_buf *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  for (i = 0; i < 11; i++){
    lock_acquire(lock);
    if ((circ_buf->head + 1) % 5 == circ_buf->tail) {
      i--; //So when u release the lock it doesn't increment the for loop
    } else {
      // buffer is not full
        circ_buf->data[circ_buf->head] = str[i];
        circ_buf->head = (circ_buf->head + 1) % 5;
        Printf("Producer %d inserted: %c\n", getpid(), str[i]);
    }
    lock_release(lock);
  }

  // Now print a message to show that everything worked
//  Printf("spawn_me: This is one of the %d instances you created.  ", mc->numprocs);
//  Printf("spawn_me: Missile code is: %c\n", mc->really_important_char);
//  Printf("spawn_me: My PID is %d\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  Printf("spawn_me: PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
