///
/// @brief Interface for configuration profiles.
///
/// The STFProfile classes are used to store configuration data in a tree-like
/// structure.
/// Where the data is stored is masked through the IVDRNonVolatileMemory interface; 
/// You just have to create the profile with a data range and it's size, then you can use
/// the public methods to store or retrieve key/entry/value triplets.
/// To write back the profile data you can retrieve teh complete data range and again use 
/// the IVDRNonVolatileMemory interface.
/// 

#ifndef STFPROFILE_H
#define STFPROFILE_H

#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFString.h"
#include "STF/Interface/STFDataManipulationMacros.h"
#include "STF/Interface/STFMutex.h"
#include "STF/Interface/STFDebug.h"



/// @name Errorcodes
///
/// Webtool generated errorcodes
/// Currently not in use!!!
//@{
static const STFResult STFRES_PROFILE_WRITE		= 0x85000001;
static const STFResult STFRES_PROFILE_READ		= 0x85000002;
static const STFResult STFRES_PROFILE_NO_LEVEL	= 0x85000003;
//@}

///
/// @class STFGenericProfile
///
/// @brief this serves as a base class for our profiles
///
class STFGenericProfile
	{
	protected:
		uint8			*	dataContainer;
		uint32			size;

	protected:
		/// @brief Empty constructor
		STFGenericProfile (void);

	public:
		/// @brief Construction of the base class
		///
		/// @param size	The size of the data block
		/// @param data	the data block which contains the flattened profile tree.
		STFGenericProfile (uint32 size, uint8 * data);

		/// @brief The destructor.
		virtual ~STFGenericProfile (void);

	public:
		/// @brief Retrieve the cached data block and it's size.
		///
		/// When writing the profile back to some memory this method can be used to retrieve the
		/// cached data block where the profile is stored as a flattend tree structure and it's size.
		///
		/// @param size	the size of the cached data block.
		/// @param data	a pointer to the cached data block.
		/// @return			A STFResult value.
		STFResult GetDataBlock	(uint32 & size, uint8 *& data);

	public:
		/// @name Virtual Read / Write
		/// These methods write to the location given in the 2 parameters key and name,
		/// where key is the absolute path name and name is the entry name.
		/// Value is the value to be read / written.
		/// "deflt", where used, is the value to return from a Read function when
		/// the desired entry does not exist. If in such a case no parameter for
		/// "deflt" is passed, the value of "value" is not changed.
		///
		/// @param key		Absolute path name where the entry is located. Path devider must be "/".
		/// @param name	The entry name where the value is loacated.
		/// @param value	Data container which (will) contain the value
		/// @param deflt	Default value that is returned as @c value by Read if nothing is found 
		///					at @c entry
		/// @return A STFResult value.
		// @{
		virtual STFResult Write (STFString key, STFString name, int32 value) = 0;
		virtual STFResult Read  (STFString key, STFString name, int32 & value, int32 deflt) = 0;
		
		virtual STFResult Write (STFString key, STFString name, bool value) = 0;
		virtual STFResult Read  (STFString key, STFString name, bool & value, bool deflt)= 0;
		
		virtual STFResult Write (STFString key, STFString name, uint32 value) = 0;
		virtual STFResult Read  (STFString key, STFString name, uint32 & value, uint32 deflt) = 0;
		
		virtual STFResult Write (STFString key, STFString name, STFString value) = 0;
		virtual STFResult Read  (STFString key, STFString name, STFString & value, STFString deflt) = 0;
		//@}
	};

