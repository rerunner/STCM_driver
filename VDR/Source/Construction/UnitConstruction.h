
//
// PURPOSE:		Generic Unit Construction Implementation
//

#ifndef UNITCONSTRUCTION_H
#define UNITCONSTRUCTION_H

#include "STF/Interface/Types/STFResult.h"
#include "VDR/Source/Unit/IPhysicalUnit.h"
#include "VDR/Source/Construction/IUnitConstruction.h"


class MappingNode
   {
   public:
   MappingNode * next;

   IPhysicalUnit * unit;
   uint32 id;
   uint64 * subUnitConfigs;

   int visitOrder;
   int index;
		
   bool cycleDetect;

   MappingNode()
         {
         next = NULL;
         visitOrder = 0;
         cycleDetect	= false;
         }

   MappingNode(int index, uint32 id, IPhysicalUnit * unit, uint64 * subUnitConfigs, MappingNode * next = NULL)
         {
         this->index	= index;
         this->id		= id;
         this->unit	= unit;
         this->subUnitConfigs = subUnitConfigs;
         this->next	= next;
         visitOrder	= 0;
         cycleDetect	= false;
         }
   };


typedef MappingNode * MappingNodePtr;

class PhysicalMappingVisitor 
   {
   public:
   virtual ~PhysicalMappingVisitor(){}; // NHV: Added for g++ 4.1.1
   virtual STFResult Initialize() = 0;
   virtual STFResult Complete() = 0;
   virtual STFResult VisitChild(MappingNodePtr fromNode, MappingNodePtr nodeToVisit) = 0;
   virtual STFResult VisitNode(MappingNodePtr nodeVisited) = 0;
   };


class PhysicalUnitMapping
   {
   private:
   MappingNode	** mapping;
   int	numNodes;
   int	totalNodes;

   int	visitCount;

   protected:
   STFResult FindUnit(uint32 id, MappingNode * & node);

   public:
   STFResult TraverseUnit(int mapIdx, PhysicalMappingVisitor * visitor);
   STFResult TopologicalTraverse(PhysicalMappingVisitor * visitor);
		
   public:
   PhysicalUnitMapping()
         {
         numNodes =	0;
         this->totalNodes = 25;
         mapping = new MappingNodePtr[totalNodes];
         };

   virtual ~PhysicalUnitMapping();

   STFResult EnterUnit(uint32 id, IPhysicalUnit * unit, uint64 * subUnitConfigs);
   STFResult ConnectUnits(uint32 targetID, int localID, uint32 sourceID);
   STFResult TestVirtuals(void);

   STFResult GetBoardUnit(IVDRBase *& board);
   };


// Global Board Creation function
STFResult VDRCreateBoard(uint64 * config, IVDRBase *& board);


#endif	// #ifndef UNITCONSTRUCTION_H
