#include "inodemap.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

/*
 * THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR
 * CODE WRITTEN BY OTHER STUDENTS - ANDREW CHOI
 */

uint32_t MAGICNUMBER = 0x7261746D;
void *tar_file(FILE *tarfileptr, char *directory);

void *tar_dir(FILE *tarfileptr, char *directory)
{
  DIR *dir;
  struct dirent * dent;
  if((dir = opendir(directory)) == NULL){
    perror("opendir()");
    exit(-1);
  }
  // create char array to keep track of path of file
  // traverse the directory
  while ((dent = readdir(dir)) != NULL) {
    if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){
      continue;
    }
    tar_file(tarfileptr, directory);
  }
  // within each traversal, call tar_file
  // if the file is deep inside, concatenate the file name to the char array
  if(closedir(dir) == -1){
    perror("closedir()");
    exit(-1);
  }
}

void *tar_file(FILE *tarfileptr, char *directory)
{
  struct stat stat_buf;

  DIR *dir;
  struct dirent *dent;
  if((dir = opendir(directory)) == NULL){
    perror("opendir()");
    exit(-1);
  }

  while ((dent = readdir(dir)) != NULL) {
    if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){
      continue;
    }
    // insert the files into the tar file
    // inode number, file name length, and file name
    char * file_with_path = malloc(sizeof(directory) + sizeof(dent->d_name + 1));
    strcpy(file_with_path, directory);
    strcat(file_with_path, "/");
    strcat(file_with_path, dent->d_name);
    if (lstat(file_with_path, &stat_buf) == -1)
    {
      perror("lstat()");
      exit(-1);
    }
  
    // check for dir (S_ISDIR)
    if(!get_inode(stat_buf.st_ino)){
      set_inode(stat_buf.st_ino, file_with_path);
      fwrite(&stat_buf.st_ino, sizeof(uint64_t), 1, tarfileptr);
      uint32_t filenamelength = strlen(file_with_path);
      fwrite(&filenamelength, sizeof(uint32_t), 1, tarfileptr);
      fwrite(file_with_path, sizeof(char), filenamelength, tarfileptr);

      if(S_ISDIR(stat_buf.st_mode)){
        // then we insert mode, modification time
        fwrite(&stat_buf.st_mode, sizeof(uint32_t), 1, tarfileptr);    //file mode
        fwrite(&stat_buf.st_mtime, sizeof(uint64_t), 1, tarfileptr);   //modif. time
        //traverse the directory depth first
        tar_dir(tarfileptr, file_with_path);
      }
      else { 
        // insert the mode, modification time, size and content
        fwrite(&stat_buf.st_mode, sizeof(uint32_t), 1, tarfileptr);    //file mode
        fwrite(&stat_buf.st_mtime, sizeof(uint64_t), 1, tarfileptr);   //modif. time
        fwrite(&stat_buf.st_size, sizeof(uint64_t), 1, tarfileptr);    //file size
        FILE *fp;
        if((fp = fopen(file_with_path, "r")) == NULL){
          perror("fopen()");
          exit(-1);
        }
        free(file_with_path);
        char *buffer = malloc(stat_buf.st_size);
        fread(buffer, sizeof(char), stat_buf.st_size, fp);
        fclose(fp);
        fwrite(buffer, sizeof(char), stat_buf.st_size, tarfileptr);
        free(buffer);
      }
    }
    else{ //hard link
      fwrite(&stat_buf.st_ino, sizeof(uint64_t), 1, tarfileptr);
      uint32_t filenamelength = strlen(file_with_path);
      fwrite(&filenamelength, sizeof(uint32_t), 1, tarfileptr);
      fwrite(file_with_path, sizeof(char), filenamelength, tarfileptr);
    }
  }

  if(closedir(dir) == -1){
    perror("closedir()");
    exit(-1);
  }
}

void *create_tar_file(char *tarfilename, char *directory){

  FILE *tarfileptr;
  struct stat stat_buf_dir; // keeps track of directory data
  // check if directory exists:
  if (lstat(directory, &stat_buf_dir) == -1){
    perror("lstat()");
    exit(-1);
  }
  
  if((tarfileptr = fopen(tarfilename, "w")) == NULL){
    perror("fopen()");
    exit(-1);
  }
  //length of the directory that is the main directory being tar'd
  uint32_t dirnamelength = strlen(directory);

  // write magic number
  fwrite(&MAGICNUMBER, sizeof(uint32_t), 1, tarfileptr);
  // add directory to tar_file;
  fwrite(&stat_buf_dir.st_ino, sizeof(uint64_t), 1, tarfileptr);    //inode number
  fwrite(&dirnamelength, sizeof(uint32_t), 1, tarfileptr);      //file name length
  fwrite(directory, sizeof(char), dirnamelength, tarfileptr); //file name
  fwrite(&stat_buf_dir.st_mode, sizeof(uint32_t), 1, tarfileptr);    //file mode
  fwrite(&stat_buf_dir.st_mtime, sizeof(uint64_t), 1, tarfileptr);   //modif. time

  tar_file(tarfileptr, directory);

  fclose(tarfileptr);
  
}

