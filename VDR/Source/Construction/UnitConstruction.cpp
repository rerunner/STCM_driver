//
// PURPOSE:		Generic Unit Construction Implementation
//

#include "UnitConstruction.h"

#include "VDR/Interface/Unit/Board/IVDRBoard.h"
#include "VDR/Source/Construction/IUnitConstruction.h"
#include "STF/Interface/STFDebug.h"


#include <stdio.h>
#include <string.h>


#if _DEBUG_VERBOSE_CREATE_UNIT 
// _DEBUG_VERBOSE_CREATE_UNIT gives access to the strings via DebugUnitNameFromID()
// this is useful during runtime, therefore this is 1 for all _DEBUG builds.
// _DEBUG_VERBOSE_UNIT_CREATION_OUTPUT shows the detailed startup information below,
// and its scope is local to this file; disabling does not interfere with DebugUnitNameFromID().
#define _DEBUG_VERBOSE_UNIT_CREATION_OUTPUT		0
#define _DEBUG_WRITE_CONSTRUCTION_SUB_TREE		0

#endif

#if _DEBUG_VERBOSE_UNIT_CREATION_OUTPUT
#define DPV DP
#else
#define DPV while(0) DebugPrintEmpty
#endif




//
// char *DebugUnitNameFromID(uint32 unitID) can be used from anywhere in the host software
// as long as it is within _DEBUG.  It will return "" unless _DEBUG_VERBOSE_CREATE_UNIT 
// is defined in VDR/Source/Construction/IUnitConstruction.h
// 
// Code example:
// #if _DEBUG
// extern char * DebugUnitNameFromID(uint32 unitID);
// 	DP("AudioFrameStreamDecoder::Connect(localID=%d, &physicalFrameDecoder=0x%lX=%s\n",
//		  localID, (int) source, DebugUnitNameFromID(source->GetUnitID()));
// #endif
//

uint64 * debugGlobalConfig = GlobalBoardConfig;

char * DebugUnitNameFromID(uint32 unitID) // this can be used anywhere!!!
	{
	static char	nullStr[] = "";
#if _DEBUG_VERBOSE_CREATE_UNIT
	if (debugGlobalConfig)
		{
		uint64		*	tc = debugGlobalConfig;

		while (*tc != UNITS_DONE)
			{
			if (unitID == tc[2])
				return ((char *) tc[0]); // SUCCESS!
			if (tc[2] & VDRUIDF_CONSTUCTION_CONDITIONAL)				
				tc += 2;	// scip conditional mask and value
				
			tc += 4;
			while (*tc != PARAMS_DONE)
				tc += 2;	// A parameter is a pair (type, value)!
			tc++;
			// Skip mappings
			while (*tc != MAPPING_DONE)
				{
				tc++;
				// Skip parameters of mappings
				while (*tc != PARAMS_DONE)
					tc += 2;
				tc++;
				}
			tc++;
			}
		}
#endif // _DEBUG_VERBOSE_CREATE_UNIT
	return nullStr;
	}



#if _DEBUG
//
// DebugDetectUnitIDCollisions(uint32 * config) is called from
//

