#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#define ACCESS_MASK             0x0FFF
#define MODE_MASK               0xF000

#define MODE_FILE               0x0000
#define MODE_DIRECTORY          0x1000

#define CHMOD_GET               0
#define CHMOD_SET               1

#define CHOWN_GET_UID           0
#define CHOWN_SET_UID           1

#define CHGRP_GET_GID           0
#define CHGRP_SET_GID           1

typedef struct __attribute__((__packed__)) {
        uint16_t i_mode;
        uint16_t i_uid;
        uint16_t i_gid;
        size_t i_size;
        uint32_t i_blocks;
        uint32_t i_block[15];
        // 0,11 - direct blocks, 12 - indirect, 13 - doubly indirect, 14 - triply-indirect; 0 terminated
} f_inode;

typedef struct __attribute__((__packed__)) {
        uint16_t uid;
        uint16_t gid;
        uint16_t perm;
        size_t size;
} f_stat_struct;

typedef struct __attribute__((__packed__)) {
        uint32_t inode;
        //uint16_t rec_len;
        //uint8_t  name_len;
        char     name[255];
} f_directory_entry;

#define FEXT_MAGIC 0xFF10471C
typedef struct __attribute__((__packed__)) {
        uint32_t header;
        uint32_t magic_start;
        uint32_t block_size;
        uint16_t free_blocks_count;
        uint16_t free_inodes_count;

        uint16_t used_dirs_count;

        uint16_t block_bitmap_block;
        uint16_t inode_bitmap_block;

        uint16_t inode_table_block;
        uint16_t data_block;

        uint32_t magic_end;
} f_superblock;

enum { BITS_PER_WORD = sizeof(uint32_t) *CHAR_BIT};
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)

void set_bit(uint32_t *bm, unsigned n);
void clear_bit(uint32_t *bm, unsigned n);
int get_bit(uint32_t *bm, unsigned n);
