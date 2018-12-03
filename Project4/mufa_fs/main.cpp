#include "defs.hpp"
#include <limits>
#include "colors.h"
#include <fcntl.h>
#include <errno.h>
#include <stdexcept>
#include <exception>
#include <tuple>

#define KB(x)   ((size_t) (x) << 10)
#define MB(x)   ((size_t) (x) << 20)
#define GB(x)   ((size_t) (x) << 30)
#define TB(x)   ((size_t) (x) << 40)

/* Some stats:
max file size = 2^64*256/2^70 = 4 Zettabyte. lol
*/
struct Volume {
	char volume_name[16] = "new volume";
	uint16_t block_size = 256; // max block size: 65,536 byte = 65 KB
	uint64_t total_blocks; // max total_blocks = 4,722,366,482,869.64521 // calculate this with given drive size and other reserved blocks
	uint16_t inode_size = 1; // in terms of blocks
	uint64_t total_inodes; // = total_blocks / inode_size
	uint64_t reserved_blocks = 10; // just in case
	uint64_t data_start_pos; // reserved_blocks + 1

	uint64_t drive_size; // derived value. drive_size = block_size * total_blocks
};

// size: 16+2 + 8+8 + 8+2+8 + 8+8 + 2=70
// 4 * 2 = 8
struct super_block {
	char volume_name[16] = "new volume";
	uint16_t block_size = 256; // max block size: 65,536 byte = 65 KB // min block size = 64

	uint64_t total_blocks; // max total_blocks = 4,722,366,482,869.64521 // calculate this with given drive size and other reserved blocks
	uint64_t reserved_blocks = 1; // number of reserved blocks

	uint64_t inodes_start;
	uint16_t inode_size = 1; // in terms of blocks
	uint64_t total_inodes; // = total_blocks / inode_size

	uint64_t data_block_start; // reserved_blocks + 1
	uint64_t total_data_blocks;

	uint16_t next_volumes; // next volumes. no limit on number of indirections
} __attribute((packed));

// size: 1 + 8 + 80 + 8*3 = 113
struct inode {
	char isDirectory;
	uint64_t parent;
	
	uint64_t data_blocks[10];
	uint64_t indirect1;
	uint64_t indirect2;
	uint64_t indirect3;
} __attribute((packed));

struct directory {
	char data[256];
} __attribute((packed));


void write_inode(inode& _inode, FILE* drive, super_block* sb) {
	// find inode spot // TODO: implement
	size_t index = 0; // inode index to write
	fseek(drive, sb->inodes_start + index * sb->inode_size * sb->block_size, SEEK_SET);
	printf(GREEN "now at 0x%X\n" RESET, ftell(drive));
	dumpbytes((unsigned char*)&_inode, sizeof(inode));
	fwrite(&_inode, sizeof(inode), 1, drive);
}

void write_root_inode(inode& _inode, FILE* drive, super_block* sb) {
	size_t index = 0; // inode index to write
	fseek(drive, sb->inodes_start + index * sb->inode_size * sb->block_size, SEEK_SET);
	printf(GREEN "now at 0x%X\n" RESET, ftell(drive));
	dumpbytes((unsigned char*)&_inode, sizeof(inode));
	fwrite(&_inode, sizeof(inode), 1, drive);
}

uint64_t find_inode_index(FILE* drive, super_block* sb) {
	fseek(drive, sb->inodes_start, SEEK_SET);
	while(true) {

		// fread()
	}
}
uint64_t find_data_index(FILE* drive, super_block* sb) {

}
void write_data(inode& i, FILE* drive, super_block* sb) {
	
}

void mkdir(uint64_t parent, string name, FILE* drive, super_block* sb) {
	inode _inode;
	memset(&_inode, '\0', sizeof(inode));
	_inode.isDirectory = 1;
	_inode.parent = parent;

	if(parent == NULL)
		write_root_inode(_inode, drive, sb);
	else
		write_inode(_inode, drive, sb);
}

void print_superblock(super_block* sb) {
	printf(BOLDWHITE "volume_name: %s\n" RESET, sb->volume_name);
	printf(BOLDWHITE "block_size: %i\n" RESET, sb->block_size);
	printf(BOLDWHITE "total_blocks: %i\n" RESET, sb->total_blocks);
	printf(BOLDWHITE "inode_size: %i\n" RESET, sb->inode_size);
	printf(BOLDWHITE "total_inodes: %i\n" RESET, sb->total_inodes);
	printf(BOLDWHITE "reserved_blocks: %i\n" RESET, sb->reserved_blocks);
	printf(BOLDWHITE "data_block_start: %i\n" RESET, sb->data_block_start);
	printf(BOLDWHITE "next_volumes: %i\n" RESET, sb->next_volumes);
}


