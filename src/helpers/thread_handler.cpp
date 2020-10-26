#include "thread_handler.h"

using namespace std;

void *producer(void *id);
void *consumer(void *id);

pthread_mutex_t consumer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_contains = PTHREAD_COND_INITIALIZER;

queue<string> work;
FILE* fp;

void thread_handler(int num_threads, int id) {
	pthread_t ntid[num_threads];
	pthread_t ptid;

	string log_file_name = "prodcon." + to_string(id) + ".log";
	fp = fopen(log_file_name.c_str(), "w+");
	if(pthread_create(&ptid, NULL, producer, NULL) != 0) {
		perror("Error creating thread");
	}
	for (int i=0; i<num_threads; i++) {
		int *arg = (int *) malloc(sizeof(int));
		*arg = i;
		if(pthread_create(&ntid[i], NULL, consumer, (void *)arg) != 0) {
			perror("Error creating thread");
		}
	}

	for(int i=0; i < num_threads; i++) {
		pthread_join(ntid[i], NULL);
	}
	pthread_join(ptid, NULL);
}

void *consumer(void *arg) {
	pthread_mutex_lock(&consumer_mutex);
	fprintf(fp, "%d\n", *(int *) arg);
	cout << "Thread " << *(int *) arg << " waiting" << endl;
	pthread_cond_wait(&queue_contains, &consumer_mutex);
	cout << "Thread " << *(int *) arg << " got work" << endl;
	pthread_mutex_unlock(&consumer_mutex);
	free(arg);
	return (void *) NULL;
}

void *producer(void *arg) {

	string command;
	while(!cin.eof()) {
		cin >> command;
		if(command[0] == 'T') {
			work.push(command);
			pthread_cond_signal(&queue_contains);
		} else {
			Sleep((int)command[1]);
		}
	}

	return (void *) NULL;
}