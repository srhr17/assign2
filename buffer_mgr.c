#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "buffer_mgr.h"
#include "storage_mgr.h"

typedef struct BufferFrame
{
	struct BufferFrame *prevFrame;
	int dirtyFlag;
	struct BufferFrame *nextFrame;
	int pageNumber;
	int count;
	char *data;
} BufferFrame;


typedef struct BufferManager
{
	BufferFrame *head, *start, *tail;
	int numRead;
	int numWrite;
	SM_FileHandle *smFileHandle;
	int count;
	void *strategyData;
} BufferManager;

void AssignBufferManagerValues(BufferManager *bufferManager, BufferFrame *frame)
{
	bufferManager->head = bufferManager->start;

	if (bufferManager->head != NULL)
	{
		bufferManager->tail->nextFrame = frame;
		frame->prevFrame = bufferManager->tail;
		bufferManager->tail = bufferManager->tail->nextFrame;
	}
	else
	{
		bufferManager->head = frame;
		bufferManager->tail = frame;
		bufferManager->start = frame;
	}

	bufferManager->tail->nextFrame = bufferManager->head;
	bufferManager->head->prevFrame = bufferManager->tail;
}

void createBufferFrame(BufferManager *bufferManager)
{
	BufferFrame *frame = (BufferFrame *)malloc(sizeof(BufferFrame));
	frame->data = calloc(PAGE_SIZE, sizeof(char *));
	frame->dirtyFlag = 0;
	frame->count = 0;
	frame->pageNumber = -1;
	AssignBufferManagerValues(bufferManager, frame);
}

void CleanBufferPool(BufferManager *bufferManager, BM_BufferPool *bufferPool)
{
	int NumberofPages;
	bufferPool->pageFile = NULL;
	bufferPool->mgmtData = NULL;
	bufferPool->numPages = NumberofPages;
	bufferManager->start = NULL;
	bufferManager->head = NULL;
	bufferManager->tail = NULL;
	free(bufferManager);

}

bool CheckValidManagementData(BM_BufferPool *const bufferPool)
{
	return (bufferPool->mgmtData == NULL) ? false : true;
}

BufferManager *GetBufferManager()
{
	BufferManager *bufferManager = (BufferManager *)malloc(sizeof(BufferManager));
	bufferManager->smFileHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
	return bufferManager;
}

/*
Jason Scott - A20436737
1. This method initiazatizes buffer pool
2. Opens a existing page with new frames
3. Initial data is stored within page and closed
*/
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
	int i;

	if (CheckValidManagementData(bm))
		return RC_BUFFER_POOL_ALREADY_INIT;

	BufferManager *bufferManager = GetBufferManager();

	RC OpenPageReturnCode = openPageFile((char *)pageFileName, bufferManager->smFileHandle);
	if (OpenPageReturnCode == RC_OK)
	{
		for (i=0; i < numPages; i++)
			createBufferFrame(bufferManager);

		bufferManager->numWrite = 0;
		bufferManager->tail = bufferManager->head;
		bufferManager->numRead = 0;
		bufferManager->strategyData = stratData;
		bufferManager->count = 0;
		bm->strategy = strategy;
		bm->numPages = numPages;
		bm->mgmtData = bufferManager;
		bm->pageFile = (char *)pageFileName;

		closePageFile(bufferManager->smFileHandle);

		return RC_OK;
	}
	else
		return OpenPageReturnCode;
}


/*
Jason Scott - A20436737
1. This method opens and checks for dirty pages
2. All dirtypages with fix count zero are written to disk
3. Writes it then closes before returning to method above to be destroyed
*/
RC forceFlushPool(BM_BufferPool *const bm)
{
	BufferManager *bufferManager = bm->mgmtData;
	BufferFrame *frame = bufferManager->head;

	SM_FileHandle *sm_fileHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
	;
	bufferManager->smFileHandle = sm_fileHandle;
	RC openpageReturnCode = openPageFile((char *)(bm->pageFile), bufferManager->smFileHandle);

	if (openpageReturnCode == RC_OK)
	{
		do
		{
			if (frame->count == 0 && frame->dirtyFlag != 0)
			{
				RC writeBlockReturnCode = writeBlock(frame->pageNumber, bufferManager->smFileHandle, frame->data);
				if (writeBlockReturnCode == RC_OK)
				{
					frame->dirtyFlag = 0;
					bufferManager->numWrite++;
				}
				else
				{
					closePageFile(bufferManager->smFileHandle);
					return writeBlockReturnCode;
				}
			}
			frame = frame->nextFrame;
		} while (frame != bufferManager->head);
	}
	else
		return openpageReturnCode;

	closePageFile(bufferManager->smFileHandle);
	return RC_OK;
}

