#include "defs.hpp"
#include <limits>
#include "colors.h"
#include <fcntl.h>
#include <errno.h>
#include <stdexcept>
#include <exception>

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
	uint64_t total_blocks; // max total_blocks = 4,722,366,482,869.64521 // calculate this with given disk size and other reserved blocks
	uint16_t inode_size = 1; // in terms of blocks
	uint64_t total_inodes; // = total_blocks / inode_size
	uint64_t reserved_blocks = 10; // just in case
	uint64_t data_start_pos; // reserved_blocks + 1

	uint64_t disk_size; // derived value. disk_size = block_size * total_blocks
};

// size: 16 + 2 + 8 + 2 + 8 + 8 + 8 + 2 = 54
// 4 * 2 = 8
struct super_block {
	char volume_name[16] = "new volume";
	uint16_t block_size = 256; // max block size: 65,536 byte = 65 KB // min block size = 64
	uint64_t reserved_blocks = 10; // number of reserved blocks
	uint64_t total_blocks; // max total_blocks = 4,722,366,482,869.64521 // calculate this with given disk size and other reserved blocks
	uint16_t inode_size = 1; // in terms of blocks
	uint64_t total_inodes; // = total_blocks / inode_size
	uint64_t data_block_start; // reserved_blocks + 1
	uint16_t next_volumes; // next volumes. no limit on number of indirections
} __attribute((packed));

// size: 8 + 8 + 1 + 8 * 4 = 49
// rest of the size is for file name;
struct inode {
	uint64_t owner_group_id;
	uint64_t permissions;
	char isDirectory;
	char filename[207];
	uint64_t data_blocks[10];
	uint64_t indirect1[10];
	uint64_t indirect2[10];
	uint64_t indirect3[10];
};

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
  * @brief  creates empty disk image with one volume
  * @param  name: name of the first volume to save in super block
  * @param  path: relative path with filename
  * @param  size: size in bytes
  * @retval None
  */
void createDiskImage(string name, string path, size_t size) {
	int fd = open(path.c_str(), O_CREAT | O_RDWR | O_TRUNC /*| O_EXCL*/, S_IRUSR | S_IWUSR);
	if(fd < 0) {
		perror(("Error creating file " + path).c_str());
		exit(EXIT_FAILURE);
	}
	FILE* file = fdopen(fd, "w+");
	super_block sb;
	
	strcpy(sb.volume_name, name.substr(0, 15).c_str());
	sb.block_size = 256;
	sb.reserved_blocks = 10;
	sb.inode_size = 1;

	size_t overhead = sizeof(super_block) + 10; // 54 + 10
	sb.total_blocks = (size - overhead) / sb.block_size;
	sb.total_inodes = sb.total_blocks / 30;
	if(sb.total_inodes < 100) {
		fprintf(stderr, RED "disk too small. Minimum disk size: %i KB\n" RESET, (30*100*sb.block_size + overhead)/KB(1) + 1);
	}
	// sb.total_inodes = sb.total_blocks - sb.reserved_blocks;
	uint64_t inodes_start = overhead + sb.reserved_blocks * sb.block_size;
	sb.data_block_start = inodes_start + sb.total_inodes * sb.block_size;
	
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
	string deneme = "inode\n";
	fwrite(deneme.c_str(), deneme.length(), 1, file);


	fclose(file);
}


int main(int argc, char** argv)
{
	createDiskImage("disk_name","disk.img", MB(1));
	return 0; 
}