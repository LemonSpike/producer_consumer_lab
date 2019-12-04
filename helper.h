/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, as well as the function signatures and
 * the semaphore values (which are to be changed as needed).
 ******************************************************************/
# ifndef HELPER_H
# define HELPER_H

# include <stdio.h>
# include <stdlib.h>
# include <cstdlib>
# include <unistd.h>
# include <sys/msg.h>
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/time.h>
# include <math.h>
# include <errno.h>
# include <string.h>
# include <vector>
# include <pthread.h>
# include <ctype.h>
# include <iostream>
using namespace std;

// Errors
const int GENERIC_ERROR_CODE = -1;
const int INSUFFICIENT_NUM_ARGS = 1;
const int INVALID_ARGUMENT = 2;
const int FAILED_SEM_CREATE = 3;
const int FAILED_SEMAPHORE_INIT = 4;
const int FAILED_THREAD_JOIN = 5;
const int FAILED_THREAD_CREATION = 6;

// Semaphore set key
const int SEM_KEY = ftok("main.cc", 'B');

// Internal data structure used for initialising a
// semaphore with id, semaphore number and value.
union semun {
    int val;               /* used for SETVAL only */
    struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
    ushort *array;         /* used for GETALL and SETALL */
};

// Check if input argument to program was positive, and if number
// of program arguments are 4.
int check_arg (char *);

// Create a semaphore set.
int sem_create (key_t, int);

// Initialise a semaphore in the set with a value and index.
int sem_init (int, int, int);

// Wait for semaphore to be unlocked. Equivalent to down().
// Last argument specifies an optional timeout in seconds.
// If last argument is 0, no timeout will be used.
int sem_wait (int, short unsigned int, int);

// Unlock semaphore. Equivalent to up().
int sem_signal (int, short unsigned int);

// Close semaphore set.
int sem_close (int);

#endif
