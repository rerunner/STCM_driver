///
/// @brief      Implementation for generic messages
///

#include "STF/Interface/Types/STFMessage.h"

//#include "STF/Interface/STFMemoryManagement.h"
// Gabriel Gooslin removed 4/13/04.  This will allow the 5700 team to build with the 196 compiler.  If this is a problem, 
// uncomment it and put "#if USE_196_COMPILER" around it.

void STFMessage::BeginWait(void)
	{
	GetCurrentSTFThread(wait);
	wait->ResetThreadSignal();
	}

void STFMessage::CompleteWait(void)
	{
	wait->WaitThreadSignal();
	}

void STFMessage::Complete()
	{
	if (wait) wait->SetThreadSignal();
	}
	

DispatchedSTFMessageSink::DispatchedSTFMessageSink(STFMessageDispatcher * dispatcher)
	{
	this->dispatcher = dispatcher;
	}

STFResult DispatchedSTFMessageSink::SendMessage(STFMessage message, bool wait)
	{
	STFRES_RAISE(dispatcher->DispatchMessage(this, message, wait));
	}

STFResult DirectSTFMessageDispatcher::DispatchMessage(STFMessageSink * sink, STFMessage message, bool wait)
	{
	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else
		STFRES_RAISE(sink->ReceiveMessage(message));
	}

QueuedSTFMessageProcessingDispatcher::QueuedSTFMessageProcessingDispatcher(int queueSize) // must be power of two
	{
	this->queueSize = queueSize;
	this->queueMask = queueSize - 1;
	this->queueWrite = 0;
	this->queueRead = 0;
	this->queue = new STFMessageQueue[queueSize];
	}

QueuedSTFMessageProcessingDispatcher::~QueuedSTFMessageProcessingDispatcher()
   {
   delete[] this->queue;
   }

STFResult QueuedSTFMessageProcessingDispatcher::DispatchMessage(STFMessageSink * sink, STFMessage message, bool wait)
	{
	if (wait)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);	//	Can not wait with a dispatcher of this type
	else if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else if (queueWrite - queueRead < queueSize)
		{
		queue[queueWrite & queueMask].sink = sink;
		queue[queueWrite & queueMask].message = message;
		queueWrite++;

		Trigger();

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_FULL);
	}

STFResult QueuedSTFMessageProcessingDispatcher::ProcessMessages(void)
	{
	STFMessageSink	*	msgSink;
	STFMessage			msg;

	// For performance reasons, this check was moved outside of
	// the readLock. It is assumed to be thread safe concerning the
	// queueRead and queueWrite variables.
	while (!abort && queueRead < queueWrite)
		{
		readLock.Enter();
		if (!abort && queueRead < queueWrite)
			{
			if (queue[queueRead & queueMask].sink)
				{
				msgSink = queue[queueRead & queueMask].sink;
				msg = queue[queueRead & queueMask].message;
				queue[queueRead & queueMask].sink = NULL;
				queueRead++;
				readLock.Leave();
				msgSink->ReceiveMessage(msg);
				}
			else
				{
				queueRead++;
				readLock.Leave();
				}
			}
		else
			{

			readLock.Leave();
			}
		}

	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}

STFResult QueuedSTFMessageProcessingDispatcher::FlushMessages(void)
	{
	readLock.Enter();

	queueRead = queueWrite;

	readLock.Leave();

	STFRES_RAISE_OK;
	}

STFResult QueuedSTFMessageProcessingDispatcher::FlushMessages(STFMessageSink * sink)
	{
	int i;

	readLock.Enter();

	i = queueRead;
	while (i < queueWrite)
		{
		if (queue[i & queueMask].sink == sink)
			{
			queue[i & queueMask].sink = NULL;
			if (i == queueRead)
				queueRead++;
			}
		i++;
		}

	readLock.Leave();

	STFRES_RAISE_OK;
	}

STFResult QueuedSTFMessageProcessingDispatcher::CheckQueueState(void)
	{
	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else if (queueWrite == queueRead)
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	else if (queueWrite == queueRead + queueSize)
		STFRES_RAISE(STFRES_OBJECT_FULL);
	else
		STFRES_RAISE_OK;
	}

