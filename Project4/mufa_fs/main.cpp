#include "defs.hpp"
#include <limits>
#include "colors.h"
#include <fcntl.h>
#include <errno.h>
#include <stdexcept>
#include <exception>
#include <tuple>
#include <queue>
#include <sstream>

#include <readline/readline.h>
#include <readline/history.h>


#define KB(x)   ((size_t) (x) << 10)
#define MB(x)   ((size_t) (x) << 20)
#define GB(x)   ((size_t) (x) << 30)
#define TB(x)   ((size_t) (x) << 40)
#define nbit_char(c,n) ((n >> (8-k)) & 1)
#define ROOT_SELF -1
// #define DEBUG
// #define TEST

// watch "echo $OFFSET_TEXT; cat drive.img | xxd | head -c5000"
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

// size: 1 + 8 + 8 + 80 + 8*3 = 121
struct inode {
	char type; // 0: not occupied, 1: file, 2: directory, 3: symlink
	uint64_t parent; // parent index
	uint64_t self; // self index

	uint64_t data_blocks[10];
	uint64_t indirect1;
	uint64_t indirect2;
	uint64_t indirect3;
} __attribute((packed));

struct directory_entry {
	vector<tuple<string, uint64_t>> subdir;
} __attribute((packed));



void write_inode(inode* _inode, FILE* drive, super_block* sb) {
	fseek(drive, sb->inodes_start + _inode->self * sb->inode_size * sb->block_size, SEEK_SET);
	#if defined DEBUG
	printf(GREEN "now at 0x%X\n" RESET, ftell(drive));
	dumpbytes((unsigned char*)_inode, sizeof(inode));
	#endif
	fwrite(_inode, sizeof(inode), 1, drive);
}

void write_root_inode(inode& _inode, FILE* drive, super_block* sb) {
	size_t index = 0; // inode index to write
	fseek(drive, sb->inodes_start + index * sb->inode_size * sb->block_size, SEEK_SET);
	#if defined DEBUG
	printf(GREEN "now at 0x%X\n" RESET, ftell(drive));
	dumpbytes((unsigned char*)&_inode, sizeof(inode));
	#endif
	fwrite(&_inode, sizeof(inode), 1, drive);
}
void mkdir_root(FILE* drive, super_block* sb) {
	inode _inode;
	memset(&_inode, '\0', sizeof(inode));
	_inode.type = 2;
	_inode.parent = -1;
	_inode.self = ROOT_SELF;
	_inode.data_blocks[0] = 0; // It's already 0, but good to have things explicit
	write_root_inode(_inode, drive, sb);
}



void print_superblock(super_block* sb) {
	printf(BOLDWHITE "volume_name: %s\n" RESET, sb->volume_name);
	printf(BOLDWHITE "block_size: %i\n" RESET, sb->block_size);
	printf(BOLDWHITE "total_blocks: %i\n" RESET, sb->total_blocks);
	printf(BOLDWHITE "reserved_blocks: %i\n" RESET, sb->reserved_blocks);
	printf(BOLDWHITE "inode_start: %i\n" RESET, sb->inodes_start);
	printf(BOLDWHITE "inode_size: %i\n" RESET, sb->inode_size);
	printf(BOLDWHITE "total_inodes: %i\n" RESET, sb->total_inodes);
	printf(BOLDWHITE "data_block_start: %i\n" RESET, sb->data_block_start);
	printf(BOLDWHITE "total_data_blocks: %i\n" RESET, sb->total_data_blocks);
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

	#if defined DEBUG
	// print superblock
	print_superblock(&sb);
	dumpbytes((unsigned char*)&sb,sizeof(super_block));
	#endif


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
	
	#if defined DEBUG
	printf("now at 0x%X\n", ftell(file));
	#endif

	mkdir_root(file, &sb);

	fclose(file);
}

// size_t mmap_inodes(FILE* drive, super_block* sb, char* &buf) {
// 	size_t inodes_len = sb->inode_size * sb->block_size * sb->total_inodes;
// 	(buf = (char*)malloc(inodes_len));
// 	fread(buf, inodes_len, 1, drive);
// 	return inodes_len;
// 	// return ((buf = (char*)malloc(inodes_len)) != NULL);
// }


/**
 * @brief  returns all superblocks from drive image. currently only reads first superblock
 * @param  drive: virtual drive file
 * @retval array of super blocks
 */
