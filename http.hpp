//
//  http.hpp
//  train
//
//  Created by Gabriel Villalonga Simón on 03/01/2020.
//  Copyright © 2020 Gabriel Villalonga Simón. All rights reserved.
//

#ifndef http_hpp
#define http_hpp

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define GET     1
#define POST    2
#define OPTIONS 3
#define PUT     4
#define HEAD    5
#define DELETE  6
#define PATCH   7

#define GET_S     "GET"
#define POST_S    "POST"
#define OPTIONS_S "OPTIONS"
#define PUT_S     "PUT"
#define HEAD_S    "HEAD"
#define DELETE_S  "DELETE"
#define PATCH_S   "PATCH"

/**
 * We are gonna use const char* types with variables we know are gonna filled from user-called functions
 * We are using std::string for dynamic string alloc but will be replaced.
 */

typedef uint8_t http_method;

typedef struct {
    const char* header_name;
	const char* value;
} http_header;

//typedef struct {
//    std::map<std::string, std::string> map;
//} key_pair_http;
//
///**
// * After some thought we won't be using this. See get_query_param()
// */
//typedef struct {
//    std::string uri;
//    std::string uri_query;
//    key_pair_http query;
//} uri;

typedef struct {
    // Status (101, 200, 400...)
	char status[3];
    // String containing status info.
    std::string  status_str;
    // Array of headers
    std::vector<http_header> headers;
    // Number of headers
    uint32_t     header_n;
    // String containing the body of the response.
    std::string  content;
    // The objective client socket.
    int          _client_socket;
} http_response; // What will we use to format a response



typedef struct {
	// The method of the request. See includes
    http_method   method;
	// The route.
//    uri           route;
	std::string	  route;
	// Will store the raw body for parsing
    std::string   body_raw;
	// Buffer of the entire request
    char*         buffer;
} http_request;

typedef struct {
	// Will be implemented by the user.
    void receive(http_request&, http_response&);
} http_receiver;

typedef struct {
    int port;
    int _socket;
    struct sockaddr_in _socket_addr;
    http_receiver receiver;
    int* clients;
} http_server;

static
http_server server;

// Create the server
void create_server_and_start(const char*);

void reset_clients(int*, int);
int next(int*, int, int);
void handle_response(int, int);
void parse_uri(http_request&);

// Utils to do
const std::string get_header(http_request&, const char*);
const std::string get_body(http_request&);
const std::string get_query_param(http_request&, const char*);
// don't know if i will complete
const std::string get_uri_param(http_request&, const char*);

inline const http_method
get_method(char*);

#endif /* http_hpp */
