///
/// @brief Implementation for Linux User Mode mutexes
///

#include "OSSTFMutex.h"

const pthread_mutex_t OSSTFMutex::mutexTemplate = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
const pthread_mutex_t OSSTFSharedMutex::lockTemplate = PTHREAD_MUTEX_INITIALIZER;