vector<super_block> read_superblock(FILE* drive) {
	fseek(drive, 0x00, SEEK_SET);
	super_block sb;
	memset(&sb, '\0', sizeof(super_block));
	fread(&sb, sizeof(super_block), 1, drive);
	return {sb};
}




class Instance {
public:
	FILE* drive; // virtual drive file
	super_block* sb; // superblock for the volume

	vector<inode> inodes; // ALL inodes
	vector<bool> data_bitmap; // allocation bitmap for all data blocks. c++ std library uses bits for vector<bool> to save space
	// priority queue to find unoccupied inode indices easier
	struct comp {
		bool operator()(const inode *l, const inode *r){
			if(l->type == 0) {
				if(r->type == 0)
					return r->self > l->self;
				return false;
			}
			if(r->type == 0) return true;
			return r->self > l->self;
		}};
	std::priority_queue<inode*, vector<inode*>, comp> inodes_sorted;
	inode* cwd_node; // current working directory's inode
	directory_entry cwd; // current working directory's entry

	// pre-calculate variables
	uint64_t inode_size_byte;

	Instance() {init();}
	Instance(FILE* file, super_block* sb) {
		this->drive = file;
		this->sb = sb;
		init();
	}
	/**
	 * @brief  returns an unallocated inode
	 * @note   don't forget to reinsert the inode into the priority queue after use!
	 * @param  inodes_queue: array of ALL inodes 
	 * @retval reference to inode. If none available, returns 0. Inode index 0 is reserved for root
	 */
	inode* find_inode() {
		inode* _inode = inodes_sorted.top();
		inodes_sorted.pop();
		return _inode;
	}
	void insert_dir_entry(string name, inode* parent, inode* _inode) {
		directory_entry p_dir = get_dir_entry(parent); // read parent directory inode and dir_entry
		p_dir.subdir.push_back({name, _inode->self});
		string dir_entry_str;
		#if defined DEBUG
		if(0) { // debug test
			for(size_t i=0; i<p_dir.subdir.size(); ++i) {
				dir_entry_str += get<0>(p_dir.subdir[i]) + '\0' + to_string(get<1>(p_dir.subdir[i])) + '\0';
			}
			printf("dir_entry:"); cout << dir_entry_str << endl;
			dumpbytes((u_char*)dir_entry_str.c_str(),dir_entry_str.length());
			dir_entry_str = "";
			for(size_t i=0; i<p_dir.subdir.size(); ++i) {
				dir_entry_str += get<0>(p_dir.subdir[i]).c_str() + to_string(get<1>(p_dir.subdir[i]));
			}
			printf("dir_entry:%s\n", dir_entry_str.c_str());
			dumpbytes((u_char*)dir_entry_str.c_str(),dir_entry_str.length());
		}
		#endif
		for(size_t i=0; i<p_dir.subdir.size(); ++i)
			dir_entry_str += get<0>(p_dir.subdir[i]) + '\0' + to_string(get<1>(p_dir.subdir[i])) + '\0';
		write_file(parent, (u_char*)dir_entry_str.c_str(), dir_entry_str.length());

		#if defined DEBUG
		printf("p_dir:\n");
		for(int i=0; i<p_dir.subdir.size();++i) {
			printf("%s%i\n", get<0>(p_dir.subdir[i]).c_str(), get<1>(p_dir.subdir[i]));
		}
		#endif
	}
	void mkdir(inode* parent, string name) {
		inode* _inode = find_inode();
		// memset(&_inode, '\0', sizeof(inode));
		_inode->type = 2;
		if(parent->self == -1)
			_inode->parent = 0;
		else
			_inode->parent = parent->self;

		assert(parent != NULL);
		insert_dir_entry(name, parent, _inode);
		write_inode(_inode, drive, sb); // commit changes to drive
		inodes_sorted.push(_inode); // reinsert
	}
	void mkdir(string name) {
		mkdir(cwd_node, name);
	}
	/**
	 * @brief  create a new file and write it to parent's dir_entry
	 * @param  name: filename
	 * @retval inode reference
	 */
	inode* touch(inode* parent, string name) {
		inode* _inode = find_inode();
		_inode->parent = parent->self;
		_inode->type = 1;
		insert_dir_entry(name, parent, _inode);
		write_inode(_inode, drive, sb); // commit changes to drive
		#if defined DEBUG
		printf(YELLOW "touch inode index: %lu\n" RESET, _inode->parent);
		fseek(drive, sb->inodes_start + _inode->self * sb->inode_size * sb->block_size, SEEK_SET);
		char buf[50];
		fread(buf, 50, 1, drive);
		dumpbytes((u_char*)buf, 50);
		printf("\n");
		dumpbytes((u_char*)_inode, sizeof(inode));
		#endif
		return _inode;
	}
	inode* touch(string name) {
		touch(cwd_node, name);
	}

