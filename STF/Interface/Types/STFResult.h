///
/// @brief Generic Results
///

#ifndef STFRESULT_H
#define STFRESULT_H

#include "STFBasicTypes.h"

typedef uint32		STFResult;


/// @name STFResult definition
//@{
/// @brief STFResult bitfield
///
/// Bit31                             Bit 0
/// |                                     |
/// SS-- UUUU UUUU UUUU UUUU UUUU UUUU UUUU
///
/// U: Unique value assigned by STIVAMA registration
/// S: Severity
/// -: Reserved for future use
#define STF_RESULT_SEVERITY_BITS				(2)
#define STF_RESULT_SEVERITY_SHIFT			(30)
#define STF_RESULT_SEVERITY_MASK				(0xC0000000UL)
                                   
#define STF_RESULT_UNIQUE_BITS				(28)
#define STF_RESULT_UNIQUE_SHIFT				(0)
#define STF_RESULT_UNIQUE_MASK				(0x0FFFFFFF)


#define STF_RESULT_SEVERITY_OK				(0x0UL << STF_RESULT_SEVERITY_SHIFT)

#define STF_RESULT_SEVERITY_WARNING			(0x1UL << STF_RESULT_SEVERITY_SHIFT)

#define STF_RESULT_SEVERITY_ERROR			(0x2UL << STF_RESULT_SEVERITY_SHIFT)

#define STF_RESULT_SEVERITY_DEADLY			(0x3UL << STF_RESULT_SEVERITY_SHIFT)           
//@}


static const STFResult STFRES_OK			= 0x00000000UL;
static const STFResult STFRES_FALSE		= 0x0UL;
static const STFResult STFRES_TRUE		= 0x1UL;


/// @name Result Macros
//@{

/// Check if result is an error
#define STFRES_IS_ERROR(result)			((result) >= STF_RESULT_SEVERITY_ERROR)

/// Check if result is a warning
#define STFRES_IS_WARNING(result)		(((result) & STF_RESULT_SEVERITY_MASK) == STF_RESULT_SEVERITY_WARNING)

/// Check if result is OK
#define STFRES_IS_OK(result)				(((result) & STF_RESULT_SEVERITY_MASK) == STF_RESULT_SEVERITY_OK)

/// Evaluates to true if result is not an error
#define STFRES_SUCCEEDED(result)			(!(STFRES_IS_ERROR(result)))

/// Evaluates to true if result is an error
#define STFRES_FAILED(result)				(STFRES_IS_ERROR(result))

/// Raise result unconditionally
#define STFRES_RAISE(result)				return (result)

/// Raise success
#define STFRES_RAISE_OK						return STFRES_OK

/// Raise bool value of true
#define STFRES_RAISE_TRUE					return STFRES_TRUE

/// Raise bool value of false
#define STFRES_RAISE_FALSE					return STFRES_FALSE

/// Assert result based on arbitrary condition
//lint -emacro( 960, STFRES_ASSERT )
#define STFRES_ASSERT(cond, res)			do {if (!(cond)) return res;} while (0)

/// Return with result code if result of condition is an error, otherwise continue
//lint -emacro( 960, STFRES_REASSERT )
#define STFRES_REASSERT(cond)				do {STFResult resQXZ_1dbf47k = (cond); if (STFRES_IS_ERROR(resQXZ_1dbf47k)) return resQXZ_1dbf47k;} while (0)

/// severity(result) > severity(compareResult) ? true : false
#define STFRES_IS_HIGHER_SEVERITY(result, compareResult) \
	((result & STF_RESULT_SEVERITY_MASK) > (compareResult & STF_RESULT_SEVERITY_MASK))

/// severity(result) > severity(compareResult) ? result : compareResult
//lint -e(773)
#define STFRES_HIGHER_SEVERITY(result, compareResult) \
	result = ((result & STF_RESULT_SEVERITY_MASK) > (compareResult & STF_RESULT_SEVERITY_MASK)) ? result : compareResult
//@}




/// @name Generic errors
//@{

static const STFResult STFRES_CONNECTION_LOST          = 0x8001c01aUL;
static const STFResult STFRES_INSUFFICIENT_RIGHTS      = 0x80034015UL;
static const STFResult STFRES_INVALID_PARAMETERS       = 0x8000c019UL;

static const STFResult STFRES_OBJECT_ALREADY_JOINED    = 0x8002c103UL;
static const STFResult STFRES_OBJECT_EMPTY             = 0x8003000fUL;
static const STFResult STFRES_OBJECT_EXISTS            = 0x8002c009UL;
static const STFResult STFRES_OBJECT_FOUND             = 0x8002c00bUL;
static const STFResult STFRES_OBJECT_FULL              = 0x8003000eUL;
static const STFResult STFRES_OBJECT_IN_USE            = 0x8002c00aUL;
static const STFResult STFRES_OBJECT_INVALID           = 0x8002c014UL;
static const STFResult STFRES_OBJECT_NOT_ALLOCATED     = 0x8001c010UL;
static const STFResult STFRES_OBJECT_NOT_CURRENT       = 0x8002c102UL;
static const STFResult STFRES_OBJECT_NOT_FOUND         = 0x8002c008UL;
static const STFResult STFRES_OBJECT_READ_ONLY         = 0x8002c011UL;
static const STFResult STFRES_OBJECT_WRITE_ONLY        = 0x8002c012UL;

