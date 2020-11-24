//
// PURPOSE:   
//

#include <memory.h>
#include "STF/Interface/STFAsyncStream.h"
#include "STF/Interface/STFDataManipulationMacros.h"


STFBufferedAsyncOutputStream::STFBufferedAsyncOutputStream(STFAsyncOutputStream * strm,
                                               uint32            size,
                                               uint32				 threshold)
	{
	this->strm = strm;
	
	this->size = size;
	this->threshold = threshold;
	this->start = 0;
	this->end = 0;
	this->used = 0;     
	
	semaphore = 0;
	
	buffer = new uint8[size];
	if (!buffer) this->size = 0;
	
	strm->SetRefillRequest(this);
	strm->SetCompletionMsg(this);
	}

void STFBufferedAsyncOutputStream::RefillRequest(uint32 size)
	{
	ReqForwardData();
	if (used < threshold) STFAsyncOutputStream::RefillRequest(this->size-used);
	}
	
void STFBufferedAsyncOutputStream::CompletionMsg(void)
	{
	completed = true;
	STFAsyncOutputStream::CompletionMsg();
	}

STFBufferedAsyncOutputStream::~STFBufferedAsyncOutputStream(void)
	{
	if (buffer) 
		delete[] buffer;
	}
				

STFResult STFBufferedAsyncOutputStream::SendDataToStream(void)
	{
	uint32 done;   
	
	semaphore++;
	
	if (semaphore == 1)
		{
		if (end > start)
			{
			STFRES_REASSERT(strm->WriteData(buffer+start, end-start, done));
			start += done;
			used -= done;
			}
		else if (start != end)
			{
			STFRES_REASSERT(strm->WriteData(buffer+start, size-start, done));
			start += done;
			used -= done;
			if (start == size)
				{
				start = 0;
				if (end)
					{
					STFRES_REASSERT(strm->WriteData(buffer, end, done));
					start += done;
					used -= done;
					}
				}
			}
		
		if (completing && used == 0) 
			STFRES_REASSERT(strm->CompleteTransfer());
		}

	semaphore--;
				
	STFRES_RAISE_OK;
	}

STFResult STFBufferedAsyncOutputStream::ForwardData(void)
	{
	return SendDataToStream();
	}

STFResult STFBufferedAsyncOutputStream::ReqForwardData(void)
	{
	return SendDataToStream();
	}

STFResult STFBufferedAsyncOutputStream::WriteData(void * data, uint32 size, uint32 &done)
	{
	uint32 avail;
	uint32	ssize = size;
		
	completing = false;	

	STFRES_REASSERT(ForwardData());
	
	semaphore++;
	
	if (semaphore == 1 && size)
		{
		if (start == 0)
			{
		 	avail = this->size-end-1;
			if (avail > size) avail = size;
		
			memcpy(buffer+end, data, avail);
			end+=avail;
			used+=avail;
			done= avail;
			}
		else
			{
			if (end >= start)
				{
				avail = this->size-end;
				if (avail > size) avail = size;
				
				if (avail)
					{			
					memcpy(buffer+end, data, avail);
						
					used += avail;
					end  += avail;
					done  = avail;
					size -= avail;
					(uint8 * &)data += avail;
					if (end == this->size) end = 0;
					}
				}
			else
				done = 0;
	
			if (start > end+1)
				{
				avail = start-1-end;
				if (avail > size) avail = size;

				if (avail)
					{			
					memcpy(buffer+end, data, avail);
		
					end  += avail;
					used += avail;
					done += avail;		
					}
				}		
			}
		}
	else
		done = 0;
			
	semaphore--;
		
	STFRES_RAISE_OK;
	}
		
uint32 STFBufferedAsyncOutputStream::AvailSpace(void)
	{
	return size-used-1+strm->AvailSpace();
	}
		
uint32 STFBufferedAsyncOutputStream::AvailData(void)
	{
	return used+strm->AvailData();
	}
		
uint32 STFBufferedAsyncOutputStream::GetTransferLocation(void)
	{
	return strm->GetTransferLocation();
	}
		
void STFBufferedAsyncOutputStream::SetTransferLocation(uint32 pos)
	{
	strm->SetTransferLocation(pos);
	}
		
STFResult STFBufferedAsyncOutputStream::RecoverData(void * buffer, uint32 size, uint32 &done)
	{
	uint32 ddone, avail;
	
	STFRES_REASSERT(strm->RecoverData(buffer, size, ddone));
	
	if (ddone < size)
		{
		size   -= ddone;
		(uint8 * &)buffer += ddone;
		
		if (start > end)
			{
			avail = this->size-start;
			if (avail > size) avail = size;
			
			memcpy(buffer, this->buffer+start, avail);
			
			used   -= avail;
			start  += avail;
			ddone  += avail;
			size   -= avail;
			(uint8 * &)buffer += avail;
			
			if (start == this->size) start = 0;					
			}
		
		if (start < end && size)
			{
			avail = end-start;
			if (avail > size) avail = size;
			
			memcpy(buffer, this->buffer+start, avail);
			
			used  -= avail;
			start += avail;
			
			ddone += avail;
			}
		}

	done = ddone;
		
	STFRES_RAISE_OK;	
	}
		
STFResult STFBufferedAsyncOutputStream::StartTransfer(void)
	{
	STFRES_REASSERT(ForwardData());
	STFRES_REASSERT(strm->StartTransfer());
	
	STFRES_RAISE_OK;	
	}
		
STFResult STFBufferedAsyncOutputStream::StopTransfer(void)
	{
	return strm->StopTransfer();
	}
		