void TriggeredQueuedSTFMessageProcessingDispatcher::Trigger(void)
	{
	trigger.SetSignal();
	}

TriggeredQueuedSTFMessageProcessingDispatcher::TriggeredQueuedSTFMessageProcessingDispatcher(int queueSize)
	: QueuedSTFMessageProcessingDispatcher(queueSize)
	{
	}

TriggeredQueuedSTFMessageProcessingDispatcher::~TriggeredQueuedSTFMessageProcessingDispatcher()
   {
   }

STFResult TriggeredQueuedSTFMessageProcessingDispatcher::WaitMessage(void)
	{
	while (!abort && queueRead == queueWrite)
		{
		trigger.Wait();
		}

	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else
		STFRES_RAISE_OK;
	}


SlavedQueuedSTFMessageProcessingDispatcher::SlavedQueuedSTFMessageProcessingDispatcher(MasterSTFMessageProcessor * master, int queueSize)
	: QueuedSTFMessageProcessingDispatcher(queueSize)
	{
	master->AttachSlave(this);
	}


void TriggeringSlavedQueuedSTFMessageProcessingDispatcher::Trigger(void)
	{
	master->TriggerMessageArrival();
	}


TriggeringSlavedQueuedSTFMessageProcessingDispatcher::TriggeringSlavedQueuedSTFMessageProcessingDispatcher(TriggeredMasterSTFMessageProcessor * master, int queueSize)
	: SlavedQueuedSTFMessageProcessingDispatcher(master, queueSize)
	{
	this->master = master;
	}


typedef STFMessageProcessor * STFMessageProcessorPtr;

MasterSTFMessageProcessor::MasterSTFMessageProcessor(void)
	{
	numSlaves = 0;
	totalSlaves = 4;
	slaves = new STFMessageProcessorPtr[totalSlaves];
	}

MasterSTFMessageProcessor::~MasterSTFMessageProcessor(void)
   {
   delete[] slaves;
   }

STFResult MasterSTFMessageProcessor::AttachSlave(STFMessageProcessor * slave)
	{
	STFMessageProcessor	**	tempSlaves;
	int i;
	//lint --e{613}
	if (numSlaves == totalSlaves)
		{
		tempSlaves = slaves;
		totalSlaves *= 2;
		slaves = new STFMessageProcessorPtr[totalSlaves];
		for(i=0; i<numSlaves; i++)
			slaves[i] = tempSlaves[i];
		delete[] tempSlaves;
		}

	slaves[numSlaves++] = slave;

	STFRES_RAISE_OK;
	}

STFResult MasterSTFMessageProcessor::RemoveSlave(STFMessageProcessor * slave)
	{
	int i = 0;
	//lint --e{613}
	while (i < numSlaves && slave != slaves[i])
		i++;

	if (i < numSlaves)
		{
		slaves[i] = slaves[numSlaves-1];
		numSlaves--;
		}

	STFRES_RAISE_OK;
	}

STFResult MasterSTFMessageProcessor::AbortProcessor(void)
	{
	int i;
	//lint --e{613}
	for(i=0; i<numSlaves; i++)
		{
		slaves[i]->AbortProcessor();
		}

	STFRES_RAISE_OK;
	}

STFResult MasterSTFMessageProcessor::ProcessMessages(void)
	{
	int i;
	//lint --e{613}
	for(i=0; i<numSlaves; i++)
		{
		slaves[i]->ProcessMessages();
		}

	STFRES_RAISE_OK;
	}

STFResult MasterSTFMessageProcessor::FlushMessages(void)
	{
	int i;
	//lint --e{613}
	for(i=0; i<numSlaves; i++)
		{
		slaves[i]->FlushMessages();
		}

	STFRES_RAISE_OK;
	}

STFResult MasterSTFMessageProcessor::FlushMessages(STFMessageSink * sink)
	{
	int i;
	//lint --e{613}
	for(i=0; i<numSlaves; i++)
		{
		slaves[i]->FlushMessages(sink);
		}

	STFRES_RAISE_OK;
	}

