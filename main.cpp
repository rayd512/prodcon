#include <iostream>
#include <string>
#include "thread_handler.h"

using namespace std;

// Simple main file that handles if correct args are given.
int main(int argc, char** argv) {

	if(argc > 3) {
		cerr <<  "Error: prodcon expects maximum 2 arguments\n"
				 "prodcon <nthreads> [id] \n"
				 "nthreads: number of threads \n"
				 "id      : id for output files, defaults to 0"
				 " if not specified \n"
			 << endl;
		exit(1);
	} else if (argc == 1) {
		cerr <<  "Error: prodcon expects atleast one argument\n"
				 "prodcon <nthreads> [id] \n"
				 "nthreads: number of threads \n"
				 "id      : id for output files, defaults to 0"
				 " if not specified \n"
			 << endl;
		exit(1);
	} else if (argc == 2) {
		thread_handler(stoi(argv[1]), 0);
	} else if (argc == 3) {
		thread_handler(stoi(argv[1]), stoi(argv[2]));
	} else {
		cerr << "An unexpected error occured" << endl;
		exit(1);
	}
}