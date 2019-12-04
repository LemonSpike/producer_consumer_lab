/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

// Producers and consumers need this data.
struct SharedData {
  int semaphore_id;
  const int queue_size;
  const int num_of_consumers;
  const int num_of_producers;
  int * const job_durations;
  vector<pthread_t> threads;

  SharedData(int q_size, int num_consumers, int num_producers):
    semaphore_id(GENERIC_ERROR_CODE), queue_size(q_size),
    num_of_consumers(num_consumers), num_of_producers(num_producers),
    job_durations(new int[queue_size]()) { };
};

// Consumers need this data.
struct ConsumerThreadData {
  SharedData *shared;
  int cons_id = GENERIC_ERROR_CODE;
  int *cons_job_id = nullptr;
  const int CONSUMER_WAIT = 20;

  ConsumerThreadData() {
    shared = nullptr;
  };

  ConsumerThreadData(SharedData *shared, int cons_id): shared(shared),
                                                       cons_id(cons_id) { };
};

// Producers need this data.
struct ProducerThreadData {
  SharedData *shared;
  int prod_id = GENERIC_ERROR_CODE;
  int *prod_job_id = nullptr;
  int num_producer_jobs = 0;
  const int PRODUCER_WAIT = 20;

  ProducerThreadData() {
    shared = nullptr;
  };

  ProducerThreadData(SharedData *shared, int prod_id, int num_p_jobs):
    shared(shared), prod_id(prod_id), num_producer_jobs(num_p_jobs) { };
};

// This function creates 3 semaphores, and updates the shared data with
// the Semaphore ID.
void setup_semaphores (SharedData *data);

// This function prints an error if there was an error creating semaphors.
void print_sem_error_if_needed(int result, int index, const int semaphore_id);

// This function creates producer threads passing in the data needed.
void setup_producers (SharedData *data, int num_producer_jobs,
                      int *prod_job_id);

// This function creates consumer threads passing in the data needed.
void setup_consumers (SharedData *data, int *cons_job_id);

// This function is called when a producer thread is created,
// producing and depositing the required number of jobs.
void *producer (void *params);

// This function is called when a consumer thread is created,
// fetching and consuming jobs until there are no more jobs available.
void *consumer (void *params);

