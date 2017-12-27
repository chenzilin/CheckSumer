#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define BUFFER_SIZE 30*1024*1024

uint64_t  sub_checksum32(unsigned char* buf, size_t len)
{
    uint64_t sum = 0;
    size_t index = 0;

    while (index < len) {
        sum += (uint64_t)buf[index++];
    }

    return sum;
}

int main (int argc, char *argv[])
{
    FILE *fp = NULL;
    unsigned char *buffer = NULL;
    size_t buffer_len = 0;
    uint64_t checksum32 = 0;

    if (2 != argc) {
        printf("Usage: checksum your-file-path\n");
        return -1;
    }

    if (NULL == (fp = fopen(argv[1], "rb"))) {
        printf("Failed to open file: %s\n", argv[1]);
        return -1;
    }

    if (NULL == (buffer = (unsigned char *)malloc(BUFFER_SIZE))) {
        printf("Failed malloc enough buffer size!\n");
        return -1;
    }

    while (!feof(fp)) {
        if (0 != (buffer_len = fread(buffer, 1, BUFFER_SIZE, fp))) {
            checksum32 += sub_checksum32(buffer, buffer_len);
        }
    }

    checksum32 %= 0x100000000;

    printf("%08X\n", (unsigned int)checksum32);


    fclose(fp);

    return 0;
}
