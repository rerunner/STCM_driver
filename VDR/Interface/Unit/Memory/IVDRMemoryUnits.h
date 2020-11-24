///
/// @brief      VDR public memory unit IDs.
///

#ifndef IVDR_MEMORYUNITS_H
#define IVDR_MEMORYUNITS_H

#include "VDR/Interface/Unit/IVDRGlobalUnitIDs.h"



/// A generic pool allocator without special features, usable by VDR clients.
/// Note that VDR clients can create several virtual units of this.
/// Note that the variable type for QueryInterface() must be exactly IVDRMemoryPoolAllocator.
static const VDRUID VDRUID_GENERIC_PLAYBACK_POOL_ALLOCATOR = 0x00000022;   // created by the ID value manager

/// A pool allocator usable by VDR clients and intended to reside in a partition shared by LX and Host.
static const VDRUID VDRUID_SHARED_LX_LINEAR_MEMORY_POOL = 0x00000079;

//
// Specific memory pools for VBI application use
//
/// VPS/PDC information
static const VDRUID VDRUID_VPSPDC_MEMORY_POOL = 0x0000004e;
/// Gemstar information
static const VDRUID VDRUID_GEMSTAR_MEMORY_POOL = 0x0000004f;
/// Teletext information
static const VDRUID VDRUID_TELETEXT_MEMORY_POOL = 0x00000050;
/// Teletext Page information
static const VDRUID VDRUID_TELETEXT_PAGE_MEMORY_POOL = 0x00000051;


#endif
