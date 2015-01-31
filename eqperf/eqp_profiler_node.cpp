#include "eqp_profiler_node.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

EQP::CPU::ProfilerNode::ProfilerNode() {
	count_ = 0;
	time_ = 0;
	parent_ = nullptr;;
}

EQP::CPU::ProfilerNode::~ProfilerNode() {
	for(auto &iter : nodes_) {
		delete iter.second;
	}
}

void EQP::CPU::ProfilerNode::Dump(std::ostream &stream, const std::string &func, uint64_t total_time, int node_level) {

	if(node_level >= 1) {
		stream << std::setw(node_level * 2) << " ";
	}

	double m_cycles = time_ / 1000000.0;
	double m_avg_cycles = m_cycles / count_;
	double percentage = time_ * 100 / static_cast<double>(total_time);

	std::streamsize p = stream.precision();

	stream << std::fixed;
	stream.precision(2);
	stream << m_cycles << "M cycles, " << count_ << " calls, " << m_avg_cycles << "M cycles avg, ";
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

	for(auto &iter : sorted_vec) {
		iter.node->Dump(stream, iter.name, total_time, node_level + 1);
	}
}
