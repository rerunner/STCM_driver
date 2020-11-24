///
/// @brief      Debugging streaming units
///

#include "StreamingDebug.h"
#include "ThreadConfiguration.h"
#include "STF/Interface/STFDebug.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/Tools/STFCRC.h"
#include "VDR/Source/Construction/IUnitConstruction.h"



////////////////////////////////////////////////////////////////////
//
// Logging streaming unit
//
////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdarg.h>

UNIT_CREATION_FUNCTION(CreateLoggingStreamingUnit, PhysicalLoggingStreamingUnit)

STFResult PhysicalLoggingStreamingUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualLoggingStreamingUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}

STFResult PhysicalLoggingStreamingUnit::Create(uint64 * createParams)
	{
	if (createParams[0] != PARAMS_STRING || createParams[2] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	strncpy(name, (char *)(createParams[1]), sizeof(name));

	STFRES_RAISE_OK;
	}

STFResult PhysicalLoggingStreamingUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_RAISE(STFRES_RANGE_VIOLATION);
	}

STFResult PhysicalLoggingStreamingUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


VirtualLoggingStreamingUnit::VirtualLoggingStreamingUnit(PhysicalLoggingStreamingUnit * physicalUnit)
	: VirtualStreamingUnit(physicalUnit), inputConnector(0, this), outputConnector(0, 1, this)
	{
	this->physicalUnit = physicalUnit;

	this->AddConnector(&inputConnector);
	this->AddConnector(&outputConnector);

	insideSegment = false;
	insideGroup = false;
	}

STFResult VirtualLoggingStreamingUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	//
	// We are not queueing, so nothing we could actually do...
	//
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}


STFResult VirtualLoggingStreamingUnit::PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState)
	{
	switch (command)
		{
		case VDR_STRMCMD_NONE:
			DPR("%s-PRPSC: NONE\n", physicalUnit->name);
			break;
		case VDR_STRMCMD_BEGIN:
			DPR("%s PRPSC: BEGIN(%d)\n", physicalUnit->name, param);
			break;
		case VDR_STRMCMD_DO:
			DPR("%s PRPSC: DO(%08x)\n", physicalUnit->name, param);
			break;
		case VDR_STRMCMD_STEP:	
			DPR("%s PRPSC: STEP(%d)\n", physicalUnit->name, param);
			break;
		case VDR_STRMCMD_FLUSH:
			DPR("%s PRPSC: FLUSH\n", physicalUnit->name);
			break;
		}

	STFRES_RAISE(VirtualStreamingUnit::PrepareStreamingCommand(command, param, targetState));
	}

STFResult VirtualLoggingStreamingUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	switch (command)
		{
		case VDR_STRMCMD_NONE:
			DPR("%s BGNSC: NONE\n", physicalUnit->name);
			break;
		case VDR_STRMCMD_BEGIN:
			DPR("%s BGNSC: BEGIN(%d)\n", physicalUnit->name, param);
			break;
		case VDR_STRMCMD_DO:
			DPR("%s BGNSC: DO(%08x)\n", physicalUnit->name, param);
			break;
		case VDR_STRMCMD_STEP:	
			DPR("%s BGNSC: STEP(%d)\n", physicalUnit->name, param);
			break;
		case VDR_STRMCMD_FLUSH:
			DPR("%s BGNSC: FLUSH\n", physicalUnit->name);
			insideSegment = false;
			insideGroup = false;
			break;
		}

	STFRES_RAISE(VirtualStreamingUnit::BeginStreamingCommand(command, param));
	}

STFResult VirtualLoggingStreamingUnit::CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{
 	switch (command)
		{
		case VDR_STRMCMD_NONE:
			DPR("%s CPLSC: NONE\n", physicalUnit->name);
			break;
		case VDR_STRMCMD_BEGIN:
			DPR("%s CPLSC: BEGIN\n", physicalUnit->name);
			break;
		case VDR_STRMCMD_DO:
			DPR("%s CPLSC: DO\n", physicalUnit->name);
			break;
		case VDR_STRMCMD_STEP:	
			DPR("%s CPLSC: STEP\n", physicalUnit->name);
			break;
		case VDR_STRMCMD_FLUSH:
			DPR("%s CPLSC: FLUSH\n", physicalUnit->name);
			break;
		}


	STFRES_REASSERT(VirtualStreamingUnit::CompleteStreamingCommand(command, targetState));	

	STFRES_RAISE_OK;
	}

STFResult VirtualLoggingStreamingUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	STFResult	result;
	STFHiPrec64BitTime	startTime, stopTime;
	int			i;
	uint32		size, flags, numRanges;

	flags = packet->vdrPacket.flags;

	DPR("%s pack+:", physicalUnit->name);

	if (flags & VDR_MSMF_SEGMENT_START)
		{
		if (flags & VDR_MSCF_SEGMENT_START_NOTIFICATION)
			DPR(" SSN(%04x)", packet->vdrPacket.segmentNumber);
		else
			DPR(" SS(%04x)", packet->vdrPacket.segmentNumber);
		}

	if (flags & VDR_MSMF_DATA_DISCONTINUITY)
		DPR(" DD");

	if (flags & VDR_MSMF_GROUP_START)
		{
		if (flags & VDR_MSCF_GROUP_START_NOTIFICATION)
			DPR(" GSN(%04x)", packet->vdrPacket.groupNumber);
		else
			DPR(" GS(%04x)", packet->vdrPacket.groupNumber);
		}

	if (flags & VDR_MSMF_TAGS_VALID)
		DPR(" TAGS(%d)", packet->vdrPacket.numTags);

	if (flags & VDR_MSMF_START_TIME_VALID)
		DPR(" ST(%d)", packet->vdrPacket.startTime.Get32BitTime());

	if (packet->vdrPacket.numRanges)
		{
		size = 0;
		for(i=0; i<packet->vdrPacket.numRanges; i++)
			size += packet->vdrPacket.tagRanges.ranges[packet->vdrPacket.numTags + i].size;
		DPR(" FS(0x%x) ", packet->vdrPacket.frameStartFlags);
		DPR(" rng(%d, %08x)", packet->vdrPacket.numRanges, size);
		}
	numRanges = packet->vdrPacket.numRanges;

	if (flags & VDR_MSMF_END_TIME_VALID)
		DPR(" ET(%d)", packet->vdrPacket.endTime.Get32BitTime());

	if (flags & VDR_MSMF_GROUP_END)
		{
		if (flags & VDR_MSCF_GROUP_END_NOTIFICATION)
			DPR(" GEN(%04x)", packet->vdrPacket.groupNumber);
		else
			DPR(" GE(%04x)", packet->vdrPacket.groupNumber);
		}

	if (flags & VDR_MSMF_TIME_DISCONTINUITY)
		DPR(" TD");

	if (flags & VDR_MSMF_SEGMENT_END)
		{
		if (flags & VDR_MSCF_SEGMENT_END_NOTIFICATION)
			DPR(" SEN(%04x)", packet->vdrPacket.segmentNumber);
		else
			DPR(" SE(%04x)", packet->vdrPacket.segmentNumber);
		}

	DPR("\n");

	SystemTimer->GetTime(startTime);
	result = outputConnector.SendPacket(packet);
	SystemTimer->GetTime(stopTime);

	switch (result)
		{
		case STFRES_OK:
			DPR("%s pack-: ok %dms\n", physicalUnit->name, (stopTime - startTime).Get32BitDuration());
			if (flags & VDR_MSMF_SEGMENT_START)
				{
				if (insideSegment)
					DPR("%s *** FORMAT ERROR *** Multiple Segment Starts\n", physicalUnit->name);
				insideSegment = true;
				}
			if (flags & VDR_MSMF_GROUP_START)
				{
				if (!insideSegment)
					DPR("%s *** FORMAT ERROR *** Group Start outside Segment\n", physicalUnit->name);
				if (insideGroup)
					DPR("%s *** FORMAT ERROR *** Multiple Group Starts\n", physicalUnit->name);
				insideGroup = true;
				}

			if (numRanges > 0 && !insideGroup)
					DPR("%s *** FORMAT ERROR *** Data ranges outside of Group\n", physicalUnit->name);

			if (flags & VDR_MSMF_GROUP_END)
				{
				if (!insideSegment)
					DPR("%s *** FORMAT ERROR *** Group End outside Segment\n", physicalUnit->name);
				if (!insideGroup)
					DPR("%s *** FORMAT ERROR *** Group End without Start\n", physicalUnit->name);
				insideGroup = false;
				}
			if (flags & VDR_MSMF_SEGMENT_END)
				{
				if (!insideSegment)
					DPR("%s *** FORMAT ERROR *** Segment End without Start\n", physicalUnit->name);
				insideSegment = false;
				}
			break;
		case STFRES_OBJECT_FULL:
			DPR("%s pack-: OBJECT_FULL\n", physicalUnit->name);
			break;
		case STFRES_ILLEGAL_STREAMING_STATE:
			DPR("%s pack-: ILLEGAL_STREAMING_STATE\n", physicalUnit->name);
			break;
		default:
			DPR("%s pack-: ERROR(%08x)\n", physicalUnit->name, result);
		}

	STFRES_RAISE(result);
	}