STFResult DebugDetectUnitIDCollisions(uint64 * config)
	{	
	uint64	*	tc = config;
	uint64	*	tc2 = config;
	uint32		thisUnitID, nextUnitID;
	int		errorCount = 0;
#if _DEBUG_VERBOSE_CREATE_UNIT
	char	*	thisUnitIDString;
	char	*	nextUnitIDString;
#endif

	while (*tc != UNITS_DONE)
		{
#if _DEBUG_VERBOSE_CREATE_UNIT
		thisUnitIDString = (char *) tc[0];
		tc += 2;
#endif
		thisUnitID = tc[0];
		tc += 2;

		if (thisUnitID & VDRUIDF_CONSTUCTION_CONDITIONAL)
			tc += 2;

		while (*tc != PARAMS_DONE)
			tc += 2;	// A parameter is a pair (type, value)!
		tc++;
		// Skip mappings
		while (*tc != MAPPING_DONE)
			{
			tc++;
			// Skip parameters of mappings
			while (*tc != PARAMS_DONE)
				tc += 2;
			tc++;
			}
		tc++;
		
		tc2 = tc;
		while (*tc2 != UNITS_DONE)
			{
#if _DEBUG_VERBOSE_CREATE_UNIT			
			nextUnitIDString = (char *) tc2[0];
			tc2 += 2;
#endif
			nextUnitID = tc2[0];
			tc2 += 2;

			if (nextUnitID & VDRUIDF_CONSTUCTION_CONDITIONAL)
				tc += 2;

			if (thisUnitID == nextUnitID)
				{
				// Collision!
				errorCount++;
#if _DEBUG_VERBOSE_CREATE_UNIT
				if (strcmp(thisUnitIDString,nextUnitIDString) == 0)
					DP("ERROR: BoardConfig has duplicate CREATE_UNIT(%s=0x%08X) instances\n",
						thisUnitIDString, thisUnitID);
				else
					DP("ERROR: VDRUID conflict: CREATE_UNIT(%s=0x%08X), CREATE_UNIT(%s=0x%08X)\n",
						thisUnitIDString, thisUnitID, nextUnitIDString, nextUnitID );
#else
				DP("ERROR: CREATE_UNIT(0x%08X) is multiply defined in BoardConfig\n", thisUnitID);
#endif
				}

			while (*tc2 != PARAMS_DONE)
				tc2 += 2;	// A parameter is a pair (type, value)!
			tc2++;
			// Skip mappings
			while (*tc2 != MAPPING_DONE)
				{
				tc2++;
				// Skip parameters of mappings
				while (*tc2 != PARAMS_DONE)
					tc2 += 2;
				tc2++;
				}
			tc2++;
			}
		}

	STFRES_RAISE((errorCount != 0) ? STFRES_INVALID_PARAMETERS : STFRES_OK); 
	}

#endif // _DEBUG
 

//
// PhysicalUnitMapping destructor
//
	 

PhysicalUnitMapping::~PhysicalUnitMapping()
	{
	MappingNode * nextNode;
	MappingNode * curNode;

	int i = 0;
	
	while (i < numNodes)
		{
		curNode = mapping[i];
		while (curNode)
			{
			nextNode = curNode->next;
			delete curNode;
			curNode = nextNode;
			}
		i++;
		}

	delete[] mapping;
	}


STFResult PhysicalUnitMapping::EnterUnit(uint32 id, IPhysicalUnit * unit, uint64 * subUnitConfigs)
	{
	MappingNodePtr * tempMapping;
	int i = 0;
	//lint --e{613}
	// No more entries in mapping?
	if (numNodes == totalNodes)
		{
		// So double the size of the mapping array!

		tempMapping = mapping;
		totalNodes *= 2;

		mapping = new MappingNodePtr[totalNodes];

		for (i = 0; i < numNodes; i++)
			mapping[i] = tempMapping[i];

		delete[] tempMapping;
		}

	mapping[numNodes] = new MappingNode(numNodes, id, unit, subUnitConfigs);

	numNodes++;

	STFRES_RAISE_OK;
	}


STFResult PhysicalUnitMapping::FindUnit(uint32 id, MappingNode * & node)
	{
	int i = 0;
	//lint --e{613}
	while (i < numNodes && mapping[i]->id != id) i++;

	if (i < numNodes)
		node = mapping[i];
	else
		{
		DP("PhysicalUnitMapping::FindUnit() can't find unit with ID %08x\n", id);
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
		}

	STFRES_RAISE_OK;
	}


STFResult PhysicalUnitMapping::ConnectUnits(uint32 targetID, int localID, uint32 sourceID)
	{
	MappingNode * targetNode;
	MappingNode * sourceNode;

#if _DEBUG
	DPV("ConnectUnits(%s=0x%08X,localID=%d,%s=sourceID=0x%08X)\n", DebugUnitNameFromID(targetID), targetID, localID, DebugUnitNameFromID(sourceID), sourceID);
#endif
	STFRES_REASSERT(FindUnit(targetID, targetNode));
	STFRES_REASSERT(FindUnit(sourceID, sourceNode));

	targetNode->next = new MappingNode(sourceNode->index, sourceID, sourceNode->unit, NULL, targetNode->next);

	STFRES_RAISE(targetNode->unit->Connect(localID, sourceNode->unit));
	}


