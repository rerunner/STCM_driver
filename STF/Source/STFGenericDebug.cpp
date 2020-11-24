#if _DEBUG // this module is unnecessary for non-debug builds

///
/// @brief	This module contains generic debugging functions for all platforms
///

#include "STF/Interface/STFGenericDebug.h"

#define PRINT_BYTE(x)	((x >= ' ' && x <= '~') ? x : '.')	// used by DebugPrintDump()

//
// DebugPrintDump is a global function accessed by the macro DPDUMP.
// 
// It outputs in the standard hex dump format, using DP:
// 
// 0000  FF FC E4 00 B9 36 55 55  33 44 22 22 22 22 22 22  
// 0010  22 49 24 92 49 24 80 40  00 00 00 00 A2 00 FA AA  
// 0020  AA AA FA 5A AA 9B 76 27  E5 82 36 A2 95 B8 DE 6E  
// 0030  28 1B 91 E8 DF 92 49 22  8E 28 A4 92 59 A5 96 49 
//
// Parameters:
//  "bp" is the buffer to dump from
//  "len" is how many bytes to dump
//  "maxlen" is an upper limit that can restrict the number of bytes to
//           dump if (len > maxlen).  If maxlen is 0, it has no effect.
//  "dumpASCII" will add the ASCII chars to the right if true (or not if false)
//  "startAddr" allows that starting address to be set manually
//

void DebugPrintDump(unsigned char *bp, int len, int maxlen, bool dumpASCII, uint32 startAddr)
	{
	int		i;
	int		count, ps;
	char		printStr[48];

	if (maxlen == 0)
		count = len;
	else
		count = (len > maxlen) ? maxlen : len;

	// DP("addr  *0-*1-*2-*3-*4-*5-*6-*7--*8-*9-*A-*B-*C-*D-*E-*F\n");
	for (i=0, ps = 0; i<count; i++)
		{
		if (i % 16 == 0)
		   DP("%04X  ", i + startAddr);
		DP("%02X ", (int) bp[i]);
		if (i % 8 == 7)
		   DP(" ");
		if (dumpASCII)
			sprintf(&printStr[ps++],"%c", PRINT_BYTE(bp[i]) );
		if (i % 16 == 15)
			{
			if (dumpASCII)
				{
				DP("%s\n", printStr);
				ps = 0;
				}
			else
				DP("\n");
			}
		}

	if (ps > 0)
		{
		if (dumpASCII)
			DP("%s\n", printStr);
		else
			DP("\n");
		}

	DP("\n");
	}

//
// class DebugMultiCapturePort
// 


STFResult DebugMultiCapturePort::Initialize(int maxFileTypes)
	{
	if (pCapture)
		Cleanup();
	pCapture = new DebugCapturePortParams[maxFileTypes];
	if (pCapture == NULL)
		{
		DP("ERROR: DebugMultiCapturePort::Initialize(%d) failed, out of memory allocating %d bytes\n", maxFileTypes, maxFileTypes * sizeof(DebugCapturePortParams));
		numCaptureFiles = 0;
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
		}
	(void) memset(pCapture, 0, maxFileTypes * sizeof(DebugCapturePortParams));
	numCaptureFiles = maxFileTypes;

	STFRES_RAISE_OK;
	}


void DebugMultiCapturePort::Cleanup(void)
	{
	if (pCapture)
		{
		int i;

		for (i=0; i<numCaptureFiles; i++)
			{
			if (pCapture[i].captureBuffer)
				delete [] pCapture[i].captureBuffer;
			(void) CloseCaptureFile(i);
			}

		delete [] pCapture;
		}
	pCapture = NULL;
	}


