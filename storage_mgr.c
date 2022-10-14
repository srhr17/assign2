#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void assignFileHandle(SM_FileHandle* fHandle, char* fileName, FILE* file)
{
	fHandle->fileName = fileName;
	fHandle->curPagePos = ftell(file) / PAGE_SIZE;
	fseek(file, 0, SEEK_END);
	fHandle->totalNumPages = ftell(file) / PAGE_SIZE;
	fHandle->mgmtInfo = file;
	fseek(file, fHandle->curPagePos * PAGE_SIZE, SEEK_SET);
}

SM_PageHandle getEmptyPageHandle()
{
	SM_PageHandle page = (SM_PageHandle) malloc(PAGE_SIZE);
	int itr = 0;
	for (itr = 0; itr < PAGE_SIZE; ++itr) {
		page[itr] = '\0';
	}
	return page;
}

/*
// 
// Jason Scott A20436737
//initStorageManager
// 1. Initializes the stroage manager
*/ 
void initStorageManager(void) {
	printf("\n ~~~~~Initializing Storage Manager~~~~\n ");
	printf("CS525\n");

}

/*
// 
// Jason Scott A20436737
//createPageFile
// 1. Opens file 
// 2. Append an empty block
// 3. Write the empty block
*/ 

//Creating a page in the file
RC createPageFile (char *fileName)
{
	FILE *fp;
	int i;
	SM_PageHandle newBlock[PAGE_SIZE];

	fp = fopen(fileName, "w");
	//fopen function, returns a FILE pointer and opens file
	// The apendBlock function, but we don't have fHandle to pass through
	for(i=0;i<PAGE_SIZE;i++) { //simple for loop to make block space for leng of pagesize
        newBlock[i] = '\0';
    }
	fwrite(newBlock, 1, PAGE_SIZE, fp); //writes where new block space is free
	
	fclose(fp); //closes file
	return RC_OK;
}

/*
// 
// Jason Scott A20436737
//openPageFile
// 1. Opens existing file, else returns not found error
// 2. Outputs properties of the file
// 3. Writes using fputs 
*/ 
//open page function
RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
	FILE *fp;
	if (fp = fopen(fileName, "r+")) {//case file exists


	// Seek to end so ftell reads correct number of char
	fseek(fp, 0, SEEK_END);
	

	//initializing the attributes
	(*fHandle).fileName = fileName; 
	(*fHandle).curPagePos = 0;
	(*fHandle).mgmtInfo = fp;
	(*fHandle).totalNumPages = ftell(fp)/PAGE_SIZE;
	// return back to beginning of file since ftell complete
	fseek(fp, 0, SEEK_SET);
	return RC_OK;
	} else { //case file doesnt exist
		return RC_FILE_NOT_FOUND;
	}

}

/*
// 
// Jason Scott A20436737
//closePageFile; destroyPageFile
// 1. Opens a file
// 2. If exists, it closes or destroys
// 3. If issues with 1 or 2, it displays errors
*/ 

//closing the file

RC closePageFile (SM_FileHandle *fHandle)
{
	
	//closes the file and returns 0
	if (fclose(fHandle->mgmtInfo) == 0)
		return RC_OK;
	else //case it doesnt exist
		return RC_FILE_NOT_FOUND;	
}

//destroying the page file
RC destroyPageFile (char *fileName)
{
	if(remove(fileName) == 0)
		return RC_OK;
	else 
		return RC_FILE_NOT_FOUND;
}

/*
// Writes from memory into disk to an absolute location using the given pageNum.
// Darek Nowak A20497998
// 1. Opens file 
// 2. Fseeks given the offset from memPage to find the absolute location of where we will write
// 3. Writes using fwrite 
*/
RC writeBlock (int pageNum , SM_FileHandle * fHandle , SM_PageHandle memPage ) {
// Using the memPage file Handle, we find the offset by multiplying page Num by PAGE_SIZE(4096 bytes). pageNum 0 gives 0 bytes which is the start of the first page and pageNum gives 4096 which is the start of page 2.



    // If the page you're looking for exists, seek to the page and write.
    if(pageNum < fHandle->totalNumPages) {
        fseek(fHandle->mgmtInfo, (pageNum * PAGE_SIZE), SEEK_SET);
        fwrite(memPage, 1, PAGE_SIZE, fHandle->mgmtInfo);
        // Updates current page number to recently written file.
        fHandle->curPagePos = pageNum;
    } 
	else {
		// append block
        appendEmptyBlock(fHandle);
    }
return RC_OK;
}