STFResult VirtualLoggingStreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	STFHiPrec64BitTime	systemTime;

	switch (message)
		{
		case VDRMID_STRM_PACKET_REQUEST:
			DPR("%s UPSNT: PACKET_REQUEST\n", physicalUnit->name);
			break;

		case VDRMID_STRM_PACKET_ARRIVAL:
			DPR("%s UPSNT: PACKET_ARRIVAL\n", physicalUnit->name);
			break;

		case VDRMID_STRM_STARVING:
			DPR("%s UPSNT: STARVING\n", physicalUnit->name);
			break;

		case VDRMID_STRM_COMMAND_COMPLETED:
			DPR("%s UPSNT: COMMAND_COMPLETED\n", physicalUnit->name);
			break;

		case VDRMID_STRM_SEGMENT_START:
			DPR("%s UPSNT: SEGMENT_START(%04x)\n", physicalUnit->name, param1);
			break;

		case VDRMID_STRM_SEGMENT_START_TIME:
			SystemTimer->GetTime(systemTime);
			DPR("%s UPSNT: SEGMENT_START_TIME(%d SYS %d)\n", physicalUnit->name, (STFInt64(param1, param2) / 108000).ToUint32(), systemTime.Get32BitTime());
			break;

		case VDRMID_STRM_SEGMENT_END:
			DPR("%s UPSNT: SEGMENT_END(%04x)\n", physicalUnit->name, param1);
			break;

		case VDRMID_STRM_GROUP_START:
			DPR("%s UPSNT: GROUP_START(%04x, %d)\n", physicalUnit->name, param1, param2);
			break;

		case VDRMID_STRM_GROUP_END:
			DPR("%s UPSNT: GROUP_END(%04x, %d)\n", physicalUnit->name, param1, param2);
			break;

		case VDRMID_STRM_START_POSSIBLE:
			DPR("%s UPSNT: START_POSSIBLE\n", physicalUnit->name);
			break;

		case VDRMID_STRM_START_REQUIRED:
			DPR("%s UPSNT: START_REQUIRED\n", physicalUnit->name);
			break;
		}

	STFRES_RAISE(inputConnector.SendUpstreamNotification(message, param1, param2));
	}

STFResult VirtualLoggingStreamingUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(inputConnector.ProvideAllocator(allocator));
	}

STFResult VirtualLoggingStreamingUnit::GetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	STFRES_RAISE(outputConnector.GetStreamTagIDs(ids));
	}

#if _DEBUG
STFString VirtualLoggingStreamingUnit::GetInformation(void)
	{
	return STFString("LoggingStreamingUnit ") + STFString(physicalUnit->name) + STFString(physical->GetUnitID(), 8, 16);
	}
#endif


UNIT_CREATION_FUNCTION(CreateDumpingStreamingUnit, PhysicalDumpingStreamingUnit)

STFResult PhysicalDumpingStreamingUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualDumpingStreamingUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}

STFResult PhysicalDumpingStreamingUnit::Create(uint64 * createParams)
	{
	if (createParams[0] != PARAMS_STRING || createParams[2] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	strncpy(name, (char *)(createParams[1]), sizeof(name));

	STFRES_RAISE_OK;
	}

STFResult PhysicalDumpingStreamingUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_RAISE(STFRES_RANGE_VIOLATION);
	}

STFResult PhysicalDumpingStreamingUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


VirtualDumpingStreamingUnit::VirtualDumpingStreamingUnit(PhysicalDumpingStreamingUnit * physicalUnit)
	: VirtualStreamingUnit(physicalUnit)
	{
	//lint --e{613}
	this->physicalUnit = physicalUnit;

	this->inputConnectors = new LoggingInputConnector*[physicalUnit->numInputs];
	ASSERT(this->inputConnectors);

	this->outputConnectors = new StreamingOutputConnector*[physicalUnit->numOutputs];
	ASSERT(this->outputConnectors);
	
	uint32 i;
	for (i = 0; i < physicalUnit->numInputs; i++)
		{
		this->inputConnectors[i] = new LoggingInputConnector(i, this);
		ASSERT(this->inputConnectors[i]);
		this->AddConnector(this->inputConnectors[i]);
		}
	for (i = 0; i < physicalUnit->numOutputs; i++)
		{
		this->outputConnectors[i] = new StreamingOutputConnector(0, i + physicalUnit->numInputs, this);
		ASSERT(this->outputConnectors[i]);
		this->AddConnector(this->outputConnectors[i]);
		}

	segmentNumber = 0xffffffff;
	file = NULL;
	mfile = NULL;
	}

