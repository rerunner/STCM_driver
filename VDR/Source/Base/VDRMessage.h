
//
// PURPOSE:		Base Message Interface Implementation
//

/*! \file
	\brief Base Message Interface. Defines functions to register Message Sinks at various driver components.
*/

#ifndef VDRMESSAGE_H
#define VDRMESSAGE_H

#include "STF/Interface/Types/STFMessage.h"
#include "VDR/Interface/Base/IVDRMessage.h"
#include "VDR/Source/Base/VDRBase.h"


//! Container carrying a registered STFMessageSink
/*!
	Also carries the concerned message (and perhaps messages range)
*/
class RegisteredMessageSink
	{
	public:
		STFMessageSink	*	sink;			//! The sink which will receive the messages
		uint32				message;		//! Message ID to be filtered - valid if range is != 0
		uint32				range;		//! Range of messages to filter for this sink, starting at "message"

		RegisteredMessageSink(STFMessageSink * sink, uint32 message, uint32 range)
			{
			this->sink		= sink;
			this->message	= message;
			this->range		= range;
			}
	};

typedef RegisteredMessageSink * RegisteredMessageSinkPtr;


class MessageSinkRegistration : public virtual IVDRBase,
										  public virtual IVDRMessageSinkRegistration,
										  public DispatchedSTFMessageSink
	{
	protected:
		STFMutex		sinkListLock;		//! Mutex protecting the registeredSinks, numSinks and totalSinks variables

		RegisteredMessageSinkPtr *	registeredSinks;	//! List of registered message sinks

		int	numSinks;	//! Number of sinks in the list
		int	totalSinks;	//! Maximum sinks possible in the list

		//! Overload from STFMessageSink
		/*!
			Distributes messages arriving from the dispatcher to the registered
			message sinks.
		*/
		STFResult ReceiveMessage(STFMessage & message);

		STFResult SetDispatcher(STFMessageDispatcher * dispatcher);
	public:
		//! Message Sink Registration Constructor
		/*!
			\param dispatcher is the message dispatcher that provides the messages
			to be forwarded to the registered message sinks.
		*/
		MessageSinkRegistration(STFMessageDispatcher * dispatcher);

		virtual ~MessageSinkRegistration();

		//
		// IVDRMessageSinkRegistration functions
		//
		virtual STFResult RegisterMessageSink(STFMessageSink * sink);
		virtual STFResult RegisterFilteredMessageSink(STFMessageSink * sink, uint32 message, uint32 range);
		virtual STFResult UnregisterMessageSink(STFMessageSink * sink);

		//
		// IVDRBase functions
		//
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);
	};



/// @brief Message collector class implementation
///
/// The message collector class implement the IVDRMessageCollector interface. It can manage a 
/// number of message sink registrations. The maximum amount of message sink 
class MessageCollector : public virtual IVDRMessageCollector, 
                         public MessageSinkRegistration
	{
	
	class MessageSinkRegistrationInformation
		{
		friend class MessageCollector;
		protected:
			bool valid;
			IVDRMessageSinkRegistration * sinkRegistration;
		};

	private:	
		
		/// @brief The maximum number of elements in registeredMessageSinkRegistrationsTable	
		uint32 maximumNumberOfHandledRegistrations;				

		/// @brief The table of registered message sink registrations.
		MessageSinkRegistrationInformation * registeredMessageSinkRegistrationsTable;

		/// @brief Find the index of the next free element in the table of registered
		/// message registrations
		///
		/// @param indexOfFreeElement [out] The index of a free table entry
		/// @return Return value, indicating success or failure
		/// @retval STFRES_OK               Success. indexOfFreeElement is valid. 
		/// @retval STFRES_OBJECT_NOT_FOUND	No free entry found. indexOfFreeElement is invalid
		STFResult FindNextFreeElementInRegistrationTable(uint32 & indexOfFreeElement);
		
		/// @brief Find the index of the entry refering to a registered message sink registration
		///
		/// @param sinkRegistration [in]    The message sink registration pointer to search for
		/// @param indexOfFoundElement[out] The index of the element found (if any)
		/// @return  Return value indicating success or failure
		/// @retval  STFRES_OK					  Success. indexOfFoundElement is valid.
		/// @retval  STFRES_OBJECT_NOT_FOUND  No object refering to this sink registration could be found.
		///                                   indexOfFoundElement is invalid (0xffffffff)
		STFResult FindEntryByMessageSinkRegistration(IVDRMessageSinkRegistration * sinkRegistration, uint32 & indexOfFoundElement);
	
	public:
		MessageCollector(STFMessageDispatcher * dispatcher);
		virtual ~MessageCollector();

		/// @brief Initialize the message collector
		///
		/// This method initializes the message collector with a maximum number of 
		/// message sink registrations that are allowed to register.
		/// @param maxMessageSinkRegistrations [in] The maximum number of message sink registrations to register
		///                                         0 < maxMessageSinkRegistrations < 0xffffffff
		/// @return Result value indicating success or failure
		/// @retval STFRES_OK Succes.
		/// @retval STFRES_INVALID_PARAMETERS maxMessageSinkRegistrations is invalid
		/// @retval STFRES_NOT_ENOUGH_MEMORY  not enough memory to allocate the necessary structures.
		STFResult Initialize(uint64 maxMessageSinkRegistrations);

		/// @brief Clean up the message collector 
		/// 
		/// This method is used to cleanup the message collector. It is only allowed to call this function
		/// after a successfull call to initialize.
		/// @return STFRES_NOT_ENOUGH_MEMORY
		/// @retval STFRES_OK Success.
		STFResult Cleanup();

		//
		// IVDRBase functions
		//
		virtual STFResult QueryInterface(VDRIID iid, void * & ifp);

		//
		// IVDRMessageCollector functions
		//
		virtual STFResult RegisterMessageSinkRegistration(IVDRMessageSinkRegistration * sinkRegistration);
		virtual STFResult UnregisterMessageSinkRegistration(IVDRMessageSinkRegistration * sinkRegistration);
	};


#endif	// #ifndef IVDRMESSAGE_H
