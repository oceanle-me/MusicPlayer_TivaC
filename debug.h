////debug.h
#ifndef	__DEBUG_H
#define	__DEBUG_H

#define	DEBUG

#ifdef DEBUG
	#define DBG(fmt,...) UARTprintf("%s:  %s:  %d: "fmt, __FILE__, __FUNCTION__, __LINE__,##__VA_ARGS__) 
#else
  #define DBG(fmt, args...)
#endif
#endif