	/**
	 * @brief  this is rm -rf
	 * @retval true if item found&removed. false if not found
	 */
	bool rm(inode* parent, inode* _inode) {
		auto dir_entry = get_dir_entry(parent);
		for(size_t i=0; i<dir_entry.subdir.size(); ++i) {
			if(get<1>(dir_entry.subdir[i]) == _inode->self) {
				if(_inode->type == 2) {
					auto subdir_entry = get_dir_entry(_inode);
					for(auto &v : subdir_entry.subdir) {
						rm(_inode, &inodes[get<1>(v)]);
					}
				}
				purge_file_contents(_inode);
				dir_entry.subdir.erase(dir_entry.subdir.begin() + i);
				string dir_entry_str;
				for(size_t i=0; i<dir_entry.subdir.size(); ++i)
					dir_entry_str += get<0>(dir_entry.subdir[i]) + '\0' + to_string(get<1>(dir_entry.subdir[i])) + '\0';
				write_file(parent, (u_char*)dir_entry_str.c_str(), dir_entry_str.length());
				return true;
			}
		}
		return false;
	}
	
	void rm(inode* parent, string name) {
		auto dir_entry = get_dir_entry(parent);
		for(auto &v : dir_entry.subdir) {
			if(get<0>(v) == name) {
				uint64_t index = get<1>(v);
				inode* _inode = &inodes[index];
				rm(parent, _inode);
				return;
			}
		}
		fprintf(stderr, "file %s does not exist!\n", name.c_str());
	}

	void rm(string name) {
		rm(cwd_node, name);
	}

	void purge_file_contents(inode* _inode) {
		if(_inode->self == ROOT_SELF)
			data_bitmap[0] = 0;
		for(int i=0; i<10; ++i) {
			if(_inode->data_blocks[i] != 0)
				data_bitmap[_inode->data_blocks[i]] = 0; // keep track of unallocation
		}
		std::fill(begin(_inode->data_blocks), end(_inode->data_blocks), 0); // -TODO: indirection
	}

