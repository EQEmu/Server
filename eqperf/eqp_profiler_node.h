#pragma once

#include <unordered_map>
#include <string>
#include <stdint.h>

namespace EQP
{
	namespace CPU
	{
		class ProfilerNode
		{
		public:
			ProfilerNode();
			~ProfilerNode();

			inline void SetCount(uint64_t c) { count_ = c; }
			inline uint64_t& GetCount() { return count_; }

			inline void SetTime(uint64_t t) { time_ = t; }
			inline uint64_t& GetTime() { return time_; }

			inline void SetParent(ProfilerNode *p) { parent_ = p; }
			inline ProfilerNode* GetParent() { return parent_; }

			inline std::unordered_map<std::string, ProfilerNode*>& GetNodes() { return nodes_; }

			void Dump(std::ostream &stream, const std::string &func, uint64_t total_time, int node_level, int num);
		private:
			uint64_t count_;
			uint64_t time_;
			ProfilerNode *parent_;
			std::unordered_map<std::string, ProfilerNode*> nodes_;
		};

		struct ProfilerNodeDump
		{
			std::string name;
			ProfilerNode *node;
		};
	} // CPU
} // EQP

