#include "thread_handler.h"

using namespace std;

// Forward declare functions
void *producer(void *id);
void *consumer(void *id);
void file_write(string command, char n, int id);
void write_footer();
void init_stats(int num_threads);
double time_now();
void Trans( int n );
void Sleep( int n );

// Declare mutex's for threads
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_contains = PTHREAD_COND_INITIALIZER;

// Simple struct to keep track of all requests. Used for 
// writing the footer of the log file
struct Stats {
	int work = 0;
	int ask = 0;
	int recieve = 0;
	int complete = 0;
	int sleep = 0;
	vector< pair<int, int> > threads;
	int num_threads = 0;
};


// Declare globals for threads
Stats stats;
queue<string> work;
FILE* fp;
struct timeval start_time;
bool work_done = false;
int threads_running = 0;

/** Spawns threads and setups program
 *
 * Spawns num_threads amount of consumer threads and one producer thread. Will
 * also create the log file.
 *
 * @param num_threads the number of consumer threads to spawn
 * @param id the id number for the log file
 */
void thread_handler(int num_threads, int id) {
	pthread_t ntid[num_threads];
	pthread_t ptid;

	// Used to end all thread when work is done
	threads_running = num_threads;

	// Get globals ready for processing
	init_stats(num_threads);
	gettimeofday(&start_time, NULL);

	// Open or create a log file
	string log_file_name = "prodcon." + to_string(id) + ".log";
	fp = fopen(log_file_name.c_str(), "w+");

	if (fp == NULL) {
		perror("Error opening file");
	}

	// Create consumer threads
	for (int i=0; i<num_threads; i++) {
		int *arg = (int *) malloc(sizeof(int));
		*arg = i + 1;
		if(pthread_create(&ntid[i], NULL, consumer, (void *)arg) != 0) {
			perror("Error creating thread");
		}
	}

	// Create producer thread
	if(pthread_create(&ptid, NULL, producer, NULL) != 0) {
		perror("Error creating thread");
	}

	// Wait for threads to finish
	for(int i=0; i < num_threads; i++) {
		pthread_join(ntid[i], NULL);
	}

	pthread_join(ptid, NULL);
	
	// Write the footer and close the file
	write_footer();
	fclose(fp);
}

/** Does work in queue
 *
 * Consumer will wait around until work is put into the queue. When
 * work is present, consumer will grab until producer signals no more
 * work is to be done. All actions will be logged in a log file
 *
 * @param *arg pointer to the id number of the thread
 */
void *consumer(void *arg) {
	// Loop while there is still jobs
	while(true) {

		// write to file that thread is asking for work
		file_write("Ask", ' ', *(int *) arg);
		// Check if there is no more work to be done
		if(work_done && work.size() == 0) break;


		if (work.size() == 0) {
			// Wait until work is available
			pthread_mutex_lock(&wait_mutex);
			pthread_cond_wait(&queue_contains, &wait_mutex);
			pthread_mutex_unlock(&wait_mutex);
		}


		// Check if there is no more work to be done
		if(work_done && work.size() == 0) break;
		

		// Lock mutex before getting work
		pthread_mutex_lock(&work_mutex);
		string command = "";
		// Get work if available
		if(work.size() > 0) {
			command = work.front();
			work.pop();
		} else {
			pthread_mutex_unlock(&work_mutex);
			continue;
		}
		
		pthread_mutex_unlock(&work_mutex);
		
		// Write to log file we recieved work
		file_write("Recieve", command[1], *(int *) arg);
		
		// Do work
		Trans(command[1] - '0');

		// Write to log that work was finished
		file_write("Complete", command[1], *(int *) arg);

		// Signal to producer that a queue is not full anymore
		pthread_cond_signal(&queue_full);

		// Increase the num of job counter for the thread
		stats.threads[*(int *) arg-1].second++;

	}
	// Decrement thread running counter when thread exits
	pthread_mutex_lock(&thread_mutex);
	threads_running--;	
	pthread_mutex_unlock(&thread_mutex);
	// Free arg
	free(arg);
	return (void *) NULL;
}

/** Puts work in queue
 *
 *
 * Puts work into the queue for consumers to consume. Will
 * wait around before putting more items into queue if it
 * is full
 *
 * @param *arg pointer to the id number of the thread
 */
