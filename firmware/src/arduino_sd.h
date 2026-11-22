#pragma once

// Starts SPI and tries to mount the SD card at 4 MHz, 1 MHz and 400 kHz.
// Prints the speed that worked, or a failure line. Safe to call repeatedly.
bool mountSd();