void DebugMultiCapturePort::SetBytesToCapture(int fileTypeIndex, int count)
	{
	bool	deleteBuffer = false;

// DP("this=0x%08X SetBytesToCapture(%d,%d)\n", (int) this, fileTypeIndex, count);
	pCapture[fileTypeIndex].bytesToCapture = count;
	pCapture[fileTypeIndex].bytesCaptured = 0;
	pCapture[fileTypeIndex].captureEnabled = true;
	if (count && (count <= CAPTURE_FILE_MAX_MEMORY_BUFFER_SIZE))
		{
		if (pCapture[fileTypeIndex].captureBuffer)
			{
			if (pCapture[fileTypeIndex].captureBufferSize < count)
				deleteBuffer = true;
			// else
			//		reuseBuffer = true; // implied
			}
		else
			{
			pCapture[fileTypeIndex].captureBuffer = new unsigned char [CAPTURE_FILE_MAX_MEMORY_BUFFER_SIZE];
			pCapture[fileTypeIndex].captureBufferSize = CAPTURE_FILE_MAX_MEMORY_BUFFER_SIZE;
			}
		}
	else
		deleteBuffer = true;
	
	if (deleteBuffer && pCapture[fileTypeIndex].captureBuffer)
		{
		delete [] pCapture[fileTypeIndex].captureBuffer;
		pCapture[fileTypeIndex].captureBuffer = NULL;
		}

	pCapture[fileTypeIndex].captureBufferByteCount = 0;
	}

void DebugMultiCapturePort::SetCaptureFileBaseName(int fileTypeIndex, char *basename)
	{
// DP("this=0x%08X SetCaptureFileBaseName(%d,\"%s\")\n", (int) this, fileTypeIndex, basename);
	strncpy(pCapture[fileTypeIndex].captureFileBaseName,basename,CAPTURE_FILE_BASENAME_MAXLEN-1);
	pCapture[fileTypeIndex].captureFileBaseName[CAPTURE_FILE_BASENAME_MAXLEN-1] = '\0';// Nico, was = NULL;
	}

void DebugMultiCapturePort::SetCaptureToStdout(int fileTypeIndex, bool b)
	{
	pCapture[fileTypeIndex].captureToStdout = b;
	}

STFResult DebugMultiCapturePort::OpenCaptureFile(int fileTypeIndex)
	{
	pCapture[fileTypeIndex].captureFileCount += 1;
	sprintf(pCapture[fileTypeIndex].captureFileName,"%s_%03d.dat", 
			  pCapture[fileTypeIndex].captureFileBaseName, pCapture[fileTypeIndex].captureFileCount);
	// the filename is still used with stdout capture to identify the stream
	if (!pCapture[fileTypeIndex].captureToStdout)
		{
		if (pCapture[fileTypeIndex].captureFile != (FILE *) NULL)
			(void) CloseCaptureFile(fileTypeIndex);
		pCapture[fileTypeIndex].captureFile = fopen(pCapture[fileTypeIndex].captureFileName,"wb");
		if (pCapture[fileTypeIndex].captureFile == (FILE *) NULL)
			{
			DP("ERROR: Could not open capture file \"%s\" for output\n", pCapture[fileTypeIndex].captureFileName);
			STFRES_RAISE(STFRES_FILE_WRITE_ERROR);
			}
		DP("Opened capture file \"%s\" for output\n", pCapture[fileTypeIndex].captureFileName);
		pCapture[fileTypeIndex].captureEnabled = true;
		}
	else
		{
		DP("Beginning stdout capture of \"%s\"\n", pCapture[fileTypeIndex].captureFileName);
		pCapture[fileTypeIndex].captureEnabled = true;
		}

	STFRES_RAISE_OK;
	}


bool DebugMultiCapturePort::CaptureFileIsOpen(int fileTypeIndex)
	{
	if ((pCapture[fileTypeIndex].captureEnabled) &&
		 ((pCapture[fileTypeIndex].captureFile != (FILE *) NULL) ||
		  (pCapture[fileTypeIndex].captureToStdout)))
		return true;
	else
		return false;
	}