const char zeros[4096] = {0};
void purge(FILE* file, size_t length) {
	while(length>sizeof(zeros))
		length -= fwrite(zeros, sizeof(zeros), 1, file) * sizeof(zeros);
	while(length)
		length -= fwrite(zeros, length, 1, file) * length;
}

 /**
  * @brief  creates empty drive image with one volume
  * @param  name: name of the first volume to save in super block
  * @param  path: relative path with filename
  * @param  size: size in bytes
  * @retval None
  */
void createDriveImage(string name, string path, size_t size) {
	int fd = open(path.c_str(), O_CREAT | O_RDWR | O_TRUNC /*| O_EXCL*/, S_IRUSR | S_IWUSR);
	if(fd < 0) {
		perror(("Error creating file " + path).c_str());
		exit(EXIT_FAILURE);
	}
	FILE* file = fdopen(fd, "rb+");
	super_block sb;
	size_t overhead = sizeof(super_block) + 10; // 70 + 10
	
	// set up superblock
	strcpy(sb.volume_name, name.substr(0, 15).c_str());
	sb.block_size = 256;
	
	sb.total_blocks = (size - overhead) / sb.block_size;
	sb.reserved_blocks = 1;

	sb.inodes_start = overhead + sb.reserved_blocks * sb.block_size;
	sb.inode_size = 1;
	sb.total_inodes = sb.total_blocks / 30;

	if(sb.total_inodes < 10) {
		fprintf(stderr, RED "drive too small. Minimum drive size: %i KB\n" RESET, (30*10*sb.block_size + overhead)/KB(1) + 1);
		exit(1);
	}

	sb.data_block_start = sb.inodes_start + sb.total_inodes * sb.block_size * sb.inode_size;
	sb.total_data_blocks = sb.total_blocks - sb.total_inodes;

	// print superblock
	print_superblock(&sb);
	dumpbytes((unsigned char*)&sb,sizeof(super_block));


	// create empty file of given size
	fseek(file, size, SEEK_SET);
	fputc('\0', file);

	// write super block
	fseek(file, 0x00, SEEK_SET);
	fwrite(&sb, sizeof(super_block), 1, file);

	// padding + reserved blocks
	purge(file, 10 + sb.reserved_blocks * sb.block_size);

	// purge inode area
	purge(file, sb.total_inodes * sb.inode_size * sb.block_size);
	
	printf("now at 0x%X\n", ftell(file));

	mkdir(0, "root", file, &sb);

	fclose(file);
}

size_t mmap_inodes(FILE* drive, super_block* sb, char* &buf) {
	size_t inodes_len = sb->inode_size * sb->block_size * sb->total_inodes;
	(buf = (char*)malloc(inodes_len));
	fread(buf, inodes_len, 1, drive);
	return inodes_len;
	// return ((buf = (char*)malloc(inodes_len)) != NULL);
}

void print_inodes() {

}

/**
 * @brief  returns all superblocks from drive image. currently only reads first superblock
 * @param  drive: drive file
 * @retval array of super blocks
 */
vector<super_block> read_superblock(FILE* drive) {
	fseek(drive, 0x00, SEEK_SET);
	super_block sb;
	memset(&sb, '\0', sizeof(super_block));
	fread(&sb, sizeof(super_block), 1, drive);
	return {sb};
}

int main(int argc, char** argv)
{
	createDriveImage("drive_name", "drive.img", MB(76));
	// return 0;
	int fd = open("drive.img", O_RDWR /*| O_EXCL*/, S_IRUSR | S_IWUSR);
	FILE* file = fdopen(fd, "rb+");
	if(fd < 0) {
		perror("Error reading file drive.img");
		exit(EXIT_FAILURE);
	}
	printf(MAGENTA "reading super_block:\n" RESET);
	vector<super_block> sb = read_superblock(file);
	
	print_superblock(&sb[0]);
	char* inodes;
	size_t inodes_len = mmap_inodes(file, &sb[0], inodes);

	printf("inodes_len = %lu\n", inodes_len);
	// dumpbytes((unsigned char*)inodes, 0x200);
	fclose(file);
	return 0;
}