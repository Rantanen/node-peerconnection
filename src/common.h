#if !defined( COMMON_H )
#define COMMON_H

#if defined(TRACING)

#include <stdio.h>

#define TRACE(msg) printf("   TRACE: %s\n", msg)
#define TRACE_S(msg, s) printf("   TRACE: %s : %s\n", msg, s)
#define TRACE_I(msg, i) printf("   TRACE: %s : %d\n", msg, i)
#define TRACE_CALL printf("-> TRACE: Call::%s\n", __FUNCTION__)
#define TRACE_CALL_I(p1) printf("-> TRACE: Call::%s(%d)\n", __FUNCTION__, p1)
#define TRACE_END printf("<- Call::%s\n", __FUNCTION__)

#else

#define TRACE(msg)
#define TRACE_S(msg, s)
#define TRACE_I(msg, i)
#define TRACE_CALL
#define TRACE_CALL_I(p1)
#define TRACE_END

#endif

#define REQ_OBJ_ARG(I, VAR)                                      \
  if (args.Length() <= (I) || !args[I]->IsObject())            \
		return ThrowException(Exception::TypeError(              \
		  String::New("Argument " #I " must be an object")));   \
  Local<Object> VAR = Local<Object>::Cast(args[I]);

#define REQ_FUN_ARG(I, VAR)                                      \
  if (args.Length() <= (I) || !args[I]->IsFunction())            \
		return ThrowException(Exception::TypeError(              \
		  String::New("Argument " #I " must be a function")));   \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#endif
