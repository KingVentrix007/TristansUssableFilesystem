#ifndef __DISK_INTERFACE_H
#define __DISK_INTERFACE_H
#include <stdio.h>
#include "stdint.h"

void read_sector(FILE *disk, uint32_t sector, void *buffer);
void write_sector(FILE *disk, uint32_t sector, void *buffer);
int create_file(char filename[10]);
#endif