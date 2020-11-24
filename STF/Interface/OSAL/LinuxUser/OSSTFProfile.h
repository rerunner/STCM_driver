///
/// @brief      Linux PC User Mode specific classes for configuration (INI) files.
///

#ifndef OSSTFPROFILE_H
#define OSSTFPROFILE_H

#include "STF/Interface/Types/STFString.h"


class STFProfile;

///
/// @class OSSTFProfile
///
/// @brief Entry for the most-used standard profile.
///
/// Profiles get written into the registry.
/// See explanation of STFGenericProfile for more info. 

class OSSTFProfile
	{
	private:

		bool				level;
		STFString	iniFileName;
		STFString	section;

	public:
		/// @name Constructors
      //@{
		OSSTFProfile(STFString name);
		OSSTFProfile(STFProfile * parent, STFString name);
		OSSTFProfile(OSSTFProfile * parent, STFString name);
		OSSTFProfile(STFString main, STFString section);
		OSSTFProfile(STFString vendor, STFString product, STFString version);
      //@}

		~OSSTFProfile(void);

		/// @name ReadDirect / WriteDirect
      ///
		/// XXXDirect methods write directly to the absolute location given through "entry".
		/// See explanation of STFGenericProfile for more info.
      //@{
		STFResult WriteDirect(STFString entry, int32 value);
		STFResult ReadDirect(STFString entry, int32 & value, int32 deflt);
		
		STFResult WriteDirect(STFString entry, bool value);
		STFResult ReadDirect(STFString entry, bool & value, bool deflt);
		
		STFResult WriteDirect(STFString entry, uint16 value, int32 base);
		STFResult ReadDirect(STFString entry, uint16 & value, int32 base, uint16 deflt);
		
		STFResult WriteDirect(STFString entry, uint32 value, int32 base);
		STFResult ReadDirect(STFString entry, uint32 & value, int32 base, uint32 deflt);
		
		STFResult WriteDirect(STFString entry, STFString value);
		STFResult ReadDirect(STFString entry, STFString & value, STFString deflt);
      //@}
		
      /// @name Read / Write
      ///
		/// These Write/Read functions use "section" as relative path, starting at
		/// the location of "this" object.
      /// See explanation of STFGenericProfile for more info.
      //@{
		STFResult Write(STFString section, STFString entry, int32 value);
		STFResult Read(STFString section, STFString entry, int32 & value, int32 deflt);
		
		STFResult Write(STFString section, STFString entry, bool value);
		STFResult Read(STFString section, STFString entry, bool & value, bool deflt);
		
		STFResult Write(STFString section, STFString entry, uint32 value, int32 base = 10);
		STFResult Read(STFString section, STFString entry, uint32 & value, int32 base, uint32 deflt);
		
		STFResult Write(STFString section, STFString entry, uint16 value, int32 base);
		STFResult Read(STFString section, STFString entry, uint16 & value, int32 base, uint16 deflt);
		
		STFResult Write(STFString section, STFString entry, STFString value);
		STFResult Read(STFString section, STFString entry, STFString & value, STFString deflt);
      //@}
		};


#endif
