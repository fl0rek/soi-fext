#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
//strtok
#include <string.h>

#include <getopt.h>

#include <fext.h>

#include "cp.h"
#include "helpers.h"

int main(int argc, char** argv) {
        short touch_mode = 0;
        short mkdir_mode = 0;

        char *partname;

        int option = 0;
        while((option = getopt(argc, argv, "m:f:")) != -1) {
                switch(option) {
                        printf("%c", option);
                        case 'm':
                                printf("%s", optarg);
                                if(!strcmp(optarg, "touch"))
                                        touch_mode = 1;
                                else if(!strcmp(optarg, "mkdir"))
                                        mkdir_mode = 1;
                                break;
                        case 'f':
                                partname = optarg;
                                break;
                }
        }



        if(touch_mode || mkdir_mode) {
                printf("no cp\n");
                if(!partname)
                        return -1;
                while(optind != argc) {
                        char* filename = argv[optind++];
                        char* name;

                        unsigned last_slash = 0;
                        unsigned it = 0;
                        while(filename[++it])
                                if(filename[it] == '/')
                                        last_slash = it;

                        filename[last_slash] = 0;
                        name = &filename[last_slash+1];
                        void *fs = fext_init(partname);
                        uint32_t parent_dir = f_open(fs, filename);

                        if(mkdir_mode) {
                                f_mkdir(fs, parent_dir, name);
                        } else {
                                f_touch(fs, parent_dir, name);
                        }
                }
        } else if(optind +2 == argc) {
                short fext_source,
                        fext_dest;

                void* buffer = malloc(512 *sizeof(char));

                char *src, *dest;
                void *fs_src, *fs_dest;
                uint32_t ffile_src, ffile_dest;
                FILE *nfile_src, *nfile_dest;

                src = argv[optind++];
                dest = argv[optind++];

                char *prefix, *postfix;

                prefix = strtok(src, ":");
                postfix = strtok(0, ":");
                if(postfix == 0) {
                        fext_source = 0;
                        nfile_src = fopen(prefix, "r");
                } else {
                        fs_src = fext_init(prefix);
                        ffile_src = f_open(fs_src, postfix);
                        fext_source = 1;
                }

                prefix = strtok(dest, ":");
                postfix = strtok(0, ":");
                if(postfix == 0) {
                        fext_dest = 0;
                        nfile_dest = fopen(prefix, "w");
                } else {
                        fs_dest = fext_init(prefix);
                        ffile_dest = f_open(fs_dest, postfix);
                        fext_dest = 1;
                }

                size_t read;
                size_t read_offset = 0,
                        write_offset = 0;
                do {
                        if(fext_source) {
                                read_offset += read = read_inode(fs_src, ffile_src, 512, read_offset, buffer);
				printf("read: %s, %d", buffer, read);
                        } else {
                                read = fread(buffer, sizeof(char), 512, nfile_src);
                        }

                        if(fext_dest) {
                                write_offset += write_inode(fs_dest, ffile_dest, read, write_offset, buffer);
                        } else {
                                fwrite(buffer, sizeof(char), read, nfile_dest);
                        }
                } while(read == 512);
        } else
                return -1;
        return 0;
}
