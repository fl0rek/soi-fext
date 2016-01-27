#include <fext.h>
#include "fext_utils2.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <signal.h>

#define DEBUG 1

int protect_zero = 0;

void mkfs(void* ptr, size_t size, //uint32_t block_num,
        uint32_t inode_num, uint32_t block_size) {

        memset(ptr, -1, size);

        uint32_t block_num = size /block_size;
        uint16_t block_bitmap_blocks = ((block_num /BITS_PER_WORD) /block_size) +1;
        uint16_t inode_bitmap_blocks = ((inode_num /BITS_PER_WORD) /block_size) +1;
        uint16_t inode_table_blocks  = ((inode_num *sizeof(f_inode)) /block_size);

        //size_t body_size = block_size * block_num;
        size_t header_size;

        f_superblock sb;
        sb.header = FEXT_MAGIC;
        sb.magic_start = 0xCDCDCDCD;
        sb.block_size = block_size;
        sb.free_blocks_count = block_num
                -block_bitmap_blocks -inode_bitmap_blocks -inode_table_blocks;
        sb.free_inodes_count = inode_num;
        sb.used_dirs_count = 0;

        sb.block_bitmap_block = 1;
        sb.inode_bitmap_block = sb.block_bitmap_block + block_bitmap_blocks +1;
        sb.inode_table_block  = sb.inode_bitmap_block + inode_bitmap_blocks +1;
        sb.data_block         = sb.inode_table_block + inode_table_blocks  +1;
//        _bitmap_blocks
//                +inode_bitmap_blocks +inode_bitmap_blocks;

        sb.magic_end = 0xDCDCDCDC;


        void *p = ptr;
        memcpy(p, &sb, sizeof(sb));

printf("wot free: %u %u %u %u\n", sb.free_blocks_count, sb.inode_bitmap_block, sb.inode_table_block, sb.data_block);
        memset(get_block(ptr, sb.inode_bitmap_block-1), 0xCC, sb.block_size);
        memset(get_block(ptr, sb.inode_table_block-1), 0xDD, sb.block_size);
        memset(get_block(ptr, sb.data_block-1), 0xEE, sb.block_size);

        for(uint32_t i = 0; i < sb.data_block; i++) {
                printf("h %u\n", i);
                fflush(stdout);
                memset(get_block(ptr, i), i < sb.inode_table_block ? 0 : i, sb.block_size);
        }

        allocate_block_range(p, 0, sb.data_block);

        allocate_inode(ptr, 0);
        f_inode *root = get_inode(ptr, 0);
        root->i_mode |= MODE_DIRECTORY;

        printf("firstree %u", get_empty_block(ptr));
        //size_t block_bitmap_len =
        protect_zero = 1;
}

void* get_block_bitmap(void* ptr) {
        f_superblock *sb = ptr;
        return get_block(ptr, sb->block_bitmap_block);
}
void* get_inode_bitmap(void* ptr) {
        f_superblock *sb = ptr;
        return get_block(ptr, sb->inode_bitmap_block);
}
void* get_inode_table(void* ptr) {
        f_superblock *sb = ptr;
        return get_block(ptr, sb->inode_table_block);
}

void f_tree(char *ptr) {
        tree_at_node(ptr, 0, 0);
}

void tree_at_node(char *ptr, uint32_t inode, uint32_t depth) {
        uint32_t dir_count = ls(ptr, inode, 0, 0);
        printf("<>\n");
        for(uint32_t i= 0; i < dir_count; i++) {
                f_directory_entry fde;
                ls(ptr, inode, i, &fde);
                printf("% *d %s\n", depth, 0, fde.name);
                tree_at_node(ptr, fde.inode, depth +1);

        }
}

void* get_block(char* ptr, uint32_t block) {
        f_superblock *sb  = (f_superblock*) ptr;
        uint32_t block_size = sb->block_size;

        if(!block_size)
                raise(SIGSEGV);
        if(!block) {
                fprintf(stderr, "access to 0 block, u wot m8\n");
                if(protect_zero)
                        raise(SIGABRT);
        }

        return ptr +sizeof(f_superblock) + block*block_size;
}