///
///  @brief Checks queues of all attached slaves
///
///  This method calls CheckQueueState() on all attached slaves and
///  evaluates a common queue state which is returned.
///            
///  @return STFResult (see implementation comments for details)
///              
STFResult MasterSTFMessageProcessor::CheckQueueState(void)
	{
	int i;
	STFResult err;
	//lint --e{613}
	bool anyNotEmpty, anyFull;
   //bool anyNotFull; //not needed for new implementation

   anyNotEmpty = false;
   anyFull = false;

	//anyNotFull = false; //not needed for new implementation
	
   /*
   The new implementation calls CheckQueueState on all attached slaves.
   The algorithm starts with the assumption that every queue is empty.
   If a queue returns STFRES_OK the bool value anyNotEmpty is set to TRUE
   If any of the queues is full, both bool values, anyNotEmpty and anyFull, are set to TRUE. 
   None of the bool values can be reset.
   As soon as any of the dispatchers is aborted STFRES_OPEARTION_ABORTED is returned immediately.
   */
   for(i=0; i<numSlaves; i++)
      {
      err = slaves[i]->CheckQueueState();
      if (err == STFRES_OPERATION_ABORTED)
         STFRES_RAISE(STFRES_OPERATION_ABORTED);
      else if (err == STFRES_OK)
         anyNotEmpty = true;
      else if (err == STFRES_OBJECT_FULL)
         {
         anyFull = true;
         anyNotEmpty = true;
         }
      }

   if (anyFull)
      {
      if (anyNotEmpty)
         STFRES_RAISE(STFRES_OBJECT_FULL);
      else
         STFRES_RAISE(STFRES_OPERATION_ABORTED); //This case can not happen and is present only for algorithm completion
      }
   else 
      {
      if (anyNotEmpty)
         STFRES_RAISE_OK;
      else
         STFRES_RAISE(STFRES_OBJECT_EMPTY);
      }
		
	/*
   //removed old implementation 
   for(i=0; i<numSlaves; i++)
		{
		err = slaves[i]->CheckQueueState();
		if (err == STFRES_OK)
			STFRES_RAISE_OK;
		else if (err == STFRES_OBJECT_EMPTY)
			anyNotFull = true;
		else if (err == STFRES_OBJECT_FULL)
			anyNotEmpty = true;
		}

	if (anyNotFull)
		{
		if (anyNotEmpty)
			STFRES_RAISE_OK;
		else
			STFRES_RAISE(STFRES_OBJECT_EMPTY);
		}
	else if (anyNotEmpty)
		{
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}
	   else
		   STFRES_RAISE(STFRES_OPERATION_ABORTED);
   */
	}

TriggeredMasterSTFMessageProcessor::TriggeredMasterSTFMessageProcessor(void)
	{
	}

STFResult TriggeredMasterSTFMessageProcessor::WaitMessage(void)
	{
	int i;
	STFResult err = STFRES_OBJECT_NOT_FOUND;
	bool any;
	//lint --e{613}
	for(;;)
		{
		any = false;
		
		for(i=0; i<numSlaves; i++)
			{
			err = slaves[i]->CheckQueueState();
			if (err == STFRES_OBJECT_FULL || err == STFRES_OK)
				STFRES_RAISE_OK;
			else if (err == STFRES_OBJECT_EMPTY)
				any = true;
			}

		if (any || numSlaves == 0)
			{
			trigger.Wait();
			}
		else
			STFRES_RAISE(err);
		}
	}

STFResult TriggeredMasterSTFMessageProcessor::TriggerMessageArrival(void)
	{
	trigger.SetSignal();

	STFRES_RAISE_OK;
	}


WaitableQueuedSTFMessageProcessingDispatcher::WaitableQueuedSTFMessageProcessingDispatcher(int queueSize) // must be power of two
	{
	this->queueSize = queueSize;
	this->queueMask = queueSize - 1;
	this->queueWrite = 0;
	this->queueRead = 0;
	this->queue = new STFMessageQueue[queueSize];
	}

WaitableQueuedSTFMessageProcessingDispatcher::~WaitableQueuedSTFMessageProcessingDispatcher()
   {
   delete[] this->queue;
   }

