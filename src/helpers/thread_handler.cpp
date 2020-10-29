#include "thread_handler.h"

using namespace std;

void *producer(void *id);
void *consumer(void *id);
void file_write(string command, char n, int id);
void write_footer();
void init_stats(int num_threads);
double time_now();

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_contains = PTHREAD_COND_INITIALIZER;

struct Stats {
	int work = 0;
	int ask = 0;
	int recieve = 0;
	int complete = 0;
	int sleep = 0;
	vector< pair<int, int> > threads;
	int num_threads = 0;
};


Stats stats;
queue<string> work;
FILE* fp;
struct timeval start_time;
bool work_done = false;
// bool consumers_done = false;


void thread_handler(int num_threads, int id) {
	pthread_t ntid[num_threads];
	pthread_t ptid;

	init_stats(num_threads);
	gettimeofday(&start_time, NULL);

	string log_file_name = "prodcon." + to_string(id) + ".log";
	fp = fopen(log_file_name.c_str(), "w+");

	if (fp == NULL) {
		perror("Error opening file");
	}

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
	// // cout << "Consumers Done" << endl;
	// consumers_done = true;

	pthread_join(ptid, NULL);
	write_footer();
	fclose(fp);
}

void *consumer(void *arg) {
	while(true) {
		
		// fprintf(fp, "%.3f ID=%d      Ask\n", time_now(), *(int *) arg);
		file_write("Ask", ' ', *(int *) arg);
		// cout << "Thread " << *(int *) arg << ": asking for work" << endl;
		// pthread_mutex_lock(&work_mutex);
		if(work_done && work.size() == 0) break;
		// pthread_mutex_unlock(&work_mutex);

		cout << "Thread " << *(int *) arg << ": Queue size is " << (int)work.size() << endl;

		// pthread_mutex_lock(&work_mutex);
		if (work.size() == 0) {
			pthread_mutex_lock(&wait_mutex);
			// cout << "Thread " << *(int *) arg << ": Stuck waiting" << endl;
			pthread_cond_wait(&queue_contains, &wait_mutex);
			pthread_mutex_unlock(&wait_mutex);
		}
		// pthread_mutex_unlock(&work_mutex);

		if(work_done && work.size() == 0) break;
		
		// cout << "Thread " << *(int *) arg << ": Before getting work" << endl;
		// // cout << "Thread " << *(int *) arg << ": Queue size 2 is " << (int)work.size() << endl;
		pthread_mutex_lock(&work_mutex);
		// cout << "Thread " << *(int *) arg << ": After work mutex"<< endl;
		string command = "";
		if(work.size() > 0) {
			// cout << "Thread " << *(int *) arg << ": Popping work" << endl;
			command = work.front();
			work.pop();
		} else {
			// cout << "Continuing" << endl;
			pthread_mutex_unlock(&work_mutex);
			continue;
		}
		
		// cout << "Thread " << *(int *) arg << ": After getting work" << endl;
		pthread_mutex_unlock(&work_mutex);
		
		// cout << "Thread " << *(int *) arg << ": got work" << endl;
		
		file_write("Recieve", command[1], *(int *) arg);
		
		Trans(command[1] - '0');
		file_write("Complete", command[1], *(int *) arg);
		pthread_cond_signal(&queue_full);
		pthread_mutex_unlock(&queue_mutex);
		stats.threads[*(int *) arg-1].second++;
		// cout << "Thread " << *(int *) arg << ": done work" << endl;

	}
	// cout << "Thread " << *(int *) arg << " done done" << endl;
	free(arg);
	
	// pthread_exit(NULL);
	return (void *) NULL;
}

void *producer(void *arg) {

	string command;
	while(!cin.eof()) {
		// cout << "Producer: waiting for input" << endl;
		cin >> command;
		cin.sync();
		// cout << "Producer Recieved: " << command << endl;
		if(command[0] == 'T') {
			if((int)work.size() < stats.num_threads*2) {
				pthread_mutex_lock(&work_mutex);
				work.push(command);
				pthread_mutex_unlock(&work_mutex);
				file_write("Work", command[1], 0);
				pthread_cond_signal(&queue_contains);
				pthread_mutex_unlock(&wait_mutex);
				// cout << "Producer: Adding work " << stats.work << endl;
			} else {
				cout << "Producer: Queue is full" << endl;
				pthread_mutex_lock(&queue_mutex);
				pthread_cond_wait(&queue_full, &queue_mutex);
				pthread_mutex_unlock(&queue_mutex);
			}
		} else {
			file_write("Sleep", command[1], 0);
			// // cout << command[1]-'0' << endl;
			// cout << "Producer: Sleeping" << endl;
			Sleep(command[1]-'0');
		}
	}
	work_done = true;
	// cout << "Producer is done" << endl;
	file_write("End", ' ', 0);
	pthread_cond_broadcast(&queue_contains);
	// pthread_exit(NULL);
	// while(!consumers_done) {
	// 	// cout << "Producer Signalling" << endl;
	// 	pthread_cond_broadcast(&queue_contains);
	// 	pthread_mutex_unlock(&wait_mutex);
	// }
	return (void *) NULL;

}

void file_write(string command, char n, int id) {
	pthread_mutex_lock(&file_mutex);
	if(command == "End") {
	
		fprintf(fp, "    %-5.3f ID=%d      %s \n",
				time_now(), id, command.c_str());
	
	} else if (command == "Sleep" || command == "Complete") {
		
		fprintf(fp, "    %-5.3f ID=%d      %-10s %c\n",
				time_now(), (int)work.size(), command.c_str(), n);
	
		if (command == "Sleep") stats.sleep++;
		else stats.complete++;

	} else if (command == "Work" || command == "Recieve") {
		
		fprintf(fp, "    %-5.3f ID=%d Q= %d %-10s %c\n",
				time_now(), id, (int)work.size(), command.c_str(), n);
		
		if (command == "Work") stats.work++;
		else stats.recieve++;

	}else if (command == "Ask") {
		
		fprintf(fp, "    %-5.3f ID=%d      Ask\n", time_now(), id);
		stats.ask++;

	}
	pthread_mutex_unlock(&file_mutex);
}

double time_now() {
	struct timeval now;
	gettimeofday(&now, NULL);
	// // cout << (double) (now.tv_usec - start_time.tv_usec) / 1000000 + (double)(now.tv_sec - start_time.tv_sec) << endl;
	return (double) (now.tv_sec - start_time.tv_sec) +
		   (double) (now.tv_usec - start_time.tv_usec) / 1000000;
}

void init_stats(int num_threads) {
	for(int i = 0; i < num_threads; i++) {
		pair<int, int> thread;
		thread.first = i;
		thread.second = 0;
		stats.threads.push_back(thread);
	}
	stats.num_threads = num_threads;
}

void write_footer() {
	fprintf(fp, "Summary:\n");
	fprintf(fp, "     %-10s %-5d\n", "Work", stats.work);
	fprintf(fp, "     %-10s %-5d\n", "Ask", stats.ask);
	fprintf(fp, "     %-10s %-5d\n", "Recieve", stats.recieve);
	fprintf(fp, "     %-10s %-5d\n", "Complete", stats.complete);
	fprintf(fp, "     %-10s %-5d\n", "Sleep", stats.sleep);
	for(int i = 0; i < (int)stats.threads.size(); i++) {
		string thread_name = "Thread " + to_string(i+1);
		// // cout << thread_name << endl;
		fprintf(fp, "     %-10s %-5d\n", thread_name.c_str(), stats.threads[i].second);
	}

	fprintf(fp, "Transactions per second: %.2f", (double)stats.work/time_now());
}