/// 
/// @brief Abstract definition of asynchronous streams.
///
	 
#ifndef STFASYNCSTREAM_INC
#define STFASYNCSTREAM_INC


#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFBasicTypes.h"

// 
//	Callback classes
// 

//! Callback class for completion message
class STFAsyncCompletionMessage
	{
	public:
		virtual void CompletionMsg(void) = 0;
	};
	
//
// Callback for refilling requests of ASyncOutStreams.  The callback
// function will be called when a significant amount of buffer space
// is available in the stream.
//	
class STFAsyncRefillRequest {
	public:
		virtual void RefillRequest(uint32 free) = 0;
	};


class STFAsyncEmptyRequest {

	public:
		virtual void EmptyRequest(uint32 avail) = 0;

	};

	
class STFAsyncOutputStream {
	
	private:
		STFAsyncRefillRequest	*	req;   
		STFAsyncCompletionMessage	*	cpl;
	
	protected:
		//
		// Function called by the stream implementation, whenever buffer-
		// space is available.
		//
		void RefillRequest(uint32 free) 
			{
			if (req) req->RefillRequest(free);
			}
		void CompletionMsg(void) {
			if (cpl) 
				cpl->CompletionMsg();
			}
		int32	suspendCnt;

	public:                                                                                       
		STFAsyncOutputStream(void) 
			{
			req = NULL; 
			cpl = NULL; 
			completing = false; 
			completed = false; 
			suspended = false; 
			suspendCnt = 0;
			}

		virtual ~STFAsyncOutputStream(void) {};
		
		bool	completing;		// The last piece of data is inside the stream buffer
		bool	completed;		// The stream data is completely sent.
		bool	suspended;		// The transfer is currently suspended, cause of
									// not enough data, or a call to SuspendTransfer().

      virtual STFResult Initialize(void) 
			{
			STFRES_RAISE_OK;
			}
      
		virtual bool  lowData(void) 
			{
			return suspended;
			}
		
		//
		// Send data to the output stream.  Done will be the number
		// of bytes sent.
		//
		virtual STFResult WriteData(void * data, uint32 size, uint32  &done) = 0;
		
		//
		// Inquire available buffer space.
		//
		virtual uint32 AvailSpace(void) = 0;
		
		//
		// Inquire size of valid data in buffer.
		//
		virtual uint32 AvailData(void) = 0;
		
		//
		// Get current transfer location counted from start of the stream
		// or the last changed location.
		//
		virtual uint32 GetTransferLocation(void) = 0;
		
		//
		// Set current transfer location.
		//
		virtual void SetTransferLocation(uint32 pos) = 0;
		
		//
		// Read the data out of the buffer again.  This is usefull
		// when the transfer had to be canceled, but should be
		// retried later.
		//
		virtual STFResult RecoverData(void * buffer, uint32 size, uint32  &done) 
			{
			STFRES_RAISE(STFRES_UNIMPLEMENTED);
			}
		
		//
		// Start the asynchronous transfer.
		//
		virtual STFResult StartTransfer(void) = 0;
		
		//
		// Stop the asynchronous transfer.
		//
		virtual STFResult StopTransfer(void) = 0;
		
		//
		// Suspends the transfer, will complete with Resume
		//
		virtual STFResult SuspendTransfer(void)
			{
			suspendCnt++;
			if (suspendCnt == 1)
				{              
				StopTransfer();
				suspended = true;
				}      

			STFRES_RAISE_OK;
			}
   	
   	virtual STFResult ResumeTransfer(void)
   		{
   		suspendCnt--;
   		if (suspendCnt == 0)
   			{
   			suspended = false;
   			StartTransfer();
   			}

			STFRES_RAISE_OK;
   		}
				
		//
		// Write all data out to the destination
		//		
		virtual STFResult SyncTransfer(void) = 0;
		
		//
		// Inform the stream that no more data will arive.  This will
		// put the stream in "completing" mode, until all data has
		// been sent.
		//
		virtual STFResult CompleteTransfer(void) = 0;
		
		//
		// Flush the buffers contents.
		//
		virtual STFResult FlushBuffer(void) = 0;		

		//
		// Set the refill callback, to be called when bufferspace is available.
		//		
		STFResult SetRefillRequest(STFAsyncRefillRequest * req) 
			{
			this->req = req;STFRES_RAISE_OK;
			}		
		STFResult SetCompletionMsg(STFAsyncCompletionMessage * cpl) 
			{
			this->cpl = cpl;STFRES_RAISE_OK;
			}		

		virtual uint32 GetVideoBitBufferLevel(void) = 0;
		virtual uint32 GetAudioBitBufferLevel(void) = 0;

	};