STFResult WaitableQueuedSTFMessageProcessingDispatcher::DispatchMessage(STFMessageSink * sink, STFMessage message, bool wait)
	{
	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else if (queueWrite - queueRead < queueSize)
		{
		writeLock.Enter();
		queue[queueWrite & queueMask].sink = sink;
		if (wait)
			{
			message.BeginWait();

			queue[queueWrite & queueMask].message = &message;

			queueWrite++;

			Trigger();

			writeLock.Leave();

			message.CompleteWait();
			}
		else
			{
			queue[queueWrite & queueMask].message_ = message;
			queue[queueWrite & queueMask].message = &(queue[queueWrite & queueMask].message_);

			queueWrite++;

			Trigger();

			writeLock.Leave();
			}

		STFRES_RAISE_OK;
		}
	else
		{
		DPR("!! OBJECT_FULL !! Failed to Put Message 0x%x\n", message.message);
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}
	}

STFResult WaitableQueuedSTFMessageProcessingDispatcher::ProcessMessages(void)
	{
	STFMessageSink	*	msgSink;
	STFMessage			msg;
	bool					messagesPresent = true;

	while (messagesPresent)
		{
		readLock.Enter();
		if (!abort && queueRead < queueWrite)
			{
			if (queue[queueRead & queueMask].sink)
				{
				msgSink = queue[queueRead & queueMask].sink;
				msg = STFMessage(*(queue[queueRead & queueMask].message));
				queue[queueRead & queueMask].sink = NULL;
				queueRead++;
				readLock.Leave();
				msgSink->ReceiveMessage(msg);
				}
			else
				{
				queueRead++;
				readLock.Leave();
				}
			}
		else
			{
			messagesPresent = false;
			readLock.Leave();
			}
		}

	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}

STFResult WaitableQueuedSTFMessageProcessingDispatcher::FlushMessages(void)
	{
	readLock.Enter();

	while (queueRead < queueWrite)
		{
		queue[queueRead & queueMask].message->Complete();
		queueRead++;
		}

	readLock.Leave();

	STFRES_RAISE_OK;
	}

STFResult WaitableQueuedSTFMessageProcessingDispatcher::FlushMessages(STFMessageSink * sink)
	{
	int i;

	readLock.Enter();

	i = queueRead;
	while (i < queueWrite)
		{
		if (queue[i & queueMask].sink == sink)
			{
			queue[i & queueMask].sink = NULL;
			queue[i & queueMask].message->Complete();
			if (i == queueRead)
				queueRead++;
			}
		i++;
		}

	readLock.Leave();

	STFRES_RAISE_OK;
	}

STFResult WaitableQueuedSTFMessageProcessingDispatcher::CheckQueueState(void)
	{
	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else if (queueWrite == queueRead)
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	else if (queueWrite == queueRead + queueSize)
		STFRES_RAISE(STFRES_OBJECT_FULL);
	else
		STFRES_RAISE_OK;
	}




void TriggeredWaitableQueuedSTFMessageProcessingDispatcher::Trigger(void)
	{
	trigger.SetSignal();
	}

TriggeredWaitableQueuedSTFMessageProcessingDispatcher::TriggeredWaitableQueuedSTFMessageProcessingDispatcher(int queueSize) // must be power of two
	: WaitableQueuedSTFMessageProcessingDispatcher(queueSize)
	{
	}

STFResult TriggeredWaitableQueuedSTFMessageProcessingDispatcher::WaitMessage(void)
	{
	while (!abort && queueRead == queueWrite)
		{
		trigger.Wait();
		}

	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else
		STFRES_RAISE_OK;
	}

#if 1 //DEBUG_PERFORMANCE
WaitablePriorityQueuedSTFMessageProcessingDispatcher::WaitablePriorityQueuedSTFMessageProcessingDispatcher(int queueSize, int prQueueSize) // must be power of two
																	  :WaitableQueuedSTFMessageProcessingDispatcher(queueSize) // must be power of two
	{
	this->priorityQueueSize = prQueueSize;
	this->priorityQueueMask = prQueueSize - 1;
	this->priorityQueueWrite = 0;
	this->priorityQueueRead = 0;
	this->priorityQueue = new STFMessageQueue[prQueueSize];
	}

