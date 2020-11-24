///
/// @brief Class sources for configuration (INI) files (WIN32 specific).
///

#include "STF/Interface/OSAL/WIN32/OSSTFProfile.h"
#include "STF/Interface/STFDebug.h"
#include "STF/Interface/STFProfile.h"

////////////////////////////////
// Default Win32 Implementation 
////////////////////////////////


OSSTFProfile::OSSTFProfile(STFString name)
	{
	int32		status;
	DWORD 	dummy;

	status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
									"software\\CompanyName\\"+name,
									NULL,
									"",
									REG_OPTION_NON_VOLATILE,
									KEY_READ | KEY_WRITE,
									NULL,
									&key,
									&dummy);
	if (status != ERROR_SUCCESS) key = NULL;
	}

OSSTFProfile::OSSTFProfile(STFProfile * parent, STFString name)
	{
	int32		status;
	DWORD		dummy;

	status = RegCreateKeyEx(parent->osp.key,
									name,
									NULL,
									"",
									REG_OPTION_NON_VOLATILE,
									KEY_READ | KEY_WRITE,
									NULL,
									&key,
									&dummy);
	if (status != ERROR_SUCCESS) key = NULL;
	}

OSSTFProfile::OSSTFProfile(OSSTFProfile * parent, STFString name)
	{
	int32		status;
	DWORD		dummy;

	status = RegCreateKeyEx(parent->key,
									name,
									NULL,
									"",
									REG_OPTION_NON_VOLATILE,
									KEY_READ | KEY_WRITE,
									NULL,
									&key,
									&dummy);
	if (status != ERROR_SUCCESS) key = NULL;
	}

OSSTFProfile::OSSTFProfile(STFString main, STFString section)
	{
	int32		status;
	DWORD		dummy;

	status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
									"software\\CompanyName\\"+ main + "\\" + section,
									NULL,
									"",
									REG_OPTION_NON_VOLATILE,
									KEY_READ | KEY_WRITE,
									NULL,
									&key,
									&dummy);
	if (status != ERROR_SUCCESS) key = NULL;
	}

OSSTFProfile::OSSTFProfile(STFString vendor, STFString product, STFString version)
	{
	int32		status;
	DWORD	dummy;

	status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
									"software\\"+ 
										vendor + "\\" + 
										product + "\\" +
										version,
									NULL,
									"",
									REG_OPTION_NON_VOLATILE,
									KEY_READ | KEY_WRITE,
									NULL,
									&key,
									&dummy);
	if (status != ERROR_SUCCESS) key = NULL;
	}

OSSTFProfile::~OSSTFProfile(void)
	{
	if (key) RegCloseKey(key);
	}


