//
// PURPOSE:		Internal Interface of Units supporting Tags
//

#ifndef ITAGUNIT_H
#define ITAGUNIT_H

#include "VDR/Interface/Unit/IVDRTagUnit.h"

#if _DEBUG
#define _DEBUG_ALL_TAGS			0	// this will require a LONG rebuild
#define _DEBUG_CONFIGURE_TAGS		0	// see also VDR/Source/Unit/VirtualUnit.cpp
#define DP_SETTAG						DP
#define _DEBUG_CHANGESET_WATCHPOINT	0	// additional debugging facility for changeSet flags

#else // ! _DEBUG
#define _DEBUG_ALL_TAGS			0	// do not change this
#define _DEBUG_CONFIGURE_TAGS		0	// do not change this
#define DP_SETTAG			DP_EMPTY // do not change this
#endif

#if _DEBUG_CHANGESET_WATCHPOINT && _DEBUG
// To take advantage of this, set _DEBUG_CHANGESET_WATCHPOINT above and add the line
// INIT_DEBUG_CHANGESET_WATCHPOINT(&changeSet) to a specific virtual unit's constructor
// These 3 globals are declared in VirtualUnit.cpp (set a manual breakpoint there):
extern uint32	* pDebugChangedWatchPoint;
extern uint32	lastChangedWatchPointValue;
extern void DebugWatchPointFunction(char *str, uint32 lastValue, uint32 * pCurrentValue, char *filename);
// Initialized in your specific virtual unit's constructor, using:
#define INIT_DEBUG_CHANGESET_WATCHPOINT(addr) \
	do { pDebugChangedWatchPoint = addr; lastChangedWatchPointValue = 0; } while (0)
#define CHECK_DEBUG_CHANGESET_WATCHPOINT(str) \
	if (pDebugChangedWatchPoint && (*pDebugChangedWatchPoint != lastChangedWatchPointValue)) \
		{ \
		DebugWatchPointFunction(str, lastChangedWatchPointValue, pDebugChangedWatchPoint, __FILE__); \
		lastChangedWatchPointValue = *pDebugChangedWatchPoint; \
		}
#else // _DEBUG_CHANGESET_WATCHPOINT && _DEBUG
#define INIT_DEBUG_CHANGESET_WATCHPOINT(var)	DP_EMPTY("junk")
#define CHECK_DEBUG_CHANGESET_WATCHPOINT(str)
#endif

//! Internal Tag Unit interface ID
static const VDRIID VDRIID_TAG_UNIT = 0x80000002;

//! Internal interface for units that support Tags
class ITagUnit : virtual public IVDRTagUnit
   {
   public:
   virtual ~ITagUnit(){}; // NHV: Added for g++ 4.1.1
   /// Retrieve the Tag Unit IDs supported by the unit exposing this interface
   virtual STFResult GetTagIDs(VDRTID * & ids) = 0;

   /// Internal Begin of Configuration process (for counting)
   virtual STFResult InternalBeginConfigure(void) = 0;

   /// Abort configuration process after it was started with InternalBeginConfigure()
   virtual STFResult InternalAbortConfigure(void) = 0;

   /// Internal Completion of Configuration, leads to actual commitment of changes
   virtual STFResult InternalCompleteConfigure(void) = 0;

   /// Internal Tag configuration function
   virtual STFResult InternalConfigureTags(TAG * tags) = 0;

   /// Internal Update function to program just the required changes
   virtual STFResult	InternalUpdate(void) = 0;
   };


///////////////////////////////////////////////////////////////////////////////
// Macros for easy parsing of TAG lists during InternalConfigure() calls
///////////////////////////////////////////////////////////////////////////////

// A typical TAG parsing routine will look like this
//
//	Error VirtualMPEGDecoder::InternalConfigureTags(TAG *tags)
//		{
//		PARSE_TAGS_START(changeSet, tags)
//			GETSET(TAG_ID, variable, changeSetGroup)
//		PARSE_TAGS_END
//	
//		GNRAISE_OK;
//		}
//
 

// Macro to start parsing of a tag list
// tags: tag list
// affectedChangeSet: change set variable receiving the change set group flags
#define PARSE_TAGS_START(tags, affectedChangeSet) \
	TAG * tp = tags; \
	uint32 * __currentChangeSetX2f14 = &affectedChangeSet; \
	CHECK_DEBUG_CHANGESET_WATCHPOINT("PARSE_TAGS_START") \
	while (tp->id) { \
		switch (tp->id & ANYTAGKEY) {

// Macro to end parsing of a tag list
#define PARSE_TAGS_END \
	default: \
		*__currentChangeSetX2f14 |= 0x0; \
		break; \
		} tp += tp->skip; } \
	CHECK_DEBUG_CHANGESET_WATCHPOINT("PARSE_TAGS_END")


