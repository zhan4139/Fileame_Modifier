/* mini_filesystem.c
Commentary: Please Refer "README.txt"

Usage Requirements:
username@Directory/path:$ make
username@Directory/path:$ [test] [input_dir] [output_dir] [logfile]
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "mini_filesystem.h"
int inode_number = -1;
int next_free_dir = 0;
FILE* logFile;
/*------------------------------Helper Functions------------------------------*/
//gettime helper function
struct timespec getTime(){
  struct timespec tim;
	clock_gettime(CLOCK_REALTIME, &tim);
	return tim;
}

//This function is used to write into logFile.
void logging(char* structure, char* operation){
  if (logFile == NULL){
    	printf("ERROR: cannot access log\n");
    	exit(1);
	}
	struct timespec currentTime = getTime();
	time_t raw;
	time (&raw);
	struct tm *ltime = localtime (&raw);
	long microSec = currentTime.tv_nsec / 1000;
	char buf[200];
	strftime (buf,200,"%H:%M:%S", ltime);
	fprintf(logFile, "%s:%06lu\t %s\t %s \n",buf, microSec, structure, operation);
}
//close log file
void close_logging(){
  sleep(2);
  fclose(logFile);
}

/*---------------------------Lower Level Functions----------------------------*/

//Search Directory_Structures for the given filename, return inode number if
//found, else return -1
int Search_Directory(char* filename){
  logging("Directory", "Search");
  Count++;
  int i;
  for (i = 0; i < MAXFILES; i++){
    if(strcmp(Directory_Structure[i].Filename, filename) == 0)
      return Directory_Structure[i].Inode_Number;
  }
  return -1;
}

//Add entry to Directory_Structures with filename and inode_number
//Success return 1, otherwise return -1;
int Add_to_Directory(char* filename, int inode_number){
  logging("Directory", "Add to");
  Count++;
  if(next_free_dir >= MAXFILES){
    return -1;
  }
  else{
    strcpy(Directory_Structure[next_free_dir].Filename, filename);
    Directory_Structure[next_free_dir].Inode_Number = inode_number;
    next_free_dir++;
    return 1;
  }
}

//If given inode_number exists, return the inode structure
//return -1 (Inode_Number) if not exists;
Inode Inode_Read(int inode_number){
  logging("Inode\t", "Read");
  Count++;
  if(inode_number > -1 && inode_number < MAXFILES) {
    return Inode_List[inode_number];
  }
  else{
    Inode badInode;
    badInode.Inode_Number = 1;
    return badInode;
  }
}

//Copy the content of Inode structure input_inode to the Inode present at the inode_number
//If failed, return -1;
int Inode_Write(int inode_number, Inode input_inode){
  logging("Inode\t", "Write");
  Count++;
  if(inode_number < 0 || inode_number >= MAXFILES)
    return -1;
  else{
    Inode_List[inode_number] = input_inode;
    return 0;
  }
}

//Read the given num_bytes from block_number and write it to the provided
//String to_read, return the number of bytes read;
int Block_Read(int block_number, int num_bytes, char* to_read){
  logging("Block", "Read");
  Count++;
  if(block_number < 0 || block_number > MAXBLOCKS)
    return -1;
  else{
    if(Disk_Blocks[block_number] != NULL){
      int i = 0;
      for(i = 0; i < num_bytes; i++){
        to_read[i] = Disk_Blocks[block_number][i];
      }
      return num_bytes;
    }
    return 0;
  }
}

//Given the ​ block_number​ , write the contents of the string ​ to_write to the block
//and return the number of bytes written.
int Block_Write(int block_number, int num_bytes, char* to_write){
  logging("Block\t", "Write");
  Count++;
  if(block_number < 0 || block_number > MAXBLOCKS)
    return -1;
  else{
    if(Disk_Blocks[block_number] == NULL)
      Disk_Blocks[block_number] = (char*) malloc (sizeof(char)*BLOCKSIZE);

    int i;
    for(i = 0; i < num_bytes; i++){
      Disk_Blocks[block_number][i] = to_write[i];
    }
    return num_bytes;
  }
}

//Return the superblock structure
Super_block Superblock_Read(){
  logging("Superblock", "Read");
  Count++;
  return Superblock;
}

//Copy the contents of ​ input_superblock​ to the superblock structure
int Superblock_Write(Super_block input_superblock){
  logging("Superblock", "Write");
  Count++;
  if(input_superblock.next_free_inode <= -1 || input_superblock.next_free_inode >=
    MAXFILES || input_superblock.next_free_block <= -1 || input_superblock.next_free_block >=
    MAXBLOCKS){
    return -1;
  }
  else{
    Superblock = input_superblock;
    return 0;
  }
}

/*---------------------------Higher Level Functions----------------------------*/