void *extract_tar_file(char *tarfilename)
{
  FILE *tarfileptr;
  if((tarfileptr = fopen(tarfilename, "r")) == NULL){
    perror("fopen()");
    exit(-1);
  }
  //check the magic number
  uint32_t *tarmagicnumber = malloc(sizeof(uint32_t));
  fread(tarmagicnumber, sizeof(uint32_t), 1, tarfileptr);
  if(*tarmagicnumber != MAGICNUMBER){
    fprintf(stderr, "Error: Bad magic number (%d), should be: %d.\n", *tarmagicnumber, MAGICNUMBER);
    exit(-1);
  }
  free(tarmagicnumber);

  while(feof(tarfileptr) == 0){
    uint64_t *inodenumber = malloc(sizeof(uint64_t));
    uint32_t *filenamelength = malloc(sizeof(uint32_t));
    
    fread(inodenumber, sizeof(uint64_t), 1, tarfileptr);
    if(feof(tarfileptr) != 0){
      continue;
    }
    fread(filenamelength, sizeof(uint32_t), 1, tarfileptr);
    char *filename = malloc(*filenamelength * sizeof(char)+1);
    fread(filename, sizeof(char), *filenamelength, tarfileptr);
    if(!get_inode(*inodenumber)){
      //checking mode
      uint32_t *filemode = malloc(sizeof(uint32_t));
      fread(filemode, sizeof(uint32_t), 1, tarfileptr);

      if(S_ISDIR(*filemode)){ //directory
        set_inode(*inodenumber, filename);
        uint64_t *modtime = malloc(sizeof(uint64_t));
        fread(modtime, sizeof(uint64_t), 1, tarfileptr);

        //create directory and change mode and time:
        if (mkdir(filename, S_IRWXU) == -1){
          perror("mkdir()");
          exit(-1);
        }

        struct stat stat_buf;
        if(lstat(filename, &stat_buf) == -1){
          perror("lstat()");
          exit(-1);
        }
        struct timeval tv;
        if ((tv.tv_sec = gettimeofday(&tv, NULL)) == -1){
          perror("gettimeodday()");
          exit(-1);
        }
        stat_buf.st_atime = tv.tv_sec;
        if(chmod(filename, stat_buf.st_mode) == -1){
          perror("chmod()");
          exit(-1);
        }
        free(modtime);
      }
      else if(S_ISREG(*filemode)){ //regular file/executable
        set_inode(*inodenumber, filename);
        uint64_t *modtime = malloc(sizeof(uint64_t));
        uint64_t *filesize = malloc(sizeof(uint64_t));

        fread(modtime, sizeof(uint64_t), 1, tarfileptr);
        fread(filesize, sizeof(uint64_t), 1, tarfileptr);

        char *filecontent = malloc(*filenamelength * sizeof(char)+1);
        fread(filecontent, sizeof(char), *filesize, tarfileptr);
        
        FILE * newfileptr;
        if((newfileptr = fopen(filename, "w")) == NULL){
          perror("fopen()");
          exit(-1);
        }

        struct stat stat_buf;
        if(lstat(filename, &stat_buf) == -1){
          perror("lstat()");
          exit(-1);
        }
        
        //copy file contents:
        fwrite(filecontent, sizeof(char), *filesize, newfileptr);
        fclose(newfileptr);

        //change time using utimes()
        struct timeval atime, mtime;
        struct timeval times[] = {atime, mtime};
        if ((times[0].tv_sec = gettimeofday(&times[0], NULL)) == -1){
          perror("gettimeodday()");
          exit(-1);
        }
        times[1].tv_sec = *modtime;
        times[1].tv_usec = 0;

        if(utimes(filename, times) == -1){
          perror("utimes()");
          exit(-1);
        }

        free(modtime);
        free(filesize);
        free(filecontent);
        if(chmod(filename, stat_buf.st_mode) == -1){
          perror("chmod()");
          exit(-1);
        }
        printf("%s created", filename);
      }
    }
    else{ //is a hard link
      const char * path = get_inode(*inodenumber);
      if(link(path, filename) != 0){
        perror("link()");
        exit (-1);
      }
    }
    free(inodenumber);
    free(filenamelength);
    free(filename);
  }
  fclose(tarfileptr);
}

