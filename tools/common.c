//stat
#include <sys/stat.h>
//mmap
#include <sys/mman.h>
// open flags
#include <fcntl.h>
// exit
#include <stdlib.h>
// perror
#include <stdio.h>

void* fext_init(char* filename) {
        struct stat st;
        stat(filename, &st);

        int fd = open(filename, O_RDWR | O_CREAT, 0600);
        //void* p = malloc(part);
        void* p = mmap(0, st.st_size, PROT_WRITE |PROT_READ, MAP_SHARED|MAP_NORESERVE, fd, 0);
        if(p == -1)
                perror("mmap fail"), exit(-1);
        return p;
}
