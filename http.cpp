//
//  http.cpp
//  train
//
//  Created by Gabriel Villalonga Simón on 03/01/2020.
//  Copyright © 2020 Gabriel Villalonga Simón. All rights reserved.
//

#include "utils.h"
#include "http.hpp"
#include "http_thread.h"

/// TODO:s
//  fill body in http_request

//  be able to add headers into http_response
//  be able to answer with http_response
//  be able to add a body into http_response
//  be able to add a status into http_response

#define CLIENT_SIZE 10
#define MAX_UNSIGNED_SHORT 65535

const char* response_data = "{ \"uwudsdsdsdsdssdsddssd\": 2xsdsds }\n";
char http_headers[2048] = "HTTP/2.0 400 OK\r\n"
"content-type: application/json; charset=utf-8\r\n"
"status: 200\r\n"
"X-DNS-Prefetch-Control: off\r\n";



void
reset_clients(int* clients, int size) {
    int i = 0;
    while (i < size)
        clients[i++] = -1;
}

int
next(int size, int current) {
    int i = current;
    while(server.clients[i] != -1) {
        i = (i + 1) % size;
        printf("i < %i, %i\n", i, server.clients[i]);
    }
    return i;
}

void
create_server_and_start(const char* port) {
    spawn();
    struct addrinfo hints, *res;
    server.clients = (int*) malloc(sizeof(int) * CLIENT_SIZE);
    reset_clients(server.clients, CLIENT_SIZE);
    int current_client = 0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
      perror ("getaddrinfo() error");
      exit(1);
    }
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
      int option = 1;
      server._socket = socket (p->ai_family, p->ai_socktype, 0);
      setsockopt(server._socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
      if (server._socket == -1) continue;
      if (bind(server._socket, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    
    freeaddrinfo(res);
    
    if (server._socket == -1) {
        printf("error on socket");
        return;
    }
    // listen on server_socket
    if (listen(server._socket, 100000) != 0) {
        std::cout << "ERROR LISTENING";
      return;
    }
    
    int client_socket;
    std::cout << "EVERYTHING INITIALIZED ON: " << port << std::endl;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    signal(SIGCHLD, SIG_IGN);
    
    // just for testing, this will be deleted... TODO: DELETE
    strcat(http_headers, "Content-Length: "); //\r\n\r\n
    strcat(http_headers, itoa((int) strlen(response_data), 10));
    strcat(http_headers, "\r\n\r\n");
    strcat(http_headers, response_data);
    
    while (true) {
        addrlen = sizeof(clientaddr);
        client_socket = accept(server._socket, (struct sockaddr*) &clientaddr, &addrlen);
        server.clients[current_client] = client_socket;
        add_connection_to_queue(client_socket, current_client);
        current_client = next(CLIENT_SIZE, current_client);
    }
}

void
handle_response(int client_socket,
                int original_pos) {
    http_request http_r;
    /// TODO:(GABI) we don't need dynamic allocation here
    http_r.buffer = (char*) malloc(MAX_UNSIGNED_SHORT);

    // Fill buffer of the information
    int rcvd  = (int) recv(server.clients[original_pos], http_r.buffer, MAX_UNSIGNED_SHORT, 0);
    
    // TODO: Handle these better?
    if (rcvd < 0) {
       fprintf(stderr, "recv() error\n");
    } else if (rcvd == 0) {
       fprintf(stderr, "Client disconnected unexpectedly.\n");
    }
    
    // Get the method of request
    http_r.method = get_method(http_r.buffer);
    printf("HEADER %s\n", get_header(http_r, "asdfasdfasdfsdaasfdfasdfsdafasdfsdaf").c_str());
    
    // DEBUG //////////////////////////////////////////
    printf("CLIENT POSITION: %d\n", original_pos);
    printf("Request Buffer: %s Method Number: %i\n", http_r.buffer, http_r.method);
    /////////////////////////////////////////////////
    
    http_response response;
    // Client socket for a response by the user.
    response._client_socket = client_socket;
    
    // Send the response (will be deleted) TODO: (GABI)Delete
    send(client_socket, http_headers, strlen(http_headers), 0);
    
    // Shuts down the connection
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    server.clients[original_pos] = -1;
}

inline const http_method
get_method(char* buffer) {
    int idx = 0;
    char buildup[8] = "";
    // Buildup
    while (buffer[idx] != '\t' && buffer[idx] != ' ' && buffer[idx] != '\r' && idx < 8) {
        buildup[idx] = buffer[idx];
        idx++;
    }
    
    /// NOTE(GABI): Maybe we can do better than this.
    if (!strcmp(buildup, GET_S))       return GET;
    if (!strcmp(buildup, POST_S))      return POST;
    if (!strcmp(buildup, PUT_S))       return PUT;
    if (!strcmp(buildup, DELETE_S))    return DELETE;
    if (!strcmp(buildup, HEAD_S))      return HEAD;
    if (!strcmp(buildup, OPTIONS_S))   return OPTIONS;

    return 0;
}

/**
 * Returns the header value in the header name.
 * @param request The request that you wanna get the header from
 * @param header_name The header you wanna get
 * @returns Empty string if header not found else the @header_value
 */
const std::string 
get_header(http_request& request, const char* header_name) {
    int request_length = (int) strlen(request.buffer);
    int i = 0;
    std::string builder = "";
    // skip first line
    while (request.buffer[i+1] != '\n' && request.buffer[i]!='\r') i++;
    // move into first letter
    i++; i++;
    // get first letter
    char current_char = request.buffer[i];
    //  for keeping track of header_name and header_value
    int current_char_header = 0;
    bool found = false;
    // Move into the :
    for (; i < request_length; ++i) {
        current_char = request.buffer[i];
        char char_header = header_name[current_char_header++];
        if (current_char == ':' && (found = true)) break;
        if (char_header != current_char) {
            // skip header_name
            while (i < request_length && request.buffer[i+1] != '\n' && request.buffer[i] != '\r') {
                i++;
            }
            // move into the first char
            i += 2;
            // if there are no more headers.
            if (i < request_length &&
                request.buffer[i] == '\r' &&
                request.buffer[i] == '\n'
            ) {
                return "";
            }
            continue;
        }
    }
    // Skip : and whitespace
    i += 2;
    if (!found) return "";
    
    std::string header_value = "";
    // get the string
    while (i < request_length && request.buffer[i] != '\r') {
        header_value += request.buffer[i];
        i++;
    }
    
    return header_value;
}