void *print_tar_file(char *tarfilename)
{
  FILE *tarfileptr;
  if((tarfileptr = fopen(tarfilename, "r")) == NULL){
    perror("fopen()");
    exit(-1);
  }

  //check the magic number
  uint32_t *tarmagicnumber = malloc(sizeof(uint32_t));
  fread(tarmagicnumber, sizeof(uint32_t), 1, tarfileptr);
  if(*tarmagicnumber != MAGICNUMBER){
    fprintf(stderr, "Error: Bad magic number (%d), should be: %d.\n", *tarmagicnumber, MAGICNUMBER);
    exit(-1);
  }
  free(tarmagicnumber);
  //readfile function:
  while(feof(tarfileptr) == 0){
    uint64_t *inodenumber = malloc(sizeof(uint64_t));
    uint32_t *filenamelength = malloc(sizeof(uint32_t));
    
    fread(inodenumber, sizeof(uint64_t), 1, tarfileptr);
    if(feof(tarfileptr) != 0){
      continue;
    }
    fread(filenamelength, sizeof(uint32_t), 1, tarfileptr);

    char *filename = malloc(*filenamelength * sizeof(char)+1);
    fread(filename, sizeof(char), *filenamelength, tarfileptr);
    //checking mode
    uint32_t *filemode = malloc(sizeof(uint32_t));
    fread(filemode, sizeof(uint32_t), 1, tarfileptr);

    if(S_ISDIR(*filemode)){ //directory
      uint64_t *modtime = malloc(sizeof(uint64_t));

      fread(modtime, sizeof(uint64_t), 1, tarfileptr);
      //print:
      printf("%s/ -- inode: %lu, mode: %o, mtime: %lu\n", filename, *inodenumber, *filemode, *modtime);
      free(modtime);
    }
    else if(S_ISREG(*filemode)){ //regular/executable file
      uint64_t *modtime = malloc(sizeof(uint64_t));
      uint64_t *filesize = malloc(sizeof(uint64_t));

      fread(modtime, sizeof(uint64_t), 1, tarfileptr);
      fread(filesize, sizeof(uint64_t), 1, tarfileptr);

      //char *filecontent = malloc(*filenamelength * sizeof(char)+1);
      fseek(tarfileptr, *filesize, SEEK_CUR);
      //fread(filecontent, sizeof(char), *filesize, tarfileptr);
      //print:
      printf("%s -- inode: %lu, mode: %o, mtime: %lu, size: %lu\n", filename, *inodenumber, *filemode, *modtime, *filesize);
      free(modtime);
      free(filesize);
      //free(filecontent);
    }
    else{ //hard link
      //print:
      printf("%s/ -- inode: %lu\n", filename, *inodenumber);
    }
    free(inodenumber);
    free(filenamelength);
    free(filename);
  }
  fclose(tarfileptr);
}

int main(int argc, char *argv[])
{

  int cflag = 0;
  int xflag = 0;
  int tflag = 0;
  int fflag = 0;
  int numflag = 0;

  int c;

  char *tarfilename;
  char *directory;

  // process command line
  while ((c = getopt(argc, argv, "cxtf:")) != -1)
  {
    switch (c)
    {
    case 'c':
      cflag = 1;
      if (argc == 4)
      {
        fprintf(stderr, "Error: No directory target specified.\n");
        exit(-1);
      }
      directory = argv[argc - 1];
      numflag++;
      break;
    case 'x':
      xflag = 1;
      numflag++;
      break;
    case 't':
      tflag = 1;
      numflag++;
      break;
    case 'f':
      fflag = 1;
      tarfilename = optarg;
      break;
    }
  }
  // process bugs in the command line:
  if (numflag == 0)
  {
    fprintf(stderr, "Error: No mode specified.\n”");
    exit(-1);
  }
  if (numflag > 1)
  {
    fprintf(stderr, "Error: Multiple modes specified.\n");
    exit(-1);
  }
  if (fflag == 0 || tarfilename == NULL)
  {
    fprintf(stderr, "Error: No tarfile specified.\n");
    exit(-1);
  }

  // do the things
  if (cflag == 1)
  {
    if (tarfilename == NULL)
    {
      fprintf(stderr, "Error: Specified target (\"%s\") does not exist.\n", tarfilename);
      exit(-1);
    }
    if (opendir(directory) == NULL)
    {
      fprintf(stderr, "Error: Specified target (\"%s\") is not a directory.\n", directory);
      exit(-1);
    }

    create_tar_file(tarfilename, directory);
  }

  if (xflag == 1)
  {
    if (tarfilename == NULL)
    {
      fprintf(stderr, "Error: Specified target (\"%s\") does not exist.\n", tarfilename);
      exit(-1);
    }
    extract_tar_file(tarfilename);
  }

  if (tflag == 1)
  {
    if (tarfilename == NULL)
    {
      fprintf(stderr, "Error: Specified target (\"%s\") does not exist.\n", tarfilename);
      exit(-1);
    }
    print_tar_file(tarfilename);
  }

  return 0;
}
