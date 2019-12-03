/******************************************************************
0;95;0c * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

struct SharedData {
  int semaphore_id;
  const int queue_size;
  const int num_of_consumers;
  const int num_of_producers;
  vector<int> job_durations;
  int current_job_id;
  vector<pthread_t> threads;

  SharedData(int q_size, int num_consumers, int num_producers,
             vector<int> job_durations):
    semaphore_id(GENERIC_ERROR_CODE), queue_size(q_size),
    num_of_consumers(num_consumers), num_of_producers(num_producers),
    current_job_id(1) { };
};

struct ConsumerThreadData {
  SharedData *shared = nullptr;
  int cons_id = GENERIC_ERROR_CODE;
  const int CONSUMER_WAIT = 20;

  ConsumerThreadData& operator=(const ConsumerThreadData& d) {
    shared = d.shared;
    cons_id = d.cons_id;
    return *this;
  }

  ConsumerThreadData() { };

  ConsumerThreadData(SharedData *shared, int cons_id): shared(shared),
                                                       cons_id(cons_id) { };
};

struct ProducerThreadData {
  SharedData *shared = nullptr;
  int prod_id = GENERIC_ERROR_CODE;
  int num_producer_jobs = 0;
  const int PRODUCER_WAIT = 20;

  ProducerThreadData& operator=(const ProducerThreadData& d) {
    shared = d.shared;
    prod_id = d.prod_id;
    num_producer_jobs = d.num_producer_jobs;
    return *this;
  }

  ProducerThreadData() { };

  ProducerThreadData(SharedData *shared, int prod_id, int num_p_jobs):
    shared(shared), prod_id(prod_id), num_producer_jobs(num_p_jobs) { };
};

void print_sem_error_if_needed(int result, int index, const int semaphore_id);
void setup_producers (SharedData *data, int num_producer_jobs);
void setup_consumers (SharedData *data);
void setup_semaphores (SharedData *data);
void *producer (void *params);
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
  SharedData *data = new SharedData(queue_size, num_of_producers,
                                    num_of_consumers);

  // Setup semaphores.
  setup_semaphores(data);

  // Setup and initialise consumer threads.
  setup_consumers(data);

  // Setup and initialise producer threads.
  setup_producers(data, num_producer_jobs);

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
  pthread_exit (0);
  exit(FAILED_SEMAPHORE_INIT);
}

void setup_producers (SharedData *data, int num_producer_jobs)
{
  int num_of_producers = data -> num_of_producers;
  vector<ProducerThreadData> prod_data;

  for (int index = 0; index < num_of_producers; index++) {

    prod_data.push_back(ProducerThreadData(data, index + 1, num_producer_jobs));

    pthread_t producerid;
    int result = pthread_create (&producerid, NULL, producer,
                                 (void *) &prod_data[index]);
    if (result < 0) {
      cerr << "Producer(" << index + 1 << "): Thread creation failed" << endl;
      return;
    }
    (data -> threads).push_back(producerid);
  }
}

void setup_consumers (SharedData *data)
{
  int num_of_consumers = data -> num_of_consumers;
  vector<ConsumerThreadData> cons_data;

  for (int index = 0; index < num_of_consumers; index++) {

    cons_data.push_back(ConsumerThreadData(data, index + 1));

    pthread_t consumerid;
    int result = pthread_create (&consumerid, NULL, consumer,
                                 (void *) &cons_data[index]);
    if (result < 0) {
      cerr << "Consumer(" << index + 1 << "): Thread creation failed" << endl;
      return;
    }
    (data -> threads).push_back(consumerid);
  }
}

void *producer (void *params)
{

  ProducerThreadData *data = (ProducerThreadData *) params;
  int job_counter = 1;
  const int SEM_ID = data -> shared -> semaphore_id;
  int prod_id = data -> prod_id;
  int wait = (int) data -> PRODUCER_WAIT;
  int space = 0;
  int item = 1;
  int mutex = 2;
  int queue_size = data -> shared -> queue_size;

  while (job_counter <= data -> num_producer_jobs) {

    // Produce job.
    int add_time = rand() % 5 + 1;
    sleep (add_time);
    int job = rand() % 10 + 1;

    // Wait.
    int result = sem_wait(SEM_ID, space, wait);
    if (errno == EAGAIN) {
      cerr << "Producer(" << prod_id << "): Timed out" << endl;
      pthread_exit(0);
      break;
    } else if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in waiting for ";
      cerr << "space semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }
    result = sem_wait(SEM_ID, mutex, 0);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in waiting for ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }

    int current_job_id = data -> shared -> current_job_id;
    cerr << "Producer(" << prod_id << "): Job id ";
    cerr << current_job_id << " duration " << job << endl;

    // Deposit job.
    (data -> shared -> job_durations).push_back(job);

    // Increment Job ID.
    if (current_job_id == queue_size)
      data -> shared -> current_job_id = 1;
    else 
      (data -> shared -> current_job_id)++;

    result = sem_signal(SEM_ID, mutex);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in signalling ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }
    result = sem_signal(SEM_ID, item);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Producer(" << prod_id << "): Error occurred in signalling ";
      cerr << "item semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }
    job_counter++;
  }

  cerr << "Producer(" << prod_id << "): No more jobs to generate." << endl;
  pthread_exit(0);
}

void *consumer (void *params)
{

  ConsumerThreadData * data = (ConsumerThreadData *) params;
  const int SEM_ID = (data -> shared) -> semaphore_id;
  int cons_id = data -> cons_id;
  int wait = (int) data -> CONSUMER_WAIT;
  int space = 0;
  int item = 1;
  int mutex = 2;

  while (1) {

    // Wait for item and utex semaphores.
    int result = sem_wait(SEM_ID, item, wait);
    if (errno == EAGAIN) {
      cerr << "Consumer(" << cons_id << "): No more jobs left." << endl;
      pthread_exit (0);
      break;
    } else if (result == GENERIC_ERROR_CODE) {
      cerr << "Consumer(" << cons_id << "): Error occurred in waiting for ";
      cerr << "item semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }

    result = sem_wait(SEM_ID, mutex, 0);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Consumer(" << cons_id << "): Error occurred in waiting for ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }

    vector<int> job_durations = data -> shared -> job_durations;
    int job = job_durations[0];
    job_durations.erase(job_durations.begin());

    int current_job_id = data -> shared -> current_job_id;
    cerr << "Consumer(" << cons_id << "): Job id " << current_job_id;
    cerr << " executing sleep duration " << job << endl;

    if (current_job_id != 1)
      (data -> shared -> current_job_id)--;

    // Signal mutex and space semaphores.
    result = sem_signal(SEM_ID, mutex);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Consumer(" << cons_id << "): Error occurred in signalling ";
      cerr << "mutex semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }

    result = sem_signal(SEM_ID, space);
    if (result == GENERIC_ERROR_CODE) {
      cerr << "Consumer(" << cons_id << "): Error occurred in signalling ";
      cerr << "space semaphore. Error code: " << errno << endl;
      pthread_exit (0);
      break;
    }

    // Consume job.
    sleep(job);

    cerr << "Consumer(" << cons_id << "): Job id " << current_job_id;
    cerr << " completed" << endl;
  }
}
