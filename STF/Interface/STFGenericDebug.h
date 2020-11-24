#ifndef STFGENERICDEBUG_H
#define STFGENERICDEBUG_H
///
/// @brief	This module contains generic debugging functions for all platforms
///

#if _DEBUG // this module is unnecessary for non-debug builds

#include "STF/Interface/STFDebug.h"
#include <stdio.h>

//
// class DebugMultiCapturePort allows binary logging to one or more files
// By convention, if DebugMultiCapturePort::Initialize(2) is called, file 0
// is to be the input file, and file 1 is to be the output file.
//
// To see this class in use, look at "MMEFrameDecoder.h".  In there,
// VirtualMMEFrameDecoder has a DebugMultiCapturePort member and it
// implements its own member functions to simplify interfacing with 
// the input and output stream capture ports:
//
//	void DebugSetFrameCapture(int inputBytesToCapture, int outputBytesToCapture, 
//                           bool dumpToStdout = false)
//	STFResult DebugCaptureInput(void *buffer, int count)
// STFResult DebugCaptureOutput(void *buffer, int count)
//	bool DebugInputCaptureEnabled(void)
//	bool DebugOutputCaptureEnabled(void)
// void DebugRestartCapture(void)
// void DebugStopCapture(void)
// 

#define	CAPTURE_FILE_MAXLEN							128
#define	CAPTURE_FILE_BASENAME_MAXLEN				120
#define  CAPTURE_FILE_MAX_MEMORY_BUFFER_SIZE		8192 // 4096

class DebugMultiCapturePort
	{
	protected:
		struct DebugCapturePortParams
			{
			bool		captureEnabled;
			FILE	*	captureFile;
			int		captureFileCount; // can be more than one file in series (different filenames)
			int		bytesToCapture;
			int		bytesCaptured;
			bool		captureToStdout; // use DPDUMP instead of opening a file
			char		captureFileBaseName[CAPTURE_FILE_BASENAME_MAXLEN];
			char		captureFileName[CAPTURE_FILE_MAXLEN];
			unsigned char	*captureBuffer;
			int				captureBufferSize;
			int				captureBufferByteCount;
			} *pCapture;
		int	numCaptureFiles;

		void Cleanup(void);

	public:
		DebugMultiCapturePort(void) {	numCaptureFiles = 0;	pCapture = NULL; }
		~DebugMultiCapturePort(void)	{ Cleanup(); }

		STFResult Initialize(int maxFileTypes);

		void SetBytesToCapture(int fileTypeIndex, int count);
		void SetCaptureToStdout(int fileTypeIndex, bool b);
		void SetCaptureFileBaseName(int fileTypeIndex, char *basename);
		STFResult OpenCaptureFile(int fileTypeIndex);
		bool CaptureFileIsOpen(int fileTypeIndex);
		STFResult WriteToCaptureFile(int fileTypeIndex, unsigned char *buffer, int count);
		STFResult CloseCaptureFile(int fileTypeIndex);
		void RestartCapture(int fileTypeIndex);
		bool CaptureIsEnabled(int fileTypeIndex);
	};

inline bool DebugMultiCapturePort::CaptureIsEnabled(int fileTypeIndex)
	{	return ((pCapture) ? pCapture[fileTypeIndex].captureEnabled : false); }

#endif // _DEBUG // this module is unnecessary for non-debug builds

#endif // STFGENERICDEBUG_H