int main (int argc, char **argv)
{

  // Handle insufficient number of parameters.
  if (argc != 5) {
    cerr << "Please enter 4 command line arguments." << endl;
    return INSUFFICIENT_NUM_ARGS;
  }

  // Read arguments.
  int queue_size = check_arg(argv[1]);
  if (queue_size == GENERIC_ERROR_CODE) {
    cerr << "Please enter a non-negative number for the queue size";
    cerr << " (argument 1)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_producer_jobs = check_arg(argv[2]);
  if (num_producer_jobs == GENERIC_ERROR_CODE) {
    cerr << "Please enter a non-negative number for the number of jobs";
    cerr << " to generate for each producer (argument 2)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_of_producers = check_arg(argv[3]);
  if (num_of_producers == GENERIC_ERROR_CODE) {
    cerr << "Please enter a non-negative number for the number of";
    cerr << " producers (argument 3)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_of_consumers = check_arg(argv[4]);
  if (num_of_consumers == GENERIC_ERROR_CODE) {
    cerr << "Please enter a non-negative number for the number of";
    cerr << " consumers (argument 4)." << endl;
    return INVALID_ARGUMENT;
  }

  // Setup and initialise data structure.
  SharedData *data = new SharedData(queue_size, num_of_consumers,
                                    num_of_producers);

  int prod_job_id = 0;
  int cons_job_id = 0;

  // Setup semaphores.
  setup_semaphores(data);

  // Setup and initialise consumer threads.
  setup_consumers(data, &cons_job_id);

  // Setup and initialise producer threads.
  setup_producers(data, num_producer_jobs, &prod_job_id);

  // Wait for threads to complete.
  for (unsigned int i = 0; i < (data -> threads).size(); i++) {
    int result = pthread_join ((data -> threads)[i], NULL);
    if (result < 0) {
      cerr << "Thread join failed." << endl;
      pthread_exit (0);
      return FAILED_THREAD_JOIN;
    }
  }

  // Tidy up semaphors and allocated memory.
  int result = sem_close(data -> semaphore_id);
  if (result < 0) {
    cerr << "sem_close failed for sem_id: " << (data -> semaphore_id) << endl;
    return GENERIC_ERROR_CODE;
  }

  delete [] (data -> job_durations);
  delete data;
  return 0;
}

void setup_semaphores (SharedData *data)
{
  const int NUM_SEMAPHORES = 3;
  const int SEM_ID = sem_create(SEM_KEY,
                                NUM_SEMAPHORES);
  if (SEM_ID < 0) {
    cerr << "sem_create failed with error code: " << errno << endl;
    pthread_exit (0);
    exit(FAILED_SEM_CREATE);
  }

  // These initialise the semaphore indices.
  int space = 0;
  int item = 1;
  int mutex = 2;

  data -> semaphore_id = SEM_ID;

  int result = sem_init (SEM_ID, space, data -> queue_size);
  print_sem_error_if_needed(result, space, SEM_ID);

  result = sem_init (SEM_ID, item, 0);
  print_sem_error_if_needed(result, item, SEM_ID);

  result = sem_init (SEM_ID, mutex, 1);
  print_sem_error_if_needed(result, mutex, SEM_ID);
}

void print_sem_error_if_needed(int result, int index, const int semaphore_id)
{
  if (result != GENERIC_ERROR_CODE)
    return;

  string sem_type;
  if (index == 0)
    sem_type = "space";
  else if (index == 1)
    sem_type = "item";
  else if (index == 2)
    sem_type = "mutex";
  else
    sem_type = "other semaphore";

  cerr << "sem_init for " << sem_type << "has failed. ";
  cerr << "sem_init error code: " << errno << endl;
  sem_close(semaphore_id);
  return;
}

void setup_producers (SharedData *data, int num_producer_jobs, int *prod_job_id)
{
  int num_of_producers = data -> num_of_producers;
  vector<ProducerThreadData *> prod_data;

  for (int index = 0; index < num_of_producers; index++) {

    auto thread = new ProducerThreadData(data, index + 1, num_producer_jobs);
    thread -> prod_job_id = prod_job_id;
    prod_data.push_back(thread);

    pthread_t producerid;
    int result = pthread_create (&producerid, NULL, producer,
                                 (void *) prod_data.back());
    if (result < 0) {
      cerr << "Producer(" << index + 1 << "): Thread creation failed" << endl;
      sem_close(data -> semaphore_id);
      exit(FAILED_THREAD_CREATION);
    }
    (data -> threads).push_back(producerid);
  }
}

void setup_consumers (SharedData *data, int *cons_job_id)
{
  int num_of_consumers = data -> num_of_consumers;
  vector<ConsumerThreadData *> cons_data;

  for (int index = 0; index < num_of_consumers; index++) {

    auto thread = new ConsumerThreadData(data, index + 1);
    thread -> cons_job_id = cons_job_id;
    cons_data.push_back(thread);

    pthread_t consumerid;
    int result = pthread_create (&consumerid, NULL, consumer,
                                 (void *) cons_data.back());
    if (result < 0) {
      cerr << "Consumer(" << index + 1 << "): Thread creation failed" << endl;
      sem_close(data -> semaphore_id);
      exit(FAILED_THREAD_CREATION);
    }
    (data -> threads).push_back(consumerid);
  }
}

void *producer (void *params)
{

  // Setup variables for easy access.
  ProducerThreadData *data = (ProducerThreadData *) params;
  int job_counter = 0;
  const int SEM_ID = data -> shared -> semaphore_id;
  int prod_id = data -> prod_id;
  int wait = (int) data -> PRODUCER_WAIT;
  int space = 0;
  int item = 1;
  int mutex = 2;
  int queue_size = data -> shared -> queue_size;
  int *prod_job_id = data -> prod_job_id;

  while (job_counter < data -> num_producer_jobs) {

    // Produce job.
    int add_time = rand() % 5 + 1;
    sleep (add_time);
    int job = rand() % 10 + 1;

    // Wait for a space to become available.
    int result = sem_wait(SEM_ID, space, wait);
    if (errno == EAGAIN) {
      cerr << "Producer(" << prod_id << "): Timed out" << endl;
      delete data;
      pthread_exit(0);
    } else if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in waiting for ";
      cerr << "space semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }

    // Ensure mutual exclusion is ensure before depositing job.
    result = sem_wait(SEM_ID, mutex, 0);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in waiting for ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }

    // Print out status.
    cerr << "Producer(" << prod_id << "): Job id ";
    cerr << (*prod_job_id) + 1 << " duration " << job << endl;

    // Deposit job.
    (data -> shared -> job_durations)[*prod_job_id] = job;

    // Increment producer job ID.
    (*prod_job_id)++;

    if (*prod_job_id == queue_size)
      *prod_job_id = 0;

    // Signal to other waiting threads.
    result = sem_signal(SEM_ID, mutex);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in signalling ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }
    result = sem_signal(SEM_ID, item);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in signalling ";
      cerr << "item semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }
    job_counter++;
  }

  cerr << "Producer(" << prod_id << "): No more jobs to generate." << endl;

  delete data;
  pthread_exit(0);
}

void *consumer (void *params)
{

  // Setup variables for easy access.
  ConsumerThreadData * data = (ConsumerThreadData *) params;
  const int SEM_ID = (data -> shared) -> semaphore_id;
  int cons_id = data -> cons_id;
  int *cons_job_id = data -> cons_job_id;
  int wait = (int) data -> CONSUMER_WAIT;
  int space = 0;
  int item = 1;
  int mutex = 2;
  int queue_size = data -> shared -> queue_size;

  while (1) {

    // Wait for item and mutex semaphores.
    int result = sem_wait(SEM_ID, item, wait);
    if (errno == EAGAIN) {
      cerr << "Consumer(" << cons_id << "): No more jobs left." << endl;
      delete data;
      pthread_exit (0);
    } else if (result == GENERIC_ERROR_CODE) {
      cerr << "Consumer(" << cons_id << "): Error occurred in waiting for ";
      cerr << "item semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }

    result = sem_wait(SEM_ID, mutex, 0);
    if (result == GENERIC_ERROR_CODE) {
      delete data;
      cerr << "Consumer(" << cons_id << "): Error occurred in waiting for ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }

    // Fetch and remove item.
    int *job_durations = data -> shared -> job_durations;
    int job = job_durations[*cons_job_id];

    while (job == 0) {
      (*cons_job_id)++;
      job = job_durations[*cons_job_id];
    }

    job_durations[*cons_job_id] = 0;

    // Print status.
    cerr << "Consumer(" << cons_id << "): Job id " << (*cons_job_id) + 1;
    cerr << " executing sleep duration " << job << endl;

    int id_finished =  *cons_job_id;

    // Increment consumer job ID.
    (*cons_job_id)++;

    if (*cons_job_id == queue_size)
      *cons_job_id = 0;

    // Signal mutex and space semaphores.
    result = sem_signal(SEM_ID, mutex);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Consumer(" << cons_id << "): Error occurred in signalling ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }

    result = sem_signal(SEM_ID, space);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Consumer(" << cons_id << "): Error occurred in signalling ";
      cerr << "space semaphore. Error code: " << errno << endl;
      delete data;
      pthread_exit (0);
    }

    // Consume job.
    sleep(job);

    // Print completed job status.
    cerr << "Consumer(" << cons_id << "): Job id " << id_finished + 1;
    cerr << " completed" << endl;
  }
}