VirtualDumpingStreamingUnit::~VirtualDumpingStreamingUnit()
	{
	uint32 i;
	//lint --e{613}
	for (i = 0; i < physicalUnit->numInputs; i++)
		{
		delete[] this->inputConnectors[i];
		}
	for (i = 0; i < physicalUnit->numOutputs; i++)
		{
		delete[] this->outputConnectors[i];
		}
	}

STFResult VirtualDumpingStreamingUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	//
	// We are not queueing, so nothing we could actually do...
	//
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}

STFResult VirtualDumpingStreamingUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	STFResult				result;	
	char						fname[300];
	VDRDataRange			ranges[VDR_MAX_TAG_DATA_RANGES_PER_PACKET];
	TAGITEM					tags[VDR_MAX_TAG_DATA_RANGES_PER_PACKET];
	uint32					numRanges, numTags, i, flags, frames, groupNumber;
	STFHiPrec64BitTime	startTime, endTime;
	//lint --e{613}
	flags = packet->vdrPacket.flags;
	if (flags & VDR_MSMF_SEGMENT_START)
		{
		segmentNumber = packet->vdrPacket.segmentNumber;
		segmentSize = 0;
		}
	if (segmentNumber == 0xffffffff)
		{
		segmentNumber = 0;
		segmentSize = 0;
		flags |= VDR_MSMF_SEGMENT_START;
		}

	groupNumber = packet->vdrPacket.groupNumber;
	startTime = packet->vdrPacket.startTime;
	endTime = packet->vdrPacket.endTime;

	numTags = packet->vdrPacket.numTags;
	for(i=0; i<numTags; i++)
		tags[i] = packet->vdrPacket.tagRanges.tags[i];

	numRanges = packet->vdrPacket.numRanges;
	for(i=0; i<numRanges; i++)
		{
		ranges[i] = packet->vdrPacket.tagRanges.ranges[packet->vdrPacket.numTags + i];
		ranges[i].AddRef();
		}
	frames = packet->vdrPacket.frameStartFlags;

	if (physicalUnit->numOutputs > 0)
		{
		result = outputConnectors[connectorID]->SendPacket(packet);
		}
	else
		{
		result = STFRES_OK;
		}
	if (STFRES_SUCCEEDED(result))
		{
		//lint -e{668} suppress "possibly passing a null pointer"		
		if (flags & VDR_MSMF_SEGMENT_START)
			{
			if (file)
				fclose(file);

			if (mfile)
				fclose(file);

			sprintf(fname, "%s%03d.dmp", physicalUnit->name, segmentNumber);
			file = fopen(fname, "wb");			

			sprintf(fname, "%s%03d.mdm", physicalUnit->name, segmentNumber);
		
			mfile = fopen(fname, "wb");
			}

		if (flags & VDR_MSMF_SEGMENT_START)
			fprintf(mfile, "%02x %08x : SEGMENT_START %d\n", connectorID, segmentSize, segmentNumber);
		if (flags & VDR_MSMF_DATA_DISCONTINUITY)
			fprintf(mfile, "%02x %08x : DATA_DISCONTINUITY\n", connectorID, segmentSize);
		if (flags & VDR_MSMF_GROUP_START)
			fprintf(mfile, "%02x %08x : GROUP_START %d\n", connectorID, segmentSize, groupNumber);
		if (flags & VDR_MSMF_START_TIME_VALID)
			fprintf(mfile, "%02x %08x : START_TIME %d\n", connectorID, segmentSize, startTime.Get32BitTime());

		for(i=0; i<numTags; i++)
			{
			fprintf(mfile, "%02x %08x : TAG %08x %08x %08x\n", connectorID, segmentSize, tags[i].id, tags[i].data, tags[i].data2);
			}

		for(i=0; i<numRanges; i++)
			{
			if (frames & (1 << i))
				fprintf(mfile, "%02x %08x : FRAME_START\n", connectorID, segmentSize);
			
			uint16 crc;
			STFCRC::CalculateCRC(ranges[i].GetStart(), ranges[i].size, 0, crc);		

			fprintf(mfile, "%02x %08x : RANGE %d %d\n", connectorID, segmentSize, ranges[i].size, crc);
			fwrite(ranges[i].GetStart(), ranges[i].size, 1, file);			
			
			segmentSize += ranges[i].size;
			}

		if (flags & VDR_MSMF_END_TIME_VALID)
			fprintf(mfile, "%02x %08x : END_TIME %d\n", connectorID, segmentSize, endTime.Get32BitTime());
		if (flags & VDR_MSMF_TIME_DISCONTINUITY)
			fprintf(mfile, "%02x %08x : TIME_DISCONTINUITY\n", connectorID, segmentSize);
		if (flags & VDR_MSMF_GROUP_END)
			fprintf(mfile, "%02x %08x : GROUP_END %d\n", connectorID, segmentSize, groupNumber);
		if (flags & VDR_MSMF_SEGMENT_END)
			fprintf(mfile, "%02x %08x : SEGMENT_END %d\n", connectorID, segmentSize, segmentNumber);

		fflush(mfile);
		fflush(file);
		}

	for(i=0; i<numRanges; i++)
		ranges[i].Release();

	STFRES_RAISE(result);
	}

