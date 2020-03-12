//
//  http_thread.h
//  train
//
//  Created by Gabriel Villalonga Simón on 10/03/2020.
//  Copyright © 2020 Gabriel Villalonga Simón. All rights reserved.
//

#ifndef http_thread_h
#define http_thread_h

#include <pthread.h>

#define N_THREADS 10


void spawn();
void add_connection_to_queue(int, int);


#endif /* http_thread_h */
