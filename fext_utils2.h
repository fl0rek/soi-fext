/* This file was automatically generated.  Do not edit! */
uint32_t f_remove_directory_entry(char *ptr,uint32_t parent_dir,uint32_t to_remove);
uint32_t get_inode_block_no(char *ptr,uint32_t inode_no,uint32_t block_no);
uint32_t f_unlink(char *ptr,uint32_t inode_no);
uint32_t free_inode(char *ptr,uint32_t inode_no);
void clear_bit(uint32_t *bm,unsigned n);
uint32_t free_block(char *ptr,uint32_t block_no);
uint32_t f_mkdir(char *ptr,uint32_t parent_inode,char *name);
uint32_t allocate_free_inode(char *ptr);
uint32_t f_touch(char *ptr,uint32_t parent_inode,char *name);
uint32_t f_open(char *ptr,char *name);
uint32_t open_inode(char *ptr,uint32_t inode,char *name);
size_t write_block(char *ptr,uint32_t block,void *buff);
size_t read_block(void *ptr,uint32_t block,void *buff);
size_t write_inode(char *ptr,uint32_t inode_no,size_t size,size_t offset,void *buff);
int32_t f_chgrp(char *ptr,uint32_t inode_no,uint8_t op,uint16_t gid);
int32_t f_chown(char *ptr,uint32_t inode_no,uint8_t op,uint16_t uid);
int32_t f_chmod(char *ptr,uint32_t inode_no,uint8_t op,uint16_t mod);
void f_stat(char *ptr,uint32_t inode_no,f_stat_struct *stat);
int add_block(char *ptr,uint32_t inode_no,uint32_t block);
int allocate_block(char *ptr,uint32_t block);
uint32_t f_truncate(char *ptr,uint32_t inode_no,size_t newsize);
size_t read_inode(void *ptr,uint32_t inode_no,size_t size,size_t offset,void *buff);
void set_bit(uint32_t *bm,unsigned n);
int get_bit(uint32_t *bm,unsigned n);
uint32_t ls(char *ptr,uint32_t inode_no,uint32_t at,f_directory_entry *ret);
void tree_at_node(char *ptr,uint32_t inode,uint32_t depth);
void f_tree(char *ptr);
void *get_inode_table(void *ptr);
void *get_inode_bitmap(void *ptr);
void *get_block_bitmap(void *ptr);
uint32_t get_empty_block(char *ptr);
f_inode *get_inode(char *ptr,uint32_t inode);
uint32_t allocate_inode(char *ptr,uint32_t inode);
int allocate_block_range(char *ptr,uint32_t start,uint32_t end);
void *get_block(char *ptr,uint32_t block);
void mkfs(void *ptr,size_t size,uint32_t inode_num,uint32_t block_size);
extern int protect_zero;
