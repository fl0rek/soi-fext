#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <getopt.h>

#include <fext.h>

#include "ls.h"
#include "helpers.h"

int main(int argc, char** argv) {

        char* filename;
        char* partname = 0;

        int option = 0;
        while((option = getopt(argc, argv, "f:")) != -1) {
                switch(option) {
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

                uint32_t to_remove = f_open(p, filename);
                printf("%s: %u\n", filename, to_remove);

                if(to_remove == -1) {
                        printf("Not such file or directory %s\n", filename);
                        continue;
                }
                int32_t dir_count = ls(p, to_remove, 0, 0);

                if(dir_count > 0) {
                        printf("Connot remove '%s': Is not empty\n", filename);
                        continue;
                }
                f_unlink(p, to_remove); // is file empty directory

                unsigned last_slash = 0;
                unsigned it = 0;
                while(filename[++it])
                        if(filename[it] == '/')
                                last_slash = it;

                filename[last_slash] = 0;
                //name = &filename[last_slash+1];
                uint32_t parent_dir = f_open(p, filename);

                f_remove_directory_entry(p, parent_dir, to_remove);
        }
}
