#include "eqp_profiler_node.h"
#include "eqp_profile_timer.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

EQP::CPU::ProfilerNode::ProfilerNode() {
	count_ = 0;
	time_ = 0;
	started_ = 0;
	parent_ = nullptr;;
}

EQP::CPU::ProfilerNode::~ProfilerNode() {
	for(auto &iter : nodes_) {
		delete iter.second;
	}
}

void EQP::CPU::ProfilerNode::Dump(std::ostream &stream, const std::string &func, uint64_t total_time, int node_level, int num) {

	if(node_level >= 1) {
		stream << std::setw(node_level * 4) << " ";
	}

	double m_cycles = static_cast<double>(time_);
	if(started_) {
		m_cycles += GetCurrentTimer() - started_;
	}

	double percentage = m_cycles * 100 / static_cast<double>(total_time);
	m_cycles = m_cycles / 1000.0;
	double m_avg_cycles = m_cycles / count_;

	std::streamsize p = stream.precision();

	stream << std::fixed;
	stream.precision(2);
	stream << m_cycles << "k cycles, " << count_ << " calls, " << m_avg_cycles << "k cycles avg, ";
	stream << func.c_str() << " ";
	stream << percentage << "%";
	stream << std::endl;
	stream.precision(p);

	std::vector<ProfilerNodeDump> sorted_vec;
	sorted_vec.reserve(nodes_.size() + 1);

	for(auto &iter : nodes_) {
		ProfilerNodeDump n;
		n.name = iter.first;
		n.node = iter.second;
		sorted_vec.push_back(n);
	}

	std::sort(sorted_vec.begin(), sorted_vec.end(), 
			  [](const ProfilerNodeDump& a, const ProfilerNodeDump& b) { return a.node->GetTime() > b.node->GetTime(); });

	int i = 0;
	for(auto &iter : sorted_vec) {
		if(num > 0 && i >= num) {
			break;
		}
		iter.node->Dump(stream, iter.name, total_time, node_level + 1, num);
		++i;
	}
}