	void write_file(inode* _inode, unsigned char* data, uint64_t size) {
		purge_file_contents(_inode);
		size_t j = 0;
		// find unallocated data block
		for(size_t i=0; i<data_bitmap.size(); ++i) {
			if(data_bitmap[i] == 0) {
				data_bitmap[i] = 1; // keep track of allocation
				size_t chunk_size;
				if(size < sb->block_size) {
					chunk_size = size;
					size = 0;
				} else {
					chunk_size = sb->block_size;
					size -= sb->block_size;
				}
				// write_data_block
				uint64_t data_block_index = sb->data_block_start + i * sb->block_size;
				fseek(drive, data_block_index, SEEK_SET);
				purge(drive, sb->block_size); // zero-out the data block first // This is ridiculously unnecessary, but I will optimize later
				fseek(drive, -sb->block_size, SEEK_CUR); // go back 256 bytes (purge pushes forward. That was tricky to debug)
				fwrite(data, chunk_size, 1, drive);
				_inode->data_blocks[j++] = i;
				if(size == 0) break; // break when done
				data += chunk_size; // push pointer to next chunk
			}
		}
		write_inode(_inode, drive, sb); // commit changes to drive
	}
   	/**
	 * @brief  writes to file. if file does not exist, creates one
	 * @param  name: file name
	 * @param  data: file content
	 * @retval None
	 */
	bool write_file(string name, string data) {
		auto dir_entry = get_dir_entry(cwd_node);
		for(auto &v : dir_entry.subdir) {
			if(get<0>(v) == name) {
				uint64_t index = get<1>(v);
				inode* _inode = &inodes[index];
				if(_inode->type != 1) {
					fprintf(stderr, "%s is not a file!\n", name.c_str());
					return false;
				}
				write_file(_inode,(u_char*)data.c_str(), data.length());
				return true;
			}
		}
		// file does not exist:
		inode* _inode = touch(cwd_node, name);
		write_file(_inode,(u_char*)data.c_str(), data.length());
		return true;
	}
	string read_file(inode* _inode) {
		if(_inode->data_blocks[0] == 0) return "";
		string data;
		char buffer[sb->block_size];
		fseek(drive, sb->data_block_start + _inode->data_blocks[0] * sb->block_size, SEEK_SET);
		fread(buffer, sb->block_size, 1, drive);
		data += buffer; // meh..
		for(int i=1; i<10; ++i) {
			if(_inode->data_blocks[i] == 0) break;
			fseek(drive, _inode->data_blocks[i] - _inode->data_blocks[i-1], SEEK_CUR);
			fread(buffer, sb->block_size, 1, drive);
			data += buffer;
		}
		return data;
	}
	string read_file(string name) {
		auto dir_entry = get_dir_entry(cwd_node);
		for(auto &v : dir_entry.subdir) {
			if(get<0>(v) == name) {
				uint64_t index = get<1>(v);
				inode* _inode = &inodes[index];
				if(_inode->type != 1) {
					fprintf(stderr, "%s is not a file!\n", name.c_str());
					return "";
				}
				return read_file(_inode);
			}
		}
		fprintf(stderr, "file %s does not exist!\n", name.c_str());
		return "";
	}
	/**
	 * @brief  common init function for constructors
	 */
	void init() {
		// pre-calculate variables
		inode_size_byte = sb->inode_size * sb->block_size;
		// read all inodes
		fseek(drive, sb->inodes_start, SEEK_SET); // go to the index 0 of inodes
		// allocate space for all inodes in memory, init to NULL
		inodes.resize(sb->total_inodes * inode_size_byte);
		// allocate space for block allocation bitmap, init to NULL
		data_bitmap.resize(sb->total_data_blocks);
		int j;
		data_bitmap[0] = 1; // data_block index 0 belongs to the root directory
		for(size_t i=0; i<sb->total_inodes; ++i) {
			inode* _inode = &inodes[i];
			if(!fread(_inode, inode_size_byte, 1, drive)) {
				fprintf(stderr, RED "could not read directory!\n" RESET);
				return;
			}
			if(_inode->type == 0)
				_inode->self = i; // augment inode.self in memory to real index when it is 0 (when inode is not occupied)	
			else {
				// populate block allocation bitmap
				j = 0;
				while(_inode->data_blocks[j++]) {
					data_bitmap[_inode->data_blocks[j]] = 1;
				} // -TODO indirections
			}
			inodes_sorted.push(&inodes[i]);
			
			// printf(BOLDBLUE "now at 0x%X\n" RESET, ftell(drive));
		} // this for loop will not do n disk requests, as the OS returns larger results from disk to increase effectiveness already. Reading the whole thing into vector directly is a problem with fread

		/*for(int i=0; i<sb->total_inodes; ++i) {
			inode* v = inodes_sorted.top();
			printf("self:%i\t\ttype:%i\n", v->self, v->type);
			inodes_sorted.pop();
		}*/
		#if defined DEBUG
		for(size_t i=0; i<200; ++i) {
			cout << data_bitmap[i];
		}
		#endif
		cout << endl;

		// cd into root directory
		cwd_node = &inodes[0];
		#if defined DEBUG
		printf(YELLOW "read root inode:\n" RESET);
		dumpbytes((unsigned char*)&cwd_node, sizeof(inode));
		#endif
		cwd = get_dir_entry(cwd_node);
	}
	void writeSomething2() {
		inode* f = touch(cwd_node, "asdf");
		char* str = "lalalasometnsometn";
		write_file(f, (u_char*)str, strlen(str));
		printf("file content:\n%s\n", read_file(f).c_str());
	}

	void writeSomething() {
		tuple<string, uint64_t> subdir = {"subdir", 0};
		cwd.subdir.push_back(subdir);
		fseek(drive, sb->data_block_start + cwd_node->data_blocks[0] * sb->inode_size * sb->block_size, SEEK_SET);
		string dirstr;
		for(auto [name, p] : cwd.subdir) {
			dirstr += name + to_string(p);
		}
		printf("dirstr:%s\n", dirstr.c_str());
		// fwrite(&cwd.subdir,)
	}