STFResult OSSTFProfile::WriteDirect(STFString entry, int32 value)
	{
	int32 status;

	if (key)
		{
		DP("Write %s with %ld\n", (char *)entry, (uint32)value);

		if ((status = RegSetValueEx(key, entry, NULL, REG_DWORD, (uint8 *)&value, sizeof(value))) == ERROR_SUCCESS)
			STFRES_RAISE_OK;
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, int32 & value, int32 deflt)
	{
	unsigned long	type;
	uint8		buffer[4];
	unsigned long	size;

	if (key)
		{
		size = sizeof(buffer);
		if (RegQueryValueEx(key, entry, NULL, &type, buffer, &size) == ERROR_SUCCESS)
			{
			if (type == REG_DWORD)
				value = *(int32 *)&buffer;
			else
				STFRES_RAISE(STFRES_PROFILE_READ);
			}
		else
			value = deflt;

		STFRES_RAISE_OK;
		}
	else
		value = deflt;
	
	STFRES_RAISE(STFRES_PROFILE_READ);
	}

STFResult OSSTFProfile::WriteDirect(STFString entry, bool value)
	{
	return WriteDirect(entry, value ? 1 : 0);
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, bool & value, bool deflt)
	{
	int32 val;

	STFRES_REASSERT(ReadDirect(entry, val, deflt ? 1 : 0));

	value = val != 0;

	STFRES_RAISE_OK;
	}


STFResult OSSTFProfile::WriteDirect(STFString entry, uint32 value, int32 base)
	{
	int32 status;

	if (key)
		{
		DP("Write %s with %ld\n", (char *)entry, (uint32)value);

		if ((status = RegSetValueEx(key, entry, NULL, REG_DWORD, (uint8 *)&value, sizeof(value))) == ERROR_SUCCESS)
			STFRES_RAISE_OK;
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, uint32 & value, int32 base, uint32 deflt)
	{
	unsigned long	type;
	uint8		buffer[4];
	unsigned long	size;

	if (key)
		{
		size = sizeof(buffer);
		if (RegQueryValueEx(key, entry, NULL, &type, buffer, &size) == ERROR_SUCCESS)
			{
			if (type == REG_DWORD)
				value = *(uint32 *)&buffer;
			else
				STFRES_RAISE(STFRES_PROFILE_READ);
			}
		else
			value = deflt;

		STFRES_RAISE_OK;
		}
	else
		value = deflt;
	
	STFRES_RAISE(STFRES_PROFILE_READ);
	}

STFResult OSSTFProfile::WriteDirect(STFString entry, uint16 value, int32 base)
	{
	if (key)
		{
		uint32 val = value;

		DP("Write %s with %ld\n", (char *)entry, (uint32)value);

		if (RegSetValueEx(key, entry, NULL, REG_DWORD, (uint8 *)&val, sizeof(val)) == ERROR_SUCCESS)
			STFRES_RAISE_OK;
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, uint16 & value, int32 base, uint16 deflt)
	{
	unsigned long	type;
	uint8		buffer[4];
	unsigned long	size;

	if (key)
		{
		size = sizeof(buffer);
		if (RegQueryValueEx(key, entry, NULL, &type, buffer, &size) == ERROR_SUCCESS)
			{
			if (type == REG_DWORD)
				value = (uint16)*(uint32 *)&buffer;
			else
				STFRES_RAISE(STFRES_PROFILE_READ);
			}
		else
			value = deflt;

		STFRES_RAISE_OK;
		}
	else
		value = deflt;
	
	STFRES_RAISE(STFRES_PROFILE_READ);
	}

STFResult OSSTFProfile::WriteDirect(STFString entry, STFString value)
	{
	if (key)
		{
		DP("Write %s with %s\n", (char *)entry, (char *)value);

		if (RegSetValueEx(key, entry, NULL, REG_SZ, (uint8 *)(char *)value, value.Length()) == ERROR_SUCCESS)
			STFRES_RAISE_OK;
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::ReadDirect(STFString entry, STFString & value, STFString deflt)
	{
	unsigned long	type;
	uint8		buffer[256];
	unsigned long	size;

	if (key)
		{
		size = sizeof(buffer);
		if (RegQueryValueEx(key, entry, NULL, &type, buffer, &size) == ERROR_SUCCESS)
			{
			if (type == REG_SZ)
				value = STFString((char *)buffer);
			else
				STFRES_RAISE(STFRES_PROFILE_READ);
			}
		else
			value = deflt;

		STFRES_RAISE_OK;
		}
	else
		value = deflt;
	
	STFRES_RAISE(STFRES_PROFILE_READ);
	}







STFResult OSSTFProfile::Write(STFString section, STFString entry, int32 value)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.WriteDirect(entry, value);
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, int32 & value, int32 deflt)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.ReadDirect(entry, value, deflt);
		}
	else
		value = deflt;

	STFRES_RAISE(STFRES_PROFILE_READ);
	}

STFResult OSSTFProfile::Write(STFString section, STFString entry, bool value)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.WriteDirect(entry, value);
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, bool & value, bool deflt)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.ReadDirect(entry, value, deflt);
		}
	else
		value = deflt;

	STFRES_RAISE(STFRES_PROFILE_READ);
	}


STFResult OSSTFProfile::Write(STFString section, STFString entry, uint32 value, int32 base)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.WriteDirect(entry, value, base);
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, uint32 & value, int32 base, uint32 deflt)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.ReadDirect(entry, value, base, deflt);
		}
	else
		value = deflt;

	STFRES_RAISE(STFRES_PROFILE_READ);
	}

STFResult OSSTFProfile::Write(STFString section, STFString entry, uint16 value, int32 base)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.WriteDirect(entry, value, base);
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, uint16 & value, int32 base, uint16 deflt)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.ReadDirect(entry, value, base, deflt);
		}
	else
		value = deflt;

	STFRES_RAISE(STFRES_PROFILE_READ);
	}

STFResult OSSTFProfile::Write(STFString section, STFString entry, STFString value)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.WriteDirect(entry, value);
		}

	STFRES_RAISE(STFRES_PROFILE_WRITE);
	}

STFResult OSSTFProfile::Read(STFString section, STFString entry, STFString & value, STFString deflt)
	{
	if (key)
		{
		OSSTFProfile prof(this, section);
		return prof.ReadDirect(entry, value, deflt);
		}
	else
		value = deflt;

	STFRES_RAISE(STFRES_PROFILE_READ);
	}



