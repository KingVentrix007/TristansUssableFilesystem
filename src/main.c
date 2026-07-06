#include <stdint.h>
#include <stdio.h>
#include "disk_interface.h"
#include "filesystem_interface.h"
#include <string.h>
#include <stdlib.h>
int main(void)
{
    FILE *disk = fopen("disk.img", "r+b");
    // int ret = format_disk(disk,2.5e+8);
    int ret = mount(disk);
    // create_file("test");
    // char *write = "Hello, World!";
    // int stat = write_file_data("test", write, strlen(write));
    // printf("Write status = %d\n",stat);

    // create_file("test2");
    // char *write2 = "Hello, World! This is a test of the filesystem.";
    // int stat2 = write_file_data("test2", write2, strlen(write2));
    // printf("Write status = %d\n",stat2);

    uint8_t read[100];
    uint32_t size;
    int stat3 = read_file_data("test2", read, &size);
    // read[size+1] = '\0';
    printf("Read status = %d\n",stat3);
    printf("Read size = %u\n",size);
    printf("Read data = %s\n", read);


    flush_master_superblock();
    char *path = malloc(100);
    strcpy(path,"/dir1/dir2/");
    create_directory("dir1");
    create_nested_directory(path);
    return 0;
}