#include "json_config.h"
#include <fstream>
#include <iostream>

EQ::JsonConfigFile::JsonConfigFile() = default;

EQ::JsonConfigFile::JsonConfigFile(const Json::Value &value)
{
	m_root = value;
}

EQ::JsonConfigFile::~JsonConfigFile() = default;

/**
 * @param filename
 * @return
 */
EQ::JsonConfigFile EQ::JsonConfigFile::Load(const std::string &filename)
{
	JsonConfigFile ret;
	ret.m_root = Json::Value();

	std::ifstream ifs;
	ifs.open(filename, std::ifstream::in);

	if (!ifs.good()) {
		return ret;
	}

	try {
		ifs >> ret.m_root;
	}
	catch (std::exception) {
		return ret;
	}

	return ret;
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
std::string EQ::JsonConfigFile::GetVariableString(
	const std::string &title,
	const std::string &parameter,
	const std::string &default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asString();
		}
	}
	catch (std::exception) {
		return default_value;
	}

	return default_value;
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
int EQ::JsonConfigFile::GetVariableInt(
	const std::string &title,
	const std::string &parameter,
	const int default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asInt();
		}
	}
	catch (std::exception) {
		return default_value;
	}

	return default_value;
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
bool EQ::JsonConfigFile::GetVariableBool(
	const std::string &title,
	const std::string &parameter,
	const bool default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asBool();
		}
	}
	catch (std::exception) {
		return default_value;
	}

	return default_value;
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
double EQ::JsonConfigFile::GetVariableDouble(
	const std::string &title,
	const std::string &parameter,
	const double default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asDouble();
		}
	}
	catch (std::exception) {
		return default_value;
	}

	return default_value;
}