int allocate_block_range(char *ptr, uint32_t start, uint32_t end) {
        f_superblock *sb = (f_superblock*) ptr;
        void *block_bitmap = get_block_bitmap(ptr);

        if(sb->free_blocks_count <= end-start)
                return -1; //not enough blocks

        uint32_t it;
        for(it = start; it < end; it++)
                if(get_bit(block_bitmap, it))
                        return -1; // already allocated

        sb->free_blocks_count -= end-start;

        for(it = start; it < end; it++)
                set_bit(block_bitmap, it);

        return 0;
}

uint32_t ls(char *ptr, uint32_t inode_no, uint32_t at, f_directory_entry* ret) {
        f_inode* inode = get_inode(ptr, inode_no);
        if((inode->i_mode & MODE_DIRECTORY) != MODE_DIRECTORY)
                return -1;

        uint32_t dir_entries = inode->i_size /sizeof(f_directory_entry);
        //printf("ent ::: %u\n", dir_entries);
        if(ret && dir_entries)
                read_inode(ptr, inode_no, sizeof(f_directory_entry), at*sizeof(f_directory_entry), ret);

        return dir_entries;
}

uint32_t f_truncate(char *ptr, uint32_t inode_no, size_t newsize) {
        f_superblock *sb = (f_superblock*) ptr;
        uint32_t block_size = sb->block_size;
        f_inode *inode = get_inode(ptr, inode_no);

        if(newsize - inode->i_blocks *block_size <= 0)
                return 0;

        printf("truncate to : %u from %u", newsize, inode->i_blocks *block_size);
        uint32_t more_blocks = ((newsize - (inode->i_blocks *block_size)) / block_size) +1;
        for(uint32_t i = 0; i < more_blocks; i++) {
                uint32_t new_block = get_empty_block(ptr);
                if(new_block == -1) {
                        fprintf(stderr, "Error allocating block");
                        return -1;
                }
                allocate_block(ptr, new_block);
                if(add_block(ptr, inode_no, new_block) == -1)
                        return -1;
        }
        printf("allocated %u\n", more_blocks);
        return more_blocks;
        //inode->blocks += more_blocks;
}

void f_stat(char* ptr, uint32_t inode_no, f_stat_struct* stat) {
        f_superblock *sb = (f_superblock*) ptr;
        f_inode *inode = get_inode(ptr, inode_no);

        stat->gid = inode->i_gid;
        stat->uid = inode->i_uid;
        stat->perm = inode->i_mode;
        stat->size = inode->i_size;
}

int add_block(char *ptr, uint32_t inode_no, uint32_t block) {
        f_superblock *sb = (f_superblock*) ptr;
        f_inode *inode = get_inode(ptr, inode_no);
        uint32_t block_size = sb->block_size;
        printf("[]adding %u at %u\n", block, inode->i_blocks);
        if(inode->i_blocks < 12) {
                inode->i_block[inode->i_blocks++] = block;
        } else if(inode->i_blocks -11 < block_size /sizeof(uint32_t)) {
                if(!inode->i_block[12]) {
                        uint32_t new_block = get_empty_block(ptr);
                        if(new_block == -1) {
                                return -1;
                        }
                        allocate_block(ptr, new_block);
                        inode->i_block[12] = new_block;
                }
                uint32_t *block_of_blocks = get_block(ptr, inode->i_block[12]);
                block = block_of_blocks[inode->i_blocks++ -11];
        }
        return 0;
}

int32_t f_chmod(char *ptr, uint32_t inode_no, uint8_t op, uint16_t mod) {
        f_inode *inode = get_inode(ptr, inode_no);

        if(op == CHMOD_SET) {
                inode->i_mode =
                        (inode->i_mode & MODE_MASK) |
                        (mod & ACCESS_MASK);
        }
        return inode->i_mode;
}

int32_t f_chown(char *ptr, uint32_t inode_no, uint8_t op, uint16_t uid) {
        f_inode *inode = get_inode(ptr, inode_no);

        if(op == CHOWN_SET_UID) {
                inode->i_uid = uid;
        }
        return inode->i_uid;
}

