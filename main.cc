/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

struct Data {
  const int PRODUCER_WAIT;
  const int CONSUMER_WAIT;
  int space;
  int item;
  int mutex;
  int semaphore_id;
  int queue_size;
  int num_producer_jobs;
  int num_of_producers;
  int num_of_consumers;
  int *job_durations;
  int job_counter;

  Data(): PRODUCER_WAIT(0), CONSUMER_WAIT(0), space(0),
          item(1), mutex(2), job_counter(0) { };
};

void print_sem_error_if_needed(int result, int index);
void setup_producers (Data *data);
void setup_consumers (Data *data);
void setup_semaphores (Data *data);
void *producer (void *params);
void *consumer (void *params);

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

  // Setup and initialise data structure.
  Data *data = new Data();
  data -> queue_size = queue_size;
  data -> num_producer_jobs = num_producer_jobs;
  data -> job_durations = new int[queue_size];
  data -> num_of_consumers = num_of_consumers;
  data -> num_of_producers = num_of_producers;

  // Setup semaphores.
  setup_semaphores(data);

  // Setup and initialise consumer threads.
  setup_consumers(data);

  // Setup and initialise producer threads.
  setup_producers(data);

  delete [] data -> job_durations;

  return 0;
}

void setup_semaphores (Data *data)
{
  const int NUM_SEMAPHORES = 3;
  const int SEM_ID = sem_create(SEM_KEY,
                                NUM_SEMAPHORES);
  data -> semaphore_id = (int) SEM_ID;

  int result = sem_init (SEM_ID, data -> space, data -> queue_size);
  print_sem_error_if_needed(result, data -> space);

  result = sem_init (SEM_ID, data -> item, 0);
  print_sem_error_if_needed(result, data -> item);

  result = sem_init (SEM_ID, data -> mutex, 1);
  print_sem_error_if_needed(result, data -> mutex);
}

void print_sem_error_if_needed(int result, int index)
{
  if (result == GENERIC_ERROR_CODE) {
    cout << "sem_init for " << index << " in producer has failed.";
    cout << "sem_init error code: " << result << endl;
    pthread_exit(0);
    exit(FAILED_SEMAPHORE_INIT);
  }
}

void setup_producers (Data *data)
{
  int num_of_producers = data -> num_of_producers;

  for (int index = 0; index < num_of_producers; index++) {

    pthread_t producerid;
    pthread_create (&producerid, NULL, producer, (void *) data);

    pthread_join (producerid, NULL);

    cout << "Doing some work after the join" << endl;
  }
}

void setup_consumers (Data *data)
{

  int num_of_consumers = data -> num_of_consumers;

  for (int index = 0; index < num_of_consumers; index++) {

    pthread_t consumerid;
    pthread_create (&consumerid, NULL, consumer, (void *) data);

    pthread_join (consumerid, NULL);

    cout << "Doing some work after the join" << endl;
  }
}

void *producer (void *params)
{

  Data *data = (Data *) params;
  int job_counter = data -> job_counter;
  const int SEM_ID = data -> semaphore_id;


  while (1) {
    int add_time = rand() % 5 + 1;
    sleep (add_time);

    cout << "\nThat was a good sleep - thank you \n" << endl;

    int job = rand() % 10 + 1;
    sem_wait(SEM_ID, data -> space, (int) data -> PRODUCER_WAIT);
    
    data -> job_durations[job_counter] = job;
    
    sem_wait(SEM_ID, data -> space, (int) data -> PRODUCER_WAIT);
    pthread_exit(0);
  }
}

void *consumer (void *params)
{
    // TODO
  Data * data = (Data *) params;

  sleep (5);

  cout << "\nThat was a good sleep - thank you \n" << endl;

  pthread_exit(0);

}