class STFProfile : public STFGenericProfile
	{
	// This is needed to enable the testing of protected methods. May be deleted when only publics are tested.
	friend class STFProfileTest;

	/*
	protected:
		STFString		parentKey;
		STFProfile	*	parentProfile;
	*/

	protected:
		/// @brief Empty contructor
		STFProfile (void);

	public:
		/// @brief Construction of the profile class
		///
		/// @param size	The size of the data block
		/// @param data	the data block which contains the flattened profile tree.
		STFProfile (uint32 size, uint8 * data);

		/// @brief The destructor.
		virtual ~STFProfile (void);

		/*
	protected:
		STFProfile (STFString key, STFProfile * root);
		
	public:
		STFResult Initialize (void);
		*/

	public:
		//STFProfile* GetSubProfile	(STFString key);
		
	public:
		/// @name Implementation for Read / Write
		/// These methods write to the location given in the 2 parameters key and name,
		/// where key is the absolute path name and name is the entry name.
		/// Value is the value to be read / written.
		/// "deflt", where used, is the value to return from a Read function when
		/// the desired entry does not exist. If in such a case no parameter for
		/// "deflt" is passed, the value of "value" is not changed.
		///
		/// @param key		Absolute path name where the entry is located. Path devider must be "/".
		/// @param name	The entry name where the value is loacated.
		/// @param value	Data container which (will) contain the value
		/// @param deflt	Default value that is returned as @c value by Readif nothing is found 
		///					at @c entry
		/// @return A STFResult value.
		// @{
		virtual STFResult Write (STFString key, STFString name, int32 value);
		virtual STFResult Read  (STFString key, STFString name, int32 & value, int32 deflt);

		virtual STFResult Write (STFString key, STFString name, bool value);
		virtual STFResult Read  (STFString key, STFString name, bool & value, bool deflt);

		virtual STFResult Write (STFString key, STFString name, uint32 value);
		virtual STFResult Read  (STFString key, STFString name, uint32 & value, uint32 deflt);

		virtual STFResult Write (STFString key, STFString name, STFString value);
		virtual STFResult Read  (STFString key, STFString name, STFString & value, STFString deflt);
		// @}

	protected:
		STFResult GetKeyStructure				(STFString key, uint8 *& nodes, uint8 & totalNum);
		STFResult GetKeyElement					(STFString key, uint8 * nodes, uint8 num, STFString& element);

		STFResult FindKey							(STFString key, uint32 & position);
		STFResult FindEntryAt					(STFString name, uint32 & position);
		STFResult FindEntry						(STFString key, STFString name, uint32 & position);

		STFResult IsChildOf						(STFString key, uint32 possiblePosition, bool & isCh);

		STFResult AdjustNextSiblingOffset	(STFString key, uint32 addSize);

		STFResult InsertNodeAt					(STFString key, STFString name);
		STFResult InsertSEntryAt				(STFString key, STFString name, STFString value);
		STFResult InsertIEntryAt				(STFString key, STFString name, uint32 value);

		STFResult ChangeSEntryAt				(STFString key, STFString name, STFString value);
		STFResult ChangeIEntryAt				(STFString key, STFString name, uint32 value);

		STFResult Read4Bytes						(uint32 position, uint32 & data);
		STFResult Write4Bytes					(uint32 position, uint32 data);
		STFResult WriteString					(uint32 position, STFString data, uint32 size);

		STFResult GetProfileUsageSize			(uint32 & size);

		STFResult GetElementSize				(uint32 position, uint32 & size);
		STFResult GetNameAt						(uint32 position, STFString & name);
		STFResult GetStringValueAt				(uint32 position, STFString & value);
		STFResult GetIntegerValueAt			(uint32 position, uint32 & value);
	};

class STFSynchronizedProfile : public STFProfile
	{
	protected:
		STFMutex mutex;

	protected:
		/// @brief Empty constructor
		STFSynchronizedProfile (void);

	public:
		/// @brief Construction of the profile class
		///
		/// @param size	The size of the data block
		/// @param data	the data block which contains the flattened profile tree.
		STFSynchronizedProfile (uint32 size, uint8 * data);

		/// @brief The destructor.
		virtual ~STFSynchronizedProfile (void);

	public:
		/// @name Synchronized implementation for Read / Write
		/// These methods write to the location given in the 2 parameters key and name,
		/// where key is the absolute path name and name is the entry name.
		/// Value is the value to be read / written.
		/// "deflt", where used, is the value to return from a Read function when
		/// the desired entry does not exist. If in such a case no parameter for
		/// "deflt" is passed, the value of "value" is not changed.
		///
		/// @param key		Absolute path name where the entry is located. Path devider must be "/".
		/// @param name	The entry name where the value is loacated.
		/// @param value	Data container which (will) contain the value
		/// @param deflt	Default value that is returned as @c value by Readif nothing is found 
		///					at @c entry
		/// @return A STFResult value.
		// @{
		virtual STFResult Write (STFString key, STFString name, int32 value);
		virtual STFResult Read  (STFString key, STFString name, int32 & value, int32 deflt);

		virtual STFResult Write (STFString key, STFString name, bool value);
		virtual STFResult Read  (STFString key, STFString name, bool & value, bool deflt);

		virtual STFResult Write (STFString key, STFString name, uint32 value);
		virtual STFResult Read  (STFString key, STFString name, uint32 & value, uint32 deflt);

		virtual STFResult Write (STFString key, STFString name, STFString value);
		virtual STFResult Read  (STFString key, STFString name, STFString & value, STFString deflt);
		// @}
	};

#endif //STFPROFILE_H