STFResult VirtualDumpingStreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	//lint --e{613}
	STFRES_RAISE(inputConnectors[connectorID - physicalUnit->numInputs]->SendUpstreamNotification(message, param1, param2));
	}

STFResult VirtualDumpingStreamingUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	//lint --e{613}
	STFRES_RAISE(inputConnectors[connectorID - physicalUnit->numInputs]->ProvideAllocator(allocator));
	}

STFResult VirtualDumpingStreamingUnit::GetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	//lint --e{613}
	if (physicalUnit->numOutputs > 0)
		{
		STFRES_RAISE(outputConnectors[connectorID]->GetStreamTagIDs(ids));
		}
	else
		{
		STFRES_RAISE_OK;
		}
	}

#if _DEBUG
STFString VirtualDumpingStreamingUnit::GetInformation(void)
	{
	// By default, we do not know anything about ourself!
	return STFString("DumpingStreamingUnit ") + STFString(physicalUnit->name) + STFString(physical->GetUnitID(), 8, 16);
	}
#endif


UNIT_CREATION_FUNCTION(CreateMultichannelDumpingStreamingUnit, PhysicalMultichannelDumpingStramingUnit)

STFResult PhysicalMultichannelDumpingStramingUnit::Create(uint64 * createParams)
	{
	if (createParams[0] != PARAMS_STRING || 
		 createParams[2] != PARAMS_DWORD || 
		 createParams[4] != PARAMS_DWORD ||
		 createParams[6] != PARAMS_DONE) 		
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	strncpy(name, (char *)(createParams[1]), sizeof(name));

	STFRES_REASSERT(GetDWordParameter(createParams, 1, this->numInputs));
	STFRES_REASSERT(GetDWordParameter(createParams, 2, this->numOutputs));

	STFRES_RAISE_OK;
	}

STFResult PhysicalMultichannelDumpingStramingUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualMultichannelDumpingStreamingUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}

STFResult VirtualMultichannelDumpingStreamingUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	STFAutoMutex lock(&classLock);

	STFRES_RAISE(VirtualDumpingStreamingUnit::ReceivePacket(connectorID, packet));
	}





UNIT_CREATION_FUNCTION(CreateDumpFedStreamingUnit, PhysicalDumpFedStreamingUnit)

STFResult PhysicalDumpFedStreamingUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualDumpFedStreamingUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}
	
STFResult PhysicalDumpFedStreamingUnit::Create(uint64 * createParams)
	{
	if (createParams[0] != PARAMS_STRING || 
		 createParams[2] != PARAMS_STRING || 
		 createParams[4] != PARAMS_DWORD)		 
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	strncpy(metafileName, (char *)(createParams[1]), sizeof(metafileName));
	strncpy(datafileName, (char *)(createParams[3]), sizeof(datafileName));
	
	STFRES_REASSERT(GetDWordParameter(createParams, 2, this->numOutputs));

	// two optional parameters are allowes
	if (STFRES_FAILED(GetDWordParameter(createParams, 3, this->blockSize)))
		this->blockSize = 0;

	if (STFRES_FAILED(GetDWordParameter(createParams, 4, this->blockNum)))
		this->blockNum = 0;

	STFRES_RAISE_OK;
	}

STFResult PhysicalDumpFedStreamingUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	if (localID == 0)
		{
		// we have an allocator
		physicalAllocator = source;
		}
	else
		{
		STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}

STFResult PhysicalDumpFedStreamingUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}



