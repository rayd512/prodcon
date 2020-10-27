#include "thread_handler.h"

using namespace std;

void *producer(void *id);
void *consumer(void *id);
void file_write(string command, char n, int id);
double time_now();

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_contains = PTHREAD_COND_INITIALIZER;

queue<string> work;
FILE* fp;
struct timeval start_time;
bool work_done = false;

void thread_handler(int num_threads, int id) {
	pthread_t ntid[num_threads];
	pthread_t ptid;


	gettimeofday(&start_time, NULL);

	string log_file_name = "prodcon." + to_string(id) + ".log";
	fp = fopen(log_file_name.c_str(), "w+");

	for (int i=0; i<num_threads; i++) {
		int *arg = (int *) malloc(sizeof(int));
		*arg = i + 1;
		if(pthread_create(&ntid[i], NULL, consumer, (void *)arg) != 0) {
			perror("Error creating thread");
		}
	}

	if(pthread_create(&ptid, NULL, producer, NULL) != 0) {
		perror("Error creating thread");
	}

	for(int i=0; i < num_threads; i++) {
		pthread_join(ntid[i], NULL);
	}
	pthread_join(ptid, NULL);
	fclose(fp);
}

void *consumer(void *arg) {
	while(true) {
		
		// fprintf(fp, "%.3f ID=%d      Ask\n", time_now(), *(int *) arg);
		file_write("Ask", ' ', *(int *) arg);
		// cout << "Thread " << *(int *) arg << " waiting" << endl;

		if(work_done) break;

		pthread_mutex_lock(&wait_mutex);
		pthread_cond_wait(&queue_contains, &wait_mutex);
		pthread_mutex_unlock(&wait_mutex);

		
		
		pthread_mutex_lock(&work_mutex);
		string command = work.front();
		work.pop();
		cout << (int)work.size() << endl;
		
		pthread_mutex_unlock(&work_mutex);
		cout << "Thread " << *(int *) arg << " got work" << endl;
		
		file_write("Recieve", command[1], *(int *) arg);
		
		Trans(command[1] - '0');
		file_write("Complete", command[1], *(int *) arg);
		cout << "Thread " << *(int *) arg << " done work" << endl;

	}
	free(arg);
	// pthread_exit(NULL);
	return (void *) NULL;
}

void *producer(void *arg) {

	string command;
	while(!cin.eof()) {
		cin >> command;
		if(command[0] == 'T') {
			work.push(command);
			file_write("Work", command[1], 0);
			pthread_cond_signal(&queue_contains);
			pthread_mutex_unlock(&file_mutex);
		} else {
			file_write("Sleep", command[1], 0);
			// cout << command[1]-'0' << endl;
			Sleep(command[1]-'0');
		}
	}
	work_done = true;
	file_write("End", ' ', 0);
	// pthread_cond_broadcast(&queue_contains);
	// pthread_exit(NULL);
	return (void *) NULL;

}

void file_write(string command, char n, int id) {
	pthread_mutex_lock(&file_mutex);
	if(command == "End") {
		fprintf(fp, "%-5.3f ID=%d      %s \n",
				time_now(), id, command.c_str());
	} else if (command == "Sleep" || command == "Complete") {
		fprintf(fp, "%.3f ID=%d      %-10s %c\n",
				time_now(), (int)work.size(), command.c_str(), n);
	} else if (command == "Work" || command == "Recieve") {
		fprintf(fp, "%.3f ID=%d Q= %d %-10s %c\n",
				time_now(), id, (int)work.size(), command.c_str(), n);
	}else if (command == "Ask") {
		fprintf(fp, "%.3f ID=%d      Ask\n", time_now(), id);
	}
	pthread_mutex_unlock(&file_mutex);
}

double time_now() {
	struct timeval now;
	gettimeofday(&now, NULL);
	// cout << (double) (now.tv_usec - start_time.tv_usec) / 1000000 + (double)(now.tv_sec - start_time.tv_sec) << endl;
	return (double) (now.tv_usec - start_time.tv_usec) / 1000000 + (double)(now.tv_sec - start_time.tv_sec);
}