int32_t f_chgrp(char *ptr, uint32_t inode_no, uint8_t op, uint16_t gid) {
        f_inode *inode = get_inode(ptr, inode_no);

        if(op == CHGRP_SET_GID) {
                inode->i_gid = gid;
        }
        return inode->i_gid;
}

size_t write_inode(char *ptr, uint32_t inode_no, size_t size, size_t offset, void* buff) {
        printf("inwrite : %p\n", ptr);
        f_superblock *sb = (f_superblock*) ptr;
        f_inode *inode = get_inode(ptr, inode_no);

        size_t wrote_bytes = 0;

        if(size+offset > inode->i_size) {
                if(-1 == f_truncate(ptr, inode_no, size+offset))
                        return 0;
                inode->i_size = size+offset;
        }

        uint32_t block_size = sb->block_size;

        uint32_t first_block_no = offset / block_size;
        size_t inblock_offset_begin = offset % block_size;
        uint32_t last_block_no = (offset + size) / block_size;
        size_t inblock_offset_end = (offset + size) % block_size;

        void *helper_buffer = malloc(block_size);

        uint32_t block_it;
        for(block_it = first_block_no; block_it <= last_block_no; block_it++) {
                uint32_t block;
                fprintf(stderr, "write block  no : %u", block_it);
                if(block_it < 12)
                        block = inode->i_block[block_it];
                else if(block_it -11 < block_size /sizeof(uint32_t)) {
                        uint32_t *block_of_blocks = get_block(ptr, inode->i_block[12]);
                        block = block_of_blocks[block_it -11];
                } else {
                        fprintf(stderr, "Write out of bounds: %u", block_it);
                        raise(SIGABRT);
                }
                fprintf(stderr, " located at %u\n", block);
                if(block_it == first_block_no || block_it == last_block_no) {
                        read_block(ptr, block, helper_buffer);
                        void* start = helper_buffer;
                        size_t len = block_size;
                        if(block_it == first_block_no) {
                                helper_buffer += inblock_offset_begin;
                                len = block_size -inblock_offset_begin;
                        }

                        if(block_it == last_block_no)
                                len = inblock_offset_end;
                        memcpy(helper_buffer, buff, len);
                        write_block(ptr, block, start);
                        buff += len;
                } else {
                        wrote_bytes += write_block(ptr, block, buff);
                        //read_bytes += read_block(ptr, block, buff)
                        buff += wrote_bytes;
                }
        }
        return wrote_bytes;
}

size_t read_inode(void* ptr, uint32_t inode_no, size_t size, size_t offset, void* buff) {
        f_superblock *sb = (f_superblock*) ptr;
        f_inode *inode = get_inode(ptr, inode_no);

        size_t read_bytes = 0;

        if(offset > inode->i_size)
                return 0;
        if(size+offset > inode->i_size)
                size = inode->i_size - offset;

        uint32_t block_size = sb->block_size;
        uint32_t first_block_no = offset / block_size;
        size_t inblock_offset_begin = offset % block_size;
        uint32_t last_block_no = (offset + size) / block_size;
        size_t inblock_offset_end = (offset + size) % block_size;

        void *helper_buffer = malloc(block_size);

        uint32_t block_it;
        for(block_it = first_block_no; block_it <= last_block_no; block_it++) {
                //printf("reading [[[ %u ]]]", block_it);
                uint32_t block;
                if(block_it < 12)
                        block = inode->i_block[block_it];
                else if(block_it -11 < block_size /sizeof(uint32_t)) {
                        uint32_t *block_of_blocks = get_block(ptr, inode->i_block[12]);
                        block = block_of_blocks[block_it -11];
                } else {
                        fprintf(stderr, "Read out of bounds");
                        raise(SIGABRT);
                }
                if(block_it == first_block_no || block_it == last_block_no) {
                        read_block(ptr, block, helper_buffer);
                        size_t len = block_size;
                        if(block_it == first_block_no) {
                                helper_buffer += inblock_offset_begin;
                                len = block_size -inblock_offset_begin;
                        }
                        if(block_it == last_block_no)
                                len = inblock_offset_end;
                        memcpy(buff, helper_buffer, len);
			read_bytes += len;
                        buff += len;

                } else {
                        read_bytes += read_block(ptr, block, buff);
                        buff += read_bytes;
                }
        }
        return read_bytes;
}

