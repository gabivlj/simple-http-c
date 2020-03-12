//
//  utils.h
//  train
//
//  Created by Gabriel Villalonga Simón on 10/03/2020.
//  Copyright © 2020 Gabriel Villalonga Simón. All rights reserved.
//

#ifndef utils_h
#define utils_h

//typedef uint8_t uint8;

char* itoa(int val, int base){
    
    static char buf[32] = {0};
    
    int i = 30;
    
    for(; val && i ; --i, val /= base)
    
        buf[i] = "0123456789abcdef"[val % base];
    
    return &buf[i+1];
    
}

#endif /* utils_h */
