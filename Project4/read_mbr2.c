#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "colors.h"
#pragma pack(push, 1)


void dumpbytes(unsigned char* charPtr, size_t size) {
	for(int i=0; i<size; ++i) {
		printf("%02x ", charPtr[i]);
	}
	printf("\n");
}

typedef struct {
	unsigned char status; // 0x80 or 0x00. Otherwise invalid
	unsigned char start_chs[3];
	unsigned char partition_type;
	unsigned char end_chs[3];
	uint32_t start_sector;
	uint32_t length_sectors;
} __attribute((packed)) PartitionTable;
typedef struct {
	unsigned char aa;
	unsigned char start_chs[3];
	unsigned char partition_type;
	unsigned char end_chs[3];
	uint32_t start_sector;
} __attribute((packed)) asdf;

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    uint16_t sector_size;
    unsigned char sectors_per_cluster;
    uint16_t reserved_sectors;
    unsigned char number_of_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_short; // if zero, later field is used
    unsigned char media_descriptor;
    uint16_t fat_size_sectors;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;
    
    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    uint16_t boot_sector_signature;
} __attribute((packed)) Fat16BootSector;

typedef struct {
	unsigned char filename[8];
	unsigned char ext[3];
	unsigned char attributes;
	unsigned char reserved[10];
	uint16_t modify_time;
	uint16_t modify_date;
	uint16_t starting_cluster;
	unsigned int file_size;
} __attribute((packed)) Fat16Entry;

void print_file_info(Fat16Entry *entry) {
    switch(entry->filename[0]) {
    case 0x00:
        return; // unused entry
    case 0xE5:
        printf("Deleted file: [?%.7s.%.3s]\n", entry->filename+1, entry->ext);
        return;
    case 0x05:
        printf("File starting with 0xE5: [%c%.7s.%.3s]\n", 0xE5, entry->filename+1, entry->ext);
        break;
    case 0x2E:
        printf("Directory: [%.8s.%.3s]\n", entry->filename, entry->ext);
        break;
    default:
        printf("File: [%.8s.%.3s]\n", entry->filename, entry->ext);
    }
    
    printf("  Modified: %04d-%02d-%02d %02d:%02d.%02d    Start: [%04X]    Size: %d\n", 
        1980 + (entry->modify_date >> 9), (entry->modify_date >> 5) & 0xF, entry->modify_date & 0x1F,
        (entry->modify_time >> 11), (entry->modify_time >> 5) & 0x3F, entry->modify_time & 0x1F,
        entry->starting_cluster, entry->file_size);
}

int main(int argc, char** argv)
{
	FILE * in = fopen("sample.fat16", "rb");
	PartitionTable pt[4];
	Fat16BootSector bs;
	if(0) {
		fseek(in, 0x1BE, SEEK_SET); // go to partition table start
		fread(pt, sizeof(PartitionTable), 4, in); // read all four entries
		int i;
		for(i=0; i<4; ++i) {
			// printf("Partition %d\n type %02X\n", i, pt[i].partition_type);
			// printf(" start sector %08X\n %d sectors long\n", pt[i].start_sector, pt[i].length_sectors);
			printf(
				GREEN "Partition %d\n" RESET
				BOLDWHITE " status" RESET " %02X\n"
				BOLDWHITE " start_chs" RESET " %02X %02X %02X\n"
				BOLDWHITE " partition_type" RESET " %02X\n"
				BOLDWHITE " end_chs" RESET " %02X %02X %02X\n"
				BOLDWHITE " start_sector" RESET " %08X\n"
				BOLDWHITE " length_sectors" RESET " %08X\n",
				i,
				pt[i].status,
				pt[i].start_chs[0], pt[i].start_chs[1], pt[i].start_chs[2],
				pt[i].partition_type,
				pt[i].end_chs[0], pt[i].end_chs[1], pt[i].end_chs[2],
				pt[i].start_sector,
				pt[i].length_sectors
			);
			dumpbytes(&pt[i], sizeof(PartitionTable));
			if(pt[i].partition_type == 4 || pt[i].partition_type == 6 || pt[i].partition_type == 14) {
				printf( BOLDGREEN "FAT16 Filesystem found at partition" RESET " %d\n", i);
				break;
			}
		}
		if(i==4) {
			printf("No Fat16 filesystem found, exiting...\n");
			return -1;
		}
	}

	// fseek(in, 0x1020A, SEEK_SET);
	fseek(in, 0x0, SEEK_SET);
	fread(&bs, sizeof(Fat16BootSector), 1, in);
	printf("now at 0x%X\n", ftell(in));
	
    printf(BOLDWHITE "  Jump code:" RESET " %02X:" RESET "%02X:" RESET "%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);
    printf(BOLDWHITE "  OEM code:" RESET " [%.8s]\n", bs.oem);
    printf(BOLDWHITE "  sector_size:" RESET " %d\n", bs.sector_size);
    printf(BOLDWHITE "  sectors_per_cluster:" RESET " %d\n", bs.sectors_per_cluster);
    printf(BOLDWHITE "  reserved_sectors:" RESET " %d\n", bs.reserved_sectors);
    printf(BOLDWHITE "  number_of_fats:" RESET " %d\n", bs.number_of_fats);
    printf(BOLDWHITE "  root_dir_entries:" RESET " %d\n", bs.root_dir_entries);
    printf(BOLDWHITE "  total_sectors_short:" RESET " %d\n", bs.total_sectors_short);
    printf(BOLDWHITE "  media_descriptor:" RESET " 0x%02X\n", bs.media_descriptor);
    printf(BOLDWHITE "  fat_size_sectors:" RESET " %d\n", bs.fat_size_sectors);
    printf(BOLDWHITE "  sectors_per_track:" RESET " %d\n", bs.sectors_per_track);
    printf(BOLDWHITE "  number_of_heads:" RESET " %d\n", bs.number_of_heads);
    printf(BOLDWHITE "  hidden_sectors:" RESET " %d\n", bs.hidden_sectors);
    printf(BOLDWHITE "  total_sectors_long:" RESET " %d\n", bs.total_sectors_long);
    printf(BOLDWHITE "  drive_number:" RESET " 0x%02X\n", bs.drive_number);
    printf(BOLDWHITE "  current_head:" RESET " 0x%02X\n", bs.current_head);
    printf(BOLDWHITE "  boot_signature:" RESET " 0x%02X\n", bs.boot_signature);
    printf(BOLDWHITE "  volume_id:" RESET " 0x%08X\n", bs.volume_id);
    printf(BOLDWHITE "  Volume label:" RESET " [%.11s]\n", bs.volume_label);
    printf(BOLDWHITE "  Filesystem type:" RESET " [%.8s]\n", bs.fs_type);
    printf(BOLDWHITE "  Boot sector signature:" RESET " 0x%04X\n", bs.boot_sector_signature);
    

	fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors*bs.number_of_fats) * bs.sector_size, SEEK_CUR);
	printf("now at 0x%X\n", ftell(in));

	for(int i=0; i<bs.root_dir_entries; ++i) {
		Fat16Entry* entry;
		fread(&entry, sizeof(entry), 1, in);
		print_file_info(entry);
	}

	fclose(in);
	return 0; 
}