size_t read_block(void* ptr, uint32_t block, void *buff) {
        //printf("reading %u\n", block);
        f_superblock *sb = (f_superblock*) ptr;
        void* src = get_block(ptr, block);
        uint32_t block_size = sb->block_size;
        memcpy(buff, src, block_size);
        return block_size;
}

size_t write_block(char *ptr, uint32_t block, void *buff) {
        f_superblock *sb = (f_superblock*) ptr;
        uint32_t block_size = sb->block_size;
        char *dest = get_block(ptr, block);
        printf("writing at %p\n", dest - ptr);
        memcpy(dest, buff, block_size);
        return block_size;
}

uint32_t open_inode(char *ptr, uint32_t inode, char* name) {
        uint32_t dirs = ls(ptr, inode, 0, 0);
        uint32_t i;
        for(i = 0; i < dirs; i++) {
                f_directory_entry fde;
                ls(ptr, inode, i, &fde);
                printf("%s vs %s : %d\n", fde.name, name, strcmp(fde.name, name));

                //if(slen == strlen(fde.name) && strncmp(fde.name, name, slen) == 0)
                if(strcmp(name, fde.name) == 0)
                        return fde.inode;
        }
        return -1;
}

uint32_t f_open(char *ptr, char* name) {
        if(!strcmp(name, "/"))
                return 0;

        char *begin, *last_begin = name;
        begin = strtok(name, "/");
        uint32_t parent_inode = 0;
        while(begin) {
                printf("%s<<\n", begin);
                parent_inode = open_inode(ptr, parent_inode, begin);
                last_begin = begin;
                begin = strtok(0, "/");
        }
        return parent_inode;//open_inode(ptr, parent_inode, last_begin);;
}

uint32_t f_touch(char *ptr, uint32_t parent_inode, char* name) {
        uint32_t dirs = ls(ptr, parent_inode, 0, 0);

	if(open_inode(ptr, parent_inode, name) != -1) {
		fprintf(stderr, "Already exists: %s", name);
		raise(SIGABRT);
		exit(-1);
	}


        printf("already at %u dirs\n", dirs);

        f_directory_entry new;
        new.inode = allocate_free_inode(ptr);
        if(new.inode == -1) {
                return -1;
	}
        strncpy(new.name, name, 255);

        printf("on: %p\n", ptr);

        write_inode(ptr, parent_inode, sizeof(f_directory_entry), dirs*sizeof(f_directory_entry), &new);

        dirs = ls(ptr, parent_inode, 0, 0);
        printf("now at %u dirs", dirs);

        return new.inode;
}

uint32_t f_mkdir(char *ptr, uint32_t parent_inode, char* name) {
        f_superblock *sb = (f_superblock*) ptr;
        uint32_t node = f_touch(ptr, parent_inode, name);
        f_inode *in = get_inode(ptr, node);

        in->i_mode |= MODE_DIRECTORY;
        sb->used_dirs_count++;

        return node;
}

uint32_t allocate_inode(char *ptr, uint32_t inode) {
        f_superblock *sb = (f_superblock*) ptr;
        void *inode_bitmap = get_inode_bitmap(ptr);

        if(get_bit(inode_bitmap, inode))
                return -1; // already allocated

        set_bit(inode_bitmap, inode);

        f_inode *fin = get_inode(ptr, inode);
        memset(fin, 0, sizeof(*fin));
        return 0;
}

