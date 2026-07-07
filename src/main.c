#include <stdint.h>
#include <stdio.h>
#include "disk_interface.h"
#include "filesystem_interface.h"
#include <string.h>
#include <stdlib.h>
int main(void)
{
    FILE *disk = fopen("disk.img", "r+b");
    int ret = format_disk(disk,2.5e+8);
    ret = mount(disk);
    


    flush_master_superblock();
    char *path = malloc(100);
    strcpy(path,"/dir1/");
    // create_directory("dir1");
    create_nested_directory(path);
    free(path);
    path = malloc(100);
    strcpy(path,"/dir1/file.txt");
    printf("Creating nested file %s\n",path);
    create_nested_file(path);
    char *data_to_write = "Hello from TUF_FS";
    int write_ret = write_file_data(path, (uint8_t *)data_to_write, strlen(data_to_write));
    printf("Write return value %d\n",write_ret);
    return 0;
}