#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



#include "disk.h"
#include "fs.h"


//https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
//helper function to compute the power of base to the exp
int pwer(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}


//function that converts binary string (ect. '010100') to decimal int
int BinaryToDecimal(char* newStr, int length) {   
	int ret = 0;
	for (int i = length-1; i>=0; i--){
		if (newStr[i]=='1'){
			double result = pwer(2, length-1-i);
			ret += result;
		}
	}return ret;}


//https://www.scaler.com/topics/decimal-to-binary-in-c/
//function that converts decimal to binary string of 0s and 1s
char* decimalTo8BitBinary(char * newStr, int num) {   
    if (num == 0) {
		for (int i=0; i<8;i++){

        newStr[i]= '0';
		}
		return NULL;
    }
   
   // Stores binary representation of number.
   int binaryNum[8] = {0,0,0,0,0,0,0,0}; // Assuming 8 bit integer.
   int i=0;
   
   for ( ;num > 0; ){
      binaryNum[i++] = num % 2;
      num /= 2;
   }


   int p = 0;
   for (int j = 8-1; j >= 0; j--){
		if (binaryNum[j]==0){
			newStr[p] = '0';
		} else {
			newStr[p] = '1';
		}
		p++;	
   }}

char* decimalTo32BitBinary(char * newStr, int num) {   
    if (num == 0) {
        return NULL;
    }
   
   // Stores binary representation of number.
   int binaryNum[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // Assuming 32 bit integer.
   int i=0;
   
   for ( ;num > 0; ){
      binaryNum[i++] = num % 2;
      num /= 2;
   }


   int p = 0;
   for (int j = 32-1; j >= 0; j--){
		if (binaryNum[j]==0){
			newStr[p] = '0';
		} else {
			newStr[p] = '1';
		}
		p++;	
   }}   

char* decimalTo16BitBinary(char * newStr, int num) {   
    if (num == 0) {
        return NULL;
    }
   
   // Stores binary representation of number.
   int binaryNum[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // Assuming 16 bit integer.
   int i=0;
   
   for ( ;num > 0; ){
      binaryNum[i++] = num % 2;
      num /= 2;
   }


   int p = 0;
   for (int j = 16-1; j >= 0; j--){
		if (binaryNum[j]==0){
			newStr[p] = '0';
		} else {
			newStr[p] = '1';
		}
		p++;	
   }}   


//SuperBlock struct to store data about file
struct superblock{
	int total_blk_count;//total amount of blocks
	int fat_blk_count;//number of blocks the FAT part takes up
	int rdir_blk;//index of root block
	int data_blk;//index of first data blokc
	int data_blk_count;//how many data blocks


	int num_fat_spaces;//how many FAT spaces there are
	int num_fat_spaces_taken;//how many of the FAT spaces are taken


	int rdir_occupied;//how many spots of the root block are occupied

};

//file descriptor struct
struct fileDescriptor
{
	int rootIndex;//where in the root index is this file descriptor
	int *dataIndices;//list of integers corresponding to blocks that store the file
	//for example, if the file had data in blocks #4, 6, and 10, this member would 
	//be {4, 6, 10}

	int numBlocks;//how many total blocks
	int lseek;//current seek position
	int16_t size;//size of file
	char *filename;//name of file
};




struct fileDescriptor **fileDescriptors;//array of 32 max file descriptors



static struct superblock superBlock;//declares superblock
static int fileIndex = 0;//index of files in root block(goes up by 32 every time)
static int nextAvailableBlock = 0;//next available block to write to

static int mounted = 0;//tells the program if a disk has been mounted or not



char** fileNameList;//list of char* filename's to keep track of which files have already been opened
int fs_mount(const char *diskname)
{
	//printf("FS_MOUNT: %d\n", mounted);
	if (mounted==1){
		return 0;
	}
	//open the block fro disk
	//printf("mountin\n");
	block_disk_open(diskname);
	//printf("done mounting\n");

	//read block into buffer "buf"
	int8_t *buf = malloc(BLOCK_SIZE*sizeof(int8_t));
	block_read(0, buf);


	/* --------------------------------------------------------------------------------------------- */
	/*                                 BYTES 0-7 SHOULD BE ECS150FS                                */
	/* --------------------------------------------------------------------------------------------- */
	char* string= "ECS150FS";
	for (int i=0;i<8;i++){
		//printf("%c\n", buf[i]);
		if (buf[i]!=string[i]){
			//printf("signature not epresent\n");
			return -1;
		}
	}



	/* --------------------------------------------------------------------------------------------- */
	/*                                BYTES 8-9 ARE TOTAL BLOCK COUNT                                */
	/* --------------------------------------------------------------------------------------------- */

	//store 8-bit string of byte 9 as binary string
	char* firstStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(firstStr, buf[9]);//converts to binary array

	//store 8-bit string ofbyte 8 as binary string
	char* secondStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(secondStr, buf[8]);

	//since the 2 bytes together make up the total block count, we need to 
	//combine block 8 and 9 into a new binary string, then get the decimal value
	//of the new binary string

	//get 16-bit string = byte 8 + byte 9
	char* newStr = malloc(16*sizeof(char));
	for (int i = 0; i<8; i++){
		newStr[i] = firstStr[i];
	}

	for (int o = 0; o<8; o++){
		newStr[o+8] = secondStr[o];
	}
	//store total block count	
	
	superBlock.total_blk_count = BinaryToDecimal(newStr, 16);


	/* --------------------------------------------------------------------------------------------- */
	/*                           BYTES 10-11 ARE ROOT DIRECTORY BLOCK INDEX                          */
	/* --------------------------------------------------------------------------------------------- */
	//same as above, we need to store byte 10 and 11 as binary string, 
	//combine into one binary string, then get decimal value
	//get 8-bit string of byte 11
	firstStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(firstStr, buf[11]);

	//get8-bit string of byte 10
	secondStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(secondStr, buf[10]);

	//combine into one 16-bit binary string
	newStr = malloc(16*sizeof(char));
	for (int i = 0; i<8; i++){
		newStr[i] = firstStr[i];
	}
	for (int o = 0; o<8; o++){
		newStr[o+8] = secondStr[o];
	}

	//store root block index
	superBlock.rdir_blk = BinaryToDecimal(newStr, 16);	


	/* --------------------------------------------------------------------------------------------- */
	/*                             BYTES 12-13 ARE DATA BLOCK START INDEX                            */
	/* --------------------------------------------------------------------------------------------- */
	
	//get 8-bit string of byt 13
	firstStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(firstStr, buf[13]);

	//get 8-bit string of byt 11
	secondStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(secondStr, buf[12]);

	//combine into one 16-bit string
	newStr = malloc(16*sizeof(char));
	for (int i = 0; i<8; i++){
		newStr[i] = firstStr[i];
	}
	for (int o = 0; o<8; o++){
		newStr[o+8] = secondStr[o];
	}

	//store data block
	superBlock.data_blk = BinaryToDecimal(newStr,16);
	nextAvailableBlock = superBlock.data_blk;//next available block to write to is initially the index of the first data block


	/* --------------------------------------------------------------------------------------------- */
	/*                           BYTES 14-15 ARE THE AMOUNT OF DATA BLOCKS                           */
	/* --------------------------------------------------------------------------------------------- */
	//get 8-bit string of byte 15
	firstStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(firstStr, buf[15]);

	//get 8-bit string of byte 14
	secondStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(secondStr, buf[14]);

	//combine into one 16-bit string
	newStr = malloc(16*sizeof(char));
	for (int i = 0; i<8; i++){
		newStr[i] = firstStr[i];
	}
	for (int o = 0; o<8; o++){
		newStr[o+8] = secondStr[o];
	}

	//store how many data blocks there are	
	superBlock.data_blk_count = BinaryToDecimal(newStr, 16);	

	/* --------------------------------------------------------------------------------------------- */
	/*                              BYTE 16 IS THE NUMBER OF FAT BLOCKS                              */
	/* --------------------------------------------------------------------------------------------- */
	//get 8-bit string
	firstStr = malloc(8 * sizeof(char));
	decimalTo8BitBinary(firstStr, buf[16]);
	
	//store number of FAT blocks
	superBlock.fat_blk_count = BinaryToDecimal(firstStr, 8);			





	//num_fat_spaces_taken is used for fs_info()
	superBlock.num_fat_spaces_taken = 0;

	//read the first FAT Block into memory
	//each FAT element is 16 bits wide, so read it into int16_t type
	int16_t *buf2 = malloc(BLOCK_SIZE/2*sizeof(int16_t));
	block_read(1, buf2);
	//iterate through elements in first FAT block, if any are occupied, increase num_fat_spaces_taken 
	for (int i = 0;i<BLOCK_SIZE/2;i++){
		if (buf2[i]!=0){
			superBlock.num_fat_spaces_taken++;
		}
	}
	//number of FAT spaces is the number of fat blocks times (4096\2) spaces per fat block
	superBlock.num_fat_spaces = superBlock.data_blk_count;





	//get number of roof blocks occupied for fs_info() function
	//root block is 128 elements, each of 32-bit.
	//its easier to just read each byte in (int8_t), and then remember there grouped in groups of 32
	int8_t *buf3 = malloc(BLOCK_SIZE*sizeof(int8_t));
	block_read(superBlock.rdir_blk, buf3);

	superBlock.rdir_occupied = 0;
	//iterate through root block, 32 bytes at a time(one element at a time)
	for (int i = 0;i<BLOCK_SIZE;i+=32){
		if (buf3[i]!=0){
			//if element is occupied, increase rdir_occupied
			superBlock.rdir_occupied++;
		}
	}

	mounted = 1;//tell the rest of the program that a disk has been mounted
	fileDescriptors = malloc(32*sizeof(struct fileDescriptor*));

	fileNameList =  malloc(0*sizeof(char*));

	//printf("DONE FS_MOUNT\n");
	return 0;

}

int fs_umount(void)
{
	block_disk_close();
	return 0;
	//write meta data to disk
	int8_t *rootBlk = malloc(BLOCK_SIZE*sizeof(int8_t));
	block_read(superBlock.rdir_blk, rootBlk);
	block_write(0, rootBlk);


	//ensure there are no open file descriptors
	for (int i = 0; i < 32; i++){
		if (fileDescriptors[i]){
			return -1;
		}
	}

	//close disk
	if (block_disk_close()){
		return -1;
	};
}

int fs_info(void)
{

	
	if (mounted == 0){
		return -1;
	}
	printf("FS Info:\n");
	printf("total_blk_count=%d\n", superBlock.total_blk_count);
	printf("fat_blk_count=%d\n", superBlock.fat_blk_count);
	printf("rdir_blk=%d\n", superBlock.rdir_blk);
	printf("data_blk=%d\n", superBlock.data_blk);
	printf("data_blk_count=%d\n", superBlock.data_blk_count);

	//subtract total number of fat spaces by number of fat spaces taken
	printf("fat_free_ratio=%d/%d\n", superBlock.num_fat_spaces - superBlock.num_fat_spaces_taken, superBlock.num_fat_spaces);

	//theres always 128 elements in the roof block
	printf("rdir_free_ratio=%d/%d\n", 128 - superBlock.rdir_occupied, 128);

}

int fs_create(const char *filename)
{
	if (mounted == 0 || strlen(filename) > FS_FILENAME_LEN || fileIndex >= 4096){
		return -1;
	}

	//printf("FS_CREATE\n");
		//check that file exists
	int contains = 0;
	for (int i = 0; i < fileIndex/32; i++){
		if (!strcmp(filename, fileNameList[i])){
			return -1;}
	}



	//read in the entire root block
	int8_t *rootBlk = malloc(BLOCK_SIZE*sizeof(int8_t));
	block_read(superBlock.rdir_blk, rootBlk);

	
	//file index is equal to the next available index on the root block
	//update the 32-bit root element

	//name of file(bytes 0-15)
	for (int j = 0; j < (int)strlen(filename); j++){
		rootBlk[fileIndex+j] = filename[j];
	}

	//size of file (bytes 15, 16, 17, 18)
	rootBlk[fileIndex+15] = 0;
	rootBlk[fileIndex+16] = 0;
	rootBlk[fileIndex+17] = 0;
	rootBlk[fileIndex+18] = 0;

	//inde of first data block(bytes 20, 21)
	rootBlk[fileIndex+20] = -1;
	rootBlk[fileIndex+21] = -1;

	//increment file index
	fileIndex+= 32;


	//write root block back 
	block_write(superBlock.rdir_blk, rootBlk);

	//printf("A\n");
	fileNameList =  realloc(fileNameList, (fileIndex/32)*sizeof(char*));
		//printf("A\n");

	char* newFile = malloc((strlen(filename)+1)*sizeof(char));
		//printf("B\n");

	strcpy(newFile, filename);
		//printf("%s\n", newFile);

	fileNameList[fileIndex/32-1] = newFile;
		//printf("%d: %s\n", fileIndex/32-1,  fileNameList[fileIndex/32+1]);



	return 0;
}

int fs_delete(const char *filename)
{
	if (mounted == 0){
		return -1;
	}


	//check that file exists, if it does, delete file
	int contains = 0;
	for (int i = 0; i < fileIndex/32; i++){
		if (!strcmp(filename, fileNameList[i])){
			contains = 1;
			fileNameList[i] = "";
			break;}
	}
	if (contains == 0){
		return -1;
	}

	//first, read the root block from memory
	int8_t *rootBlk = malloc(BLOCK_SIZE*sizeof(int8_t));
	block_read(superBlock.rdir_blk, rootBlk);

	//iterate through root block entries
	for (int i = 0; i < BLOCK_SIZE; i+=32){
		//find match for filename
		for (int j = 0; j < (int)strlen(filename); j++){
				if (rootBlk[i+j] != filename[j]){
					//if any character doesn't match, the filename doesn't match current entry. Break and move onto next
					
					break;
					
				}

				if (j == (int)strlen(filename)-1){
					//if we reach this point, we have a match, we want to set the whole 32-bit block to 0
					for (int k = 0; k < 32; k++){
						rootBlk[i+k]= 0;
					}
					
					break;
				}
		}
	}
	//write the new root block to disk
	block_write(superBlock.rdir_blk, rootBlk);


	

	//if no match was found, we have an error
	return 0;

}

int fs_ls(void)
{
	if (mounted == 0){
		return -1;
	}


	printf("FS Ls:\n");
	//read root block
	int8_t *rootBlk = malloc(BLOCK_SIZE*sizeof(int8_t));
	block_read(superBlock.rdir_blk, rootBlk);	

	//iterate through loop block and print out files
	for (int i=0; i<128; i+= 32){
		// for (int n = 0; n < 32; n++){
		// 	printf("%d %d\n", i+n, rootBlk[i+n]);
		// }	
		if (rootBlk[i] != 0){
			//file name
			printf("file: ");
			int cur = 0;
			while (rootBlk[i+cur] != 0){
				printf("%c", rootBlk[i+cur]);
				cur++;
			}

			//rest of string
			if ((rootBlk[i+20] == -1 && rootBlk[i+21] == -1)|| (rootBlk[i+20] == 0 && rootBlk[i+21] == 0)){
				printf(", size: %d, data_blk: %d\n",0, 65535);
			} else {
				/* ------------------------------------------------------------------------------------------ */
				/*                             GET SIZE FROM BYTES  15, 16, 17, 18                            */
				/* ------------------------------------------------------------------------------------------ */
				//Byte 15
				char * byte15String = malloc(8 * sizeof(char));
				decimalTo8BitBinary(byte15String, rootBlk[i+15]);
				//printf("\n%s\n",byte15String);						

				//Byte 16
				char * byte16String = malloc(8 * sizeof(char));
				decimalTo8BitBinary(byte16String, rootBlk[i+16]);
				//printf("%s\n",byte16String);						

				//Byte 17
				char * byte17String = malloc(8 * sizeof(char));
				decimalTo8BitBinary(byte17String, rootBlk[i+17]);
				//printf("%s\n",byte17String);						

				//Byte 18
				char * byte18String = malloc(8 * sizeof(char));
				decimalTo8BitBinary(byte18String, rootBlk[i+18]);		
				//printf("%s\n",byte18String);						

				//combine into one 4byte string
				char* sizeBitString = malloc(8*4*sizeof(char)+sizeof(char));
				//printf("A\n");
				strcat(sizeBitString, byte18String);
				strcat(sizeBitString, byte17String);
				strcat(sizeBitString, byte16String);
				strcat(sizeBitString, byte15String);
				//sizeBitString[32]='/0';

				// for (int i = 0; i<8; i++){
				// 	sizeBitString[i] = byte18String[i];
				// }
				// for (int i = 0; i<8; i++){
				// 	sizeBitString[i+8] = byte17String[i];
				// }
				// for (int i = 0; i<8; i++){
				// 	sizeBitString[i+16] = byte16String[i];
				// }
				// for (int i = 0; i<8; i++){
				// 	sizeBitString[i+24] = byte15String[i];
				// }				
				//printf("sizeBitString: %s\n", sizeBitString);

				//store how decimal value of size
				int size = BinaryToDecimal(sizeBitString, 8*4);	

			/* ------------------------------------------------------------------------------------------- */
			/*                              GET DATA BLOCK FROM BITS 20 AND 21                             */
			/* ------------------------------------------------------------------------------------------- */
				//Byte 20
				char * byte20String = malloc(8 * sizeof(char));
				decimalTo8BitBinary(byte20String, rootBlk[i+20]);
				//printf("byte 20: %d\n", rootBlk[i+20]);
				//printf("%s\n", byte20String);

				//Byte 21
				char * byte21String = malloc(8 * sizeof(char));
				decimalTo8BitBinary(byte21String, rootBlk[i+21]);
				//printf("%d\n", rootBlk[i+21]);						
				//printf("%s\n", byte21String);

				//combine into one 4byte string
				char* firstDataBlockBitString = malloc(8*2*sizeof(char)+1);
				//printf("%dB\n", size);
				strcat(firstDataBlockBitString, byte21String);
				strcat(firstDataBlockBitString, byte20String);
				// for (int i = 0; i<8; i++){
				// 	firstDataBlockBitString[i] = byte21String[i];
				// }
				// for (int i = 0; i<8; i++){
				// 	firstDataBlockBitString[i+8] = byte20String[i];
				// }			
				//printf("\nfirstDataBlockBitString: %s\n", firstDataBlockBitString);
		
				//store how decimal value of size
				int firstDataBlockIndex = BinaryToDecimal(firstDataBlockBitString, 16);	
				//printf("\nfirstDataBlockIndex: %d\n", firstDataBlockIndex);

				printf(", size: %d, data_blk: %d\n",size, firstDataBlockIndex);
			}
		}
	}
}

int fs_open(const char *filename)
{
	//printf("FS_OPEN\n");
	if (mounted==0){
		return -1;
	}


	//read root block
	int8_t *rootBlk = malloc(BLOCK_SIZE*sizeof(int8_t));
	block_read(superBlock.rdir_blk, rootBlk);	

	//iterate through root block to find file
	for (int i = 0; i < BLOCK_SIZE; i+=32){
		for (int j = 0; j < (int)strlen(filename); j++){
				if (rootBlk[i+j] != filename[j]){
					break;
				}

				//if we have found a match
				if (j == (int)strlen(filename)-1){
					//create new file descriptor
					struct fileDescriptor *newDescriptor = malloc(sizeof(struct fileDescriptor));
					newDescriptor->lseek = 0;
					newDescriptor->rootIndex=i;
					newDescriptor->size=0;
					newDescriptor->numBlocks = 0;
					newDescriptor->dataIndices = malloc(0*sizeof(int));
					newDescriptor->filename = filename;
					fileDescriptors[i/32] = newDescriptor;
					return i/32;
				}
		}

	}

	//if we've reached here, the file is not found
	return -1;
}

int fs_close(int fd)
{
	//error check
	if (mounted==0 || fd >= 32 || fd < 0 || fileDescriptors[fd]==NULL){
		return -1;
	}

	//nullify FD
	fileDescriptors[fd] = NULL;
	return 0;
	
}

int fs_stat(int fd)
{
	if (mounted==0 || fd >= 32 || fd < 0 || fileDescriptors[fd]==NULL){
		return -1;
	}


	return fileDescriptors[fd]->size;
}

int fs_lseek(int fd, size_t offset)
{
	//error check
	if (mounted==0 || fd >= 32 || fd < 0 || fileDescriptors[fd]==NULL){
		return -1;
	}

	fileDescriptors[fd]->lseek = offset;
	return 0;
}

int fs_write(int fd, void *buf, size_t count)
{

	// char *copyOfBuf = malloc(4096 * sizeof(int8_t));
	// memcpy(copyOfBuf, buf, 4096 * sizeof(int8_t));


	// char* newBuf = malloc(4096 * sizeof(int8_t));
	// for (int i = 0; i< 4096; i++){
	// 	newBuf[i] = copyOfBuf[i];
	// }

	// block_write(nextAvailableBlock, newBuf);
	// return count+1;



	if (fileDescriptors[fd]==NULL || buf == NULL || fd < 0 || fd >= 32 || mounted == 0){
		return -1;
	}

	int ret = 0;

	//get current file descriptor
	struct fileDescriptor *curDescriptor = malloc(sizeof(struct fileDescriptor));
	curDescriptor = fileDescriptors[fd];

	//total number of blocks to write
	int numBlocksToWrite = ((int)count / 4096);
	//printf("%d\n", numBlocksToWrite);

	//we create a copy of buf called bufCopy because we can't dereference a void pointer, so we caste it as int8_t
	int8_t *bufCopy = malloc(numBlocksToWrite * BLOCK_SIZE * sizeof(int8_t));
	memcpy(bufCopy, buf, numBlocksToWrite * BLOCK_SIZE * sizeof(int8_t));
	//write each block. Note that we write a buffer called newBuf which is of size 4096. The actual buffer(bufCopy) could be larger, so we only write in chunks of 4096 at a time
	for (int i = 0; i < numBlocksToWrite; i++){
		//copt the current chunk of 4096 bits from bufCopy to newBuf
		int8_t *newBuf = malloc(BLOCK_SIZE * 8);
		for (int j = 0; j < BLOCK_SIZE; j++){
			newBuf[j] = bufCopy[i*BLOCK_SIZE+j];
		}
		block_write(nextAvailableBlock, newBuf);

		curDescriptor->numBlocks++;//increase number of blocks

		//update data indices
		curDescriptor->dataIndices = realloc(curDescriptor->dataIndices, curDescriptor->numBlocks * sizeof(int));
		curDescriptor->dataIndices[curDescriptor->numBlocks-1] = nextAvailableBlock;
		//printf("%d\n", nextAvailableBlock);
		//each time you write, increase the block that you write to
		nextAvailableBlock++;


		//each time we write a block, we need to update the files size
		int8_t *rootBlk = malloc(BLOCK_SIZE*sizeof(int8_t));
		block_read(superBlock.rdir_blk, rootBlk);	
		for (int block = 0; block < BLOCK_SIZE; block+=32){
			for (int j = 0; j < (int)strlen(curDescriptor->filename); j++){
					if (rootBlk[block+j] != curDescriptor->filename[j]){
						break;
					}
					//if we have found a match
					if (j == (int)strlen(curDescriptor->filename)-1){
						/* ---------------------------------------------------------------------------------------- */
						/*                                    UPDATE SIZE OF FILE                                   */
						/* ---------------------------------------------------------------------------------------- */
						int sizeOfFile = (curDescriptor->numBlocks)*4096;
						//printf("sizeOfFile: %d\n", sizeOfFile);
						char* Size_32_bit_String = malloc(32*sizeof(char));
						decimalTo32BitBinary(Size_32_bit_String, sizeOfFile);

						//store first 8 bits in string
						char* first8bits = malloc(8*sizeof(char));
						for (int k = 0;k<8;k++ ){
							first8bits[k] = Size_32_bit_String[k];
						}

						//printf("\n32 bit file size: %s", first8bits);
						//store second 8 bits in string
						char* second8bits = malloc(8*sizeof(char));
						for (int k = 0;k<8;k++ ){
							second8bits[k] = Size_32_bit_String[k+8];
						}			

						//printf("%s", second8bits);

						//store third 8 bits in string
						char* third8bits = malloc(8*sizeof(char));
						for (int k = 0;k<8;k++ ){
							third8bits[k] = Size_32_bit_String[k+16];
						}		
						//printf("%s", third8bits);
						//store fourth 8 bits in string
						char* fourth8bits = malloc(8*sizeof(char));
						for (int k = 0;k<8;k++ ){
							fourth8bits[k] = Size_32_bit_String[k+24];
						}																
						//printf("%s", fourth8bits);
						rootBlk[block+15] = BinaryToDecimal(fourth8bits, 8);
						rootBlk[block+16] =  BinaryToDecimal(third8bits, 8);
						rootBlk[block+17] =  BinaryToDecimal(second8bits, 8);
						rootBlk[block+18] =  BinaryToDecimal(first8bits, 8);

						//printf("\n%d,%d,%d,%d\n", BinaryToDecimal(fourth8bits, 8), BinaryToDecimal(third8bits, 8), BinaryToDecimal(second8bits, 8), BinaryToDecimal(first8bits, 8));

						/* ---------------------------------------------------------------------------------------- */
						/*                               UPDATE FIRST DATA BLOCK INDEX                              */
						/* ---------------------------------------------------------------------------------------- */

						int firstDataBlockIndex = curDescriptor->dataIndices[0];
						//printf("First Data Block: %d\n", firstDataBlockIndex);
						char* Size_16_bit_String = malloc(32*sizeof(char));
						decimalTo16BitBinary(Size_16_bit_String, firstDataBlockIndex);

						//store first 8 bits in string
						first8bits = malloc(8*sizeof(char));
						for (int k = 0;k<8;k++ ){
							first8bits[k] = Size_16_bit_String[k];
						}

						//store second 8 bits in string
						second8bits = malloc(8*sizeof(char));
						for (int k = 0;k<8;k++ ){
							second8bits[k] = Size_16_bit_String[k+8];
						}			
											

						rootBlk[block+20] = BinaryToDecimal(second8bits, 8);
						rootBlk[block+21] =  BinaryToDecimal(first8bits, 8);
	

						break;
					}

			}
		}	
		block_write(superBlock.rdir_blk, rootBlk);		


	ret += 4096;
	}
	


	return ret;
}


int fs_read(int fd, void *buf, size_t count)
{
	//get current fileDescriptor
	struct fileDescriptor *curDescriptor = fileDescriptors[fd];
	//printf("FS_READ: %d\n", curDescriptor->dataIndices[0]);
	// void *tempBuf = malloc(BLOCK_SIZE * sizeof(int8_t));
	// block_read(4, tempBuf);
	// strcat(buf, tempBuf);
	//printf("read; %s\n", buf);
	for (int i = 0; i < curDescriptor->numBlocks; i++){
		// read into buffer tempBuf
		void *tempBuf = malloc(BLOCK_SIZE * sizeof(int8_t));
		if (curDescriptor->dataIndices[0]==0){
			block_read(4, tempBuf);
			
		} else {
			block_read(curDescriptor->dataIndices[i], tempBuf);
		}

		// add tempBuf to but
		strcat(buf, tempBuf);
	}

	return count;
}

