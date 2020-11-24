//
// PURPOSE:   Default Tag Unit 
//

#ifndef TAGUNIT_H
#define TAGUNIT_H


#include "ITagUnit.h"


//
// Class: 
//		TagUnit
// Description:
//		Dummy implementation of ITagUnit interface that does nothing but returning
//		a STFRES_RAISE_OK. 
//		Can be used by classes that do not need a TagUnit implementation. 
//
class TagUnit : public ITagUnit
	{
	public:
		//
		// ITagUnit interface implementation
		//
		virtual STFResult GetTagIDs(uint32 * & ids);
		virtual STFResult InternalConfigureTags(TAG * tags);
		virtual STFResult	InternalUpdate(void);
	};


#endif // TAGUNIT_H

