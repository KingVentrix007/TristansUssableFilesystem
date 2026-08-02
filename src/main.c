#include <stddef.h>
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
    strcpy(path,"/dir1/fileCat.txt");
    printf("Creating nested file %s\n",path);
    create_nested_file(path);
    char *data_to_write = "This is a secure text message";
    int write_ret = write_nested_file_data(path, (uint8_t *)data_to_write, strlen(data_to_write));
    printf("Write return value %d\n",write_ret);
    
    free(path);


    strcpy(path,"/dir1/TestingFile.txt");
    printf("Creating nested file %s\n",path);
    create_nested_file(path);
    data_to_write = "Tristan's definitely Usable File System";
    write_ret = write_nested_file_data(path, (uint8_t *)data_to_write, strlen(data_to_write));
    printf("Write return value %d\n",write_ret);


    printf("\n\nReading data\n\n");
    uint8_t *read_data = NULL;
    size_t data_size = read_nested_file_data(path, &read_data);
    printf("data_size == %zu\n",data_size);
    char *text = (char *)read_data;
    printf("Read data = %s\n",text);
    for (size_t i=0; i<data_size; i++) {
        printf("%c",text[i]);
    }
    printf("\n");
    return 0;
}