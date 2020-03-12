//
//  http_thread.cpp
//  train
//
//  Created by Gabriel Villalonga Simón on 10/03/2020.
//  Copyright © 2020 Gabriel Villalonga Simón. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <queue>

#include "http.hpp"
#include "./http_thread.h"

pthread_t threads[N_THREADS];
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

typedef struct {
    int client;
    int pos;
} client_and_pos;

std::queue<client_and_pos*> queue = std::queue<client_and_pos*>();
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void add_connection_to_queue(int client, int original_pos) {
    client_and_pos* ref = (client_and_pos*) malloc(sizeof(client_and_pos));
    ref->client = client;
    ref->pos = original_pos;
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&condition);
    queue.push(ref);
    pthread_mutex_unlock(&mutex);
}

void* new_thread(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        client_and_pos* last = nullptr;
        if (queue.empty()) {
            pthread_cond_wait(&condition, &mutex);
            if (!queue.empty()) {
                last = queue.front();
            }
        } else {
            last = queue.front();
        }
    
        if (last == NULL){
            pthread_mutex_unlock(&mutex);
            continue;
        }
        handle_response(last->client, last->pos);
        queue.pop();
        pthread_mutex_unlock(&mutex);
    }
}

void spawn() {
    
    for (int i = 0; i < N_THREADS; i++) {
        threads[i] = pthread_t{};
        pthread_create(&threads[i], NULL, new_thread, NULL);
    }
}