//Set the log file name as the filename provided as input. Set count to 0.
//Initialize (or allocate) anything else needed for your file system to work, for
//example, initial values for superblock. Return Success or Failure appropriately.
int Initialize_Filesystem(char* log_filename){
  Log_Filename = log_filename;
  logFile = fopen(Log_Filename, "a");
  Count = 0;
  next_free_dir = 0;
  Superblock.next_free_inode = 0;
  Superblock.next_free_block = 0;

  int i;
  //Initialize Directory_Structures
  for(i = 0; i < MAXFILES; i++){
    memset(Directory_Structure[i].Filename, 0, 21);
    Directory_Structure[i].Inode_Number = -1;
  }

  //Initialize Inode_List
  for(i = 0; i < MAXFILES; i++){
    Inode_List[i].Inode_Number = -1;
  }

  //Initialize Disk_Blocks
  for(i = 0; i < MAXBLOCKS; i++){
    Disk_Blocks[i] = NULL;
  }
}

//Check whether the file you are going to create already exists in the directory
//structure. If yes, return with an appropriate error. If not, get the next free inode
//from the super block and create an entry for the file in the Inode_List. Then, using the
//returned inode number and filename, add the entry to the directory structure. Also,
//update the superblock since the next free inode index needs to be incremented. Then
//return appropriately.
int Create_File(char* filename){
	//check to see if file already exits in directory
	if(Search_Directory(filename) != -1){
    printf("File already exists in the directory structure.\n");
		return -1;
	}
  //If not, do the work
  Super_block sb = Superblock_Read();
	int current = sb.next_free_inode;
	Inode newInode;
	newInode.Inode_Number = current;
	newInode.User_Id = getuid();
  newInode.Group_Id = getgid();
  newInode.File_Size = 0;
  newInode.Flag=0;
	newInode.Start_Block = -1;
  newInode.End_Block = -1;
	if(Inode_Write(current, newInode) == -1){
	   return -1;
	}
	//add to directory structure
	Add_to_Directory(filename, current);
	//update superblock
	sb.next_free_inode = current + 1;
	Superblock_Write(sb);
	return current;
}

//Search the directory for the provided file name. If found, set the inode flag
//for it to 1 and return the inode number. Otherwise, return appropriately.
int Open_File(char* filename){
  if(Search_Directory(filename)==-1)
    return -1;
  else{
    Inode_List[Search_Directory(filename)].Flag = 1;
    return Search_Directory(filename);
  }
}

//Check wheather the provided offset and num_bytes to be read is correct(by
//comparing with the size of file). If yes, read bytes and store in String, and
//return the num_bytes read. OW, return error appropriately. (May need call Block_Read)
int Read_File(int inode_number, int offset, int count, char* to_read){
	Inode temp = Inode_Read(inode_number);
  int counter = 0;
  int counter2 =0;
	int i = temp.Start_Block;
	int j = temp.End_Block;
	int filesize = temp.File_Size;
	if(count != offset){
		return -1;
	}
	for(i; i <= j-1; i++){
		char* Contents = (char*) malloc(sizeof(char)*BLOCKSIZE);
		filesize = filesize - BLOCKSIZE;
		counter2 = counter2+ Block_Read(i, BLOCKSIZE, Contents);
		int k = 0;
		for(counter; counter<counter2; counter++){
			to_read[counter] = Contents[k];
			k++;
		}
	}
	char *Contents = (char*) malloc(sizeof(char)*filesize);
	counter2 = counter2 + Block_Read(i, filesize, Contents);
	int k = 0;
		for(counter; counter<counter2; counter++){
			to_read[counter] = Contents[k];
			k++;
		}
	return counter2;
}

//Check if the offset is correct(by comparing size...). If right, write the String
//to fileblocks, update the inode and superblock and return the num_bytes written.
//(Call Block_Write)
//Loop: 1. Fetch the Superblock to get next_free_block 2. Call Block_Write
//3. Update Inode 4. Update the Superblock
int Write_File(int inode_number, int offset, int number_of_bytes, char* to_write_in){
	//offset should equal size of file
	Inode temp = Inode_Read(inode_number);
	if(offset != temp.File_Size){
		return -1;
	}
	//get next free block
	Super_block super = Superblock_Read();
	int tempBlock =  super.next_free_block;
	int blocks = 0;
	//write block if it's the first one
	if(temp.Start_Block == -1){
		blocks = Block_Write(tempBlock, number_of_bytes, to_write_in);
		temp.Start_Block = tempBlock;
		temp.End_Block = tempBlock;
		temp.File_Size = temp.File_Size + number_of_bytes;
	}
	//write block if it is not the first one
	else{
		blocks = Block_Write(tempBlock, number_of_bytes, to_write_in);
		temp.End_Block = tempBlock;
		temp.File_Size = temp.File_Size + number_of_bytes;
	}
	//update superblock
	super.next_free_block = tempBlock + 1;
	Superblock_Write(super);
	//update inode
	Inode_Write(inode_number, temp);
	return blocks;
}

//For the given inode number, set the inode flag to 0 if it is 1
//and return appropriately. (I return inode_number here)
int Close_File(int inode_number){
  Inode_List[inode_number].Flag = 0;
  if(Inode_Write(inode_number, Inode_Read(inode_number)) == -1)
    return -1;
  return inode_number;
}
