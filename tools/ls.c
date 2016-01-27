#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <getopt.h>

#include <fext.h>

#include "ls.h"
#include "helpers.h"

int main(int argc, char** argv) {
        short long_opts = 0;

        char* filename;
        char* partname = 0;

        int option = 0;
        while((option = getopt(argc, argv, "lf:")) != -1) {
                switch(option) {
                        case 'l':
                                long_opts = 1;
                                break;
                        case 'f':
                                partname = optarg;
                                break;
                }
        }

        if(!partname)
                return -1;

        void* p = fext_init(partname);

        while(optind != argc) {
                filename = argv[optind++];

                uint32_t dir = f_open(p, filename);

                if(dir == -1) {
                        printf("Not a directory\n");
                        dir = 0;
                }
                uint32_t dir_count = ls(p, dir, 0, 0);
                printf("%s: %u\n", filename, dir_count);

                for(uint32_t i= 0; i < dir_count; i++) {
                        f_directory_entry fde;
                        ls(p, dir, i, &fde);
                        if(long_opts) {
                                f_stat_struct stat;
                                f_stat(p, fde.inode, &stat);
                                printf("%c" "%c%c%c" "%c%c%c" "%c%c%c" " % 6u % 6u % 6u %s\n",
                                        (stat.perm & MODE_MASK) ? 'd' : '-',
                                        (stat.perm & 0x0100) ? 'r' : '-',
                                        (stat.perm & 0x0200) ? 'w' : '-',
                                        (stat.perm & 0x0400) ? 'x' : '-',
                                        (stat.perm & 0x0010) ? 'r' : '-',
                                        (stat.perm & 0x0020) ? 'w' : '-',
                                        (stat.perm & 0x0040) ? 'x' : '-',
                                        (stat.perm & 0x0001) ? 'r' : '-',
                                        (stat.perm & 0x0002) ? 'w' : '-',
                                        (stat.perm & 0x0004) ? 'x' : '-',
                                        stat.uid, stat.gid, stat.size, fde.name
                                );
                        } else
                                printf("%s ", fde.name);
                }
                if(!long_opts)
                        printf("\n");
        }
}
