///
/// @brief  Base Component Object Interfaces
///

#ifndef IVDRBASE_H
#define IVDRBASE_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"


/// @brief VDR Interface ID type
///
/// Bit31                              Bit0
/// |                                     |
/// PC-- ---- UUUU UUUU UUUU UUUU UUUU UUUU
/// P: Private interface flag
///    0: Public interface - available to applications using VDR Layer 
///    1: Private interface - only used inside of VDR Layer
///
/// C: Customer-defined interface flag
///    0: Interface ID defined by ST and registered with the 
///       STCM Identifier Value Manager (STIVAMA)
///    1: Interface ID defined by customer and not registered 
///
/// U: Unique value assigned by STCM Identifier Value Manager (STIVAMA) 
///    or a unique customer value
///
/// -: Reserved for future use
typedef uint32 VDRIID;


/// @brief Error code returned when an interface requested with IVDRBase::QueryInterface cannot be found
static const STFResult STFRES_INTERFACE_NOT_FOUND = 0x88844005;   // created by the STCM ID value manager


/// @brief Base Interface ID
static const VDRIID VDRIID_VDR_BASE = 0x00000006;



/// @brief Base Interface Definition
///
/// Access to all other interfaces a component object offers is gained
/// by this interface.
class IVDRBase
   {
   public:
   virtual ~IVDRBase(){}; // NHV: Added for g++ 4.1.1

   /// @brief Ask for the pointer to a specific interface
   ///
   /// @param iid IN: Interface ID
   /// @param ifp INOUT: reference to IVDRBase pointer, will be NULL if function failed
   /// @result STFRES_OK                     interface was found
   ///         STFRES_INTERFACE_NOT_FOUND    interface not found, "ifp" will be NULL
   virtual STFResult QueryInterface(VDRIID iid, void *& ifp) = 0;

   //
   // Reference counting
   //
		
   /// @brief Increase reference counter
   virtual uint32 AddRef() = 0;
		
   /// @brief Decrease reference counter, release interface when it becomes 0
   ///
   /// If reference counter becomes 0, the interface will close down. If all interfaces
   /// on a component instance have been closed down, the component instance will be
   /// released.
   virtual uint32 Release() = 0;
   };


#endif // IVDRBASE_H
