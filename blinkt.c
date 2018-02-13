//
// blinkt.c
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "blinkt.h"

// buffer size for file I/O
#define LINE_SIZE 256

#ifdef __linux__
#include <wiringPi.h>

// GPIO pin assignments
#define DAT 23
#define CLK 24

bool data_state;    // current state of data pin
#endif

// intialize GPIO library and pins
void init_gpio(void)
{
#ifdef __linux__
    wiringPiSetupGpio();
    pinMode(DAT, OUTPUT);
    pinMode(CLK, OUTPUT);

    digitalWrite(DAT, 0);
    data_state = false;
#endif
}

// initialize flags and pixels
void init_state(Flags *flags, Pixel pixels[NUM_PIXELS])
{
    flags->left_to_right = true;
    flags->leds_on = true;
    flags->holding = false;
    flags->binary_on = false;
    flags->binary_mask = 0xFF;
    clear_pixels(pixels);
}

// read flags and pixels from state file, if it exists
void read_state_file(const char *path, Flags *flags, Pixel pixels[NUM_PIXELS])
{
    bool error = false;
    FILE *file = fopen(path, "r");

    if (file != NULL) {
        char line[LINE_SIZE];
        int k;

        error = fgets(line, LINE_SIZE, file) == NULL;
        if (!error) flags->left_to_right = strcmp(line, "left\n") == 0;

        if (!error) error = fgets(line, LINE_SIZE, file) == NULL;
        if (!error) flags->leds_on = strcmp(line, "on\n") == 0;

        if (!error) error = fgets(line, LINE_SIZE, file) == NULL;
        if (!error) flags->holding = strcmp(line, "on\n") == 0;

        if (!error) error = fgets(line, LINE_SIZE, file) == NULL;
        if (!error) flags->binary_on = strcmp(line, "on\n") == 0;

        if (!error) error = fscanf(file, "%hhd", &flags->binary_mask) != 1;

        for (k = 0; k < NUM_PIXELS && !error; k++) {
            error = fscanf(file, "%hhd %hhd %hhd %hhd",
                           &pixels[k].brightness,
                           &pixels[k].blue,
                           &pixels[k].green,
                           &pixels[k].red ) != 4;
        }

        fclose(file);
        file = NULL;
    }

    if (error) {
        fprintf(stderr, "Error reading file %s\n", path);
        // don't leave state half-read
        init_state(flags, pixels);
    }
}

// write flags and pixels to state file
void write_state_file(const char *path, Flags flags, Pixel pixels[NUM_PIXELS])
{
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        fprintf(stderr, "Unable to open ~/.blink for writing\n");

    } else {
        int k;

        fprintf(file, "%s\n", flags.left_to_right ? "left" : "right");
        fprintf(file, "%s\n", flags.leds_on ? "on" : "off");
        fprintf(file, "%s\n", flags.holding ? "on" : "off");
        fprintf(file, "%s\n", flags.binary_on ? "on" : "off");
        fprintf(file, "%d\n", flags.binary_mask);
        for (k = 0; k < NUM_PIXELS; k++) {
            fprintf(file, "%d %d %d %d\n",
                    pixels[k].brightness,
                    pixels[k].blue,
                    pixels[k].green,
                    pixels[k].red);
        }

        fclose(file);
        file = NULL;
    }
}

// write pixel data to GPIO lines
void write_to_blinkt(Flags flags, Pixel pixels[NUM_PIXELS])
{
    int k;

    // start frame
    send_clocks(32);

    for (k = 0; k < NUM_PIXELS; k++) {
        uint8_t brightness = (flags.leds_on ? pixels[k].brightness : 0) | 0b11100000;
        if (flags.binary_on && ((flags.binary_mask & (1 << k)) == 0)) brightness = 0b11100000;
        send_byte(brightness);
        send_byte(pixels[k].blue);
        send_byte(pixels[k].green);
        send_byte(pixels[k].red);
    }

    // end frame
    send_clocks(36);
}

bool is_num_arg(const char *arg)
{
    bool result = false;

    if (isdigit(arg[0])) {
        result = true;

    } else if (strlen(arg) > 1) {
        char c = arg[1];
        switch (arg[0]) {
            case 'b':
                result = c == '0' || c == '1';
                break;
            case 'd':
                result = isdigit(c);
                break;
            case 'x':
                result = isdigit(c) || (c >= 'A' && c <= 'F') ||  (c >= 'a' && c <= 'f');
                break;
            case 'p':
                result = c >= '0' && c <= '7';
                break;
        }
    }

    return result;
}

// convert string to number using default base.
// if string begins with a letter and is followed by a number, use alternate base or handling:
// b - binary
// d - decimal
// x - hexadecimal
// p - pixel number; p0 = 1, p1 = 2, p2 = 4, p3 = 8, p4 = 16, p5 = 32, p6 = 64, p7 = 128
uint8_t parse_num(const char *arg, int default_base)
{
    const char *p = arg;
    int base = default_base;
    bool pixel = false;
    long result = 0;

    if (*p == 'b') {
        base = 2;
        p++;

    } else if (*p == 'd') {
        base = 10;
        p++;

    } else if (*p == 'x') {
        base = 16;
        p++;

    } else if (*p == 'p') {
        pixel = true;
        base = 10;
        p++;
    }

    result = strtol(p, NULL, base);
    if (result < 0) result = 0;

    if (pixel) result = 1 << (7 - result);

    return(uint8_t)result;
}

// swap bit order in an 8-bit byte
uint8_t swap_bits(uint8_t x)
{
    // swap bit order
    return
    ((x & 0b10000000) >> 7) |
    ((x & 0b01000000) >> 5) |
    ((x & 0b00100000) >> 3) |
    ((x & 0b00010000) >> 1) |
    ((x & 0b00001000) << 1) |
    ((x & 0b00000100) << 3) |
    ((x & 0b00000010) << 5) |
    ((x & 0b00000001) << 7);

}

// set all pixels to default values
void clear_pixels(Pixel pixels[NUM_PIXELS])
{
    int k;
    for (k = 0; k < NUM_PIXELS; k++) {
        pixels[k].brightness = 7;
        pixels[k].blue = 0;
        pixels[k].green = 0;
        pixels[k].red = 0;
    }
}

// sleep for specified milliseconds
void sleep_msec(int msec)
{
#ifdef __linux__
    delay(msec);
#else
    usleep(1000L * msec);
#endif
}

// send one byte to GPIO pins
void send_byte(uint8_t x)
{
#ifdef __linux__
    int i;
    for (i = 0; i < 8; i++) {
        bool new_state = (x & 0b10000000) != 0;
        if (data_state != new_state) {
            // only send to data pin if value has changed
            digitalWrite(DAT, new_state);
            data_state = new_state;
        }

        digitalWrite(CLK, 1);
        digitalWrite(CLK, 0);
        x = x << 1;
    }
#endif
}

// send specified number of repeated transitions to clock pin
void send_clocks(int count)
{
#ifdef __linux__
    int i;
    digitalWrite(DAT, 0);
    for (i = 0; i < count; i++) {
        digitalWrite(CLK, 1);
        digitalWrite(CLK, 0);
    }
#endif
}