void *producer(void *arg) {

	string command;

	// Loop until end of file
	while(!cin.eof()) {

		cin >> command;

		// Break if there was no input
		if (command == "") break;

		// Check what kind of command was inputted
		if(command[0] == 'T') {
			// Check if queue is full
			if((int)work.size() < stats.num_threads*2) {
				// Lock queue and add to it
				pthread_mutex_lock(&work_mutex);
				work.push(command);
				pthread_mutex_unlock(&work_mutex);
				
				// Write work to log file
				file_write("Work", command[1], 0);

				// Signal to any waiting threads, items have been
				// added to queue
				pthread_cond_signal(&queue_contains);
			
			} else {
				// Queue is full, wait until a thread finishes a job
				pthread_mutex_lock(&queue_mutex);
				pthread_cond_wait(&queue_full, &queue_mutex);
				pthread_mutex_unlock(&queue_mutex);

				pthread_mutex_lock(&work_mutex);
				work.push(command);
				pthread_mutex_unlock(&work_mutex);

				file_write("Work", command[1], 0);

				// Signal to any waiting threads, items have been
				// added to queue
				pthread_cond_signal(&queue_contains);
				pthread_mutex_unlock(&wait_mutex);
			}
		} else {
			// Write to log and sleep
			file_write("Sleep", command[1], 0);
			Sleep(command[1]-'0');
		}

		command = "";
	}

	// Set work done to true to inform consumers
	work_done = true;
	file_write("End", ' ', 0);
	// Signal threads stuck waiting to exit
	while(threads_running != 0) {
		pthread_cond_signal(&queue_contains);
	}
	return (void *) NULL;

}

/** Writes to log file
 *
 * Writes to the log file depending on what action is done.
 * Also, increases counter for stats to be used in footer of log file
 *
 * @param command the action that was performed
 * @param n the <n> of the command
 * @param id the thread calling file_write
 */
void file_write(string command, char n, int id) {
	// Lock the mutex writing to the thread
	pthread_mutex_lock(&file_mutex);
	// Write to the log and increase the counter depending on what action was 
	// done
	if(command == "End") {
		fprintf(fp, "    %-8.3f ID=%d      %s \n",
				time_now(), id, command.c_str());
	
	} else if (command == "Sleep" || command == "Complete") {
		
		fprintf(fp, "    %-8.3f ID=%d      %-10s %c\n",
				time_now(), (int)work.size(), command.c_str(), n);
		
		if (command == "Sleep") stats.sleep++;
		else stats.complete++;

	} else if (command == "Work" || command == "Recieve") {
		
		fprintf(fp, "    %-8.3f ID=%d Q= %d %-10s %c\n",
				time_now(), id, (int)work.size(), command.c_str(), n);
		
		if (command == "Work") stats.work++;
		else stats.recieve++;

	} else if (command == "Ask") {
		
		fprintf(fp, "    %-8.3f ID=%d      Ask\n", time_now(), id);
		stats.ask++;

	}
	pthread_mutex_unlock(&file_mutex);
}


/** Gets the current execution time
 *
 * Calculates how long it has been since the program has started and 
 * returns the value as a double in seconds
 *
 * @return a double of the current execution time in seconds
 */
double time_now() {
	// Get the time now
	struct timeval now;
	gettimeofday(&now, NULL);

	// Subtract the time now from the time the program started
	return (double) (now.tv_sec - start_time.tv_sec) +
		   (double) (now.tv_usec - start_time.tv_usec) / 1000000;
}



/** Initializes the vector in stats struct
 *
 * Pushes <num_threads> amount of pairs to the vector in stats struct.
 * Also sets the num_threads in stats struct.
 *
 * @param num_threads the number of consumer threads to spawn
 */
void init_stats(int num_threads) {
	for(int i = 0; i < num_threads; i++) {
		pair<int, int> thread;
		thread.first = i;
		thread.second = 0;
		stats.threads.push_back(thread);
	}
	stats.num_threads = num_threads;
}

/** Writes the footer of the logs file
 *
 * Writes all the statistics gathered in the stats structs and appends
 * them to the end of the log file. No mutual exclution used as no threads
 * will be running at this point
 *
 * @param num_threads the number of consumer threads to spawn
 */
void write_footer() {
	fprintf(fp, "Summary:\n");
	fprintf(fp, "     %-10s %-5d\n", "Work", stats.work);
	fprintf(fp, "     %-10s %-5d\n", "Ask", stats.ask);
	fprintf(fp, "     %-10s %-5d\n", "Recieve", stats.recieve);
	fprintf(fp, "     %-10s %-5d\n", "Complete", stats.complete);
	fprintf(fp, "     %-10s %-5d\n", "Sleep", stats.sleep);
	
	for(int i = 0; i < (int)stats.threads.size(); i++) {
		string thread_name = "Thread " + to_string(i+1);
		fprintf(fp, "     %-10s %-5d\n", thread_name.c_str(), stats.threads[i].second);
	}

	fprintf(fp, "Transactions per second: %.2f", (double)stats.work/time_now());
}