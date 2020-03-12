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

// TODO: Unnecesary std::queue
std::queue<client_and_pos*> queue = std::queue<client_and_pos*>();
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Tells thread pool that there is a new request.
 * @param client The client id of the request that will be handled by the sockets.
 * @param original_pos http.cpp needs to know if there are threads available and won't call this function until there is one available (will iterate until it finds a clients[i] == -1. So original_pos serves the purpose that handle_response when finished will check clients[original_pos] as available.
 */
void add_connection_to_queue(int client, int original_pos) {
    // TODO: Unnecessary memory allocation
    client_and_pos* ref = (client_and_pos*) malloc(sizeof(client_and_pos));
    ref->client = client;
    ref->pos = original_pos;
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&condition);
    queue.push(ref);
    pthread_mutex_unlock(&mutex);
}

/**
 * Thread handler that will pop out of the queue an available request (if its able to do it)
 */
void* new_thread(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        client_and_pos* last = nullptr;
        if (queue.empty()) {
            // wait for call from main thread
            pthread_cond_wait(&condition, &mutex);
            // extrasafe check
            if (!queue.empty()) {
                last = queue.front();
            }
        } else {
            // get the next request
            last = queue.front();
        }
        if (last == NULL){
            pthread_mutex_unlock(&mutex);
            continue;
        }
        int client = last->client, position = last->pos;
        // finally pop the request from the queue
        queue.pop();
        pthread_mutex_unlock(&mutex);
        // handle it
        handle_response(client, position);
    }
}

/**
 * Spawn the threads
 */
void spawn() {
    for (int i = 0; i < N_THREADS; i++) {
        threads[i] = pthread_t{};
        pthread_create(&threads[i], NULL, new_thread, NULL);
    }
}
