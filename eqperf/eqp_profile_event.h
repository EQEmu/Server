#pragma once

#include <string>
#include <stdint.h>
#include "eqp_profile_function.h"

namespace EQP
{
	namespace CPU
	{
		namespace ST
		{
			class EQP_EXPORT Event
			{
			public:
				Event(const char *function_name);
				Event(const char *function_name, const char *name);
				~Event();
			private:
				const char *function_name_;
				const char *name_;
				uint64_t start_;
				std::string identifier_;
			};
		}

		namespace MT
		{
			class EQP_EXPORT Event
			{
			public:
				Event(const char *function_name);
				Event(const char *function_name, const char *name);
				~Event();
			private:
				const char *function_name_;
				const char *name_;
				uint64_t start_;
				std::string identifier_;
			};
		}
	}
} // Profile

