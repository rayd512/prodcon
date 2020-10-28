#pragma once

#include <pthread.h>
#include <iostream>
#include <string>
#include <queue>
#include <utility>
#include <unistd.h>
#include <sys/time.h>
#include "tands.h"

void thread_handler(int num_threads, int id);
