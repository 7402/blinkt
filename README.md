## blinkt

---

### Description

This command-line tool controls the LEDs on the Blinkt! add-on board for the Raspberry Pi. All functions are available
from the command line; no additional programming needed. Can be used within shell scripts.

### Usage

To clear the LEDs to their default state, type:
**blinkt clear**

To turn on all the LEDs blue, type:
**blinkt blue**

The named colors available are **red coral orange gold yellow lime green aqua blue purple pink white**.

Pixels are numbered 0-7 from left to right. To turn pixel 1 yellow, type:
**blinkt p1 yellow**

To specify color by RGB, type, e.g.:
**blinkt rgb 10 0 50**

To turn on the left 4 LEDs blue and right 4 LEDs red, type:
**blinkt 11110000 blue**
**blinkt 00001111 red**

To change the brightness of all pixels to 12, type:
**blinkt bright 12**

For more information, see man page.

### Build and install

* Raspbian Stretch

```
sudo apt-get install git
sudo apt-get install wiringpi
git clone git://github.com/7402/blinkt
cd blinkt
make
sudo make install
```

### Notes

If you are viewing your Blinkt! board upside-down from its standard orientation, i.e., the letters "BLINKT!" on the circuit board are upside down, type this command to adjust the numbering direction:
**blinkt right**

### License

BSD