uint32_t allocate_free_inode(char *ptr) {
        f_superblock *sb = (f_superblock*) ptr;
        void *inode_bitmap = get_inode_bitmap(ptr);

        if(!sb->free_inodes_count) {
		fprintf(stderr, "Out of inodes\n");
		raise(SIGABRT);
                return -1;
	}

        uint32_t i = 0;
        while(get_bit(inode_bitmap, i))
                i++;

        allocate_inode(ptr, i);

        f_inode* in = get_inode(ptr, i);
        memset(in, 0, sizeof(*in));

        sb->free_inodes_count--;
        return i;
}


f_inode* get_inode(char *ptr, uint32_t inode) {
        f_inode * inode_ptr = get_inode_table(ptr);
        return &inode_ptr[inode];
}

int allocate_block(char *ptr, uint32_t block) {
        return allocate_block_range(ptr, block, block +1);
}

uint32_t get_empty_block(char *ptr) {
        f_superblock *sb = (f_superblock*) ptr;
        void *block_bitmap = get_block_bitmap(ptr);

        if(!sb->free_blocks_count) {
		fprintf(stderr, "Out of inodes\n");
		raise(SIGABRT);
                return -1;
	}

        uint32_t i = 0;
        while(get_bit(block_bitmap, i))
                i++;

        //set_bit(block_bitmap, i);
        //sb->free_blocks_count--;
        return i;
}

uint32_t free_block(char* ptr, uint32_t block_no) {
        f_superblock *sb = (f_superblock*) ptr;
        void *block_bitmap = get_block_bitmap(ptr);

        if(get_bit(block_bitmap, block_no)) {
                clear_bit(block_bitmap, block_no);
                sb->free_blocks_count++;
        }

        return 0;
}

uint32_t free_inode(char* ptr, uint32_t inode_no) {
        f_superblock *sb = (f_superblock*) ptr;
        void *inode_bitmap = get_inode_bitmap(ptr);

        if(get_bit(inode_bitmap, inode_no)) {
                clear_bit(inode_bitmap, inode_no);
                sb->free_inodes_count++;
        }

        return 0;
}

uint32_t f_unlink(char* ptr, uint32_t inode_no) {
        f_superblock *sb = (f_superblock*) ptr;
        void *block_bitmap = get_block_bitmap(ptr);

        f_inode *inode = get_inode(ptr, inode);

        for(uint32_t block_it = 0; block_it < inode->i_blocks; block_it++) {
                free_block(ptr, get_inode_block_no(ptr, inode_no, block_it));
        }

        free_inode(ptr,inode_no);
}

uint32_t f_remove_directory_entry(char* ptr, uint32_t parent_dir, uint32_t to_remove) {
        f_superblock *sb = (f_superblock*) ptr;
        f_inode* inode = get_inode(ptr, parent_dir);

        uint32_t dirs = ls(ptr, parent_dir, 0, 0);
        uint32_t i;
        for(i = 0; i < dirs; i++) {
                f_directory_entry fde;
                ls(ptr, parent_dir, i, &fde);
                if(fde.inode == to_remove)
                        break;
        }
        if(i == dirs)
                return -1; // not found

        inode->i_size -= sizeof(f_directory_entry);
        for(; i < dirs-1; i++) {
                f_directory_entry buff;
                printf("moving fde %s", buff.name);
                read_inode(ptr, parent_dir, sizeof(f_directory_entry), (i+1) *sizeof(f_directory_entry), &buff);
                write_inode(ptr, parent_dir, sizeof(f_directory_entry), i *sizeof(f_directory_entry), &buff);
        }
        return 0;
}

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

uint32_t get_inode_block_no(char* ptr, uint32_t inode_no, uint32_t block_no) {
        f_superblock *sb = (f_superblock*) ptr;
        f_inode *inode = get_inode(ptr, inode_no);
        uint32_t block_size = sb->block_size;
        uint32_t block = 0;

        if(block_no < 12)
                block = inode->i_block[block_no];
        else if(block_no -11 < block_size /sizeof(uint32_t)) {
                uint32_t *block_of_blocks = get_block(ptr, inode->i_block[12]);
                block = block_of_blocks[block_no -11];
        } else {
                fprintf(stderr, "Write out of bounds: %u", block_no);
                raise(SIGABRT);
        }

        return block;
}