/*
// Darek Nowak A20497998
// Writes from memory into disk to where the page handle pointer is currently pointing at. Same as writeblock, just that there is no need to seek.
// 1. Opens file
// 2. Write using fwrite
*/
RC writeCurrentBlock ( SM_FileHandle * fHandle , SM_PageHandle memPage ) {

	
	// If the page you're looking for exists, seek to the page and write.
    if(fHandle->curPagePos < fHandle->totalNumPages) {
        fwrite(memPage, 1, PAGE_SIZE, fHandle->mgmtInfo);
    } else {
        return RC_READ_NON_EXISTING_PAGE;
    }

return RC_OK;
}


/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method checks if the given FileHandle is valid and not NULL
2. If Null, it will return false
3.If not Null, it will retrun true
*/
bool checkValidfHandle(SM_FileHandle *fHandle)
{
	return (fHandle == NULL) ? false : true;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method checks if the given Management Info is valid and not NULL
2. If NULL, it will return false
3.If Not Null, it will retrun true
*/
bool checkValidMgmtInfo(SM_FileHandle *fHandle)
{
	return (fHandle->mgmtInfo == NULL) ? false : true;
}

/*
// Darek Nowak A20497998
// Increment number of pages(blocks) by 1 and create a file filled with PAGE_SIZE amount of 0 chars.
// 1. Create new page via a new char* to newBlock
// 2. Iterate through the page until you fill it with 4096 0 chars.
// 3. Update the metadata by incrementing totalNumPages
// 4. Use the writeBlock function to write a new file with the given newBlock.
*/
RC appendEmptyBlock(SM_FileHandle* fHandle)
{
	if(!checkValidfHandle(fHandle))
		return RC_FILE_HANDLE_NOT_INIT;
	if(!checkValidMgmtInfo(fHandle))
		return RC_FILE_NOT_FOUND;

	// Action
	SM_PageHandle emptyPage = getEmptyPageHandle();
	FILE* fileInAppendMode = fopen(fHandle->fileName, "r+");
	fseek(fileInAppendMode, 0, SEEK_END);
	fwrite(emptyPage, 1, PAGE_SIZE, fileInAppendMode);
	fseek(fileInAppendMode, 0, SEEK_END);
	fclose(fileInAppendMode);
	assignFileHandle(fHandle, fHandle->fileName, fHandle->mgmtInfo);
	return RC_OK;
}

/*
// Darek Nowak A20497998
// If # of pages in memory is different than # of pages in disk, increase size of pages and update metadata.
// 1. Check to see if there is a difference in pages with memory and disk.
// 2. If there are more pages in memory, then write to the disk 1 empty file.
*/
RC ensureCapacity (int numberOfPages , SM_FileHandle * fHandle ) {
	// pages in memory differs from pages in disk
    if(numberOfPages > fHandle->totalNumPages) {
		// create pages until # of pages in memory and disk equal
        while(numberOfPages > fHandle->totalNumPages) {
            appendEmptyBlock (fHandle);
        }
        return RC_OK;
    
    } else {
		// Pages in memory = pages in disk
        return RC_OK;
    }
}


/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method checks if the given page number is valid
2. Checks if page number is less than zero and it is a valid page number
3. If yes, it will return false
4. If no, it will retrun true
*/
bool checkValidPageNumber(int pageNum, int totalNumberOfPagesInTheFile)
{
	return ((pageNum<0) & (pageNum> totalNumberOfPagesInTheFile)) ? false : true;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method checks if the result of the fseek is valid
2. If seek result is not zero, return false
3. If seek result is zero, return true
*/
bool checkValidSeek(int resultOfseek)
{
	return (resultOfseek > 0 || resultOfseek < 0) ? false : true;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method checks if the result of the fseek is valid
2. If page size and the fread result is equal, return true
3. If page size and the fread result is not equal, return false
*/
bool checkValidRead(int resultOfRead)
{
	return (PAGE_SIZE == resultOfRead) ? true : false;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
This method sets the pagenumber to the current page position in the Handler property
*/
void setCurrentPosition(int pageNumber, SM_FileHandle *fHandle)
{
	fHandle->curPagePos = pageNumber;
}


/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method reads the page block of the input page number 
2. Inputs- page number which needs to be retrieved, fileHandle and pageHandle
3. returns - Returns code as per the result
*/
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int resultOfSeek, resultOfRead, totalNumberOfPagesInTheFile;
	totalNumberOfPagesInTheFile = fHandle->totalNumPages;
	//Checks if the fhandle is valid and returns error code if its not initialized
	if (!checkValidfHandle(fHandle))
		return RC_FILE_HANDLE_NOT_INIT;
		
	//Checks if the input page number is valid and returns error code if its not exist
	if (!checkValidPageNumber(pageNum, totalNumberOfPagesInTheFile))
		return RC_READ_NON_EXISTING_PAGE;

	//Checks if the management info is valid and returns error code if its not exist
	if (!checkValidMgmtInfo(fHandle))
		return RC_FILE_NOT_FOUND;

	//File seek is performed on the file with page number and returns error if the seek result is not valid
	resultOfSeek = fseek(fHandle->mgmtInfo, PAGE_SIZE * pageNum, SEEK_SET);
	if (!checkValidSeek)
		return RC_ERROR;
	
	//File read is performed on the file with page number and loaded into the memory. This returns error if the seek result is not valid
	resultOfRead = fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
	if (!checkValidRead(resultOfRead))
		return RC_READ_FAILED;

	//Sets the page number to the current page postion in fHandle
	setCurrentPosition(pageNum, fHandle);

	//return RC_OK code if the able to read the file without any exception
	return RC_OK;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method gets the current postion of the file in the file handle 
2. Inputs- page number which needs to be retrieved, fileHandle and pageHandle
3. returns the current page position if no exception
*/
int getBlockPos(SM_FileHandle *fHandle)
{
	if (checkValidfHandle(fHandle))
	{
		if (checkValidMgmtInfo(fHandle))
		{
			//returns the current page position in the 
			return fHandle->curPagePos;
		}
		else
		{
			return RC_FILE_NOT_FOUND;
		}
	}
	else
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method reads the first page of the file in the memory 
2. Inputs- fileHandle and pageHandle
3. Output - reads the first page of the file to the memory and returns RC_OK if no exception
*/

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int firstPageNumber = 0;
	if (checkValidfHandle(fHandle))
	{
		if (checkValidMgmtInfo(fHandle))
		{
			//first page position is sent to the read block and read into the memory
			return readBlock(firstPageNumber, fHandle, memPage);
		}
	}
	return RC_OK;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method takes the current page postion and reads the previous page to the memory 
2. Inputs- fileHandle and pageHandle
3. Output - reads the previous page of the file to the memory and returns RC_OK if no exception
*/

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int currentPosValue = getBlockPos(fHandle);

	//Previous page postion is sent to the readblock and read into the memory page
	return readBlock(currentPosValue - 1, fHandle, memPage);
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method reads the current page of the file in the memory 
2. Inputs- fileHandle and pageHandle
3. Output - reads the current page of the file to the memory and returns RC_OK if no exception
*/

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int currentPosValue = getBlockPos(fHandle);

	//gets the current position using getBlockPos method and sent tot he readblock to load the current page to the memory
	return readBlock(currentPosValue, fHandle, memPage);
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method reads the next page of the file in the memory 
2. Inputs- fileHandle and pageHandle
3. Output - reads the next page of the file to the memory and returns RC_OK if no exception
*/
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int currentPosValue = getBlockPos(fHandle);

	//gets the current page position using getBlockPos method and added one to read the next page to the memory
	return readBlock(currentPosValue + 1, fHandle, memPage);
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method reads the last page of the file in the memory 
2. Inputs- fileHandle and pageHandle
3. Output - reads the last page of the file to the memory and returns RC_OK if no exception
*/
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int lastPageNumber;

	//gets the total number of page from the file handle
	lastPageNumber = fHandle->totalNumPages;

	//reads the last page position in the file and loads into the memory
	return readBlock(lastPageNumber - 1, fHandle, memPage);
}



