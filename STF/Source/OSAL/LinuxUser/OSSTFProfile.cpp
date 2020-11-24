///
/// @brief      Linux PC User Mode specific classes for configuration (INI) files.
///

#include "STF/Interface/OSAL/OS20_OS21/OSSTFProfile.h"
#include "STF/Interface/STFDebug.h"
#include "STF/Interface/STFProfile.h"


///////////////////////////////////////////////////////////////////////////////
// This implementation is a dummy.
///////////////////////////////////////////////////////////////////////////////

//#include "embedded\boards\st5505eval\hwsetupinfo\hwsetupinfo.h"

OSSTFProfile::OSSTFProfile(STFString name)
	{
	level = false;
	iniFileName = name;
	section = "main";
	}

OSSTFProfile::OSSTFProfile(STFProfile * parent, STFString name)
	{
	level = true;
	iniFileName = parent->osp.iniFileName;

	if (parent->osp.level)
		section = parent->osp.section + "." + name;
	else
		section = name;
	}

OSSTFProfile::OSSTFProfile(OSSTFProfile * parent, STFString name)
	{
	level = true;
	iniFileName = parent->iniFileName;

	if (parent->level)
		section = parent->section + "." + name;
	else
		section = name;
	}

OSSTFProfile::OSSTFProfile(STFString main, STFString section)
	{
	level = true;
	iniFileName = main;
	this->section = section;
	}

OSSTFProfile::~OSSTFProfile(void)
	{
	}


STFResult OSSTFProfile::WriteDirect(STFString entry, int32 value)
	{
	// Use default section

	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, int32 & value, int32 deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::WriteDirect(STFString entry, uint32 value, int32 base)
	{
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, uint32  & value, int32 base, uint32 deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::WriteDirect(STFString entry, uint16 value, int32 base)
	{
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, uint16  & value, int32 base, uint16 deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}


STFResult OSSTFProfile::WriteDirect(STFString entry, STFString value)
	{
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, STFString & value, STFString deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}


STFResult OSSTFProfile::Write(STFString section, STFString entry, int32 value)
	{
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, int32  & value, int32 deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::Write(STFString section, STFString entry, uint32 value, int32 base)
	{
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, uint32  & value, int32 base, uint32 deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::Write(STFString section, STFString entry, uint16 value, int32 base)
	{
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, uint16  & value, int32 base, uint16 deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}


STFResult OSSTFProfile::Write(STFString section, STFString entry, STFString value)
	{
	STFRES_RAISE_OK;
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, STFString & value, STFString deflt)
	{
	value = deflt;
	STFRES_RAISE_OK;
	}


