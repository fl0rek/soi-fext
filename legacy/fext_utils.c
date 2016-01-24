#include "fext.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <limits.h>


void mkfs(void* ptr, size_t size, uint32_t group_block_num, uint32_t group_inode_num, uint32_t block_size) {

        memset(ptr, 0, size);

        size_t group_body_len = block_size * group_block_num;
        size_t group_header_len;

        f_superblock sb;
        sb.header = FEXT_MAGIC;
        sb.magic = 0xCDCDCDDD;
        sb.group_block_number = group_block_num;
        sb.group_inode_number = group_block_num;
        sb.block_size = block_size;

        size_t block_bitmap_len = group_block_num * sizeof(uint32_t) / 32;
        size_t inodes_bitmap_len = group_inode_num * sizeof(uint32_t) / 32;
        size_t inodes_table_len = group_inode_num * sizeof(f_inode);

        size_t payload_size = block_bitmap_len + inodes_bitmap_len + inodes_table_len;
        printf("block bit: %u\ninode bit: %u\ninode tab:%u\n", block_bitmap_len, inodes_bitmap_len, inodes_table_len);

        f_group_header* empty_gb = (f_group_header*) malloc(sizeof(f_group_header) + payload_size);

        //empty_gb->gh_inode_table = block_bitmap_blocks + inodes_bitmap_blocks;
        empty_gb->magic = 0xCDCDCDCD;
        empty_gb->magic_end = 0xCDCDCDCD;
        empty_gb->gh_free_blocks_count = group_block_num;
        empty_gb->gh_free_inodes_count = group_inode_num;
        empty_gb->gh_used_dirs_count = 0;

        group_header_len = empty_gb->gh_hsize = sizeof(f_group_header) + payload_size;

        printf("%u, %u\n", group_header_len, payload_size);

        empty_gb->gh_block_bitmap_offset = sizeof(f_group_header);
        empty_gb->gh_inode_bitmap_offset = sizeof(f_group_header) + block_bitmap_len;
        empty_gb->gh_inode_table_offset =  sizeof(f_group_header) + block_bitmap_len + inodes_bitmap_len;


        uint32_t full_blocks = size / (group_body_len + group_header_len);
        sb.groups_num = full_blocks;
        sb.group_size = group_header_len + group_body_len;

        void* p = ptr;
        memcpy(p, &sb, sizeof(sb));
        p += sizeof(sb);
        uint32_t i;
        for(i = 0; i < full_blocks; i++) {
                memcpy(p, empty_gb, sizeof(*empty_gb));
                p += group_header_len;
                memcpy(p, &empty_gb->magic, sizeof(uint32_t));
                //p+= group_body_len;
                uint32_t j;
                for(j = 0; j < group_block_num; j++) {
                        memcpy(p, &empty_gb->magic, sizeof(uint32_t));
                        p += block_size;
                } // block_size * group_block_num;
        }
}
/*
enum { BITS_PER_WORD = sizeof(uint32_t) *CHAR_BIT};
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)

void set_bit(uint32_t *bm, unsigned n) {
        bm[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}
void clear_bit(uint32_t *bm, unsigned n) {
        bm[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}
int get_bit(uint32_t *bm, unsigned n) {
        uint32_t bit = bm[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
        return bit != 0;
}
*/

size_t group_offset(void *ptr, uint32_t gnum) {
        f_superblock* sb = ptr;
        uint32_t group_size = sb->group_size;

        return gnum*group_size + sizeof(f_superblock);
}

f_group_header *get_gh(void *ptr, uint32_t gnum) {
        size_t g_off = group_offset(ptr, gnum);
        void *gptr = ptr + g_off;

        f_group_header *gh = gptr;
        return gh;
}

int allocate_block_in_group(void *ptr, uint32_t gnum, uint32_t bnum) {
        size_t g_off = group_offset(ptr, gnum);
        void *gptr = ptr + g_off;

        f_group_header *gh = gptr;
        uint32_t bbo = gh->gh_block_bitmap_offset;
        uint32_t *block_bitmap = gptr + bbo;

        if(get_bit(block_bitmap, bnum));
                return -1;

        set_bit(block_bitmap, bnum);
        return 0;
}

int allocate_block(void *ptr, uint64_t block) {
        f_superblock *sb = ptr;

        return allocate_block_in_group(ptr,
                (block / sb->group_block_number),
                (block % sb->group_block_number) );
}

int32_t get_first_empty(void *ptr, uint32_t gnum) {
        f_superblock *sb = ptr;
        size_t g_off = group_offset(ptr, gnum);
        f_group_header *gh = ptr + g_off;

        unsigned i = 0;
        for(i = 0; i < sb->group_block_number; i++) {
                if(!get_bit(sb->gh_block_bitmap, i)) {
                        return i;
                }
        }
        return -1;
}

#include <assert.h>

int main() {
        size_t part = 10485760;
        FILE* fh = fopen("./part", "w+");
        fseek(fh, part, SEEK_SET);
        fputc('\0', fh);
        fclose(fh);

        int fd = open("./part", O_RDWR | O_CREAT, 0600);
        //void* p = malloc(part);
        void* p = mmap(0, part, PROT_WRITE |PROT_READ, MAP_SHARED, fd, 0);
        if(p == -1)
                perror("foo"), exit(-1);
        mkfs(p, part, 512, 512, 512);

        uint32_t block = 124;
        assert(allocate_block(p, block) == 0);
        assert(allocate_block(p, block) == -1);
}