STFResult PhysicalUnitMapping::TraverseUnit(int mapIdx, PhysicalMappingVisitor * visitor)
	{	
	MappingNode *	nextNode;
	//lint --e{613}
	mapping[mapIdx]->cycleDetect = true;

	// Increment global visit counter
	visitCount++;

	// Mark sort order, and signal initialisation by setting it != 0
	mapping[mapIdx]->visitOrder = visitCount;

	// Visit all units this unit depends on
	nextNode = mapping[mapIdx]->next;

	while (nextNode != NULL)
		{
		STFRES_REASSERT(visitor->VisitChild(mapping[mapIdx], nextNode));	

		if (mapping[nextNode->index]->cycleDetect)
			{
			DP("### SYSTEM CONSTRUCTION ERROR ### Cycle detected in unit dependencies! Cur node unit: %08x, next node Unit: %08x\n", 
				mapping[mapIdx]->unit->GetUnitID(), 
				mapping[nextNode->index]->unit->GetUnitID());
				
			STFRES_RAISE(STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
			}

		if (mapping[nextNode->index]->visitOrder == 0)
			{
			// Get index of next unit
			STFRES_REASSERT(TraverseUnit(nextNode->index, visitor));
			}

		nextNode = nextNode->next;
		}

	// Now we let the visitor do its stuff
	STFRES_REASSERT(visitor->VisitNode(mapping[mapIdx]));	

	mapping[mapIdx]->cycleDetect = false;

	STFRES_RAISE_OK;
	}	


STFResult PhysicalUnitMapping::TopologicalTraverse(PhysicalMappingVisitor * visitor)
	{
	int		 i;
	//lint --e{613}
	// Initialize the visitor
	STFRES_REASSERT(visitor->Initialize());

	// Reset global visit counter
	visitCount = 0;

	// Mark all units as "unvisited"
	for (i = 0; i < numNodes; i++)
		mapping[i]->visitOrder = 0;

	// Go through array of physical units and visit them if
	// this has not already happened by recursive calls to TraverseUnit
	for (i = 0; i < numNodes; i++)
		{
		if (mapping[i]->visitOrder == 0)
			{
			STFRES_REASSERT(TraverseUnit(i, visitor));
			}
		}

	STFRES_REASSERT(visitor->Complete());

	STFRES_RAISE_OK;
	}



#if _DEBUG_WRITE_CONSTRUCTION_SUB_TREE 

class WriteConstructionTreeVisitor : public PhysicalMappingVisitor 	
	{
	private:
		FILE * f;

	public:
		STFResult Initialize()
			{
			f = fopen("constructionGraph.dot", "wt");
			if (f)
				{
				fprintf(f, "digraph ConstructionGraph { \n");
				fprintf(f, "   concentrate=TRUE; \n");
				fprintf(f, "   ration=0.724; \n");
				fprintf(f, "   rank=min; \n");
				fprintf(f, "   rankdir=LR; \n");
				}

			STFRES_RAISE_OK;
			}

		STFResult Complete()
			{
			if (f)
				{
				fprintf(f,"}\n"); 
				fclose(f);
				f = NULL;
				}

			STFRES_RAISE_OK;
			}
		
		STFResult VisitChild(MappingNodePtr fromNode, MappingNodePtr nodeToVisit)
			{		
			if (f)
				{
				char * srcName = DebugUnitNameFromID(fromNode->id);
				if (strlen(srcName) > strlen("VDRUID_"))
					srcName += strlen("VDRUID_");
				char * dstName = DebugUnitNameFromID(nodeToVisit->id);
				if (strlen(dstName) > strlen("VDRUID_"))
					dstName += strlen("VDRUID_");
				fprintf(f, "   %s -> %s;\n", srcName, dstName);
				}

			STFRES_RAISE_OK;
			}

		STFResult VisitNode(MappingNodePtr nodeToVisit)
			{
			STFRES_RAISE_OK;
			}
	};

#endif // _DEBUG_WRITE_CONSTRUCTION_SUB_TREE 


class InitializeUnitsVisitor : public PhysicalMappingVisitor
	{
	public: 
		STFResult Initialize() { STFRES_RAISE_OK; }

		STFResult Complete() { STFRES_RAISE_OK; }
		
		STFResult VisitNode(MappingNodePtr nodeVisited)
			{
			STFResult res = STFRES_OK;
		        //DP("Initializing %08x\n",nodeVisited->unit->GetUnitID()); 	
			res = nodeVisited->unit->Initialize(nodeVisited->subUnitConfigs);
			if (STFRES_FAILED(res))
				{
				DP("### SYSTEM CONSTRUCTION ERROR ### Initialize failed with error %x: global unit id: %08x\n", res, nodeVisited->unit->GetUnitID());
				STFRES_RAISE(res);
				}

			STFRES_RAISE_OK;
			}

		STFResult VisitChild(MappingNodePtr fromNode, MappingNodePtr nodeToVisit)
			{
			STFRES_RAISE_OK;
			}		
	};




STFResult PhysicalUnitMapping::GetBoardUnit(IVDRBase *& board)
	{
	MappingNode	*	tempNode;

	STFRES_REASSERT(FindUnit(VDRUID_VDR_BOARD, tempNode));

	board = static_cast<IVDRBase*>(tempNode->unit);

	STFRES_RAISE_OK;
	}



// Generic System Construction Function (to be executed in Kernel Mode)

// The function parses the GlobalBoardConfig defined in a customer-specific
// file and constructions the Physical and Virtual Units out of it that
// make up the system.
// It operates in three phases:
// 1. Allocation
// 2. Connection
// 3. Initialisation
// Please refer to document ADCSxxxx for further information.

// IN:    config - Pointer to global Board configuration structure which is
//                 representing the Unit Construction Graph
// INOUT: board  - Reference to pointer to IPhysicalUnit interface. The
//                 caller can use this to obtain access to the Board's
//                 interfaces.
#if _DEBUG_WRITE_CONSTRUCTION_SUB_TREE	
	#include <task.h> // oggn
#endif

STFResult VDRCreateBoard(uint64 * config, IVDRBase *& board)
	{
	PhysicalUnitMapping			mapping;
	InitializeUnitsVisitor	*	visitor;
	uint64							*	tc;
	int								localID;
	uint64							*	creationParameters;
	uint32							globalUnitID;
	STFResult						res;
	PhysicalUnitFactory			createFunction;
	IPhysicalUnit				*	physicalUnitInterface;

	DP("TIP: Control VDRCreateBoard() logging with _DEBUG_VERBOSE_UNIT_CREATION_OUTPUT in VDR/Source/Construction/UnitConstruction.cpp!\n");

#if _DEBUG
	// This diagnostic function could save a lot of debugging effort.  Do not disable!
	STFRES_REASSERT(DebugDetectUnitIDCollisions(config));
#endif
	//
	// Allocation phase
	//

	// Go through Board Config and enter all Physical units in the Mapping
	// structure.
	tc = config;
	while (*tc != UNITS_DONE)
		{

#if _DEBUG_VERBOSE_CREATE_UNIT
		char * debugIdName = (char *) tc[0];
		char * debugFuncName = (char *) tc[1];
		globalUnitID	= tc[2];
		tc += 2;
#endif
		
		globalUnitID	= tc[0];
		createFunction = (PhysicalUnitFactory)tc[1];		
		tc += 2;		

		if (globalUnitID & VDRUIDF_CONSTUCTION_CONDITIONAL)
			tc += 2;	// scip over condition mask and condition value

#if _DEBUG_VERBOSE_CREATE_UNIT
		DPV("%s=0x%08X %s(", debugIdName, globalUnitID, debugFuncName);
#endif
				

#if !_DEBUG_VERBOSE_CREATE_UNIT		
		// Skip parameters of Physical Unit
		creationParameters = tc;
		while (*tc != PARAMS_DONE)
			{
			tc += 2;	// A parameter is a pair (type, value)!
			}
#else
		// output the parameters...
		creationParameters = tc;
		while (*tc != PARAMS_DONE)
			{
			if (tc[0] == PARAMS_STRING)
				DPV("\"%s\"", (char *) tc[1]);
			else if ((tc[0] == PARAMS_POINTER) || ((int) tc[1] < 0))
				DPV("0x%08x", tc[1]);
			else
				DPV("%d", tc[1]);
			if (tc[2] != PARAMS_DONE)
				DPV(",");
			tc += 2;	// A parameter is a pair (type, value)!
			}
		DPV(")\n");
#endif

		tc++;		// scip the PARAMS_DONE

		// Create unit and enter it into the mapping
		res = createFunction(globalUnitID, creationParameters, physicalUnitInterface);
		if (STFRES_FAILED(res))
			{
#if _DEBUG
			DP("### SYSTEM CONSTRUCTION ERROR ### Create failed %08x: global unit: %s=%08x\n", res, DebugUnitNameFromID(globalUnitID), globalUnitID);
#endif
			BREAKPOINT;
			STFRES_RAISE(res);
			}
		res = mapping.EnterUnit(globalUnitID, physicalUnitInterface, tc);
		if (STFRES_FAILED(res))
			{
#if _DEBUG
			DP("### SYSTEM CONSTRUCTION ERROR ### EnterUnit() error %08x: global unit: %s=%08x\n", res, DebugUnitNameFromID(globalUnitID), globalUnitID);
#endif
			BREAKPOINT;
			STFRES_RAISE(res);
			}

		// Skip mappings
		while (*tc != MAPPING_DONE)
			{
			tc++;
			// Skip parameters of mappings
			while (*tc != PARAMS_DONE)
				tc += 2;
			tc++;
			}

		tc++;
		}

	//
	// Connection phase
	//

	// Resolve all dependencies between Physical Units by connecting them.
	tc = config;
	while (*tc != UNITS_DONE)
		{					 
#if _DEBUG_VERBOSE_CREATE_UNIT
		globalUnitID = tc[2];
		tc += 2;
#endif
		globalUnitID = tc[0];
		tc += 2;
		if (globalUnitID & VDRUIDF_CONSTUCTION_CONDITIONAL)
			tc += 2;	// scip the condition mask and value if available

		// Skip parameters of Physical Unit
		while (*tc != PARAMS_DONE)
			tc += 2;	// A parameter is a pair (type, value)!

		tc++;

		localID = 0;
		while (*tc != MAPPING_DONE)
			{
			res = mapping.ConnectUnits(globalUnitID, localID, tc[0]);
			if (STFRES_FAILED(res))
				{
#if _DEBUG
				DP("### SYSTEM CONSTRUCTION ERROR ### ConnectUnits() error 0x%08x: global unit: %s=0x%08x, 0x%08x\n", res, DebugUnitNameFromID(globalUnitID), globalUnitID, tc[0]);
#endif
				BREAKPOINT;
				STFRES_RAISE(res);
				}

			tc++;
			while (*tc != PARAMS_DONE)
				tc += 2;
			tc++;
			localID++;
			}

		tc++;
		}

	//
	// Initialization phase
	//

	// Do a topological sort of the Unit Construction Graph and initialize the Physical
	// Units in that order. In case of a Shared Hardware Reference between two
	// Physical Units, a corresponding Virtual Unit of the target unit is created.
	visitor = new InitializeUnitsVisitor();
	res = mapping.TopologicalTraverse(visitor);
	delete visitor;	

	STFRES_REASSERT(res);	

	// STFRES_REASSERT(mapping.TestVirtuals());


	// The top unit of the sorted Unit Construction Graph is the Board itself.
	// It is the anchor-point of our driver as all driver interfaces can be queried from it.
	STFRES_REASSERT(mapping.GetBoardUnit(board));

	// Here "board" already has a reference count of 1 which was set when creating the unit.

#if _DEBUG_WRITE_CONSTRUCTION_SUB_TREE	
	// debug output of graph creation output
	task_priority_set(NULL, 8);
	task_delay(15625);
	mapping.TopologicalTraverse(new WriteConstructionTreeVisitor());
#endif

	STFRES_RAISE_OK;
	}