STFResult DebugMultiCapturePort::WriteToCaptureFile(int fileTypeIndex, unsigned char *buffer, int count)
 	{
// DP("this=0x%08X (%s) WriteToCaptureFile(%d,0x%08X,%d)\n", (int) this, pCapture[fileTypeIndex].captureFileBaseName, fileTypeIndex, (int) buffer, count);
	if (pCapture[fileTypeIndex].captureEnabled)
		{
		bool			closeCaptureAfterThisWrite;

		if (pCapture[fileTypeIndex].bytesToCapture == 0)
			STFRES_RAISE(CloseCaptureFile(fileTypeIndex));

		if ((count + pCapture[fileTypeIndex].bytesCaptured) >= pCapture[fileTypeIndex].bytesToCapture)
			{
			count -= ((count + pCapture[fileTypeIndex].bytesCaptured) - pCapture[fileTypeIndex].bytesToCapture);
			closeCaptureAfterThisWrite = true;
			}
		else
			closeCaptureAfterThisWrite = false;

		pCapture[fileTypeIndex].bytesCaptured += count;

		if (!pCapture[fileTypeIndex].captureToStdout)
			{
			int				written = 0;
			unsigned char	*p, *endP;
			FILE				*fp;
			bool				deferWrite = false;

			if (!CaptureFileIsOpen(fileTypeIndex))
				STFRES_REASSERT(OpenCaptureFile(fileTypeIndex));

			fp = pCapture[fileTypeIndex].captureFile;
			p = (unsigned char *) buffer;
			endP = &buffer[count];

			// while (p != endP && EOF != fputc((int) *(p++), fp))
			//	;

			if (pCapture[fileTypeIndex].captureBuffer && 
				 ((count + pCapture[fileTypeIndex].captureBufferByteCount) <= pCapture[fileTypeIndex].captureBufferSize))
				{
				memcpy(&pCapture[fileTypeIndex].captureBuffer[pCapture[fileTypeIndex].captureBufferByteCount],buffer,count);
				pCapture[fileTypeIndex].captureBufferByteCount += count;
				if (pCapture[fileTypeIndex].captureBufferByteCount == pCapture[fileTypeIndex].captureBufferSize)
					{
					written = fwrite(pCapture[fileTypeIndex].captureBuffer, 1, pCapture[fileTypeIndex].captureBufferByteCount, fp);
					pCapture[fileTypeIndex].captureBufferByteCount = 0;
					}
				else
					deferWrite = true;
				}
			else
				written = (int) fwrite(buffer, 1, count, fp);

			// if (p != endP)
			if ((written != (int)(size_t) count) && (!deferWrite))
				{
				DP("ERROR writing to capture file \"%s\"\n", pCapture[fileTypeIndex].captureFileName);
				CloseCaptureFile(fileTypeIndex);
				STFRES_RAISE(STFRES_FILE_WRITE_ERROR);
				}
			}
		else
			{
			DP("Dumping %d bytes of buffer 0x%08X (%s)\n", count, (int) buffer, pCapture[fileTypeIndex].captureFileName);
			DPDUMP(buffer, count);
			}

		if (closeCaptureAfterThisWrite)
			CloseCaptureFile(fileTypeIndex);
		}

	STFRES_RAISE_OK;
	}

STFResult DebugMultiCapturePort::CloseCaptureFile(int fileTypeIndex)
	{
	if ((!pCapture[fileTypeIndex].captureToStdout) &&
		 (pCapture[fileTypeIndex].captureFile))
		{
		if (pCapture[fileTypeIndex].captureBuffer && 
			 pCapture[fileTypeIndex].captureBufferByteCount)
			{
			int			written;

			written = (int) fwrite(pCapture[fileTypeIndex].captureBuffer, 1, pCapture[fileTypeIndex].captureBufferByteCount, 
				pCapture[fileTypeIndex].captureFile);
			if (written != pCapture[fileTypeIndex].captureBufferByteCount)
				DP("ERROR writing to capture file \"%s\"\n", pCapture[fileTypeIndex].captureFileName);
			pCapture[fileTypeIndex].captureBufferByteCount = 0;
			}
		fclose(pCapture[fileTypeIndex].captureFile);
		DP("%d bytes captured to \"%s\"\n", pCapture[fileTypeIndex].bytesCaptured, pCapture[fileTypeIndex].captureFileName);
		pCapture[fileTypeIndex].captureFile = NULL;
		}

	pCapture[fileTypeIndex].captureEnabled = false;

	STFRES_RAISE_OK;
	}


void DebugMultiCapturePort::RestartCapture(int fileTypeIndex)
	{
	if (CaptureFileIsOpen(fileTypeIndex))
		(void) CloseCaptureFile(fileTypeIndex);

	if (pCapture[fileTypeIndex].bytesToCapture)
		{
		pCapture[fileTypeIndex].captureEnabled = true;
		pCapture[fileTypeIndex].bytesCaptured = 0;
		}
	}

#endif // _DEBUG
