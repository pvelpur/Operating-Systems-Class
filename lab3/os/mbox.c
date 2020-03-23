#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

static mbox mboxes[MBOX_NUM_MBOXES];
static mbox_message  mbox_messages[MBOX_NUM_BUFFERS];

//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words, 
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
    int i, j;
    dbprintf('p', "MboxModuleInit: Entering MboxModuleInit\n");

    //initializing mboxes
    for (i=0; i < MBOX_NUM_MBOXES; i++){
        mboxes[i].inuse = 0;
        for (j=0; j < PROCESS_MAX_PROCS; j++){
            mboxes[i].pids[j] = 0;
        }
        mboxes[i].numProcs = 0;
        AQueueInit(&mboxes[i].msgs);
    }

    //initializing mbox messages
    for (i=0; i < MBOX_NUM_BUFFERS; i++){
        mbox_messages[i].inuse = 0;
    }
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use. 
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
  mbox_t mbox;

  for(mbox=0; mbox<MBOX_NUM_MBOXES; mbox++)
  {
    if(mboxes[mbox].inuse == 0){
        mboxes[mbox].inuse = 1;
        break;
    }
  }

  //Attach lock
  mboxes[mbox].lock = LockCreate();

  //Create 2 condition variables
  mboxes[mbox].notFull = CondCreate(mboxes[mbox].lock);
  mboxes[mbox].notEmpty = CondCreate(mboxes[mbox].lock);

  //Initialize Message Queue
  if(AQueueInit(&mboxes[mbox].msgs) != QUEUE_SUCCESS) {
    printf("FATAL ERROR: could not initialize mbox msgs queue in MboxCreate!\n");
    exitsim();
  }

  // Check no process has opened the mailbox (can use for loop as well)
  if(mboxes[mbox].numProcs != 0){
    printf("Some process has opened the mailbox\n");
    return MBOX_FAIL;
  }

  return mbox;
}