/*
Jason Scott - A20436737
1. This method destroyes buffer pool
2. Utilizes forceflush method to write all the dirtyPages back again, before destroying
3. It frees up the buffer pool's resources
*/
RC shutdownBufferPool(BM_BufferPool *const bm)
{
	int i;

	if (!CheckValidManagementData(bm))
		return RC_BUFFER_POOL_NOT_INIT;
	BufferManager *bufferManager = bm->mgmtData;
	BufferFrame *frame = bufferManager->head;

	forceFlushPool(bm);

	frame = frame->nextFrame;
	for (i=0; frame != bufferManager->head; i++)
	{
		free(frame->data);
		frame = frame->nextFrame;
	}
	free(frame);
	CleanBufferPool(bufferManager, bm);
	return RC_OK;
}


/* Darek Nowak A20497998
//  1. Searches for given page
//  2. If found, marks as dirty to no
*/
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	int flag = 1;
	if (!CheckValidManagementData(bm))
		return RC_BUFFER_POOL_NOT_INIT;

	BufferManager *bufferManager = bm->mgmtData;
	BufferFrame *frame = bufferManager->head;
	do
	{
		if (frame->pageNumber == page->pageNum)
		{
			frame->dirtyFlag = flag;
			return RC_OK;
		}
		else
			frame = frame->nextFrame;

	} while (frame != bufferManager->head);

	return RC_OK;
}

/* Darek Nowak A20497998
// 1. Checks to see if page is in the buffer pool
// 2. If it is, decrements fixCount by 1. Else nothing happens
*/
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	BufferManager *bufferManager = bm->mgmtData;
	BufferFrame *frame = bufferManager->head;
	if (bufferManager->head->pageNumber == page->pageNum)
	{
		bufferManager->head->count--;
		return RC_OK;
	}
	frame = frame->nextFrame;
	int i;
	for (i=0; frame != bufferManager->head; i++)
	{
		if (frame->pageNumber == page->pageNum)
		{
			frame->count--;
			return RC_OK;
		}
		frame = frame->nextFrame;
	}

	return RC_OK;
}

/* Darek Nowak A20497998
// 1. Checks if the page exists
// 2. If it does, it will check to see if it is dirty
// 3. If it is dirty, writes to disk
*/
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	BufferManager *bufferManager = bm->mgmtData;
	BufferFrame *frame = bufferManager->head;

	SM_FileHandle *sm_fileHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
	;
	bufferManager->smFileHandle = sm_fileHandle;

	RC openpageReturnCode = openPageFile((char *)(bm->pageFile), bufferManager->smFileHandle);
	if (openpageReturnCode == RC_OK)
	{
		do
		{
			if (frame->dirtyFlag == 1 && frame->pageNumber == page->pageNum)
			{
				RC writeBlockReturnCode = writeBlock(frame->pageNumber, bufferManager->smFileHandle, frame->data);
				if (writeBlockReturnCode == RC_OK)
				{
					frame->dirtyFlag = 0;
					bufferManager->numWrite++;
				}
				else
				{
					closePageFile(bufferManager->smFileHandle);
					return writeBlockReturnCode;
				}
			}
			frame = frame->nextFrame;
		} while (frame != bufferManager->head);
	}
	else
		return openpageReturnCode;
	closePageFile(bufferManager->smFileHandle);
	return RC_OK;
}


RC CheckIfPageExists(int algo, const PageNumber pageNum, BufferManager *bufferManager, BM_PageHandle *const page)
{
	int value = 0;
	BufferFrame *frame = bufferManager->head;
	do
	{
		if (frame->pageNumber != pageNum)
			frame = frame->nextFrame;
		else
		{
			page->pageNum = pageNum;
			page->data = frame->data;

			frame->pageNumber = pageNum;
			frame->count++;
			if (algo == RS_LRU)
			{
				bufferManager->tail = bufferManager->head->nextFrame;
				bufferManager->head = frame;
			}
			value = 1;	
			return RC_OK;
		}
	} while (frame != bufferManager->head);
	if (value == 0)
		return RC_IM_KEY_NOT_FOUND;
	return RC_OK;
}