	/**
	 * @brief  reads and generates a directory entry structure for given the inode for the directory
	 * @param  _inode: inode for the directory
	 * @retval directory entry populated at run time. Changes to entry need to be committed
	 */
	directory_entry get_dir_entry(inode* _inode){
		directory_entry dir;
		string cur = "";
		for(int i=0; i<10; ++i) {
			if(_inode->data_blocks[i] != 0 || (_inode->self==ROOT_SELF && i == 0)) { // only first data block on root directory can have index 0
				fseek(drive, sb->data_block_start + _inode->data_blocks[i] * sb->block_size, SEEK_SET);
				#if defined DEBUG
				printf(GREEN "now at 0x%X\n" RESET, ftell(drive));
				#endif
				char data[sb->block_size]; // I thought this allocation was illegal in c++ hehe
				char* p = data;
				fread(data, 1, sb->block_size, drive); // filling into an array, items each of size 1
				size_t l = 0;
				#if defined DEBUG
				printf(RED);
				dumpbytes((u_char*)data, 256);
				printf(RESET);
				#endif
				while((l = strlen(p)) > 0) {
					string dirname(p);
					p += l+1; // skip name and padding zero
					if((l = strlen(p)) <= 0) break; // corrupt file entry
					string addr_str(p);
					p += l+1; // skip address and padding zero
					uint64_t addr;
					istringstream iss(addr_str); iss >> addr;
					dir.subdir.push_back({dirname, addr});
				}
			}
		}
		return dir;
	}
	void cd(inode* _inode){
		cwd = get_dir_entry(_inode);
		cwd_node = _inode;
	}
	void cd(string name){
		auto dir_entry = get_dir_entry(cwd_node);
		for(auto &v : dir_entry.subdir) {
			if(get<0>(v) == name) {
				uint64_t index = get<1>(v);
				inode* _inode = &inodes[index];
				if(_inode->type != 2) {
					fprintf(stderr, "%s is not a directory!\n", name.c_str());
				}
				cwd = get_dir_entry(_inode);
				cwd_node = _inode;
				return;
			}
		}
		fprintf(stderr, "directory %s does not exist\n", name.c_str());
	}
	void ls(inode* dir) {
		string res;
		directory_entry dir_entry = get_dir_entry(dir);
		for(auto &v : dir_entry.subdir) {
			res += get<0>(v) + "\n";
		}
		cout << res;
	}
	void ls() {
		string res;
		directory_entry dir_entry = get_dir_entry(cwd_node);
		for(auto &v : dir_entry.subdir) {
			res += get<0>(v) + "\n";
		}
		cout << res;
	}
};

enum class command {
    ls = 0,
    cd = 1,
	cat = 2,
	mkdir = 3,
	touch = 4,
	write = 5,
	rm = 6,
	reformat = 7
};
std::map<string, command> built_in_commands;

char promptBuffer[PATH_MAX];
char* prompt() {
    sprintf(promptBuffer, "mufafs" GREEN " ➜ " RESET);
    cout << promptBuffer;
    return "";
}

