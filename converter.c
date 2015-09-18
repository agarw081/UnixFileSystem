
/*****headers******/
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<stdio.h>
#include "converter.h"


//This function takes in an input file and an output file. It uses fork to create a child process to convert the file using exec.
void converter(char* infile, char* outfile)
{
  
  pid_t child_pid;
  
  
  child_pid = fork();
  if(child_pid == 0)         //This is the child
    {
      printf("Converting file:%s\n",infile);
      execlp("/usr/bin/convert","usr/bin/convert","-thumbnail","200x200",infile,outfile,(char*) 0);
      exit(0);
    }
  else
    while(wait(NULL)>0);
}
      
//This function takes in the file name and the corresponding thumbnail and creates and writes the image references and image sources to the html file.
void createhtml(char* file_name, char* thumb, char* hfilename,char* path)
{
  
FILE *fp;
fp = fopen(hfilename, "a");
 char* filepath = malloc(200);
 char* thumbpath = malloc(200); 
 strcpy(filepath,path);
 strcat(filepath,"/");
 strcat(filepath, file_name);
 
 strcpy(thumbpath,path);
 strcat(thumbpath,"/");
 strcat(thumbpath,thumb);
 
fprintf(fp, "<a href=\"");
fprintf(fp, file_name);
fprintf(fp, "\">\n<img src=\"");
fprintf(fp, thumb);
fprintf(fp, "\"/></a>\n\n");

}



