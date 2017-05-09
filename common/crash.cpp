#include "global_define.h"
#include "eqemu_logsys.h"
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

		Log(Logs::General, Logs::Crash, buffer);
		StackWalker::OnOutput(szText);
	}
};

LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS *ExceptionInfo)
{
	switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			Log(Logs::General, Logs::Crash, "EXCEPTION_ACCESS_VIOLATION");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			Log(Logs::General, Logs::Crash, "EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
			break;
		case EXCEPTION_BREAKPOINT:
			Log(Logs::General, Logs::Crash, "EXCEPTION_BREAKPOINT");
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			Log(Logs::General, Logs::Crash, "EXCEPTION_DATATYPE_MISALIGNMENT");
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			Log(Logs::General, Logs::Crash, "EXCEPTION_FLT_DENORMAL_OPERAND");
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			Log(Logs::General, Logs::Crash, "EXCEPTION_FLT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			Log(Logs::General, Logs::Crash, "EXCEPTION_FLT_INEXACT_RESULT");
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			Log(Logs::General, Logs::Crash, "EXCEPTION_FLT_INVALID_OPERATION");
			break;
		case EXCEPTION_FLT_OVERFLOW:
			Log(Logs::General, Logs::Crash, "EXCEPTION_FLT_OVERFLOW");
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			Log(Logs::General, Logs::Crash, "EXCEPTION_FLT_STACK_CHECK");
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			Log(Logs::General, Logs::Crash, "EXCEPTION_FLT_UNDERFLOW");
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			Log(Logs::General, Logs::Crash, "EXCEPTION_ILLEGAL_INSTRUCTION");
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			Log(Logs::General, Logs::Crash, "EXCEPTION_IN_PAGE_ERROR");
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			Log(Logs::General, Logs::Crash, "EXCEPTION_INT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_INT_OVERFLOW:
			Log(Logs::General, Logs::Crash, "EXCEPTION_INT_OVERFLOW");
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			Log(Logs::General, Logs::Crash, "EXCEPTION_INVALID_DISPOSITION");
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			Log(Logs::General, Logs::Crash, "EXCEPTION_NONCONTINUABLE_EXCEPTION");
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			Log(Logs::General, Logs::Crash, "EXCEPTION_PRIV_INSTRUCTION");
			break;
		case EXCEPTION_SINGLE_STEP:
			Log(Logs::General, Logs::Crash, "EXCEPTION_SINGLE_STEP");
			break;
		case EXCEPTION_STACK_OVERFLOW:
			Log(Logs::General, Logs::Crash, "EXCEPTION_STACK_OVERFLOW");
			break;
		default:
			Log(Logs::General, Logs::Crash, "Unknown Exception");
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
