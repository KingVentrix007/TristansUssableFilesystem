#include <stdint.h>
#include <stdio.h>
#include "disk_interface.h"
#include "filesystem_interface.h"
#include <string.h>
int main(void)
{
    FILE *disk = fopen("disk.img", "r+b");
    int ret = format_disk(disk,2.5e+8);
    ret = mount(disk);
    create_file("test");
    char *write = "Hello, World!";
    int stat = write_file_data("test", write, strlen(write));
    printf("Write status = %d\n",stat);

    create_file("test2");
    char *write2 = "Hello, World! This is a test of the filesystem.";
    int stat2 = write_file_data("test2", write2, strlen(write2));
    printf("Write status = %d\n",stat2);
    return 0;
}