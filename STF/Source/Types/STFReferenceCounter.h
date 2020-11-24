//
// PURPOSE:   
//

#ifndef STFREFCNT_H
#define STFREFCNT_H

class STFReferenceCounter
	{
	protected:
		int	useCnt;
		virtual void SelfDestruct(void) {delete this;}
	public:
		STFReferenceCounter(void) {useCnt = 1;}
		virtual ~STFReferenceCounter(void) {}
		void Obtain(void) {useCnt++;}
		void Release(void) {if (!(--useCnt)) SelfDestruct();}
	};

#endif //STFREFCNT_H