void CheckIfBufferPoolIsEmpty(const PageNumber pageNumber, BM_BufferPool *const bufferPool)
{
	BufferManager *bufferManager = bufferPool->mgmtData;
	BufferFrame *frame = bufferManager->head;

	if (bufferPool->numPages > bufferManager->count)
	{
		frame = bufferManager->head;
		frame->pageNumber = pageNumber;
		if (frame->nextFrame != bufferManager->head)
		{
			bufferManager->head = frame->nextFrame;
		}
		frame->count = frame->count + 1;
		bufferManager->count = bufferManager->count + 1;
	}
}

RC CheckReturnCode(SM_FileHandle sm_fileHandle, RC returnCode)
{
	closePageFile(&sm_fileHandle);
	return returnCode;
}



RC LRU(SM_FileHandle sm_FileHandle, BM_PageHandle *const page, const PageNumber pageNumber,
		BufferFrame *frame, BM_BufferPool *const bm, BufferManager *mgmt)
{
	if (bm->numPages <= mgmt->count)
	{
		frame = mgmt->tail;
		do
		{
			if (frame->count != 0)
				frame = frame->nextFrame;
			else
			{
				if (frame->dirtyFlag != 0)
				{
					ensureCapacity(frame->pageNumber, &sm_FileHandle);
					RC writeBlockReturnCode = writeBlock(frame->pageNumber, &sm_FileHandle, frame->data);
					if (writeBlockReturnCode == RC_OK)
						mgmt->numWrite++;
					else
						return CheckReturnCode(sm_FileHandle, writeBlockReturnCode);
				}

				if (mgmt->tail == mgmt->head)
				{
					frame = frame->nextFrame;
					frame->pageNumber = pageNumber;
					frame->count++;
					mgmt->tail = frame;
					mgmt->head = frame;
					mgmt->tail = frame->prevFrame;
					break;
				}
				else
				{
					frame->pageNumber = pageNumber;
					frame->count++;
					mgmt->tail = frame;
					mgmt->tail = frame->nextFrame;
					break;
				}
			}
		} while (frame != mgmt->tail);
	}
	else CheckIfBufferPoolIsEmpty(pageNumber, bm);
	ensureCapacity((pageNumber + 1), &sm_FileHandle);
	RC readBlockReturnCode = readBlock(pageNumber, &sm_FileHandle, frame->data);
	if (readBlockReturnCode == RC_OK) mgmt->numRead++;
	else return readBlockReturnCode;
	page->pageNum = pageNumber;
	page->data = frame->data;
	closePageFile(&sm_FileHandle);
	return RC_OK;
}


RC FIFO(SM_FileHandle sm_FileHandle, BM_PageHandle *const page, const PageNumber pageNumber,
		BufferFrame *frame, BM_BufferPool *const bm, BufferManager *mgmt)
{
	if (bm->numPages <= mgmt->count)
	{
		frame = mgmt->tail;
		do
		{
			if (frame->count != 0) frame = frame->nextFrame;
			else
			{
				if (frame->dirtyFlag != 0)
				{
					ensureCapacity(frame->pageNumber, &sm_FileHandle);
					RC writeBlockReturnCode = writeBlock(frame->pageNumber, &sm_FileHandle, frame->data);
					if (writeBlockReturnCode == RC_OK)
						mgmt->numWrite++;
					else
						return CheckReturnCode(sm_FileHandle, writeBlockReturnCode);
				}

				mgmt->tail = frame->nextFrame;
				frame->pageNumber = pageNumber;
				mgmt->head = frame;
				frame->count++;
				break;
			}

		} while (frame != mgmt->head);
	}
	
	else
		CheckIfBufferPoolIsEmpty(pageNumber, bm);
	ensureCapacity((pageNumber + 1), &sm_FileHandle);
	RC readBlockReturnCode = readBlock(pageNumber, &sm_FileHandle, frame->data);
	if (readBlockReturnCode == RC_OK)
		mgmt->numRead++;
	else
		return CheckReturnCode(sm_FileHandle, readBlockReturnCode);
	page->pageNum = pageNumber;
	page->data = frame->data;
	closePageFile(&sm_FileHandle);
	return RC_OK;
}



