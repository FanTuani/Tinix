#include "fs/file_system.h"
#include <iostream>
#include <cstring>

FileSystem::FileSystem(DiskDevice* disk)
    : disk_(disk), mounted_(false), bitmap_dirty_(false) {
    inode_bitmap_.resize(BLOCK_SIZE);
    data_bitmap_.resize(BLOCK_SIZE);
}

FileSystem::~FileSystem() {
    if (mounted_ && bitmap_dirty_) {
        save_bitmaps();
        save_superblock();
    }
}

bool FileSystem::format() {
    std::cout << "[FS] Formatting file system..." << std::endl;
    
    // 初始化 SuperBlock
    superblock_ = SuperBlock();
    superblock_.magic = FS_MAGIC;
    superblock_.total_blocks = TOTAL_BLOCKS;
    superblock_.total_inodes = MAX_INODES;
    superblock_.free_blocks = MAX_DATA_BLOCKS;
    superblock_.free_inodes = MAX_INODES - 1;  // 减去根目录的inode
    
    superblock_.inode_bitmap_block = INODE_BITMAP_BLOCK;
    superblock_.data_bitmap_block = DATA_BITMAP_BLOCK;
    superblock_.inode_table_start = INODE_TABLE_START;
    superblock_.inode_table_blocks = INODE_TABLE_BLOCKS;
    superblock_.data_blocks_start = DATA_BLOCKS_START;
    
    // 清空位图
    std::fill(inode_bitmap_.begin(), inode_bitmap_.end(), 0);
    std::fill(data_bitmap_.begin(), data_bitmap_.end(), 0);
    
    // 标记根目录的 inode 为已使用
    set_bit(inode_bitmap_, ROOT_INODE);
    
    bitmap_dirty_ = true;
    
    // 写入 SuperBlock
    if (!save_superblock()) {
        std::cerr << "[FS] Format failed: unable to write SuperBlock" << std::endl;
        return false;
    }
    
    // 写入位图
    if (!save_bitmaps()) {
        std::cerr << "[FS] Format failed: unable to write bitmaps" << std::endl;
        return false;
    }
    
    // 初始化 inode 表区域（清零）
    std::vector<uint8_t> zero_block(BLOCK_SIZE, 0);
    for (uint32_t i = 0; i < INODE_TABLE_BLOCKS; i++) {
        if (!disk_->write_block(INODE_TABLE_START + i, zero_block.data())) {
            std::cerr << "[FS] Format failed: unable to clear inode table" << std::endl;
            return false;
        }
    }
    
    // 创建根目录
    if (!init_root_directory()) {
        std::cerr << "[FS] Format failed: unable to create root directory" << std::endl;
        return false;
    }
    
    mounted_ = true;
    std::cout << "[FS] Format complete!" << std::endl;
    std::cout << "[FS] Total blocks: " << superblock_.total_blocks 
              << ", Total inodes: " << superblock_.total_inodes << std::endl;
    
    return true;
}

bool FileSystem::mount() {
    std::cout << "[FS] Mounting file system..." << std::endl;
    
    if (!load_superblock()) {
        std::cerr << "[FS] Mount failed: unable to read SuperBlock" << std::endl;
        return false;
    }
    
    // 验证魔数
    if (superblock_.magic != FS_MAGIC) {
        std::cerr << "[FS] Mount failed: magic number mismatch (expected: 0x" 
                  << std::hex << FS_MAGIC << ", actual: 0x" 
                  << superblock_.magic << std::dec << ")" << std::endl;
        return false;
    }
    
    if (!load_bitmaps()) {
        std::cerr << "[FS] Mount failed: unable to read bitmaps" << std::endl;
        return false;
    }
    
    mounted_ = true;
    bitmap_dirty_ = false;
    
    std::cout << "[FS] Mount successful!" << std::endl;
    std::cout << "[FS] Free blocks: " << superblock_.free_blocks 
              << ", Free inodes: " << superblock_.free_inodes << std::endl;
    
    return true;
}

