///
/// @brief  Base Message Interface
///

#ifndef IVDRMESSAGE_H
#define IVDRMESSAGE_H

#include "VDR/Interface/Base/IVDRBase.h"
#include "STF/Interface/Types/STFMessage.h"

/// @brief Type definition for VDRMID ("VDR Message ID")
///
/// Bit31                              Bit0
/// |                                     |
/// ---- UUUU UUUU UUUU UUUU RRRR RRRR RRRR
///
/// R: Range of 4096 Message IDs available for declaring messages of the registered type
///
/// U: Unique value assigned by STCM Identifier Value Manager (STIVAMA) 
///
/// -: Reserved for future use
typedef uint32	VDRMID;


/// @brief Standard Message Sink Registration Interface ID
static const VDRIID VDRIID_VDR_MESSAGE_SINK_REGISTRATION = 0x00000019;




//////////////////////////////////////////////////////////////////////
//
// Error result values
//
//////////////////////////////////////////////////////////////////////
static const STFResult STFRES_MESSAGE_SINK_ALREADY_REGISTERED = 0x8884400d;
static const STFResult STFRES_MESSAGE_SINK_NOT_REGISTERED     = 0x8884400e;

/// @brief Standard Message Sink Registration Interface
///
/// Used to register a message sink at a driver component, e.g. unit.
/// The registered sink will receive all messages of that component,
/// until it unregisters itself.
class IVDRMessageSinkRegistration : public virtual IVDRBase
   {
   public:
   virtual ~IVDRMessageSinkRegistration(){}; // NHV: Added for g++ 4.1.1
   /// @brief Register a message sink at a driver component
   virtual STFResult RegisterMessageSink(STFMessageSink * sink) = 0;

   /// @brief Register a message sink for a range of messages
   ///
   /// The caller registers its message sink and range of message IDs it
   /// wants to receive, starting at @param message.
   /// By default, @param messageRange is 1, so that reception of one particular
   /// type of message can be selected.
   virtual STFResult RegisterFilteredMessageSink(STFMessageSink * sink, 
                                                 VDRMID message = 0, 
                                                 uint32 messageRange = 1) = 0;

   /// @brief Unregister a message sink
   virtual STFResult UnregisterMessageSink(STFMessageSink * sink) = 0;
   };







/// @brief Message collectro interface ID
static const VDRIID VDRIID_MESSAGE_COLLECTOR = 0x00000073;

/// @brief Message collector interface
///
/// The message collector interface is able to register on several message think registrations and 
/// send these messages to one message interface. It provides itself a message think registration 
/// interface.
class IVDRMessageCollector : public virtual IVDRMessageSinkRegistration
   {
   public:
   virtual ~IVDRMessageCollector(){}; // NHV: Added for g++ 4.1.1
   /// @brief Register a message sink registration on the message collector interface
   ///
   /// @param sinkRegistration [in] The message sink registration to register
   /// @retval STFRSE_OK                 The message sink has been registered successfully
   /// @retval STFRES_OBJECT_FULL        The message sink has not been registered due to resource
   ///                                   limitations (maximum number of message sink registrations reached)
   /// @retval STFRES_OBJECT_INVALID     The message collector is in an invalid state (not initialized?)
   virtual STFResult RegisterMessageSinkRegistration(IVDRMessageSinkRegistration * sinkRegistration) = 0;

   /// @brief Unregister a message sink registration, previously registered
   ///
   /// @param sinkRegistration[in] The message sink registration to unregister
   /// @return Result valud indicating success or failure
   /// @retval STFRES_OK                 The message sink has been unregistered successfully
   /// @retval STFRES_OBJECT_NOT_FOUND   The given message sink could not be found
   /// @retval STFRES_INVALID_PARAMETERS The sink registration pointer is invalid
   /// @retval STFRES_OBJECT_INVALID     The message collector is in an invalid state (not initialized?)
   virtual STFResult UnregisterMessageSinkRegistration(IVDRMessageSinkRegistration * sinkRegistration) = 0;
   };


#endif	// #ifndef IVDRMESSAGE_H