VirtualDumpFedStreamingUnit::VirtualDumpFedStreamingUnit(PhysicalDumpFedStreamingUnit * physical) : 
	VirtualStreamingUnitCollection(physical, 1), 
	STFThread(TCTN_STREAM_DUMP_UNIT, TCSS_STREAM_DUMP_UNIT, TCTP_STREAM_DUMP_UNIT),
	allocator(this, 0)
	{
	//lint --e{613}
	this->physicalFeedUnit = physical;

	outputBlock = NULL;
	actualBlockSize = 0;
	nextBlockStart = 0;
	currentPosition = 0;
	
	this->outputConnectors = new StreamingOutputConnector*[physicalFeedUnit->numOutputs];
	ASSERT(this->outputConnectors);

	this->outputFormatters = new OutputConnectorStreamingFormatter*[physicalFeedUnit->numOutputs];
	ASSERT(this->outputFormatters);
	
	uint32 i;
	for (i = 0; i < physicalFeedUnit->numOutputs; i++)
		{
		this->outputConnectors[i] = new StreamingOutputConnector(16, i, this);
		ASSERT(this->outputConnectors[i]);
		this->AddConnector(this->outputConnectors[i]);

		this->outputFormatters[i] = new OutputConnectorStreamingFormatter();
		ASSERT(this->outputFormatters[i]);
		this->outputFormatters[i]->SetOutputConnector(this->outputConnectors[i]);
		}
	}


STFResult VirtualDumpFedStreamingUnit::AllocateChildUnits(void)
	{
	IVirtualUnit * tempVU;

	if (physicalFeedUnit->physicalAllocator)
		{
		// Create PCM Memory Pool Allocator
		STFRES_REASSERT(physicalFeedUnit->physicalAllocator->CreateVirtual(tempVU, this, root ? root : this));
		STFRES_REASSERT(VirtualUnitCollection::AddLeafUnit(tempVU));	// Add to virtual unit collection

		// Get interface to memory pool allocator
		IVDRMemoryPoolAllocator * theAllocator;
		STFRES_REASSERT(VirtualUnitCollection::leafUnits[0]->QueryInterface(VDRIID_VDR_MEMORYPOOL_ALLOCATOR, (void*&) theAllocator));
		STFRES_REASSERT(this->allocator.SetAllocator(theAllocator));

		IVDRTagUnit * tagUnit;		
		STFRES_REASSERT(VirtualUnitCollection::leafUnits[0]->QueryInterface(VDRIID_VDR_TAG_UNIT, (void*&) tagUnit));
		STFResult res = tagUnit->Configure(SET_MEMPOOL_TOTALSIZE(physicalFeedUnit->blockSize * physicalFeedUnit->blockNum) +
								 SET_MEMPOOL_BLOCKSIZE(physicalFeedUnit->blockSize) +
								 SET_MEMPOOL_ALIGNMENT_FACTOR(1) +
								 SET_MEMPOOL_POOLNAME("Dump Fed Unit") +
								 TAGDONE);

		STFRES_REASSERT(res);

		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDumpFedStreamingUnit::OpenFiles()
	{	
	metaFile = fopen(physicalFeedUnit->metafileName, "rb");
	if (!metaFile)
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);

	fseek(metaFile, 0, SEEK_END);
	metaFileLength = ftell(metaFile);
	nextBlockStart = 0;
		
	dataFile = fopen(physicalFeedUnit->datafileName, "rb");
	if (!dataFile)
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);

	STFRES_RAISE_OK;
	}


STFResult VirtualDumpFedStreamingUnit::CloseFiles()
	{
	//lint --e{668} suppress"possibly passing a null pointer"	
	ASSERT(dataFile);
	ASSERT(metaFile);

	fclose(dataFile);
	fclose(metaFile);

	STFRES_RAISE_OK;
	}


STFResult VirtualDumpFedStreamingUnit::ReadMetaFileBlock()
	{
	fseek(metaFile, nextBlockStart, SEEK_SET);
	//lint --e{661}
	uint32 sizeToRead = min(metaFileLength, MAX_BLOCK_SIZE);
	if (sizeToRead == 0)
		STFRES_RAISE(STFRES_END_OF_FILE);
		
	fread(&metaFileBlock, MAX_BLOCK_SIZE, 1, metaFile);

	// from backwards, search for the last buffer end
	actualBlockSize = sizeToRead;
	while (metaFileBlock[actualBlockSize] != 0x0a)
		actualBlockSize--;
	actualBlockSize++;

	nextBlockStart += actualBlockSize;
	if (metaFileLength > actualBlockSize)
		metaFileLength -= actualBlockSize;
	else
		metaFileLength = 0;
	currentPosition = 0;

	STFRES_RAISE_OK;
	}


inline uint32 VirtualDumpFedStreamingUnit::GetDecimal()
	{
	uint32 number = 0;
	while ( (metaFileBlock[currentPosition] >= '0') && (metaFileBlock[currentPosition] <= '9'))
		{
		number *= 10;
		number += (metaFileBlock[currentPosition++] - '0');
		}
	return number;
	}

