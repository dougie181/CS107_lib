#include "thread_107.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/errno.h>
#include "vector2.h"
#include "bool.h"

bool kDebug = false;
int kMaxThreads = 256;
int kThreadCount = 0;
int kReadyToRunThreads = 0;

pthread_mutex_t kmutexLibrayLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t kmutexThreadLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t kmutexSemaphoreLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t kcondStartThreads = PTHREAD_COND_INITIALIZER;
pthread_mutex_t kmutexStartThreads = PTHREAD_MUTEX_INITIALIZER;

Semaphore finishProgram;

vector2 threads;
vector2 semaphores;

typedef struct {
  void (*function)();
  long numArgs;
  void *args;
} params;

typedef struct {
  int id;
  pthread_t t;
  char *name;
} threadItem;

typedef struct {
  int id;
  Semaphore s;
  char *name;
} semaphoreItem;

void functionWrapper(void *args);
void freeThreadItem(void *elem);
void freeSemaphoreItem(void *elem);
void printThread(void *elem, void *auxData);
void printSemaphore(void *elem, void *auxData);
int threadCompare(const void *key, const void *elem);
int threadCompareID(const void *key, const void *elem);
int semaphoreCompare(const void *key, const void *elem);
int semaphoreNameCompare(const void *key, const void *elem);
void print_thread_id(pthread_t id);

void InitThreadPackage(bool traceFlag)
{
  if (traceFlag) kDebug = true;
  if (kDebug) printf("::InitThreadPackage::\n");

  // create the thread and semaphore lists (as vector2)
  VectorNew2(&threads, sizeof(threadItem), freeThreadItem, 5);
  VectorNew2(&semaphores, sizeof(semaphoreItem), freeSemaphoreItem, 5);

  // add this thread to the thread table
  threadItem thisThread;
  thisThread.id = 1;
  thisThread.name = strdup("main()");
  thisThread.t  = pthread_self();
  VectorAppend2(&threads, &thisThread);
  
  // create a semaphore to prevent the main thread from finishing
  // prior to the child threads completing
  finishProgram = SemaphoreNew("ThreadsAllCompleted", 0);
}

void ThreadNew(const char *debugName, void (*func)(), int nArg, ...)
{
  if (kDebug) printf("::ThreadNew::\n");

  // make a deep copy of the param list and arguments that are passed
  params *paramList = malloc(sizeof(params));
  void *arg = malloc(nArg  * sizeof(long));
  assert(arg != NULL);

  paramList->function = func;
  paramList->numArgs = nArg;
  paramList->args = arg;

  long *ptr = (void *)arg;		// use a ptr to args

  // retrieve argument list from ... using va_ macros
  if (kDebug) printf("numArgs = %ld\n",paramList->numArgs);
  va_list argptr;
  va_start(argptr, nArg);
  for (int i = 0; i < nArg; i++) {
    ptr[i] = va_arg(argptr, long);	// add each argument to pointer list
    if (kDebug) printf("arg[%d] = %ld\n",i, ptr[i]);
  }
  va_end(argptr);

  // recast the function to call the wrapper function
  void *function = *(&functionWrapper);

  // start the child thread
  pthread_t t;
  int result_code = pthread_create(&t, NULL, function, paramList);
  if (result_code) {
    printf("ThreadNew:: pthread_create errror (%d) - %s\n", result_code, strerror(errno));
    exit(-1);
  }

  // add the new thread to the thread table
  threadItem newThread;

  errno=0;
  int result1 = pthread_mutex_lock(&kmutexThreadLock);
  if (result1 != 0) printf("ThreadNew:: mutex_lock error - %s\n",strerror(errno));
  assert(result1 == 0);

  newThread.id = VectorLength2(&threads) + 1;
  newThread.t = t;
  newThread.name = strdup(debugName);
  VectorAppend2(&threads, &newThread); 

  errno=0;
  int result2 = pthread_mutex_unlock(&kmutexThreadLock);
  if (result2 != 0) printf("ThreadNew:: mutex_unlock error - %s\n", strerror(errno));
  assert(result2 == 0);
}