//-------------------------------------------------------
// 
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle 
// of the mailbox and the inuse flag will not be changed 
// during execution.  This allows us to get the a valid 
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {

    if((LockHandleAcquire(mboxes[handle].lock)) != SYNC_SUCCESS) {
        printf("Lock could not be acquired in MboxOpen\n");
    }
    mboxes[handle].pids[GetCurrentPid()] = 1;
    mboxes[handle].numProcs++;

    if((LockHandleRelease(mboxes[handle].lock)) != SYNC_SUCCESS) {
        printf("Lock could not be released in MboxOpen\n");
    }

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
  Link *l;

  if((LockHandleAcquire(mboxes[handle].lock)) != SYNC_SUCCESS) {
    printf("Lock could not be acquired in MboxClose\n");
    return MBOX_FAIL;
  }
  mboxes[handle].pids[GetCurrentPid()] = 0;
  mboxes[handle].numProcs--;

  if(mboxes[handle].numProcs == 0){
    while(!AQueueEmpty(&mboxes[handle].msgs)) {
        l = AQueueFirst(&mboxes[handle].msgs);
        AQueueRemove(&l);
    }
    mboxes[handle].inuse = 0;
  }

  if((LockHandleRelease(mboxes[handle].lock)) != SYNC_SUCCESS) {
    printf("Lock could not be released in MboxClose\n");
    return MBOX_FAIL;
  }

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call 
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the 
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {
  int intrval;
  int i;
  Link *l;

  printf("Sending message (MboxSend)!\n");

  // Error checks
  if(length <= 0 || length > MBOX_MAX_MESSAGE_LENGTH) {
    return MBOX_FAIL;
  }
  if (handle < 0 || handle > MBOX_NUM_MBOXES){
    return MBOX_FAIL;
  }
  if(mboxes[handle].pids[GetCurrentPid()] == 0) {
    return MBOX_FAIL;
  }

  //Disable instructions
  intrval = DisableIntrs();

  if((LockHandleAcquire(mboxes[handle].lock)) != SYNC_SUCCESS) {
    printf("Lock could not be acquired in MboxSend\n");
    return MBOX_FAIL;
  }

  // if mbox is full, wait not full
  if(AQueueLength(&mboxes[handle].msgs) >= MBOX_MAX_BUFFERS_PER_MBOX){
    CondHandleWait(mboxes[handle].notFull);
  }

  // get unused buffer <- atomic
  for (i = 0; i < MBOX_NUM_BUFFERS; i++){
    if(mbox_messages[i].inuse == 0) {
        mbox_messages[i].inuse = 1;
        break;
    }
  }

  // Set length and copy data (bcopy)
  // bcopy(char* source, char* destination, int numbytes);
  mbox_messages[i].mes_len = length;
  bcopy(message, mbox_messages[i].byte_message, length);

  // Add message to Queue (Queue msgs)
  if((l = AQueueAllocLink(&mbox_messages[i])) == NULL){
    printf("FATAL ERROR: Could not allocate link for mbox_message in MBoxSend!\n");
  }
  AQueueInsertLast(&mboxes[handle].msgs, l);

   //Signal not empty
   CondHandleSignal(mboxes[handle].notEmpty);

  // Release lock
  if((LockHandleRelease(mboxes[handle].lock)) != SYNC_SUCCESS) {
    printf("Lock could not be released in MboxSend\n");
    return MBOX_FAIL;
  }

  RestoreIntrs(intrval);

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call 
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".  
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox 
// via MboxOpen.
//   
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {
  int intrval;
  Link *l;
  mbox_message *m;

  printf("Receiving message (MboxRecv)!\n");

  // Error checks
  if (handle < 0 || handle > MBOX_NUM_MBOXES){
    return MBOX_FAIL;
  }
  if(mboxes[handle].pids[GetCurrentPid()] == 0) {
    return MBOX_FAIL;
  }

  //Disable instructions
  intrval = DisableIntrs();

  if((LockHandleAcquire(mboxes[handle].lock)) != SYNC_SUCCESS) {
    printf("Lock could not be acquired in MboxSend\n");
    return MBOX_FAIL;
  }

  // if mbox is empty, wait not empty
  if(AQueueEmpty(&mboxes[handle].msgs) == 1){
    CondHandleWait(mboxes[handle].notEmpty);
  }

  // get the message from the queue
  l = AQueueFirst(&mboxes[handle].msgs);
  m = (mbox_message *) l->object;

  //Error check: if the message is larger than maxlength
  if(m->mes_len > maxlength) {
    printf("ERROR: message is larger than maxlength, %d > %d\n", m->mes_len, maxlength);
    return MBOX_FAIL;
  }

  // bcopy(char* source, char* destination, int numbytes);
  bcopy(m->byte_message, message, m->mes_len);
  m->inuse = 0;

  //Remove message from Queue
  AQueueRemove(&l);

  //Signal
  CondHandleSignal(mboxes[handle].notFull);

  // Release lock
  if((LockHandleRelease(mboxes[handle].lock)) != SYNC_SUCCESS) {
    printf("Lock could not be released in MboxSend\n");
    return MBOX_FAIL;
  }

  RestoreIntrs(intrval);

  return m->mes_len;
}

//--------------------------------------------------------------------------------
// 
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
  int i;
  Link *l;
  //Error checks
  if (pid < 0 || pid > MBOX_NUM_MBOXES) { return MBOX_FAIL; }


  // Loop through all mailboxes,
  for(i=0; i < MBOX_NUM_MBOXES; i++) {
    //if pid opened it, remove from the list
    if(mboxes[i].pids[pid] == 1){
        //Use lock
        if((LockHandleAcquire(mboxes[i].lock)) != SYNC_SUCCESS) {
          printf("Lock could not be acquired in MboxCloseAllByPid\n");
          return MBOX_FAIL;
        }

        mboxes[i].pids[pid] = 0;
        mboxes[i].numProcs -= 1;

        //If no other process using the box, then mark not in use
        if(mboxes[i].numProcs == 0) {
            while(!AQueueEmpty(&mboxes[i].msgs)) {
                l = AQueueFirst(&mboxes[i].msgs);
                AQueueRemove(&l);
            }
            mboxes[i].inuse = 0;
        }

        // Release lock
        if((LockHandleRelease(mboxes[i].lock)) != SYNC_SUCCESS) {
          printf("Lock could not be released in MboxCloseAllByPid\n");
          return MBOX_FAIL;
        }
    }
  }

  return MBOX_SUCCESS;
}