inline uint32 VirtualDumpFedStreamingUnit::GetHexadecimal()
	{
	uint32 number = 0;
	while ( 1 )
		{
		number <<= 4;
		if ((metaFileBlock[currentPosition] >= '0') && (metaFileBlock[currentPosition] <= '9'))			
			number += (metaFileBlock[currentPosition++] - '0');

		else if ((metaFileBlock[currentPosition] >= 'a') && (metaFileBlock[currentPosition] <= 'f'))			
			number += (metaFileBlock[currentPosition++] - 'a' + 10);

		else if ((metaFileBlock[currentPosition] >= 'A') && (metaFileBlock[currentPosition] <= 'F'))			
			number += (metaFileBlock[currentPosition++] - 'A' + 10);

		else return number;
		}
	}


STFResult VirtualDumpFedStreamingUnit::ParseMetaFileToken(FeedAction & actionToTake)
	{
	if (currentPosition >= actualBlockSize)
		STFRES_REASSERT(ReadMetaFileBlock());	
	
	connectorId = GetHexadecimal();	
	ASSERT(connectorId < physicalFeedUnit->numOutputs);

	// skip the space
	currentPosition++;

	GetHexadecimal();	// we do not need this value

	// skip the space, the ':' and the second space
	currentPosition += 3;

	char tokenName[300];
	uint32 tokenNamePos = 0;
	while ( (metaFileBlock[currentPosition] != ' ') && (metaFileBlock[currentPosition] != 0x0a) )
		{
		tokenName[tokenNamePos++] = metaFileBlock[currentPosition];
		currentPosition++;
		}

	tokenName[tokenNamePos] = 0;

	if (strcmp(tokenName, "SEGMENT_START") == 0)
		{
		// skip the space
		currentPosition++;

		// get the segment number
		segmentNumber = GetDecimal();		

		actionToTake = ACTION_BEGIN_SEGMENT;		
		}
	
	else if (strcmp(tokenName, "DATA_DISCONTINUITY") == 0)
		{
		actionToTake = ACTION_DATA_DISCONTINUITY;
		// outputFormatters[connectorId].PutDataDiscontinuity();
		}
	
	else if (strcmp(tokenName, "GROUP_START") == 0)
		{
		// skip the space
		currentPosition++;

		// get the group number
		groupNumber = GetDecimal();				
		actionToTake = ACTTION_BEGIN_GROUP;		
		}
	
	else if (strcmp(tokenName, "START_TIME") == 0)
		{
		// skip the space
		currentPosition++;

		// get the start time 
		startTime = GetDecimal();
		actionToTake = ACTION_PUT_START_TIME;
		}
	else if (strcmp(tokenName, "TAG") == 0)
		{
		// skip the space
		currentPosition++;
		tagId = GetHexadecimal();

		// skip the space
		currentPosition++;

		tagFirstParam = GetHexadecimal();
		
		// skip the space
		currentPosition++;

		tagNextParam = GetHexadecimal();

		actionToTake = ACTION_PUT_TAG;
		}
	else if (strcmp(tokenName, "FRAME_START") == 0)
		{
		actionToTake = ACTION_PUT_FRAME_START;
		}
	else if (strcmp(tokenName, "RANGE") == 0)
		{
		// skip the space
		currentPosition++;

		rangeSize = GetDecimal();

		// skip the space
		currentPosition++;

		GetDecimal();	// we are not interested in the checksum
		actionToTake = ACTION_PUT_RANGE;
		}
	else if (strcmp(tokenName, "END_TIME") == 0)
		{
		// skip the space
		currentPosition++;

		// get the end time 
		endTime = GetDecimal();
		actionToTake = ACTION_PUT_END_TIME;
		}

	else if (strcmp(tokenName, "TIME_DISCONTINUITY") == 0)
		{
		actionToTake = ACTION_TIME_DISCONTINUITY;
		}

	else if (strcmp(tokenName, "GROUP_END") == 0)
		{
		// skip the space
		currentPosition++;

		// get the group number
		groupNumber = GetDecimal();		

		actionToTake = ACTION_COMPLETE_GROUP;		
		}
	else if (strcmp(tokenName, "SEGMENT_END") == 0)
		{
		// skip the space
		currentPosition++;

		// get the group number
		segmentNumber = GetDecimal();		

		actionToTake = ACTION_COMPLETE_SEGMENT;
		}

	// skip till the next line
	while (metaFileBlock[currentPosition++] != 0x0a) ;	

	STFRES_RAISE_OK;
	}