void functionWrapper(void *args)
{
  if (kDebug) printf("::entering functionWrapper::\n");
  
  if (kDebug) printf("  incrementing thread count\n");
  PROTECT(kThreadCount++;);
  
  // retrieve params from ptr
  params *paramList = (params *)args;

  // retrieve the args
  long *ptr = (long *)paramList->args;

  if (kDebug) printf("  waiting...\n\n");

  pthread_mutex_lock(&kmutexStartThreads);
  while (!kReadyToRunThreads) {
    pthread_cond_wait(&kcondStartThreads, &kmutexStartThreads);
  }
  pthread_mutex_unlock(&kmutexStartThreads);

  if (kDebug) printf("  done waiting...\n");

  // ok, so all good to go... call the appropriate function
  if (kDebug) printf("calling function with %ld args\n",paramList->numArgs);
  switch (paramList->numArgs) {
    case 0:
      paramList->function();
      break;
    case 1:
      paramList->function(ptr[0]);
      break;
    case 2:
      paramList->function(ptr[0], ptr[1]);
      break;
    case 3:
      paramList->function(ptr[0], ptr[1], ptr[2]);
      break;
    case 4:
      paramList->function(ptr[0], ptr[1], ptr[2], ptr[3]);
      break;
    case 5:
      paramList->function(ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
      break;
    case 6:
      paramList->function(ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
      break;
    case 7:
      paramList->function(ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6]);
      break;
    case 8:
      paramList->function(ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);
      break;
    default:
      printf("functionWrapper:: error - exceeded max paramaters of 8\n");
      exit(1);
  }
  free(paramList->args);
  free(paramList);

  // thread completed...
  if (kDebug) printf("  decrementing thread count\n");
  PROTECT(kThreadCount--;);

  PROTECT(if (kThreadCount == 0) SemaphoreSignal(finishProgram););

  if (kDebug) printf("  thread count now: %d\n",kThreadCount+1);
  if (kDebug) printf("  function finished\n");

  //remove this thread from the thread table
  pthread_t thisID = pthread_self();

  int result1 = pthread_mutex_lock(&kmutexThreadLock);
  if (result1 != 0) printf("functionWrapper:: mutex_lock error - %s\n",strerror(errno));
  assert(result1 == 0);

  int pos = VectorSearch2(&threads, &thisID, threadCompare, 0, false);
  assert(pos >= 0);
  VectorDelete2(&threads, pos);

  int result2 = pthread_mutex_unlock(&kmutexThreadLock);
  if (result2 != 0) printf("functionWrapper:: mutex_unlock error - %s\n",strerror(errno));
  assert(result2 == 0);
}

void ThreadSleep(int microSecs)
{
  struct timespec time_delay;

  time_delay.tv_nsec = microSecs * 1000L;
  time_delay.tv_sec = time_delay.tv_nsec / 1000000000;
  time_delay.tv_nsec %= 1000000000;

  double delay = time_delay.tv_sec + ((double)time_delay.tv_nsec / 1000000000);

  if (kDebug) printf("::ThreadSleep:: %s sleeping for %f seconds\n", ThreadName(),delay);

  // usleep has been deprecated; now using nanosleep
  while (nanosleep(&time_delay, &time_delay) && errno==EINTR);
}

const char *ThreadName(void)
{
  char *returnName;

  if (kDebug) printf("::ThreadName::\n");

  // get this thread id
  pthread_t thisID = pthread_self();

  // lookup threadTable using the threadID as key and return the name
  int pos = VectorSearch2(&threads, &thisID, threadCompare, 0, false);
  if (pos >=0) {
    threadItem *thisThread = VectorNth2(&threads, pos);
    returnName = thisThread->name;
  } else {
    returnName = strdup("** unknown **"); 
  }
  return returnName;
}

void RunAllThreads(void)
{
  if (kDebug) printf("::RunAllThreads::\n");

  // Unblock all threads currently blocked on the specified condition variable.
  pthread_mutex_lock(&kmutexStartThreads);

  if (kDebug) printf("There are %d threads queued\n",kThreadCount+1);
  if (kDebug) ListAllThreads();

  kReadyToRunThreads = 1;
  pthread_cond_broadcast(&kcondStartThreads);
  pthread_mutex_unlock(&kmutexStartThreads);

  // now wait for all the processes to finish before continuing this one
  SemaphoreWait(finishProgram);
                       
  // all child threads have finished - continue on main thread
  if (kDebug) ListAllSemaphores();
  if (kDebug) ListAllThreads();

  SemaphoreFree(finishProgram);
}

/*
 *
 * Usage:
 *   Semaphore ticketsLock = SemaphoreNew("Tickets Lock", 1);
 */

Semaphore SemaphoreNew(const char *debugName, int initialValue)
{
  if (kDebug) printf("::SemaphoreNew::\n");

  // semaphore names begin with a '/' character and can only contain
  // one '/' character. check if name already contains a '/'
  char *found = strstr(debugName,"/");
  assert(found == NULL);

  // cool, so no '/' characters... now need to prefix the name with one
  char *semaphoreName = malloc(strlen(debugName) + 2);
  semaphoreName[0] = '/';
  int i = 0;
  while (i <= strlen(debugName)) {
    semaphoreName[i + 1] = debugName[i];
    i++;
  }
  semaphoreName[i] = 0;

  // check if the Semaphore already exists
  int pos = VectorSearch2(&semaphores, semaphoreName, semaphoreNameCompare, 0, false);
  if (pos >= 0)
    printf("Error creating semaphore - %s already exists\n",semaphoreName);
  assert(pos < 0);

  if (kDebug) printf("  creating semaphore with name %s\n",semaphoreName);
  int oflag = O_CREAT | O_EXCL;
  //int oflag =  O_EXCL;
  //int oflag = O_CREAT;

  Semaphore sem;
  sem = sem_open(semaphoreName, oflag, 0644, initialValue);
  sem_unlink(semaphoreName);
  if (sem == SEM_FAILED) {
    printf("SemaphoreNew:: sem_open failed - %s\n", strerror(errno));
    exit(-1);
  }

  // add the semaphore to the semaphore table (lock access temporarily)
  int result1 = pthread_mutex_lock(&kmutexSemaphoreLock);
  if (result1 != 0) printf("SemaphoreNew:: mutex_lock error - %s\n",strerror(errno));
  assert(result1 == 0);

  semaphoreItem newSemaphore;
  newSemaphore.id = VectorLength2(&semaphores) + 1;
  newSemaphore.name = semaphoreName;
  newSemaphore.s = sem;
  VectorAppend2(&semaphores, &newSemaphore); 

  int result2 = pthread_mutex_unlock(&kmutexSemaphoreLock);
  if (result2 != 0) printf("SemaphoreNew:: mutex_unlock error - %s\n",strerror(errno));
  assert(result2 == 0);

  if (kDebug) ListAllSemaphores();

  return sem;
}

const char *SemaphoreName(Semaphore s)
{
  char *sem_name;
  if (kDebug) printf("::SemaphoreName::\n");

  // look up semaphore table
  int pos = VectorSearch2(&semaphores, s, semaphoreCompare, 0, false);
  if (pos >= 0) {
    semaphoreItem *theSemaphore = VectorNth2(&semaphores, pos);
    sem_name = theSemaphore->name;
  } else {
    sem_name = strdup("** unknown **");
  }

  return(sem_name);
}

/**
 *
 * Usage: SemaphoreSignal(ticketsLock);
 */

void SemaphoreSignal(Semaphore s)
{ 
  if (kDebug) printf("::SemaphoreSignal::\n");

  int result = sem_post(s);
  if (result != 0)
    printf("SemaphoreSignal:: sem_post error - %s\n", strerror(errno));
  assert(result == 0);
}

/**
 *
 * Usage: SemaphoreWait(ticketsLock);
 */

void SemaphoreWait(Semaphore s)
{
  if (kDebug) printf("::SemaphoreWait::\n");
  int result = sem_wait(s);
  if (result != 0)
    printf("SemaphoreWait:: sem_wait error - %s\n",strerror(errno));
  assert(result == 0);
}

/**
 *
 * Usage: SemaphoreFree(ticketsLock); 
 */

void SemaphoreFree(Semaphore s)
{
  if (kDebug) printf("::SemaphoreFree::\n");

  int result1 = pthread_mutex_lock(&kmutexSemaphoreLock);
  if (result1 != 0) printf("SemaphoreFree:: mutex_lock error - %s\n",strerror(errno));
  assert(result1 == 0);

  int pos = VectorSearch2(&semaphores, &s, semaphoreCompare, 0, false);
  if (pos >= 0) VectorDelete2(&semaphores, pos);

  int result2 = pthread_mutex_unlock(&kmutexSemaphoreLock);
  if (result2 != 0) printf("SemaphoreFree:: mutex_unlock error - %s\n",strerror(errno));
  assert(result2 == 0);

  int result = sem_close(s);
  if (result1 != 0) printf("SemaphoreFree:: sem_close error - %s\n",strerror(errno));
  assert(result == 0);
}

void AcquireLibraryLock(void)
{
  if (kDebug) printf("::AcquireLibraryLock::\n");

  int result = pthread_mutex_lock(&kmutexLibrayLock);
  if (result != 0) printf("AcquireLibraryLock:: mutex_lock error - %s\n",strerror(errno));
  assert(result == 0);
}

void ReleaseLibraryLock(void)
{
  if (kDebug) printf("::ReleaseLibraryLock::\n");

  int result = pthread_mutex_unlock(&kmutexLibrayLock);
  if (result != 0) printf("ReleaseLibraryLock:: mutex_unlock error - %s\n",strerror(errno));
  assert(result == 0);
}

void ListAllThreads(void)
{
  // prevent this function to be called simulatanously by other thread
  int result1 = pthread_mutex_lock(&kmutexThreadLock);
  if (result1 != 0) printf("ListAllThreads:: mutex_lock error -  %s\n",strerror(errno));
  assert(result1 == 0);

  VectorMap2(&threads,printThread,NULL);

  int result2 = pthread_mutex_unlock(&kmutexThreadLock);
  if (result2 != 0) printf("ListAllThreads:: mutex_unlock error - %s\n",strerror(errno));
  assert(result2 == 0);
}

void ListAllSemaphores(void)
{
  // prevent this function to be called simulatanously by other thread
  int result1 = pthread_mutex_lock(&kmutexSemaphoreLock);
  if (result1 != 0) printf("ListAllSemaphores: mutex_lock error - %s\n",strerror(errno));
  assert(result1 == 0);

  PROTECT(VectorMap2(&semaphores,printSemaphore,NULL););

  int result2 = pthread_mutex_unlock(&kmutexSemaphoreLock);
  if (result2 != 0) printf("ListAllSemaphores: mutex_unlock error - %s\n",strerror(errno));
  assert(result2 == 0);
}

void freeThreadItem(void *elem)
{
  threadItem *item = (threadItem *)elem;
  free(item->name);
}

void freeSemaphoreItem(void *elem)
{
  semaphoreItem *item = (semaphoreItem *)elem;
  free(item->name);
}

void printThread(void *elem, void *auxData)
{
  threadItem *item = (threadItem *)elem;

  int pos = VectorSearch2(&threads, item, threadCompareID, 0, false);
  if (pos == 0) {
    printf(" Thread Table\n");
    printf("+------------+-----------------------------+------------------+\n");
    printf("| Thread ID  | Thread Name                 | pthread_t        |\n");
    printf("+------------+-----------------------------+------------------+\n");
  }
  printf("|   %4d     | %-25s   | ",item->id, item->name);
  print_thread_id(item->t);
  printf(" |\n");
  if (pos == VectorLength2(&threads)-1)
    printf("+------------+-----------------------------+------------------+\n");
}

void printSemaphore(void *elem, void *auxData)
{
  assert(elem != NULL);
  semaphoreItem *item = (semaphoreItem *)elem;
  int pos = VectorSearch2(&semaphores, &(item->s), semaphoreCompare, 0, false);
  if (pos == 0) {
    printf(" Semaphore Table\n");
    printf("+------------+-----------------------------+\n");
    printf("|semaphoreID | Semaphore Name              |\n");
    printf("+------------+-----------------------------+\n");
  }
  printf("|   %4d     | %-25s   |\n",item->id, item->name);
  if (pos == VectorLength2(&semaphores)-1)
    printf("+------------+-----------------------------+\n");
}

void print_thread_id(pthread_t id)
{
    size_t i;
    for (i = sizeof(i); i; --i)
        printf("%02x", *(((unsigned char*) &id) + i - 1));
}

int threadCompare(const void *key, const void *elem)
{
  pthread_t *theKey = (pthread_t *)key;
  threadItem *item = (threadItem *)elem;
  
  if (pthread_equal(*theKey, item->t) !=0) return 0;
  else return 1;
}

int threadCompareID(const void *key, const void *elem)
{
  threadItem *theKey = (threadItem *)key;
  threadItem *item = (threadItem *)elem;
 
  return (theKey->id - item->id);
}

int semaphoreCompare(const void *key, const void *elem)
{
  Semaphore *theKey = (Semaphore *)key;
  semaphoreItem *item = (semaphoreItem *)elem;

  if (*theKey == item->s) return 0;
  else return 1;
}

int semaphoreNameCompare(const void *key, const void *elem)
{
  semaphoreItem *item = (semaphoreItem *)elem;
  return(strcmp((char *)key, item->name));
}