//
// The following code block defines the GETSET family of macros and their
// debug variants.
//
// Note that you can always use any of the two debug variants of each
// macro in the GETSET family (GETSETC, GETSETIC, GETSETAC, GETSETIAC):
// GETSET_DEBUG - this will log the tagname, the before and after values,
//                and the filename when the tag is received.
// GETSET_DEBUG_NONSCALAR - this will log the tagname and the filename 
//                when the tag is received (for tags with nonscalar types).
// These versions will automatically default to their non-debug equivalents
// in release builds.
//
// Since the #ifs are confusing, here's an outline of the definition block:
// 
// #if _DEBUG
// #define GETSETC_DEBUG_NONSCALAR // use these in your PARSE_TAGS loop if the
// #define GETSETIC_DEBUG_NONSCALAR // value is a struct (cannot print as int)
// #define GETSETAC_DEBUG_NONSCALAR
// #define GETSETIAC_DEBUG_NONSCALAR
// #define GETSETC_DEBUG		// use these if the value can be cast to an int or if
// #define GETSETIC_DEBUG		// you do not want to print the before & after values
// #define GETSETAC_DEBUG
// #define GETSETIAC_DEBUG
// #endif
// 
// #if _DEBUG_ALL_TAGS // this is set above - activates all tag behavior!
// #define GETSETC	GETSETC_DEBUG
// #define GETSETIC	GETSETIC_DEBUG
// #define GETSETAC	GETSETAC_DEBUG
// #define GETSETIAC	GETSETIAC_DEBUG
// #else // normal case below for non-_DEBUG_ALL_TAGS
// #define GETSETC
// #define GETSETIC
// #define GETSETAC
// #define GETSETIAC
// #if ! _DEBUG
// #define GETSETC_DEBUG	GETSETC
// #define GETSETIC_DEBUG	GETSETIC
// #define GETSETAC_DEBUG	GETSETAC
// #define GETSETIAC_DEBUG	GETSETIAC
// #define GETSETC_DEBUG_NONSCALAR		GETSETC
// #define GETSETIC_DEBUG_NONSCALAR		GETSETIC
// #define GETSETAC_DEBUG_NONSCALAR		GETSETAC
// #define GETSETIAC_DEBUG_NONSCALAR	GETSETIAC
// #endif // ! _DEBUG
// #endif // ! _DEBUG_ALL_TAGS
//