class STFDualAsyncOutputStream : protected STFAsyncRefillRequest, protected STFAsyncCompletionMessage 
	{
	
	private:
		STFAsyncRefillRequest	*	req;   
		STFAsyncCompletionMessage	*	cpl;

		STFAsyncOutputStream			*	baseStream;
		STFAsyncOutputStream			*	extStream;

	public:                                                                                       
		STFDualAsyncOutputStream(STFAsyncOutputStream * baseStream, STFAsyncOutputStream * extStream);
		
		virtual ~STFDualAsyncOutputStream(void) {};
		
		virtual bool	Completing();		// The last piece of data is inside the stream buffer
		virtual bool	Completed();		// The stream data is completely sent.
		virtual bool	Suspended();		// The transfer is currently suspended, cause of
									// not enough data, or a call to SuspendTransfer().

      virtual STFResult Initialize(void) 
			{
			STFRES_RAISE_OK;
			}
      
		virtual bool lowData(void);
		
		virtual STFResult WriteData(void * data, uint32 size, uint32  &done) 
			{
			STFRES_RAISE(WriteDataSplit(data, size, done, 0));
			}

		virtual STFResult WriteDataSplit(void * data, uint32 size, uint32  &done, int32 channel);
		
		virtual uint32 AvailSpace(void);
		virtual uint32 AvailData(void);
		
		virtual uint32 GetTransferLocation(void);
		virtual void SetTransferLocation(uint32 pos);
		
		virtual STFResult RecoverData(void* buffer, uint32 size, uint32  &done) 
			{
			STFRES_RAISE(STFRES_UNIMPLEMENTED);
			}

		virtual STFResult StartTransfer(void);
		virtual STFResult StopTransfer(void);
		virtual STFResult SuspendTransfer(void);
   	virtual STFResult ResumeTransfer(void);
		virtual STFResult SyncTransfer(void);
		virtual STFResult CompleteTransfer(void);
		virtual STFResult CompleteTransferSplit(int32 channel);
		virtual STFResult FlushBuffer(void);		

		void RefillRequest(uint32 free) 
			{
			if (req) 
				req->RefillRequest(free);
			}
		
		void CompletionMsg(void) 
			{
			if (cpl) 
				cpl->CompletionMsg();
			}

		STFResult SetRefillRequest(STFAsyncRefillRequest * req) 
			{
			this->req = req;
			STFRES_RAISE_OK;
			}		
		
		STFResult SetCompletionMsg(STFAsyncCompletionMessage * cpl) 
			{
			this->cpl = cpl;
			STFRES_RAISE_OK;
			}		
	};



class STFBufferedAsyncOutputStream : public STFAsyncOutputStream, private STFAsyncRefillRequest, private STFAsyncCompletionMessage
	{
	private:
		void RefillRequest(uint32 size);
		void CompletionMsg(void);
	protected:
		STFAsyncOutputStream * strm;

		uint32 size, start, end, used, threshold;

		uint8 *	buffer;                           

		uint16	bufferHandle;

		volatile uint16	semaphore;
		
		STFResult SendDataToStream(void);		
		
		virtual STFResult ReqForwardData(void);		
		virtual STFResult ForwardData(void);
	public:
		STFBufferedAsyncOutputStream(STFAsyncOutputStream * strm,
		                       uint32            size,
		                       uint32				 threshold);
		~STFBufferedAsyncOutputStream(void);
				
		STFResult WriteData(void * data, uint32 size, uint32  &done);
		
		uint32 AvailSpace(void);
		
		uint32 AvailData(void);
		
		uint32 GetTransferLocation(void);
		
		void SetTransferLocation(uint32 pos);
		
		STFResult RecoverData(void * buffer, uint32 size, uint32  &done);
		
		STFResult StartTransfer(void);
		
		STFResult StopTransfer(void);
		
		STFResult SuspendTransfer(void);
   	
  	   STFResult ResumeTransfer(void);
				
		STFResult SyncTransfer(void);
		
		STFResult CompleteTransfer(void);
		
		STFResult FlushBuffer(void);
	};
	



class STFAsyncInputStream {

	private:
		STFAsyncEmptyRequest	*	req;

	protected:
		int32	suspendCnt;

		void EmptyRequest(uint32 avail) 
			{
			if (req) req->EmptyRequest(avail);
			}		

	public:
		bool	suspended;

		STFAsyncInputStream(void) 
			{
			req = NULL;
			}
		virtual ~STFAsyncInputStream(void) {};

		virtual STFResult ReadData(void * data, uint32 size, uint32  &done) = 0;
		virtual uint32 AvailSpace(void) = 0;
		virtual uint32 AvailData(void) = 0;
		virtual uint32 GetTransferLocation(void) = 0;
		virtual void SetTransferLocation(uint32 pos) = 0;
		
		virtual STFResult StartTransfer(void) = 0;
		virtual STFResult StopTransfer(void) = 0;

		virtual STFResult SuspendTransfer(void)
			{
			suspendCnt++;
			if (suspendCnt == 1)
				{              
				StopTransfer();
				suspended = true;
				}      

			STFRES_RAISE_OK;
			}
   	
   	virtual STFResult ResumeTransfer(void)
   		{
   		suspendCnt--;
   		if (suspendCnt == 0)
   			{
   			suspended = false;
   			StartTransfer();
   			}

			STFRES_RAISE_OK;
   		}

		virtual STFResult SyncTransfer(void) = 0;
		virtual STFResult FlushBuffer(void) = 0;

		STFResult SetEmptyRequest(STFAsyncEmptyRequest * req) 
			{
			this->req = req;
			STFRES_RAISE_OK;
			}				
	}; 
	
#endif // STFASYNCSTREAM_INC
