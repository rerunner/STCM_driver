///
/// @brief 
///

#include "STF/Interface/STFCompatibilityErrorCodes.h"

#if !_DEBUG

const char* STFResultLookup(STFResult hr)
	{
   return "<No Error Text Available>";
	}

#else //!_DEBUG

typedef struct
	{
   uint32   code;
   char     text[30];
	} ErrorLookupType;

//??? This will have to be changed. The point of the STFResult design is that there is NOT a common
// definition file, that is why the values are registered with the value ID manager STIVAMA.

static const ErrorLookupType ErrTable[] =
	{
   {STFRES_CONNECTION_LOST              , "CONNECTION_LOST"},
   {STFRES_INSUFFICIENT_RIGHTS          , "INSUFFICIENT_RIGHTS"},     
   {STFRES_INVALID_PARAMETERS           , "INVALID_PARAMETERS"},      
   {STFRES_OBJECT_ALREADY_JOINED        , "OBJECT_ALREADY_JOINED"},   
   {STFRES_OBJECT_EMPTY                 , "OBJECT_EMPTY"},            
   {STFRES_OBJECT_EXISTS                , "OBJECT_EXISTS"},           
   {STFRES_OBJECT_FOUND                 , "OBJECT_FOUND"},            
   {STFRES_OBJECT_FULL                  , "OBJECT_FULL"},             
   {STFRES_OBJECT_IN_USE                , "OBJECT_IN_USE"},           
   {STFRES_OBJECT_INVALID               , "OBJECT_INVALID"},          
   {STFRES_OBJECT_NOT_ALLOCATED         , "OBJECT_NOT_ALLOCATED"},    
   {STFRES_OBJECT_NOT_CURRENT           , "OBJECT_NOT_CURRENT"},      
   {STFRES_OBJECT_NOT_FOUND             , "OBJECT_NOT_FOUND"},        
   {STFRES_OBJECT_READ_ONLY             , "OBJECT_READ_ONLY"},        
   {STFRES_OBJECT_WRITE_ONLY            , "OBJECT_WRITE_ONLY"},       
   {STFRES_OPERATION_ABORTED            , "OPERATION_ABORTED"},       
   {STFRES_OPERATION_FAILED             , "OPERATION_FAILED"},        
   {STFRES_OPERATION_PENDING            , "OPERATION_PENDING"},       
   {STFRES_OPERATION_PROHIBITED         , "OPERATION_PROHIBITED"},    
   {STFRES_RANGE_VIOLATION              , "RANGE_VIOLATION"},         
   {STFRES_TIMEOUT                      , "TIMEOUT"},                 
   {STFRES_UNIMPLEMENTED                , "UNIMPLEMENTED"},           
   {STFRES_WARNING_OBJECT_IN_USE        , "WARNING_OBJECT_IN_USE"},
   {STFRES_NOT_ENOUGH_MEMORY            , "NOT_ENOUGH_MEMORY"},          
   {STFRES_MEM_ALLOCATED_BEFORE         , "MEM_ALLOCATED_BEFORE"},    
   {STFRES_MEM_NOT_ALLOCATED            , "MEM_NOT_ALLOCATED"},       
   {STFRES_NOT_COMPLETELY_AVAILABLE     , "NOT_COMPLETELY_AVAILABLE"},
   {STFRES_SYNCHRONISATION_MISMATCH     , "SYNCHRONISATION_MISMATCH"},
   {STFRES_DRIVE_ALREADY_LOCKED         , "DRIVE_ALREADY_LOCKED"},    
   {STFRES_END_OF_FILE                  , "END_OF_FILE"},             
   {STFRES_FILE_IN_USE                  , "FILE_IN_USE"},             
   {STFRES_FILE_NOT_FOUND               , "FILE_NOT_FOUND"},          
   {STFRES_FILE_READ_ERROR              , "FILE_READ_ERROR"},         
   {STFRES_FILE_WRITE_ERROR             , "FILE_WRITE_ERROR"},        
   {STFRES_FILE_WRONG_FORMAT            , "FILE_WRONG_FORMAT"},       
   {STFRES_CAN_NOT_PASSIVATE_IDLE_UNIT  , "CAN_NOT_PASSIVATE_IDLE_UNIT"},
   {STFRES_INVALID_CONFIGURE_STATE      , "INVALID_CONFIGURE_STATE"}, 
   {STFRES_UNIT_INACTIVE                , "UNIT_INACTIVE"},
   {STFRES_ERROR_RESPONSE_RECEIVED      , "ERROR_RESPONSE_RECEIVED"},
   {STFRES_ERROR_RESPONSE_RETURNED      , "ERROR_RESPONSE_RETURNED"},
   {STFRES_UNDEFINED_LOCATION_ACCESS    , "UNDEFINED_LOCATION_ACCESS"},
   {STFRES_UNSOLICITED_RESPONSE_RECEIVED, "UNSOLICITED_RESPONSE_RECEIVED"},
   {STFRES_UNDEFINED_MEMORY_ACCESS      , "UNDEFINED_MEMORY_ACCESS"},
   {STFRES_UNSUPPORTED_OPERATION        , "UNSUPPORTED_OPERATION"},
   {STFRES_ALIGNMENT_ERROR              , "ALIGNMENT_ERROR"},
   {STFRES_TRANSFER_WAS_UNALIGNED       , "TRANSFER_WAS_UNALIGNED"},
   {STFRES_STALLED_BY_EXTERNAL_REQUEST  , "STALLED_BY_EXTERNAL_REQUEST"},
   {STFRES_WAITING_FOR_EXTERNAL_REQUEST , "WAITING_FOR_EXTERNAL_REQUEST"},
   {STFRES_TRANSFER_IN_PROGRESS         , "TRANSFER_IN_PROGRESS"},

   {STFRES_DISK_READ_ONLY                 , "DISK_READ_ONLY"},
   {STFRES_DRIVE_FAILURE                  , "DRIVE_FAILURE"},
   {STFRES_DRIVE_FATAL_ERROR              , "DRIVE_FATAL_ERROR"},
   {STFRES_NO_VALID_DISK                  , "NO_VALID_DISK"},
   {STFRES_NO_DRIVE                       , "NO_DRIVE"},
   {STFRES_DRIVE_DETACHED                 , "DRIVE_DETACHED"},
   {STFRES_NO_DVD_DRIVE                   , "NO_DVD_DRIVE"},
   {STFRES_BLOCK_ALREADY_LOCKED           , "BLOCK_ALREADY_LOCKED"},
   {STFRES_BLOCK_NOT_LOCKED               , "BLOCK_NOT_LOCKED"},
   {STFRES_DISK_IS_NOT_PRESENT            , "DISK_IS_NOT_PRESENT"},
   {STFRES_INVALID_DRIVE_LETTER           , "INVALID_DRIVE_LETTER"},
   {STFRES_DRIVE_LOCK_FAILED              , "DRIVE_LOCK_FAILED"},
   {STFRES_DRIVE_LOAD_FAILED              , "DRIVE_LOAD_FAILED"},
   {STFRES_DRIVE_NOT_LOADABLE             , "DRIVE_NOT_LOADABLE"},
   {STFRES_READ_ERROR                     , "READ_ERROR"},
   {STFRES_WRITE_ERROR                    , "WRITE_ERROR"},
   {STFRES_HIGH_TEMPERATURE               , "HIGH_TEMPERATURE"},
   {STFRES_COPY_PROTECTION_VIOLATION      , "COPY_PROTECTION_VIOLATION"},
   {STFRES_COPY_PROTECTION_FAILED         , "COPY_PROTECTION_FAILED"},
   {STFRES_READ_ERROR_SECTOR_ENCRYPTED    , "READ_ERROR_SECTOR_ENCRYPTED"},
   {STFRES_INVALID_UNITS                  , "INVALID_UNITS"},
   {STFRES_UNITS_BUSY                     , "UNITS_BUSY"},
   {STFRES_AUTHENTICATION_FAILED          , "AUTHENTICATION_FAILED"},
   {STFRES_CSS_NOT_SUPPORTED              , "CSS_NOT_SUPPORTED"},
   {STFRES_SPU_OVERLAY_NOT_SUPPORTED      , "SPU_OVERLAY_NOT_SUPPORTED"},
   {STFRES_SCANNER_NEWLINE_IN_CONSTANT    , "SCANNER_NEWLINE_IN_CONSTANT"},
   {STFRES_PATH_NOT_FOUND                 , "PATH_NOT_FOUND"},
   {STFRES_INVALID_PATH                   , "INVALID_PATH"},
   {STFRES_NO_FILE_SYSTEM                 , "NO_FILE_SYSTEM"},
   {STFRES_NO_VOLUME                      , "NO_VOLUME"},
   {STFRES_VOLUME_INVALID                 , "VOLUME_INVALID"},
   {STFRES_ITEM_NOT_FOUND                 , "ITEM_NOT_FOUND"},
   {STFRES_NOT_A_DIRECTORY                , "NOT_A_DIRECTORY"},
   {STFRES_ITEM_INVALID                   , "ITEM_INVALID"},
   {STFRES_FILE_READ_ONLY                 , "FILE_READ_ONLY"}
	};

static const uint32 nErrorCount = sizeof(ErrTable) / sizeof(ErrorLookupType);


/// @name		STFResultLookup
///
/// @brief		translate an STFResult into human-readable text.
///
/// @param		hr				[in]	the STFResult code to translate.
///
/// @return		const char *		pointer to English translation of error code.
///
///				The function will attempt to match an STFResult code to its equivalent
///				human-readable text.  It will save debugging time instead of having to
///				find out what is the error code each time.

const char* STFResultLookup(STFResult hr)
	{
   if (STFRES_FAILED(hr))
		{
      // locate the error text in the table
      for (uint32 i=0; i<nErrorCount; i++)
			{
         if ((hr & STF_RESULT_UNIQUE_MASK) == ((ErrTable[i].code) & STF_RESULT_UNIQUE_MASK))
				{
            return ErrTable[i].text;
				}
			}
      // the error code was not found.
      return "<No Error Text Available>";
		}

   return "SUCCEEDED";
	}

#endif // _DEBUG
