/**
 * File: thread_107.h
 * ---------------
 * Defines the interface for the thread_107.
 *
 */

#ifndef _thread_107_h_
#define _thread_107_h_
#include "bool.h"
#include <semaphore.h>
/**
 * Type: SemaphoreImplementation
 * -----------------------------
 * Semaphore is an abstract type that exported from the thread package 
 * to provide a generalized semaphore abstraction.
 */

typedef sem_t *Semaphore;

/**
 * Function: InitThreadPackage
 * ---------------------------
 * This procedure initializes the thread package. Call it once (and only once)
 * at the beginning of your main program before you call any other functions 
 * from the thread package. The Boolean traceFlag controls whether debug 
 * tracing output is produced on thread exit and semaphore wait and signal 
 * activity. Turning on tracing may be useful during debugging, although 
 * the output can be prodigious.
 */

void InitThreadPackage(bool traceFlag);

/**
 * Function: ThreadNew
 * -------------------
 * This procedure creates a new thread to execute the function func. 
 * ThreadNew is a variable-argument function, somewhat like printf, 
 * that allows you to specify different numbers and types of arguments 
 * depending on what values you need to pass to the new thread’s starting 
 * function. nArg is the number of arguments, it should be zero if no 
 * arguments are passed. At most 8 arguments can be passed. After nArg,
 * you give the arguments in order that you want passed to the starting 
 * function. Due to C’s compile- time size constraints, all of the 
 * arguments must be integers or pointers (pointers can point to any 
 * type though). 
 */

void ThreadNew(const char* name, void (*function)(), int num, ...);

/**
 * Function: ThreadSleep
 * ---------------------
 * This function causes the currently executing thread to go into a 
 * sleep state for the specified number of microseconds.
 */

void ThreadSleep(int microSecs);


/**
 * Function: ThreadName
 * --------------------
 * Returns the name of the currently executing thread as set earlier 
 * with ThreadNew. This can be useful when debugging.
 */

const char *ThreadName(void);


/**
 * Function: RunAllThreads
 * -----------------------
 * Your main function has a higher priority than any functions created 
 * with ThreadNew, so no spawned threads will run until you call the 
 * RunAllThreads function. At that time, the priority of main becomes 
 * less than all the new threads, so it will not do anything more until 
 * all the new threads exit. The function only returns after all threads 
 * (including those started from within other threads) have finished 
 * executing.
 */

void RunAllThreads(void);


/**
 * Function: SemaphoreNew
 * ----------------------
 * Allocate and return a new semaphore variable. The semaphore is 
 * initialized to the given starting value. The debugName is used to 
 * associate a name with each Semaphore for debugging and is used in 
 * the tracing routines to identify Semaphores. It makes its own copy 
 * of the string passed as the name.
 */

Semaphore SemaphoreNew(const char *debugName, int initialValue);


/**
 * Function: SemaphoreName
 * -----------------------
 * This function returns the name of the Semaphore.. useful for debugging
 */

const char *SemaphoreName(Semaphore s);


/**
 * Function: SemaphoreSignal
 * -------------------------
 * This function increments the given semaphore, signalling its 
 * availability to other potentially waiting threads.
 */

void SemaphoreSignal(Semaphore s);


/**
 * Function: SemaphoreWait
 * -----------------------
 * This function waits until the semaphore has a positive value and 
 * then decrements its value, indicating its availability has decreased.
 */

void SemaphoreWait(Semaphore s);


/**
 * Function: SemaphoreFree
 * -----------------------
 * This function deallocates a semaphore and all its resources. You 
 * should only do this after all threads are completely done using 
 * the semaphore.
 */

void SemaphoreFree(Semaphore s);


/**
 * Function: AcquireLibraryLock
 * ----------------------------
 * This function can be used before code that calls a non-MT-safe routine 
 * (such as random). The man page for any library function should tell 
 * you whether any particular call is MT- safe.
 */

void AcquireLibraryLock(void);


/**
 * Function: ReleaseLibraryLock
 * ----------------------------
 * After finishing a library function that was called under the library 
 * lock, you should release the lock to allow other threads access to 
 * the standard library.
 */

void ReleaseLibraryLock(void);

/**
 * Macro: PROTECT(code)
 * --------------------
 * This macro has been defined to allow you to concisely program an 
 * unsafe library call. For example, PROTECT(result = random();) 
 * This ensure the random will go through in its entirely before any 
 * other thread (which must also correctly use the PROTECT macro) 
 * can attempt to use the routine, and thus avoids clashes with the 
 * function.
 */

#define PROTECT(code) {						\
                        AcquireLibraryLock(); 			\
                        code; 					\
                        ReleaseLibraryLock();			\
                      };

/**
 * Function: ListAllThreads
 * ------------------------
 * Lists all current threads by debug name and current status (ready 
 * to run, blocked on some condition, etc.) Probably most useful when 
 * in a deadlock situation and need figure out who is stuck and why.
 */

void ListAllThreads(void);


/**
 * Function: ListAllSemaphores
 * ---------------------------
 * Lists all semaphores by debug name and shows their current value 
 * and number of threads currently blocked on that semaphore. Like 
 * ListAllThreads, you may get unexpected effects during the printout 
 * due to interactions with other threads or the debugger.
 */

void ListAllSemaphores(void);

#endif // _thread_107.h_
