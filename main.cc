/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

// Errors
#define INSUFFICIENT_NUM_ARGS 1
#define INVALID_ARGUMENT 2

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
  if (queue_size == -1) {
    cout << "Please enter a non-negative number for the queue size(argument 1)";
    << endl;
    return INVALID_ARGUMENT;
  }

  int num_producer_jobs = check_arg(argv[1]);
  if (num_producer_jobs == -1) {
    cout << "Please enter a non-negative number for the number of jobs";
    cout << " to generate for each producer (argument 2)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_of_producers = check_arg(argv[2]);
  if (num_of_producers == -1) {
    cout << "Please enter a non-negative number for the number of";
    cout << " producers (argument 3)." << endl;
    return INVALID_ARGUMENT;
  }

  int num_of_consumers = check_arg(argv[3]);
  if (num_of_consumers == -1) {
    cout << "Please enter a non-negative number for the number of";
    cout << " consumers (argument 4)." << endl;
    return INVALID_ARGUMENT;
  }

  pthread_t producerid;
  int parameter = 5;

  pthread_create (&producerid, NULL, producer, (void *) &parameter);

  pthread_join (producerid, NULL);

  cout << "Doing some work after the join" << endl;

  return 0;
}

void *producer (void *parameter) 
{

  // TODO

  int *param = (int *) parameter;

  cout << "Parameter = " << *param << endl;

  sleep (5);

  cout << "\nThat was a good sleep - thank you \n" << endl;

  pthread_exit(0);
}

void *consumer (void *id) 
{
    // TODO 

  pthread_exit (0);

}
