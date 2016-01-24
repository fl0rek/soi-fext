#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <getopt.h>

#include "mkfs.h"
#include "helpers.h"

int main(int argc, char** argv) {
        size_t part = 1024* 1024* 1024; //10485760;
        uint32_t inode_number = 128;
        uint32_t block_size = 128;

        char* filename;

        int option = 0;
        while((option = getopt(argc, argv, "i:b:s:")) != -1) {
                switch(option) {
                        case 'i':
                                inode_number = atoi(optarg);
                                break;
                        case 'b':
                                block_size = atoi(optarg);
                                break;
                        case 's':
                                part = atoi(optarg);
                                break;
                }
        }

        if(optind == argc)
                return -1;
        filename = argv[opterr];

        FILE* fh = fopen(filename, "w+");
        fseek(fh, part, SEEK_SET);
        fputc('\0', fh);
        fclose(fh);

        void* p = fext_init(filename);

        mkfs(p, part, inode_number, block_size);

        f_mkdir(p, 0, "foo");
        f_mkdir(p, 0, "foo0");

        f_tree(p);
}
