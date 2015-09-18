/*The following C-program is a test program for the simulation of the mini 
  file system written in mini_filesystem.c. This program looks for jpg files
  and writes them to the system. It then reads this 'data' and does an actual 
  write to the disk. This program also creates thumbnails for the images 
  and an html file containing the image thumbnails.
*/

/********header files*******/
#include<stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "mini_filesystem.h"
#include "converter.h"

/*****required global variables*****/
int sz;
char* data;
char* Log_Filename;
int ch;
int i;
int j;

/*****Some helper functions*****/

//This function takes in a file name and determines if it is a jpg.
int is_jpg(char* file_name)
{
  int i;
  if(strstr(file_name,"jpg"))
    return 1;
    
  else return 0;
}


//Thie function searches the directory-tree for any jpgs and writes them to the system.
int find_jpgs(char* in_dir, char* outdir)
{
  DIR *indir;
  struct dirent* file;
  if ((indir = opendir(in_dir)) == NULL) {
    perror ("Unable to open directory");
    return -1;
  }
  
  
  while ((file = readdir(indir))!=NULL) 
    {
      
      if(file->d_name[0]=='.')
	continue;
      else {	

	if (file->d_type == DT_DIR) 
	  {
	    char* path2 = malloc(sizeof(char) * 200);
	    printf("Entering directory:%s\n",file->d_name);  
	    strcpy(path2, in_dir);
	    strcat(path2, "/");
	    strcat(path2, file->d_name);
	    find_jpgs(path2, outdir);         //Call the method find_jpgs again on this directory 
	  }
      }
      
      if (is_jpg(file->d_name))              
	{                                                  //Found jpg
	  char* path2 = malloc(sizeof(char) * 200);   
	  char* outpath = malloc(sizeof(char) * 200);	  
	  strcpy(path2, in_dir);
	  strcat(path2, "/");
	  strcat(path2, file->d_name);
	  
	  strcpy(outpath, outdir);
	  strcat(outpath, "/");
	  strcat(outpath, file->d_name);
	  
	  writeData(path2);
	  sysWrite(file->d_name, outpath);
	}
    }
  return 0;
}

//This function writes the file data of file_name into the 'data' array. It is very similar to the first part of copy.c discussed in recitation 8.
void writeData(char* file_name)    
{
  FILE *filepoint;
  filepoint = fopen(file_name, "r");
  if (filepoint == NULL) 
    {
      perror("Can't open file");
      exit(1);
    }
  fseek(filepoint, 0L, SEEK_END);
  sz = ftell(filepoint);
  fseek(filepoint, 0L, SEEK_SET);   
  data = (char*) malloc(sizeof(char)*sz);
  i = 0;
  while( (ch = fgetc(filepoint)) != EOF ) {
    data[i] = ch;
    i++;
  }
  fcloseall();
}



//This function writes the data of file_name by reading it from the mini_filesystem. It is very similar to the second part of copy.c discussed in recitation 8.
int actualWrite(char* file_name, int numbytes, char* output)  
{
  printf("Writing %s to output directory\n",file_name);
  int new_inode = Open_File(file_name);
  FILE *filepoint;
  filepoint = fopen(output, "w+");
  if (filepoint == NULL) 
    {
      perror("Unable to open file");
      fclose(filepoint);
      exit(1);
    }
  j = 0;
  while(j < numbytes) 
    {
      fputc(data[j], filepoint);
      j++;
    }
  Close_file(new_inode);
  fcloseall();
  return 0;
}

//This function creates a file in the mini_filesystem for file_name. It then calls the actualWrite function to write to the disk.
int sysWrite(char* file_name, char* output)
{
  printf("Writing %s into mini_filesytem\n",file_name);
  int new_inode = Create_File(file_name);
  if (new_inode == - 1)
    {
	perror("Error in creating node and writing into system\n");
      return 0;
    }
  Write_File(new_inode, 0, data, sz);
  actualWrite(file_name,sz,output);
  return sz;
}

//This function creates thumbnails for all the jpg images in the passed directory dir.
void create_thumbs(char* dir)
{
  DIR* indir;
  struct dirent* file;
  if((indir=opendir(dir))==NULL)
    {
      perror("Error in opening directory\n");
      exit(1);
    }
  while((file=readdir(indir))!=NULL)
    {
      int i;
      if(is_jpg(file->d_name))
	{
	  char* infile = malloc(200);
	  strcpy(infile,dir);
	  strcat(infile,"/");
	  strcat(infile,file->d_name);
	  
	  char* thumb_name = malloc(200);
	  for(i=0;file->d_name[i]!='.';i++)
	    {
	      thumb_name[i]=file->d_name[i];
	    }
	  thumb_name[i]='_';
	  strcat(thumb_name,"thumb.jpg");
	  
	  char* outfile = malloc(200);
	  strcpy(outfile,dir);
	  strcat(outfile,"/");
	  strcat(outfile,thumb_name);
	  
	  printf("JPG read from mini_filesystem:%s\n",file->d_name);
	  char* path = malloc(200);
	  strcpy(path,dir);
	  strcpy(path,"/");
	  strcat(path,file->d_name);
	  printf("input path given to converter:%s\n",infile);
	  printf("Converting:%s\n",file->d_name);
	  converter(infile,outfile);
	}
    }
}
	
	
  
  
//Main function initializes the file system and makes calls to the appropriate functions.
int main(int argc, char* argv[]){
if (argc != 4) 
  {
    perror("Wrong number of arguments\n");
    perror("Usage: ./test input_dir output_dir log_filename\n");
    exit(1);
  }

 struct stat st = {0};
 if (stat(argv[2], &st) == -1) 
   {
     perror("Output directory doesn't exist\nCreating output directory\n");
     mkdir(argv[2], 0700);
   }

 if(Initialize_Filesystem(argv[3])!=0)
   {
perror("File system could not be initialized\n");
exit(1);
   }
else perror("File system initialized successfully\n");
   
FILE *filepointer;
   char* html = malloc(100);
   strcpy(html, argv[2]);
   strcat(html,"/");
   strcat(html, "filesystem_content.html");
   filepointer = fopen(html, "w");
	perror("Creating webpage\n");
   fprintf(filepointer, "<html>\n<head><title>Mini_Filesystem</title></head><body>");
   fclose(filepointer);
   find_jpgs(argv[1], argv[2]);
   create_thumbs(argv[2]);
   DIR* outdir;
   struct dirent* file;
   
   if((outdir=opendir(argv[2])==NULL))
     {
       perror("Unable to open directory\n");
       return -1;
     }
   
   outdir=opendir(argv[2]);
   int i;
   while((file=readdir(outdir))!=NULL)
     {
       char* file_name = malloc(500);
       char* thumb_name = malloc(500);
       if(!strstr(file->d_name,"thumb")&&strstr(file->d_name,"jpg"))
	 {
	   strcpy(file_name,file->d_name);
	   for(i=0;file->d_name[i]!='.';i++)
	     {
	       thumb_name[i] =  file->d_name[i];
	     }
	   strcat(thumb_name,"_thumb.jpg");
	   
	   createhtml(file_name,thumb_name,html,argv[2]);
	 }
     }
   
   
   FILE* filepoints; 
   filepoints = fopen(html,"a");
   fprintf(filepoints, "</body></html>");
   perror("Webpage created\n");   
   fclose(filepoints);
   printf("The value of global count is:%d\n",Count);
   perror("Operations Complete\n");
   return 0;
   
}
