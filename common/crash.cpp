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

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/fcntl.h>

#ifdef __FreeBSD__
#include <signal.h>
#include <sys/stat.h>
#endif

void print_trace()
{
	auto uid = geteuid();

	std::string temp_output_file = "/tmp/dump-output";

	char pid_buf[30];
	sprintf(pid_buf, "%d", getpid());
	char name_buf[512];
	name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
	int child_pid = fork();
	if (!child_pid) {
		int fd = open(temp_output_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

		dup2(fd, 1); // redirect output to stderr
		fprintf(stdout, "stack trace for %s pid=%s\n", name_buf, pid_buf);
		if (uid == 0) {
			execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
		}
		else {
			execlp("sudo", "gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
		}

		close(fd);

		abort(); /* If gdb failed to start */
	}
	else {
		waitpid(child_pid, NULL, 0);
	}

	std::ifstream    input(temp_output_file);
	for (std::string line; getline(input, line);) {
		LogCrash("{}", line);
	}

	std::remove(temp_output_file.c_str());

	exit(1);
}

// crash is off or an unhandled platform
void set_exception_handler()
{
	signal(SIGABRT, reinterpret_cast<void (*)(int)>(print_trace));
	signal(SIGFPE, reinterpret_cast<void (*)(int)>(print_trace));
	signal(SIGFPE, reinterpret_cast<void (*)(int)>(print_trace));
	signal(SIGSEGV, reinterpret_cast<void (*)(int)>(print_trace));
}
#endif
