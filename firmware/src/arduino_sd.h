#pragma once

// Starts SPI and tries to mount the SD card at 4 MHz, 1 MHz and 400 kHz.
// Prints the speed that worked, or a failure line. Safe to call repeatedly.
bool mountSd();

// Same, but formats the card as FAT if it has no filesystem (exFAT/blank).
// Destructive if the card has an unreadable/corrupt filesystem: call only
// from an explicit "format" command or a dedicated diagnostic.
bool formatSd();