enum class redirection
{none, pipe, redir_output, redir_input, append_output};
typedef struct exec
{
    pid_t pid;
    string path;
    bool built_in;
    vector<string> args;
    redirection redir;
    bool background;
};
void built_in_func(vs argv, Instance* instance) {

    command cmd = built_in_commands[argv[0]];
    switch (cmd) {
        case command::cd: {
			if(argv[1] == "..")
				instance->cd(&instance->inodes[instance->cwd_node->parent]);
			else 
            	instance->cd(argv[1]);
            break;
        }
		case command::ls: {
			instance->ls();
            break;
        }
		case command::cat: {
			cout << instance->read_file(argv[1]) << endl;
			break;
		}
		case command::mkdir: {
			instance->mkdir(argv[1]);
			break;
		}
		case command::touch: {
			instance->touch(argv[1]);
			break;
		}
		case command::write: {
			instance->write_file(argv[1], argv[2]);
			break;
		}
		case command::rm: {
			instance->rm(argv[1]);
			break;
		}
		case command::reformat: {
			if(argv.size() < 2)
				fprintf(stderr, "enter size in MB!\n");
			else
				createDriveImage("drive_name", "drive.img", MB(stoi(argv[1])));
			break;
		}
        default:
            break;
    }
}
void parseCommands(Instance* instance) {
    built_in_commands = (std::map<string, command>){
        {"ls" , command::ls},
        {"cd", command::cd},
        {"cat", command::cat},
		{"mkdir", command::mkdir},
		{"touch", command::touch},
		{"write", command::write},
		{"rm", command::rm},
		{"reformat", command::reformat}
    };

    char* buf;
    std::string cmd_line; // string to store next command line
    while ((buf = readline(prompt())) != nullptr) {
		
        cmd_line = std::string(buf);

        if (strlen(buf) > 0) {
            add_history(buf);
        }
        
        // lexer ##########################3

        vector<exec> cmds;
        exec first = {
            .pid = NULL,
            .path = "",
            .built_in = false,
            .args = (vs){""},
            .redir = redirection::none,
            .background = false
        };
        int currentCMD = 0;
        int currentArg = 0;
        int currentStrOffset = 0;


        cmds.push_back(first);
        size_t exec_i;
        
        size_t c = cmd_line.length();

        bool ignorespace = false;
        for(int i=0; i<c; ++i) {
            char cur = cmd_line[i];
            struct exec *curCmd = &cmds[currentCMD];
            
            if(isvalid(cur)) {
                ignorespace = false;
                curCmd->args[currentArg] += cur;
            } else {
                if(cur == ' ') {
                    if(ignorespace) { continue;}
                    ignorespace = true;
                    curCmd->args.push_back("");

                    ++currentArg;
                } else if(cur == ';' || cur == '|' || cur == '>' || cur == '&') {
                    ignorespace = true;
                    
                    if(curCmd->args[currentArg] == "") {
                        curCmd->args.pop_back();
                    }
                    if(cur == ';') curCmd->redir = redirection::none;
                    else if(cur == '|') curCmd->redir = redirection::pipe;
                    else if(cur == '>') {
                        if(i != c-1 && cmd_line[i+1] == '>') {
                            curCmd->redir = redirection::append_output;
                            ++i;
                        }
                        else curCmd->redir = redirection::redir_output;
                    } else if(cur == '<' && currentCMD != 0) {
                        cmds[currentCMD-1].redir = redirection::redir_input;
                    } else if(cur == '&') {
                        cmds[currentCMD].background = true;
                    }
                    ++currentCMD;
                    currentArg = 0;
                    cmds.push_back((exec) {
                        .pid = NULL,
                        .path = "",
                        .built_in = false,
                        .args = (vs){""},
                        .redir = redirection::none,
                        .background = false
                    });
                } else if(cur == '\\') {
                    // take next char no matter what
                    assert(cmd_line.size() >= i+1);
                    curCmd->args[currentArg] += cmd_line[++i];
                } else if(cur == '\'') {
                    bool escape = false;
                    
                    for(i++;i<c;++i) {
                        char cur2 = cmd_line[i];
                        if(cur2 == '\'' && !escape) {
                            break;
                        } else if(cur2 == '\\') {
                            escape = !escape;
                        } else {
                            escape = false;
                            curCmd->args[currentArg] += cur2;
                        }
                    }
                } else if(cur == '#') {
                    for(i++;i<c;++i) {
                        if(cmd_line[i] == '#') {
                            break;
                        }
                    }
                }
            }
        }

		built_in_func(cmds[0].args, instance); // no redirection stuff

	}
}


int main(int argc, char** argv)
{
	string drive_file = "drive.img";
	#if defined TEST
	createDriveImage("drive_name", drive_file, MB(2));
	#endif
	// return 0;
	int fd = open(drive_file.c_str(), O_RDWR /*| O_EXCL*/, S_IRUSR | S_IWUSR);
	FILE* file = fdopen(fd, "rb+");
	if(fd < 0) {
		createDriveImage("drive_name", drive_file, MB(2));
		fd = open(drive_file.c_str(), O_RDWR /*| O_EXCL*/, S_IRUSR | S_IWUSR);
		file = fdopen(fd, "rb+");
	}
	if(fd < 0) {
		perror("Error reading file drive.img");
		exit(EXIT_FAILURE);
	}
	printf(GREEN "opened virtual disk: %s\n" RESET, drive_file.c_str());
	printf(MAGENTA "reading super_block:\n" RESET);
	vector<super_block> sb = read_superblock(file);
	print_superblock(&sb[0]);

	printf(MAGENTA "creating a user instance\n" RESET);
	Instance instance(file,&sb[0]);
	inode* &cwd_node = instance.cwd_node;

	#if defined TEST
	instance.mkdir(cwd_node, "hello");
	instance.mkdir(cwd_node, "something");
	instance.mkdir(cwd_node, "yey");
	instance.mkdir(cwd_node, "subdirs!");
	instance.ls();
	printf(CYAN "---\n" RESET);
	instance.cd("hello");
	instance.mkdir(cwd_node, "dirdir");
	instance.touch(cwd_node, "this_is_a_file");
	instance.ls();
	instance.write_file("this_is_a_file", "datadatadata\nasdfdata");
	printf(GREEN"content of this_is_a_file:" RESET "\n%s\n", instance.read_file("this_is_a_file").c_str());
	printf(RED "--rm--\n" RESET);
	instance.rm("this_is_a_file");
	instance.ls();
	#else
	parseCommands(&instance);
	#endif

	fclose(file);
	return 0;
}