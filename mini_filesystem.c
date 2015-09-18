/****headers****/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "mini_filesystem.h"


/******Global Variables*****/
int Count;
int f_block;
int i;
int inodenumber;
int dir_count = 0;
int read_bytes;
int bread;
int boff;
int node_count = 0;
int num_of_blocks;
 

//This function searches for file_name in the Directory Structure and returns the inode_number if the file was found
int Search_Directory(char* file_name) 
{
  Count++;
  for (i = 0; i < dir_count; i++){
    if(strcmp(Directory_Structure[i].Filename, file_name) == 0)
      {
	return Directory_Structure[i].Inode_Number;
      }
  }
  
  return -1;
}


//This function adds file_name to the Directory Structure and returns 0 on completion
int Add_to_Directory(char* file_name, int iNode)
{
  Count++;
  Directory dir;
  strcpy(dir.Filename, file_name);
  dir.Inode_Number = iNode;
  
  Directory_Structure[dir_count] = dir;
  
  dir_count++;
  return 0;
}


//This function returns the inode correspoinding to the inode number iNode.
Inode Inode_Read(int iNode)
{
  Count++;
  for (i = 0; i < node_count; i++)
    {
      if (Inode_List[i].Inode_Number == iNode)
	{
	  return Inode_List[i];
	}
    }
 Inode nullNode;
  nullNode.Inode_Number = -1;
  nullNode.File_Size = 0;
  nullNode.Flag = -1;
  return nullNode;
  
}


//This function writes inode_structure to Inode_List corresponding to inodenumber
int Inode_Write(int inodenumber, Inode node_structure)
{
  Count++;
  Inode_List[inodenumber] = node_structure;
  if (Inode_List[inodenumber].Inode_Number != node_structure.Inode_Number){
    perror("Inode incorrect\n");
    return -1;
  }
  
  return 0;
}


int Block_Read(int bnum, int numbytes, char* string)
{
  Count++;
  string = malloc(numbytes * sizeof(char));
  char* buf = Disk_Blocks[bnum];
  strncpy(string, buf, numbytes);
  return numbytes;
}

int Block_Write(int bnum, char* string, int number_of_bytes)
{
  Count++;
  char* buf = malloc(512*sizeof(char));
  strcpy(buf, string);
  Disk_Blocks[bnum] = buf;
  return number_of_bytes;
}

Super_block Superblock_Read(void) 
{
  Count++;
  return Superblock;
}

int Superblock_Write(Super_block sup)
{
  Count++;
  Superblock = sup;
  if(Superblock.next_free_block != sup.next_free_block || Superblock.next_free_inode != sup.next_free_inode) {
    perror("Superblock incorrect\n");
    return -1;
  }
  
  return 0;	
}

int Initialize_Filesystem(char* log_file_name)
{
  FILE *fp;
  fp = fopen(log_file_name, "w+");
  
  if (fp == NULL) {
    return -1;
  }
  Count = 0;
  Superblock.next_free_block = 0;
  Superblock.next_free_inode = 0;
  if(Superblock.next_free_block != 0 || Superblock.next_free_inode != 0){
    return -1;
  }
  return 0;
}

int Create_File(char* file_name)
{
  if (Search_Directory(file_name) != -1) {
    perror("File already exists\n");
    return -1;
  }
  
  Inode inodecreated;
  Super_block super;
  super = Superblock_Read();
  int nextfreenode = super.next_free_inode;
  super.next_free_inode++;
  Superblock_Write(super);
  inodecreated.Inode_Number = inodenumber;
  inodecreated.User_Id = getuid();
  inodecreated.Group_Id = getgid();;
  inodecreated.File_Size = 0;
  inodecreated.Start_Block = 0;
  inodecreated.End_Block = 0;
  inodecreated.Flag = 0;
  Inode_Write(node_count, inodecreated);
  node_count++;
  Add_to_Directory(file_name, nextfreenode);
  return nextfreenode;
}

int Open_File(char* file_name)
{
  inodenumber = Search_Directory(file_name);
   if (inodenumber == -1) 
     { 
       perror("File not found\n"); 
       return -1; 
     }
  
  Inode new_node_create;
  new_node_create = Inode_Read(inodenumber);
  new_node_create.Flag = 1;
  
  Inode_Write(inodenumber, new_node_create);
  
  return inodenumber;
}

int Read_File(int inode_number, int offset, int count, char* str_read)
{
  read_bytes = 0;
  Inode new_node_create = Inode_Read(inode_number);
  if (offset > new_node_create.File_Size) 
    {
      perror("Error reading : Offset greater\n");
      return -1;
    }
  num_of_blocks = (count + 512 - 1) / 512;
  
  if(count <= 512) 
    {
      boff = offset/512;
      bread = new_node_create.Start_Block+boff;
      read_bytes = read_bytes + Block_Read(bread, count, str_read);	
    }
  
  else 
    {
      for (i = 0; i < num_of_blocks; i++) 
	{
	if (i == num_of_blocks-1) 
	  {
	    read_bytes += Block_Read(new_node_create.Start_Block + i, count % 512, str_read);	
	  }
	read_bytes += Block_Read(new_node_create.Start_Block + i, 512, str_read);
	}
    }
  
  return read_bytes;
}

int Write_File(int inode_number, int offset, char* to_write, int number_of_bytes) {
  Inode new_node_create = Inode_Read(inode_number);
  Super_block super = Superblock_Read();
  if (number_of_bytes <= 512)
    {
      f_block = super.next_free_block;
      Block_Write(f_block, to_write, number_of_bytes);
      new_node_create.File_Size = number_of_bytes;
      new_node_create.Start_Block = f_block;
      new_node_create.End_Block = new_node_create.Start_Block;
      Inode_Write(inode_number, new_node_create);
      super.next_free_block++;
      Superblock_Write(super);
    }
  
  else if (number_of_bytes > 512)
    {
      num_of_blocks = (number_of_bytes+512-1)/512;
      num_of_blocks = (number_of_bytes+512-1)/512;
      new_node_create.File_Size = number_of_bytes;
      new_node_create.Start_Block = f_block;
      
      for (i = 0; i < num_of_blocks; i++) 
	{
	  f_block = super.next_free_block;
	  
	  char* buff = malloc(sizeof(char)*512);
	  strncpy(buff, to_write+(512*i),512);
	  Block_Write(f_block,buff,512);
	  
	  super.next_free_block++;
	  Superblock_Write(super);
	}
      
      new_node_create.End_Block = f_block;
      Inode_Write(inode_number, new_node_create);
    }
  
  return number_of_bytes;
}

int Close_file(int inode_number)
{
  Inode new_node_create;
  new_node_create = Inode_Read(inode_number);
  new_node_create.Flag = 0;
  Inode_Write(inode_number, new_node_create);
  return 0;
}
