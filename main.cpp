#include <iostream>


using namespace std;

int main(int argc, char** argv) {

	if(argc > 3) {
		cerr << "Error: Too many inputs" << endl;
	} else if (argc == 1) {
		cerr <<  "Error: prodcon excepts atleast one argument\n"
				 "prodcon <nthreads> [id] \n"
				 "nthreads: number of threads \n"
				 "id      : id for output files, defaults to 0 if not specified \n"
			 << endl;
	}
}