bool FileSystem::init_root_directory() {
    // 创建根目录的 inode
    Inode root_inode;
    root_inode.type = FileType::DIRECTORY;
    root_inode.size = 2 * DIRENT_SIZE;  // . 和 ..
    root_inode.blocks_used = 1;
    
    // 分配一个数据块给根目录
    uint32_t root_data_block = alloc_block();
    if (root_data_block == INVALID_BLOCK) {
        std::cerr << "[FS] Unable to allocate data block for root directory" << std::endl;
        return false;
    }
    root_inode.direct_blocks[0] = root_data_block;
    
    // 写入根目录的 inode
    if (!write_inode(ROOT_INODE, root_inode)) {
        std::cerr << "[FS] Unable to write root inode" << std::endl;
        return false;
    }
    
    // 创建 . 和 .. 目录项
    std::vector<uint8_t> dir_block(BLOCK_SIZE, 0);
    DirectoryEntry* entries = reinterpret_cast<DirectoryEntry*>(dir_block.data());
    
    entries[0] = DirectoryEntry(".", ROOT_INODE);
    entries[1] = DirectoryEntry("..", ROOT_INODE);
    
    // 写入根目录的数据块
    if (!disk_->write_block(root_data_block, dir_block.data())) {
        std::cerr << "[FS] Unable to write root directory data" << std::endl;
        return false;
    }
    
    std::cout << "[FS] Root directory created (inode=" << ROOT_INODE 
              << ", block=" << root_data_block << ")" << std::endl;
    
    return true;
}

uint32_t FileSystem::alloc_inode() {
    uint32_t inode_num = find_free_bit(inode_bitmap_, MAX_INODES);
    if (inode_num == INVALID_INODE) {
        std::cerr << "[FS] No free inodes available" << std::endl;
        return INVALID_INODE;
    }
    
    set_bit(inode_bitmap_, inode_num);
    superblock_.free_inodes--;
    bitmap_dirty_ = true;
    
    std::cout << "[FS] Allocated inode: " << inode_num << std::endl;
    return inode_num;
}

void FileSystem::free_inode(uint32_t inode_num) {
    clear_bit(inode_bitmap_, inode_num);
    superblock_.free_inodes++;
    bitmap_dirty_ = true;
    
    std::cout << "[FS] Freed inode: " << inode_num << std::endl;
}

uint32_t FileSystem::alloc_block() {
    uint32_t block_num = find_free_bit(data_bitmap_, MAX_DATA_BLOCKS);
    if (block_num == INVALID_BLOCK) {
        std::cerr << "[FS] No free blocks available" << std::endl;
        return INVALID_BLOCK;
    }
    
    set_bit(data_bitmap_, block_num);
    superblock_.free_blocks--;
    bitmap_dirty_ = true;
    
    uint32_t actual_block = DATA_BLOCKS_START + block_num;
    std::cout << "[FS] Allocated block: " << actual_block 
              << " (bitmap index: " << block_num << ")" << std::endl;
    
    return actual_block;
}

void FileSystem::free_block(uint32_t block_num) {
    uint32_t bitmap_index = block_num - DATA_BLOCKS_START;
    clear_bit(data_bitmap_, bitmap_index);
    superblock_.free_blocks++;
    bitmap_dirty_ = true;
    
    std::cout << "[FS] Freed block: " << block_num << std::endl;
}

bool FileSystem::read_inode(uint32_t inode_num, Inode& out_inode) {
    // 计算 inode 在磁盘上的位置
    uint32_t inodes_per_block = BLOCK_SIZE / sizeof(Inode);
    uint32_t block_num = INODE_TABLE_START + inode_num / inodes_per_block;
    uint32_t offset = (inode_num % inodes_per_block) * sizeof(Inode);
    
    std::vector<uint8_t> block_data(BLOCK_SIZE);
    if (!disk_->read_block(block_num, block_data.data())) {
        return false;
    }
    
    memcpy(&out_inode, block_data.data() + offset, sizeof(Inode));
    return true;
}

bool FileSystem::write_inode(uint32_t inode_num, const Inode& inode) {
    // 计算 inode 在磁盘上的位置
    uint32_t inodes_per_block = BLOCK_SIZE / sizeof(Inode);
    uint32_t block_num = INODE_TABLE_START + inode_num / inodes_per_block;
    uint32_t offset = (inode_num % inodes_per_block) * sizeof(Inode);
    
    // 读取整个块
    std::vector<uint8_t> block_data(BLOCK_SIZE);
    if (!disk_->read_block(block_num, block_data.data())) {
        return false;
    }
    
    // 更新 inode 数据
    memcpy(block_data.data() + offset, &inode, sizeof(Inode));
    
    // 写回磁盘
    if (!disk_->write_block(block_num, block_data.data())) {
        return false;
    }
    
    return true;
}