WaitablePriorityQueuedSTFMessageProcessingDispatcher::~WaitablePriorityQueuedSTFMessageProcessingDispatcher()
   {
   delete[] this->priorityQueue;
   }

STFResult WaitablePriorityQueuedSTFMessageProcessingDispatcher::DispatchMessage(STFMessageSink * sink, STFMessage message, bool wait)
	{
	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else if(message.param2 == STFMSG_HIGH_PRIORITY)
		{
		if(priorityQueueWrite - priorityQueueRead < priorityQueueSize)
			{
#if DEBUG_PERFORMANCE
		switch (message.message)
			{
			case 0x4d030: // VDRMID_STRM_START_POSSIBLE
			case 0x4d031: // VDRMID_STRM_START_REQUIRED
			case 0x50022: //GVDI_FLUSH
			case 0x50011: //PSB_ABORT
				{
				int i = queueRead;
				DPS("1.msg in queue %d while posting prio msg %x\n\r", (queueWrite - queueRead), message.message);
				while (i < queueWrite)
					{
					DPS("Already in queue %x \n\r",queue[i & queueMask].message->message);
					i++;
					}
				break;
				}
			default:
				break;
			}
#endif

			writeLock.Enter();
			priorityQueue[priorityQueueWrite & priorityQueueMask].sink = sink;
			if (wait)
				{
				message.BeginWait();

				priorityQueue[priorityQueueWrite & priorityQueueMask].message = &message;

				priorityQueueWrite++;

				Trigger();

				writeLock.Leave();

				message.CompleteWait();
				}
			else
				{
				priorityQueue[priorityQueueWrite & priorityQueueMask].message_ = message;
				priorityQueue[priorityQueueWrite & priorityQueueMask].message = &(priorityQueue[priorityQueueWrite & priorityQueueMask].message_);

				priorityQueueWrite++;

				Trigger();

				writeLock.Leave();
				}

			STFRES_RAISE_OK;
			}
		else
			{
			DPR("!! OBJECT_FULL !! Failed to Put Message 0x%x in Priority Queue\n", message.message);
			STFRES_RAISE(STFRES_OBJECT_FULL);
			}
		}
	else 
		{
		STFRES_RAISE(WaitableQueuedSTFMessageProcessingDispatcher::DispatchMessage(sink, message, wait));
		}
	}

STFResult WaitablePriorityQueuedSTFMessageProcessingDispatcher::ProcessMessages(void)
	{
	STFMessageSink	*	msgSink;
	STFMessage			msg;
	bool					messagesPresent = true;

	while (messagesPresent)
		{
		readLock.Enter();
		if (!abort && priorityQueueRead < priorityQueueWrite)
			{
			if (priorityQueue[priorityQueueRead & priorityQueueMask].sink)
				{
				msgSink = priorityQueue[priorityQueueRead & priorityQueueMask].sink;
				msg = STFMessage(*(priorityQueue[priorityQueueRead & priorityQueueMask].message));
				priorityQueue[priorityQueueRead & priorityQueueMask].sink = NULL;
				priorityQueueRead++;
				readLock.Leave();
				msgSink->ReceiveMessage(msg);
				continue;
				}
			else
				{
				priorityQueueRead++;
				readLock.Leave();
				continue;
				}
			}

		if (!abort && queueRead < queueWrite)
			{
			if (queue[queueRead & queueMask].sink)
				{
				msgSink = queue[queueRead & queueMask].sink;
				msg = STFMessage(*(queue[queueRead & queueMask].message));
				queue[queueRead & queueMask].sink = NULL;
				queueRead++;
				readLock.Leave();
				msgSink->ReceiveMessage(msg);
				}
			else
				{
				queueRead++;
				readLock.Leave();
				}
			}
		else
			{
			messagesPresent = false;
			readLock.Leave();
			}
		}

	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}

