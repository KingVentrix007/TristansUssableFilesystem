#include <stdio.h>
#include "filesystem_formats.h"
#include "stdint.h"
#include "disk_interface.h"


void read_sector(FILE *disk, uint32_t sector, void *buffer)
{
    fseek(disk, sector * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}
void write_sector(FILE *disk, uint32_t sector, void *buffer)
{
    fseek(disk, sector * BLOCK_SIZE, SEEK_SET);
    fwrite(buffer, BLOCK_SIZE, 1, disk);
}

