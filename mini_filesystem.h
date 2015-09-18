typedef struct superblock
{
    int next_free_inode;
    int next_free_block;

}Super_block;

typedef struct inode
{
    int Inode_Number;
    int User_Id;
    int Group_Id;
    int File_Size;
    int Start_Block;
    int End_Block;
    int Flag;
}Inode;

typedef struct directory
{
    char Filename[16];
    int Inode_Number;
}Directory;

/* Declare Filesystem structures */
Super_block Superblock;
Directory Directory_Structure[128];
Inode Inode_List[128];
char* Disk_Blocks[8192];

/* Declare variable for Count and Log Filename */
extern int Count;
extern char* Log_Filename;
int direntCount;
int iNodeCount;

/* Filesystem Interface Declaration
See the assignment for more details */

int Initialize_Filesystem(char* log_filename);
int Create_File(char* filename);
int Open_File(char* filename);
int Read_File(int inode_number, int offset, int count, char* to_read);
int Write_File(int inode_number, int offset, char* to_write, int number_of_bytes);
int Close_file(int inode_number);

