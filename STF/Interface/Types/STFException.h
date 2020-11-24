///
/// @brief    : 
///

// These exceptions are from the old prelude.h header file. They are used at least in the bullet software decoder.

#ifndef STFEXCEPTION_INC
#define STFEXCEPTION_INC

class STFException {};
class STFObjectInUseException : public STFException {};
class STFRangeViolationException : public STFException {};
class STFObjectNotFoundException : public STFException {};
class STFFileNotFoundException : public STFException {};
class STFEndOfFileException : public STFException {};

#endif // STFEXCEPTION_INC