STFResult STFBufferedAsyncOutputStream::SuspendTransfer(void)
	{
	return strm->SuspendTransfer();
	}
   	
STFResult STFBufferedAsyncOutputStream::ResumeTransfer(void)
	{
	return strm->ResumeTransfer();
	}
				
STFResult STFBufferedAsyncOutputStream::SyncTransfer(void)
	{
	do {
		STFRES_REASSERT(ForwardData());
		} while (used);
	return strm->SyncTransfer();
	}
		
STFResult STFBufferedAsyncOutputStream::CompleteTransfer(void)
	{
	completing = true;
	
	if (!used) STFRES_REASSERT(strm->CompleteTransfer());

	STFRES_RAISE_OK;	
	}
		
STFResult STFBufferedAsyncOutputStream::FlushBuffer(void)
	{
	STFRES_REASSERT(strm->FlushBuffer());
	
	used = 0;
	start = 0;
	end = 0;
	
	STFRES_RAISE_OK;
	}
	


STFDualAsyncOutputStream::STFDualAsyncOutputStream(STFAsyncOutputStream * baseStream, STFAsyncOutputStream * extStream)
	{
	req = NULL;
	cpl = NULL;
	this->baseStream	= baseStream;

	if (baseStream)
		{
		baseStream->SetRefillRequest(this);
		baseStream->SetCompletionMsg(this);
		}

	this->extStream	= extStream;

	if (extStream)
		{
		extStream->SetRefillRequest(this);
		extStream->SetCompletionMsg(this);
		}
	}

bool STFDualAsyncOutputStream::lowData(void)
	{
	return (extStream ? (baseStream->lowData() || extStream->lowData()) : baseStream->lowData());
	}

bool STFDualAsyncOutputStream::Completing()
	{
	return (extStream ? (baseStream->completing || extStream->completing) : (baseStream->completing));
	}

bool STFDualAsyncOutputStream::Completed()
	{
	return (extStream ? (baseStream->completed && extStream->completed) : (baseStream->completed));
	}

bool STFDualAsyncOutputStream::Suspended()
	{
	return (extStream ? (baseStream->suspended || extStream->suspended) : (baseStream->suspended));
	}

STFResult STFDualAsyncOutputStream::WriteDataSplit(void * data, uint32 size, uint32 &done, int32 channel)
	{
	if (0 == channel)
		STFRES_RAISE(baseStream->WriteData(data, size, done));
	else
		{
		if (extStream)
			{
			STFRES_RAISE(extStream->WriteData(data, size, done));
			}
		}

	STFRES_RAISE_OK;
	}
		
uint32 STFDualAsyncOutputStream::AvailSpace(void)
	{
	if (extStream)
		return min(baseStream->AvailSpace(), extStream->AvailSpace());
	else
		return baseStream->AvailSpace();
	}

uint32 STFDualAsyncOutputStream::AvailData(void)
	{
	if (extStream)
		return max(baseStream->AvailData(), extStream->AvailData());
	else
		return baseStream->AvailData();
	}
		
uint32 STFDualAsyncOutputStream::GetTransferLocation(void)
	{
	return baseStream->GetTransferLocation();
	}

void STFDualAsyncOutputStream::SetTransferLocation(uint32 pos)
	{
	baseStream->SetTransferLocation(pos);
	}
		
STFResult STFDualAsyncOutputStream::StartTransfer(void)
	{
	STFRES_REASSERT(baseStream->StartTransfer());

	if (extStream)
		STFRES_REASSERT(extStream->StartTransfer());

	STFRES_RAISE_OK;
	}

STFResult STFDualAsyncOutputStream::StopTransfer(void)
	{
	STFRES_REASSERT(baseStream->StopTransfer());

	if (extStream)
		STFRES_REASSERT(extStream->StopTransfer());

	STFRES_RAISE_OK;
	}

STFResult STFDualAsyncOutputStream::SuspendTransfer(void)
	{
	STFResult error = baseStream->SuspendTransfer();

	if (extStream)
		error = extStream->SuspendTransfer();

	STFRES_RAISE(error);
	}

STFResult STFDualAsyncOutputStream::ResumeTransfer(void)
	{
	STFResult error = baseStream->ResumeTransfer();

	if (extStream)
		error = extStream->ResumeTransfer();

	STFRES_RAISE(error);
	}

STFResult STFDualAsyncOutputStream::SyncTransfer(void)
	{
	STFRES_REASSERT(baseStream->SyncTransfer());

	if (extStream)
		STFRES_REASSERT(extStream->SyncTransfer());

	STFRES_RAISE_OK;
	}

STFResult STFDualAsyncOutputStream::CompleteTransfer(void)
	{
	STFRES_REASSERT(baseStream->CompleteTransfer());

	if (extStream)
		STFRES_REASSERT(extStream->CompleteTransfer());

	STFRES_RAISE_OK;
	}

STFResult STFDualAsyncOutputStream::CompleteTransferSplit(int32 channel)
	{
	if (0 == channel)
		STFRES_REASSERT(baseStream->CompleteTransfer());
	else if (extStream)
		STFRES_REASSERT(extStream->CompleteTransfer());

	STFRES_RAISE_OK;
	}

STFResult STFDualAsyncOutputStream::FlushBuffer(void)
	{
	STFRES_REASSERT(baseStream->FlushBuffer());

	if (extStream)
		STFRES_REASSERT(extStream->FlushBuffer());

	STFRES_RAISE_OK;
	}