// Macro to set a variable or get a variable, and mark the change set group
// a SET operation belongs to.
// tagid: Tag ID
// variable: variable receiving the tag's value for a SET_ operation or read in a GET_ operation
// changeSetGroup: specifies bit number to be set in change set if tag value changes
#if _DEBUG
#define GETSETC_DEBUG_NONSCALAR(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
		DP_SETTAG("CSET_%s in %s \n", #tagid, __FILE__); \
		if ((variable) != VAL_##tagid(tp))	\
			{		\
			(variable) = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
			}		\
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = (variable); \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Variant for indexed tags
#define GETSETIC_DEBUG_NONSCALAR(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
		DP_SETTAG("CSET_%s[%d] in %s \n", #tagid, TAG_INDEX(tp->id), __FILE__); \
		if (variable[TAG_INDEX(tp->id)] != VAL_##tagid(tp))	\
			{		\
			variable[TAG_INDEX(tp->id)] = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
			}		\
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable[TAG_INDEX(tp->id)]; \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

#define GETSETAC_DEBUG_NONSCALAR(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
			DP_SETTAG("CSET_%s in %s \n", #tagid, __FILE__); \
			(variable) = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = (variable); \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Variant for indexed tags
#define GETSETIAC_DEBUG_NONSCALAR(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
			DP_SETTAG("CSET_%s[%d] in %s \n", #tagid, TAG_INDEX(tp->id), __FILE__); \
			variable[TAG_INDEX(tp->id)] = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable[TAG_INDEX(tp->id)]; \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// these debug version will print values for scalars only
#define GETSETC_DEBUG(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
		DP_SETTAG("CSET_%s from %d to %d in %s \n", #tagid, (int)(variable), (int)VAL_##tagid(tp), __FILE__); \
		if ((variable) != VAL_##tagid(tp))	\
			{		\
			(variable) = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
			}		\
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = (variable); \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Variant for indexed tags
#define GETSETIC_DEBUG(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
		DP_SETTAG("CSET_%s[%d] from %d to %d in %s \n", #tagid, TAG_INDEX(tp->id), (int) variable[TAG_INDEX(tp->id)], (int) VAL_##tagid(tp), __FILE__); \
		if (variable[TAG_INDEX(tp->id)] != VAL_##tagid(tp))	\
			{		\
			variable[TAG_INDEX(tp->id)] = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
			}		\
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable[TAG_INDEX(tp->id)]; \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

#define GETSETAC_DEBUG(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
			DP_SETTAG("CSET_%s from %d to %d in %s \n", #tagid, (int)(variable) (int) VAL_##tagid(tp), __FILE__); \
			(variable) = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = (variable); \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Variant for indexed tags
#define GETSETIAC_DEBUG(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
			DP_SETTAG("CSET_%s[%d] from %d to %d in %s \n", #tagid, TAG_INDEX(tp->id), (int) variable[TAG_INDEX(tp->id)] (int) VAL_##tagid(tp), __FILE__); \
			variable[TAG_INDEX(tp->id)] = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable[TAG_INDEX(tp->id)]; \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

#endif // _DEBUG

#if _DEBUG_ALL_TAGS
#define GETSETC	GETSETC_DEBUG_NONSCALAR
#define GETSETIC	GETSETIC_DEBUG_NONSCALAR
#define GETSETAC	GETSETAC_DEBUG_NONSCALAR
#define GETSETIAC	GETSETIAC_DEBUG_NONSCALAR
#else // normal case below for non-_DEBUG_ALL_TAGS
#define GETSETC(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
		if ((variable) != VAL_##tagid(tp))	\
			{		\
			(variable) = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
			}		\
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = (variable); \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Variant for indexed tags
#define GETSETIC(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
		if (variable[TAG_INDEX(tp->id)] != VAL_##tagid(tp))	\
			{		\
			variable[TAG_INDEX(tp->id)] = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
			}		\
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable[TAG_INDEX(tp->id)]; \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Macro to set a variable or get a variable, and mark the change set group
// a SET operation belongs to.
// tagid: Tag ID
// variable: variable receiving the tag's value for a SET_ operation or read in a GET_ operation
// changeSetGroup: specifies bit number to be set in change set Always Change
// need to use this with reference tags if object does not have == operator

#define GETSETAC(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
			(variable) = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = (variable); \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Variant for indexed tags
#define GETSETIAC(tagid, variable, changeSetGroup)	\
	case CSET_##tagid:	\
			variable[TAG_INDEX(tp->id)] = VAL_##tagid(tp);	\
			*__currentChangeSetX2f14 |= 1 << changeSetGroup; \
		break;	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable[TAG_INDEX(tp->id)]; \
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

#if ! _DEBUG
#define GETSETC_DEBUG	GETSETC
#define GETSETIC_DEBUG	GETSETIC
#define GETSETAC_DEBUG	GETSETAC
#define GETSETIAC_DEBUG	GETSETIAC
#define GETSETC_DEBUG_NONSCALAR		GETSETC
#define GETSETIC_DEBUG_NONSCALAR		GETSETIC
#define GETSETAC_DEBUG_NONSCALAR		GETSETAC
#define GETSETIAC_DEBUG_NONSCALAR	GETSETIAC
#endif // ! _DEBUG
#endif // ! _DEBUG_ALL_TAGS


// Macro for TAGs that can only be retrieved with GET.
// Value is read from variable.
// tagid: Tag ID
// variable: variable receiving the tag's value for a SET_ operation or read in a GET_ operation
#define GETONLY(tagid, variable) \
	case CSET_##tagid:	\
		STFRES_RAISE(STFRES_OBJECT_READ_ONLY);	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable;	\
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Variant for indexed tags
#define GETONLYI(tagid, variable) \
	case CSET_##tagid:	\
		STFRES_RAISE(STFRES_OBJECT_READ_ONLY);	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = variable[TAG_INDEX(tp->id)];	\
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break

// Macro for TAGs that can only be retrieved with GET.
// Value is read by function.
#define GETONLYF(tagid, function) \
	case CSET_##tagid:	\
		STFRES_RAISE(STFRES_OBJECT_READ_ONLY);	\
	case CGET_##tagid:	\
		REF_##tagid(tp) = function;	\
		break;	\
	case CQRY_##tagid:	\
		QRY_TAG(tp) = true;	\
		break


///////////////////////////////////////////////////////////////////////////////
// Macros for easy updating of changed parameters in InternalUpdate() calls
///////////////////////////////////////////////////////////////////////////////

#define UPDATE_GROUP_CALL(affectedChangeSet, changeSetGroup, call) \
	if (affectedChangeSet & (1 << changeSetGroup)) \
		STFRES_REASSERT(call);

#endif	// #ifndef ITAGUNIT_H