STFResult VirtualDumpFedStreamingUnit::PutRangeFromDateFile(uint32 connectorId, uint32 rangesSize)
	{
	//lint --e{613}
	if (!outputBlock)
		{
		uint32 numBlocks;
		STFResult result = allocator.GetMemoryBlocks(&outputBlock, 1, numBlocks);
		if (numBlocks == 0 || result == STFRES_NOT_ENOUGH_MEMORY)
			{	
			STFRES_RAISE(STFRES_OBJECT_FULL);
			}
		
		currentRange.Init(outputBlock, 0, rangeSize);

		ASSERT(rangeSize <= physicalFeedUnit->blockSize);

		fread(currentRange.GetStart(), rangeSize, 1, dataFile);
		}	
	
	STFRES_REASSERT(outputFormatters[connectorId]->PutRange(currentRange));	

	outputBlock->Release();
	outputBlock = NULL;

	STFRES_RAISE_OK;
	}

STFResult VirtualDumpFedStreamingUnit::ReceiveMessage(STFMessage & message)
	{
	SetThreadSignal();

	STFRES_RAISE_OK;
	}

STFResult VirtualDumpFedStreamingUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(this->allocator.SetAllocator(allocator));
	}

STFResult VirtualDumpFedStreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	SetThreadSignal();

	STFRES_RAISE_OK;
	}

STFResult VirtualDumpFedStreamingUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	STFResult result = STFRES_OK;
	//lint --e{613}
	VirtualStreamingUnitCollection::BeginStreamingCommand(command, param);

	switch (command)
		{
		case VDR_STRMCMD_NONE:
			break;
		case VDR_STRMCMD_BEGIN:
			STFRES_REASSERT(OpenFiles());
			StartThread();					
			break;
		case VDR_STRMCMD_DO:		
			break;
		case VDR_STRMCMD_STEP:			
			break;
		case VDR_STRMCMD_FLUSH:
			StopThread();		
			CloseFiles();
			break;
		};

	this->SignalStreamingCommandCompletion(command, result);

	STFRES_RAISE_OK;
	}


STFResult VirtualDumpFedStreamingUnit::PerformOperation(FeedAction action)
	{
	//lint --e{613}
	switch (action)
		{		
		case ACTION_BEGIN_SEGMENT:
			STFRES_REASSERT(outputFormatters[connectorId]->BeginSegment(segmentNumber, false));
			break;
		case ACTION_DATA_DISCONTINUITY:
			STFRES_REASSERT(outputFormatters[connectorId]->PutDataDiscontinuity());
			break;
		case ACTTION_BEGIN_GROUP:
			STFRES_REASSERT(outputFormatters[connectorId]->BeginGroup(groupNumber, false, false));
			break;
		case ACTION_PUT_START_TIME:
			STFRES_REASSERT(outputFormatters[connectorId]->PutStartTime(STFHiPrec64BitTime(startTime, STFTU_MILLISECS)));
			break;
		case ACTION_PUT_TAG:
			{
			TAGITEM tag;
			tag.id = tagId;
			tag.data = tagFirstParam;
			tag.data2 = tagFirstParam;
			STFRES_REASSERT(outputFormatters[connectorId]->PutTag(tag));
			}
			break;
		case ACTION_PUT_FRAME_START:
			STFRES_REASSERT(outputFormatters[connectorId]->PutFrameStart());
			break;
		case ACTION_PUT_RANGE:
			STFRES_REASSERT(PutRangeFromDateFile(connectorId, rangeSize));
			break;
		case ACTION_PUT_END_TIME:
			STFRES_REASSERT(outputFormatters[connectorId]->PutEndTime(STFHiPrec64BitTime(endTime, STFTU_MILLISECS)));
			break;
		case ACTION_TIME_DISCONTINUITY:
			STFRES_REASSERT(outputFormatters[connectorId]->PutTimeDiscontinuity());
			break;
		case ACTION_COMPLETE_GROUP:
			STFRES_REASSERT(outputFormatters[connectorId]->CompleteGroup(false));
			break;			
		case ACTION_COMPLETE_SEGMENT:
			STFRES_REASSERT(outputFormatters[connectorId]->CompleteSegment(false));
			break;
		}

	STFRES_RAISE_OK;
	}

void VirtualDumpFedStreamingUnit::ThreadEntry()
	{
	while (!terminate)
		{
		if (STFRES_FAILED(ParseMetaFileToken(actionToTake)))
			return;
		
		STFResult result = STFRES_OK;
		do 
			{
			result = PerformOperation(actionToTake);

			if (STFRES_FAILED(result))
				{				
				WaitThreadSignal();
				}
			} while (result == STFRES_OBJECT_FULL);
		}

	}