RC CheckReplacementStrategy(BM_PageHandle *const page, BufferManager *bufferManager, const PageNumber pageNum,
							  ReplacementStrategy strategy, BufferFrame *frame, SM_FileHandle sm_FileHandle, BM_BufferPool *const bufferPool)
{
	RC IsPageExistsReturnCode;
	if (bufferPool->strategy == RS_LRU)
	{
		IsPageExistsReturnCode = CheckIfPageExists(RS_LRU, pageNum, bufferManager, page);
		if (IsPageExistsReturnCode == RC_OK)
			return RC_OK;
		else
			LRU(sm_FileHandle, page, pageNum, frame, bufferPool, bufferManager);
	}
	else if (bufferPool->strategy == RS_FIFO)
	{
		IsPageExistsReturnCode = CheckIfPageExists(RS_FIFO, pageNum, bufferManager, page);
		if (IsPageExistsReturnCode == RC_OK)
			return RC_OK;
		else
			FIFO(sm_FileHandle, page, pageNum, frame, bufferPool, bufferManager);
	}
	return RC_OK;
}

/* Darek Nowak A20497998
//
*/
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
	if (!CheckValidManagementData(bm))
		return RC_BUFFER_POOL_ALREADY_INIT;

	RC IsPageExistsReturnCode;

	SM_FileHandle sm_FileHandle;
	BufferManager *bufferManager = bm->mgmtData;
	BufferFrame *frame = bufferManager->head;

	RC openPageReturnCode = openPageFile((char *)bm->pageFile, &sm_FileHandle);
	if (openPageReturnCode == RC_OK)
	{
		CheckReplacementStrategy(page, bufferManager, pageNum, bm->strategy, frame, sm_FileHandle, bm);
	}
	return RC_OK;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method gets the number of pages
2. Inputs- buffer pool object
3. returns - Returns number of pages in the buffer pool
*/
int GetPageCount(BM_BufferPool *const bm)
{
	return bm->numPages;
}

// Statistics Interface

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method gets the frame content from the buffer pool
2. Inputs- buffer pool object
3. returns - Returns number of pages stored in the page frame
*/
PageNumber *getFrameContents(BM_BufferPool *const bm)
{
	int page_count = GetPageCount(bm);
	
	BufferFrame *allFrames = ((BufferManager *)bm->mgmtData)->start;
	PageNumber *frameContent = (PageNumber *)malloc(sizeof(PageNumber) * page_count);

	if (frameContent != NULL)
	{
		int i;
		for (i = 0; i < page_count; i++)
		{
			frameContent[i] = allFrames->pageNumber;
			allFrames = allFrames->nextFrame;
		}
	}
	return frameContent;
}


/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method gets the dirty flag of all the pages from the buffer pool
2. Inputs- buffer pool object
3. returns - Returns true(if the page is modified)or false(if the page is not modified) for each pages in the buffer pool
*/
bool *getDirtyFlags(BM_BufferPool *const bm)
{
int page_count = GetPageCount(bm);
	BufferFrame *currentFrame = ((BufferManager *)bm->mgmtData)->start;

	bool *dirtyFlag = (bool *)malloc(sizeof(bool) * page_count);

	if (dirtyFlag != NULL)
	{
		int i;
		for (i = 0; i < page_count; i++)
		{
			dirtyFlag[i] = currentFrame->dirtyFlag;
			currentFrame = currentFrame->nextFrame;
		}
	}
	return dirtyFlag;
	
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method gets the dirty flag of all the pages from the buffer pool
2. Inputs- buffer pool object
3. returns - Returns fix count stored of the page stored in the page frame
*/
int *getFixCounts(BM_BufferPool *const bm)
{
	int page_count = GetPageCount(bm);

	BufferFrame *currentFrame = ((BufferManager *)bm->mgmtData)->start;

	int *fixCountResult = (int *)malloc(sizeof(int) * page_count);

	if (fixCountResult != NULL)
	{
		int i;
		for (i = 0; i < page_count; i++)
		{
			fixCountResult[i] = currentFrame->count;
			currentFrame = currentFrame->nextFrame;
		}
	}
	return fixCountResult;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method gets the number of pages read from the buffer pool since initialized
2. Inputs- buffer pool object
3. returns - Returns number of pages read from the buffer pool
*/
int getNumReadIO(BM_BufferPool *const bm)
{
	if (!CheckValidManagementData(bm))
		return RC_BUFFER_POOL_NOT_INIT;
	else
		return ((BufferManager *)bm->mgmtData)->numRead;
}

/*
Ramya Krishnan(rkrishnan1@hawk.iit.edu) - A20506653
1. This method gets the number of pages write in to the buffer pool since initialized
2. Inputs- buffer pool object
3. returns - Returns number of pages write into the buffer pool
*/
int getNumWriteIO(BM_BufferPool *const bm)
{
	if (!CheckValidManagementData(bm))
		return RC_BUFFER_POOL_NOT_INIT;
	else
		return ((BufferManager *)bm->mgmtData)->numWrite;
}