STFResult WaitablePriorityQueuedSTFMessageProcessingDispatcher::FlushMessages(void)
	{
	readLock.Enter();

	while (priorityQueueRead < priorityQueueWrite)
		{
		priorityQueue[priorityQueueRead & priorityQueueMask].message->Complete();
		priorityQueueRead++;
		}

	while (queueRead < queueWrite)
		{
		queue[queueRead & queueMask].message->Complete();
		queueRead++;
		}
	readLock.Leave();

	STFRES_RAISE_OK;
	}

STFResult WaitablePriorityQueuedSTFMessageProcessingDispatcher::FlushMessages(STFMessageSink * sink)
	{
	int i;

	readLock.Enter();

	i = priorityQueueRead;
	while (i < priorityQueueWrite)
		{
		if (priorityQueue[i & priorityQueueMask].sink == sink)
			{
			priorityQueue[i & priorityQueueMask].sink = NULL;
			priorityQueue[i & priorityQueueMask].message->Complete();
			DPS("Flushed %x\n\r", priorityQueue[i & priorityQueueMask].message->message);
			if (i == priorityQueueRead)
				priorityQueueRead++;
			}
		i++;
		}

	i = queueRead;
	while (i < queueWrite)
		{
		if (queue[i & queueMask].sink == sink)
			{
			queue[i & queueMask].sink = NULL;
			queue[i & queueMask].message->Complete();
			if (i == queueRead)
				queueRead++;
			}
		i++;
		}

	readLock.Leave();

	STFRES_RAISE_OK;
	}

void TriggeredWaitablePriorityQueuedSTFMessageProcessingDispatcher::Trigger(void)
	{
	trigger.SetSignal();
	}

TriggeredWaitablePriorityQueuedSTFMessageProcessingDispatcher::TriggeredWaitablePriorityQueuedSTFMessageProcessingDispatcher(int queueSize, int prQueueSize) // must be power of two
	: WaitablePriorityQueuedSTFMessageProcessingDispatcher(queueSize, prQueueSize)
	{
	}

STFResult TriggeredWaitablePriorityQueuedSTFMessageProcessingDispatcher::WaitMessage(void)
	{
	while (!abort && (queueRead == queueWrite) && (priorityQueueRead == priorityQueueWrite))
		{
		trigger.Wait();
		}

	if (abort)
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
	else
		STFRES_RAISE_OK;
	}

#endif 



STFResult STFMessageProcessorThread::NotifyThreadTermination(void)
	{
	STFRES_RAISE(processor->AbortProcessor());
	}

void STFMessageProcessorThread::ThreadEntry(void)
	{
	while (!terminate)
		{
		processor->WaitMessage();
		processor->ProcessMessages();
		}
	}

STFMessageProcessorThread::STFMessageProcessorThread(STFString name, int stackSize, STFThreadPriority priority, TriggeredSTFMessageProcessor * processor_)
	: STFThread(name, stackSize, priority), processor(processor_)
	{
	StartThread();
	}

STFMessageProcessorThread::~STFMessageProcessorThread(void)
	{
	TerminateThread();
	}

STFResult STFMessageProcessorThread::FlushMessages(void)
	{
	STFRES_RAISE(processor->FlushMessages());
	}

STFResult STFMessageProcessorThread::FlushMessages(STFMessageSink * sink)
	{
	STFRES_RAISE(processor->FlushMessages(sink));
	}


STFResult STFMessageProcessorThread::CheckQueueState(void)
	{
	STFRES_RAISE(processor->CheckQueueState());
	}

ThreadedSTFMessageDispatcher::ThreadedSTFMessageDispatcher(STFString name, int stackSize, STFThreadPriority priority, TriggeredSTFMessageProcessingDispatcher * dispatcher_)
	: STFMessageProcessorThread(name, stackSize, priority, dispatcher_), dispatcher(dispatcher_)
	{
	}

ThreadedSTFMessageDispatcher::~ThreadedSTFMessageDispatcher(void)
	{
	}

STFResult ThreadedSTFMessageDispatcher::AbortDispatcher(void)
	{
	STFRES_RAISE(dispatcher->AbortDispatcher());
	}

STFResult ThreadedSTFMessageDispatcher::DispatchMessage(STFMessageSink * sink, STFMessage message, bool wait)
	{
	STFRES_RAISE(dispatcher->DispatchMessage(sink, message, wait));
	}
