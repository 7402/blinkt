//
// main.c
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
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blinkt.h"
#include "text.h"

struct Color {
    const char *name;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
typedef struct Color Color;

#define FILE_PATH "/usr/local/share/blinkt"

int main(int argc, const char * argv[]) {
    Pixel pixels[NUM_PIXELS];
    Flags flags;
    int k;
    int next_arg = 1;
    uint8_t select_mask = 0xFF; // default is to change all pixels
    bool state_changed = true;

    // intialize GPIO library and pins
    init_gpio();

    // initialize data structures
    init_state(&flags, pixels);

    // read state file if present; OK if does not exist
    read_state_file(FILE_PATH, &flags, pixels);

    // read selection option, if present
    if (next_arg < argc && is_num_arg(argv[next_arg])) {
        select_mask = parse_num(argv[next_arg], 2);
        if (flags.left_to_right) select_mask = swap_bits(select_mask);
        next_arg++;
    }

    // test next arg
    if (next_arg < argc) {
        if (strcmp(argv[next_arg], "off") == 0) {
            flags.leds_on = false;

        } else if (strcmp(argv[next_arg], "on") == 0) {
            flags.leds_on = true;
            flags.holding = false;

        } else if (strcmp(argv[next_arg], "left") == 0) {
            flags.left_to_right = true;

        } else if (strcmp(argv[next_arg], "right") == 0) {
            flags.left_to_right = false;

        } else if (strcmp(argv[next_arg], "hold") == 0) {
            flags.holding = true;

        } else if (strcmp(argv[next_arg], "show") == 0) {
            flags.holding = false;

        } else if (strcmp(argv[next_arg], "clear") == 0) {
            // reset everything except left_to_right flag
            flags.leds_on = true;
            flags.holding = false;
            flags.binary_on = false;
            flags.binary_mask = 0xFF;
            clear_pixels(pixels);

        } else if (strcmp(argv[next_arg], "bright") == 0) {
            if (++next_arg < argc) {
                int brightness = parse_num(argv[next_arg], 10);

                if (brightness < 0 || brightness > 31) {
                    fprintf(stderr, "Brightness must be 0 to 31\n");

                } else {
                    for (k = 0; k < NUM_PIXELS; k++) {
                        if ((select_mask & (1 << k)) != 0) pixels[k].brightness = brightness;
                    }
                }
            }

        } else if (strcmp(argv[next_arg], "rgb") == 0) {
            int red = 0;
            int green = 0;
            int blue = 0;

            if (++next_arg < argc) red = parse_num(argv[next_arg], 10);
            if (++next_arg < argc) green = parse_num(argv[next_arg], 10);
            if (++next_arg < argc) blue = parse_num(argv[next_arg], 10);

            if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
                fprintf(stderr, "red green blue values must be 0 to 255\n");

            } else {
                for (k = 0; k < NUM_PIXELS; k++) {
                    if ((select_mask & (1 << k)) != 0) {
                        pixels[k].red = red;
                        pixels[k].green = green;
                        pixels[k].blue = blue;
                    }
                }
            }

        } else if (strcmp(argv[next_arg], "binary") == 0) {
            if (++next_arg < argc) {
                if (strcmp(argv[next_arg], "off") == 0) {
                    flags.binary_on = false;

                } else {
                    flags.binary_on = true;
                    flags.binary_mask = parse_num(argv[next_arg], 10);
                    if (flags.left_to_right) flags.binary_mask = swap_bits(flags.binary_mask);
                    flags.binary_mask |= ~select_mask;
                }
            }

        } else if (strcmp(argv[next_arg], "delay") == 0) {
            if (++next_arg < argc) {
                int msec = atoi(argv[next_arg]);
                sleep_msec(msec);
            }

            state_changed = false;

        } else if (strcmp(argv[next_arg], "rotate") == 0) {
            if (++next_arg < argc) {
                const char *option = argv[next_arg];
                if (flags.binary_on) {
                    // if in binary mode, rotate binary mask instead of pixel settings
                    if (strcmp(option, "in") == 0) {
                        uint8_t left_mask = flags.binary_mask >> 4;
                        uint8_t right_mask = flags.binary_mask & 0xF;

                        left_mask = ((left_mask >> 1) | (left_mask << 3)) & 0xF;;
                        right_mask = ((right_mask << 1) | (right_mask >> 3)) & 0xF;

                        flags.binary_mask = (left_mask << 4) | right_mask;

                    } else if (strcmp(option, "out") == 0) {
                        uint8_t left_mask = flags.binary_mask >> 4;
                        uint8_t right_mask = flags.binary_mask & 0xF;

                        left_mask = ((left_mask << 1) | (left_mask >> 3)) & 0xF;
                        right_mask = ((right_mask >> 1) | (right_mask << 3)) & 0xF;;

                        flags.binary_mask = (left_mask << 4) | right_mask;

                    } else if (strcmp(option, "left") == 0) {
                        flags.binary_mask = (flags.binary_mask << 1) | (flags.binary_mask >> 7);

                    } else if (strcmp(option, "right") == 0) {
                        flags.binary_mask = (flags.binary_mask >> 1) | (flags.binary_mask << 7);
                    }

                } else {
                    // rotate pixel settings
                    int shift = 0;
                    if (strcmp(option, "in") == 0) {
                        // from outside to center
                        Pixel temp = pixels[0];
                        pixels[0] = pixels[3];
                        pixels[3] = pixels[2];
                        pixels[2] = pixels[1];
                        pixels[1] = temp;

                        temp = pixels[7];
                        pixels[7] = pixels[4];
                        pixels[4] = pixels[5];
                        pixels[5] = pixels[6];
                        pixels[6] = temp;

                    } else if (strcmp(option, "out") == 0) {
                        // from center to outside
                        Pixel temp = pixels[0];
                        pixels[0] = pixels[1];
                        pixels[1] = pixels[2];
                        pixels[2] = pixels[3];
                        pixels[3] = temp;

                        temp = pixels[7];
                        pixels[7] = pixels[6];
                        pixels[6] = pixels[5];
                        pixels[5] = pixels[4];
                        pixels[4] = temp;

                    } else if (strcmp(option, "left") == 0) {
                        shift = 1;

                    } else if (strcmp(option, "right") == 0) {
                        shift = -1;
                    }

                    if (flags.left_to_right) shift *= -1;

                    if (shift == -1) {
                        Pixel temp = pixels[0];
                        for (k = 0; k < NUM_PIXELS - 1; k++) {
                            pixels[k] = pixels[k + 1];
                        }
                        pixels[7] = temp;

                    } else if (shift == 1) {
                        Pixel temp = pixels[7];
                        for (k = NUM_PIXELS - 2; k >= 0 ; k--) {
                            pixels[k + 1] = pixels[k];
                        }
                        pixels[0] = temp;
                    }
                }
            }

        } else if (strcmp(argv[next_arg], "state") == 0) {
            // print current state
            printf("Numbering: %s\n", flags.left_to_right ? "left to right" : "right to left");
            printf("LEDs: %s\n", flags.leds_on ? "on" : "off");
            printf("Holding: %s\n", flags.holding ? "on" : "off");
            printf("Binary: %s\n", flags.binary_on ? "on" : "off");
            printf("Binary mask: %d\n", flags.binary_mask);
            printf("\n");
            printf("# brightness red green blue\n");
            for (k = 0; k < NUM_PIXELS; k++) {
                printf("%d      %2d    %3d  %3d  %3d\n",
                       k,
                       pixels[k].brightness,
                       pixels[k].red,
                       pixels[k].green,
                       pixels[k].blue);
            }

            state_changed = false;

        } else if (strcmp(argv[next_arg], "version") == 0) {
            version();
            state_changed = false;

        } else if (strcmp(argv[next_arg], "help") == 0) {
            usage();
            state_changed = false;

        } else if (strcmp(argv[next_arg], "man-page") == 0) {
            man_page_source();
            state_changed = false;

        } else if (strcmp(argv[next_arg], "license") == 0) {
            license();
            state_changed = false;

        } else {
            // set RGB by named color
            const char *color = argv[next_arg];
            bool use_named_color = false;
            Color colors[] = {
                {"red",       255,      0,      0},
                {"coral",     255,      8,      0},
                {"orange",    255,     20,      0},
                {"gold",      255,     60,      0},
                {"yellow",    255,     88,      0},
                {"lime",      160,    255,      0},
                {"green",       0,    255,      0},
                {"aqua",        0,     80,     24},
                {"blue",        0,      0,    255},
                {"purple",     72,      0,    120},
                {"pink",      220,      0,     40},
                {"white",     255,    255,    255},
                {"black",       0,      0,      0},
            };

            int num_colors = sizeof(colors) / sizeof(Color);
            int red = 0;
            int green = 0;
            int blue = 0;

            for (k = 0; k < num_colors && !use_named_color; k++) {
                use_named_color = strcmp(color, colors[k].name) == 0;
                red = colors[k].red;
                green = colors[k].green;
                blue = colors[k].blue;
            }

            if (use_named_color) {
                for (k = 0; k < NUM_PIXELS; k++) {
                    if ((select_mask & (1 << k)) != 0) {
                        pixels[k].red = red;
                        pixels[k].green = green;
                        pixels[k].blue = blue;
                    }
                }

            } else {
                fprintf(stderr, "Unknown option\n");
                state_changed = false;
            }
        }

    } else {
        // if no options, print help
        usage();
        state_changed = false;
    }

    if (state_changed && !flags.holding) {
        write_to_blinkt(flags, pixels);
    }

    if (state_changed) {
        write_state_file(FILE_PATH, flags, pixels);
    }

    return 0;
}


