#include "debug.h"
#include "crash.h"

#if defined(_WINDOWS) && defined(CRASH_LOGGING)
#include "StackWalker.h"

class EQEmuStackWalker : public StackWalker
{
public:
	EQEmuStackWalker() : StackWalker() { }
	EQEmuStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) { }
	virtual void OnOutput(LPCSTR szText) {
		char buffer[4096];
		for(int i = 0; i < 4096; ++i) {
			if(szText[i] == 0) {
				buffer[i] = '\0';
				break;
			}

			if(szText[i] == '\n' || szText[i] == '\r') {
				buffer[i] = ' ';
			} else {
				buffer[i] = szText[i];
			}
		}

		LogFile->write(EQEMuLog::Crash, buffer);
		StackWalker::OnOutput(szText);
	}
};

LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS *ExceptionInfo)
{
	switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_ACCESS_VIOLATION");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
			break;
		case EXCEPTION_BREAKPOINT:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_BREAKPOINT");
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_DATATYPE_MISALIGNMENT");
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_FLT_DENORMAL_OPERAND");
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_FLT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_FLT_INEXACT_RESULT");
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_FLT_INVALID_OPERATION");
			break;
		case EXCEPTION_FLT_OVERFLOW:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_FLT_OVERFLOW");
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_FLT_STACK_CHECK");
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_FLT_UNDERFLOW");
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_ILLEGAL_INSTRUCTION");
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_IN_PAGE_ERROR");
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_INT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_INT_OVERFLOW:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_INT_OVERFLOW");
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_INVALID_DISPOSITION");
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_NONCONTINUABLE_EXCEPTION");
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_PRIV_INSTRUCTION");
			break;
		case EXCEPTION_SINGLE_STEP:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_SINGLE_STEP");
			break;
		case EXCEPTION_STACK_OVERFLOW:
			LogFile->write(EQEMuLog::Crash, "EXCEPTION_STACK_OVERFLOW");
			break;
		default:
			LogFile->write(EQEMuLog::Crash, "Unknown Exception");
			break;
	}

	if(EXCEPTION_STACK_OVERFLOW != ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
		EQEmuStackWalker sw; sw.ShowCallstack(GetCurrentThread(), ExceptionInfo->ContextRecord);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void set_exception_handler() {
	SetUnhandledExceptionFilter(windows_exception_handler);
}
#else
// crash is off or an unhandled platform
void set_exception_handler() {
}
#endif
