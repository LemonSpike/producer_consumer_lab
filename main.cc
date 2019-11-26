/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

// Errors
# define INSUFFICIENT_NUM_ARGS 1
# define INVALID_ARGUMENT 2
# define FAILED_SEMAPHORE_INIT 3

void setup_producers (int num_of_producers);
void setup_consumers (int num_of_consumers);
void setup_jobs ();
void *producer (void *id);
void *consumer (void *id);

int main (int argc, char **argv)
{

  // Handle insufficient number of parameters.
  if (argc < 4) {
    cout << "Please enter 4 command line arguments." << endl;
    return INSUFFICIENT_NUM_ARGS;
  }

  // Read arguments.

  int queue_size = check_arg(argv[0]);
  if (queue_size == GENERIC_ERROR_CODE) {
    cout << "Please enter a non-negative number for the queue size";
    cout << " (argument 1)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_producer_jobs = check_arg(argv[1]);
  if (num_producer_jobs == GENERIC_ERROR_CODE) {
    cout << "Please enter a non-negative number for the number of jobs";
    cout << " to generate for each producer (argument 2)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_of_producers = check_arg(argv[2]);
  if (num_of_producers == GENERIC_ERROR_CODE) {
    cout << "Please enter a non-negative number for the number of";
    cout << " producers (argument 3)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_of_consumers = check_arg(argv[3]);
  if (num_of_consumers == GENERIC_ERROR_CODE) {
    cout << "Please enter a non-negative number for the number of";
    cout << " consumers (argument 4)." << endl;
    return INVALID_ARGUMENT;
  }

  // Setup and initialise shared jobs data structure.
  job_durations = new int[queue_size];

  // Create semaphores.
  int num_semaphores = num_of_producers + num_of_consumers + 1;
  SEM_ID = sem_create(SEM_KEY,
       	              num_semaphores,
                      0666 | IPC_CREAT | IPC_EXCL);

  // Setup jobs semaphore.
  setup_jobs();

  // Setup and initialise consumer threads and semaphores.
  setup_consumers(num_of_consumers);

  // Setup and initialise producer threads and semaphores.
  setup_producers(num_of_producers);

  for (int index = 0; index < queue_size; index++)
    delete job_durations[index];
  delete [] job_durations;

  return 0;
}

void setup_producers (int num_of_producers)
{

  for (int index = 0; index < num_of_producers; index++) {

    pthread_t producerid;
    pthread_create (&producerid, NULL, producer, (void *) &index);

    pthread_join (producerid, NULL);

    cout << "Doing some work after the join" << endl;
  }
}

void setup_consumers (int num_of_consumers)
{

  for (int index = 0; index < num_of_consumers; index++) {

    pthread_t consumerid;
    pthread_create (&consumerid, NULL, consumer, (void *) &index);

    pthread_join (consumerid, NULL);

    cout << "Doing some work after the join" << endl;
  }
}

void setup_jobs()
{

}

void *producer (void *id)
{

  // TODO
  int *index = (int *) id;
  int result = sem_init (SEM_ID, *index, PRODUCER_WAIT);
  if (result != 0) {
    cout << "sem_init for " << *index << " in producer has failed.";
    cout << "sem_init error code: " << result << endl;
    pthread_exit(1);
    exit(FAILED_SEMAPHORE_INIT);
  }

  cout << "Parameter = " << *param << endl;

  sleep (5);

  cout << "\nThat was a good sleep - thank you \n" << endl;

  pthread_exit(0);
}

void *consumer (void *id)
{
    // TODO
  int *index = (int *) id;
  int result = sem_init (SEM_ID, *index, CONSUMER_WAIT);
  if (result != 0) {
    cout << "sem_init for " << *index << " in consumer has failed.";
    cout << " sem_init error code: " << result << endl;
    pthread_exit(1);
    exit(FAILED_SEMAPHORE_INIT);
  }

  sleep (5);

  cout << "\nThat was a good sleep - thank you \n" << endl;

  pthread_exit(0);

}
