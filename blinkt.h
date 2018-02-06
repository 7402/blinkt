//
// blinkt.h
// blinkt
//
// Copyright (C) 2018 Michael Budiansky. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this list of conditions
// and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions
// and the following disclaimer in the documentation and/or other materials provided with the
// distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef blinkt_h
#define blinkt_h

#include <stdbool.h>
#include <stdint.h>

#define NUM_PIXELS 8

struct Pixel {
    uint8_t brightness;
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};
typedef struct Pixel Pixel;

struct Flags {
    bool left_to_right;
    bool leds_on;
    bool holding;
    bool binary_on;
    uint8_t binary_mask;
};
typedef struct Flags Flags;

// init functions
void init_gpio(void);
void init_state(Flags *flags, Pixel pixels[NUM_PIXELS]);

// file functions
void read_state_file(const char *path, Flags *flags, Pixel pixels[NUM_PIXELS]);
void write_state_file(const char *path, Flags flags, Pixel pixels[NUM_PIXELS]);

// high-level write pixels
void write_to_blinkt(Flags flags, Pixel pixels[NUM_PIXELS]);

// utility functions
uint8_t parse_num(const char *arg, int default_base);
void clear_pixels(Pixel pixels[NUM_PIXELS]);
uint8_t swap_bits(uint8_t x);
void sleep_msec(int msec);

// low level GPIO functions
void send_byte(uint8_t x);
void send_clocks(int count);

#endif /* blinkt_h */
