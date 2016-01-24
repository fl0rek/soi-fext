#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <getopt.h>

#include <fext.h>

#include "df.h"
#include "helpers.h"

void pretty_print_block(f_superblock* ptr, uint32_t block) {
        char* map = get_block(ptr, block);
        uint32_t left = ptr->block_size;

        unsigned count = 0;
        while(left) {
                printf("%c%c%c%c%c%c%c%c ",
                        *map & 1 << 0 ? 'x' : '-',
                        *map & 1 << 1 ? 'x' : '-',
                        *map & 1 << 2 ? 'x' : '-',
                        *map & 1 << 3 ? 'x' : '-',
                        *map & 1 << 4 ? 'x' : '-',
                        *map & 1 << 5 ? 'x' : '-',
                        *map & 1 << 6 ? 'x' : '-',
                        *map & 1 << 7 ? 'x' : '-'
                );
                if(!(++count % 8))
                        printf("\n");
                left -= sizeof(*map);
                map++;
        }
}

int main(int argc, char** argv) {
        short print_inode_map = 0;
        short print_block_map = 0;

        int option = 0;
        while((option = getopt(argc, argv, "ib")) != -1) {
                switch(option) {
                        case 'i':
                                print_inode_map = 1;
                                break;
                        case 'b':
                                print_block_map = 1;
                                break;
                }
        }

        while(optind != argc) {
                char* filename = argv[optind++];
                printf("%s:\n", filename);

                f_superblock* p = fext_init(filename);
                if(print_block_map) {
                        unsigned block = p->block_bitmap_block;
                        for(; block < p->inode_bitmap_block; block++)
                                pretty_print_block(p, block);
                }
                if(print_inode_map) {
                        unsigned block = p->inode_bitmap_block;
                        for(; block < p->inode_table_block; block++)
                                pretty_print_block(p, block);
                }
        }

}