static const STFResult STFRES_OPERATION_ABORTED        = 0x8001001bUL;
static const STFResult STFRES_OPERATION_FAILED         = 0x8004401cUL;
static const STFResult STFRES_OPERATION_PENDING        = 0x88844002UL;
static const STFResult STFRES_OPERATION_PROHIBITED     = 0x80034013UL;
static const STFResult STFRES_OPERATION_NOT_SUPPORTED  = 0x87444000UL;
static const STFResult STFRES_MKB_PROCESSING				 = 0x88844049UL;

static const STFResult STFRES_RANGE_VIOLATION          = 0x8003000cUL;
static const STFResult STFRES_TIMEOUT                  = 0x80018016UL;
static const STFResult STFRES_UNIMPLEMENTED            = 0x80024007UL;
//@}

/// @name Generic warnings
//@{

static const STFResult STFRES_WARNING_OBJECT_IN_USE    = 0x47044003UL;
static const STFResult STFRES_WARNING_OBJECT_EXISTS    = 0x47044007UL;
static const STFResult STFRES_NOT_COMPLETELY_AVAILABLE = 0x47044004UL;
static const STFResult STFRES_SYNCHRONISATION_MISMATCH = 0x47044005UL;
//@}

/// @name Memory errors
//@{

static const STFResult STFRES_NOT_ENOUGH_MEMORY			 = 0x80020004UL;
//@}

/// @name Memory warnings
//@{

static const STFResult STFRES_MEM_ALLOCATED_BEFORE     = 0x40020006UL;
static const STFResult STFRES_MEM_NOT_ALLOCATED        = 0x40020005UL;
//@}

/// @name I/O errors
//@{

static const STFResult STFRES_END_OF_FILE              = 0x80004003UL;

static const STFResult STFRES_FILE_IN_USE              = 0x80004001UL;
static const STFResult STFRES_FILE_NOT_FOUND           = 0x80004000UL;
static const STFResult STFRES_FILE_READ_ERROR          = 0x80004017UL;
static const STFResult STFRES_FILE_WRITE_ERROR         = 0x80004018UL;
static const STFResult STFRES_FILE_WRONG_FORMAT        = 0x80004002UL;
//@}

/// @name I/O warnings
//@{

#ifndef STFRES_DRIVE_ALREADY_LOCKED
static const STFResult STFRES_DRIVE_ALREADY_LOCKED     = 0x47044001UL;
#endif
//@}

/// @name Unit errors
//@{

static const STFResult STFRES_CAN_NOT_PASSIVATE_IDLE_UNIT   = 0x8002c101UL;
static const STFResult STFRES_INVALID_CONFIGURE_STATE       = 0x8001c00dUL;
//@}

/// @name Unit warnings
//@{

static const STFResult STFRES_UNIT_INACTIVE						= 0x47044006UL;
//@}

/// @name DMA errors
//@{

static const STFResult STFRES_ERROR_RESPONSE_RECEIVED       = 0x8884401cUL;
static const STFResult STFRES_ERROR_RESPONSE_RETURNED       = 0x8884401dUL;
static const STFResult STFRES_UNDEFINED_LOCATION_ACCESS     = 0x8884401eUL;
static const STFResult STFRES_UNSOLICITED_RESPONSE_RECEIVED = 0x8884401fUL;
static const STFResult STFRES_UNDEFINED_MEMORY_ACCESS       = 0x88844020UL;
static const STFResult STFRES_UNSUPPORTED_OPERATION         = 0x88844021UL;
static const STFResult STFRES_ALIGNMENT_ERROR               = 0x88844022UL;
//@}

/// @name DMA warnings
//@{

static const STFResult STFRES_TRANSFER_WAS_UNALIGNED        = 0x47044008UL;
//@}

/// @name DMA messages (severity ok)
//@{

static const STFResult STFRES_STALLED_BY_EXTERNAL_REQUEST  = 0x00000002UL;
static const STFResult STFRES_WAITING_FOR_EXTERNAL_REQUEST = 0x00000003UL;
//@}


/// @name Bus errors
//@{

/// A bus transfer is in progress and the request cannot be served
static const STFResult STFRES_TRANSFER_IN_PROGRESS = 0x8884402cUL;
//@}

// Supress lint error that complains use of char. This is a debug function
//lint -e(970) -e(971)
const char * STFResultLookup (STFResult err);    // translate STFResult to human-readable format



#endif //STFRESULT_H
