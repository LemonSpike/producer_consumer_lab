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
# include <pthread.h>
# include <ctype.h>
# include <iostream>
using namespace std;

// Errors
# define GENERIC_ERROR_CODE -1
# define INSUFFICIENT_NUM_ARGS 1
# define INVALID_ARGUMENT 2
# define FAILED_SEMAPHORE_INIT 3

// Semaphore set key
# define SEM_KEY ftok("main.cc", 'B')

union semun {
    int val;               /* used for SETVAL only */
    struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
    ushort *array;         /* used for GETALL and SETALL */
};

int check_arg (char *);
int sem_create (key_t, int);
int sem_init (int, int, int);
void sem_wait (int, short unsigned int, int);
void sem_signal (int, short unsigned int);
int sem_close (int);

#endif