bool FileSystem::load_superblock() {
    std::vector<uint8_t> block_data(BLOCK_SIZE);
    if (!disk_->read_block(SUPERBLOCK_BLOCK, block_data.data())) {
        return false;
    }
    memcpy(&superblock_, block_data.data(), sizeof(SuperBlock));
    return true;
}

bool FileSystem::save_superblock() {
    std::vector<uint8_t> block_data(BLOCK_SIZE);
    memcpy(block_data.data(), &superblock_, sizeof(SuperBlock));
    return disk_->write_block(SUPERBLOCK_BLOCK, block_data.data());
}

bool FileSystem::load_bitmaps() {
    if (!disk_->read_block(INODE_BITMAP_BLOCK, inode_bitmap_.data())) {
        return false;
    }
    if (!disk_->read_block(DATA_BITMAP_BLOCK, data_bitmap_.data())) {
        return false;
    }
    return true;
}

bool FileSystem::save_bitmaps() {
    if (!disk_->write_block(INODE_BITMAP_BLOCK, inode_bitmap_.data())) {
        return false;
    }
    if (!disk_->write_block(DATA_BITMAP_BLOCK, data_bitmap_.data())) {
        return false;
    }
    bitmap_dirty_ = false;
    return true;
}

bool FileSystem::is_bit_set(const std::vector<uint8_t>& bitmap, uint32_t bit_index) {
    uint32_t byte_index = bit_index / 8;
    uint32_t bit_offset = bit_index % 8;
    return (bitmap[byte_index] & (1 << bit_offset)) != 0;
}

void FileSystem::set_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index) {
    uint32_t byte_index = bit_index / 8;
    uint32_t bit_offset = bit_index % 8;
    bitmap[byte_index] |= (1 << bit_offset);
}

void FileSystem::clear_bit(std::vector<uint8_t>& bitmap, uint32_t bit_index) {
    uint32_t byte_index = bit_index / 8;
    uint32_t bit_offset = bit_index % 8;
    bitmap[byte_index] &= ~(1 << bit_offset);
}

uint32_t FileSystem::find_free_bit(const std::vector<uint8_t>& bitmap, uint32_t max_bits) {
    for (uint32_t i = 0; i < max_bits; i++) {
        if (!is_bit_set(bitmap, i)) {
            return i;
        }
    }
    return INVALID_INODE;  // 或 INVALID_BLOCK，都是 0xFFFFFFFF
}

void FileSystem::print_superblock() const {
    std::cout << "========== SuperBlock ==========" << std::endl;
    std::cout << "Magic: 0x" << std::hex << superblock_.magic << std::dec << std::endl;
    std::cout << "Total blocks: " << superblock_.total_blocks << std::endl;
    std::cout << "Total inodes: " << superblock_.total_inodes << std::endl;
    std::cout << "Free blocks: " << superblock_.free_blocks << std::endl;
    std::cout << "Free inodes: " << superblock_.free_inodes << std::endl;
    std::cout << "Data blocks start: " << superblock_.data_blocks_start << std::endl;
    std::cout << "===============================" << std::endl;
}

void FileSystem::print_inode(uint32_t inode_num) const {
    Inode inode;
    if (!const_cast<FileSystem*>(this)->read_inode(inode_num, inode)) {
        return;
    }
    
    std::cout << "========== Inode " << inode_num << " ==========" << std::endl;
    std::cout << "Type: " << (inode.type == FileType::DIRECTORY ? "Directory" : "File") << std::endl;
    std::cout << "Size: " << inode.size << " bytes" << std::endl;
    std::cout << "Blocks used: " << inode.blocks_used << std::endl;
    std::cout << "Direct blocks: ";
    for (uint32_t i = 0; i < inode.blocks_used && i < DIRECT_BLOCKS; i++) {
        std::cout << inode.direct_blocks[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
}
