#include "log.h"

#include <assert.h>
#include <ctime>
#include <algorithm>

#include "termcolor.hpp"

static const std::string currentDateTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%m/%d/%Y %X", &tstruct);
	return buf;
}

Logger Logger::logger = Logger();
std::ofstream* Logger::logFile = nullptr;

Logger::Logger() : m_output(&std::cout) {
}

Logger::Logger(std::ostream* output) : m_output(output) {
	if (m_output->bad()) {
		assert(false);
	}
}

Logger::~Logger() {
	m_output->flush();
	static std::stringstream closed_flag;
	m_output->rdbuf(closed_flag.rdbuf());
	if (logFile && *logFile) {
		logFile->close();
		delete logFile;
	}
}

static std::string replaceAll(const std::string& in, const std::string& what, const std::string& by) {
	std::string str = in;
	size_t index = 0;
	while (true) {
		index = str.find(what, index);
		if (index == std::string::npos) break;

		str.replace(what.size(), 3, by);

		index += what.size();
	}
	return str;
}

void Logger::print(LogLevel level, const char* file, const char* function, int line, const std::string& msg) {
	// [12/12/2017 23:45] => [ERROR] [func@33] Test error!
	std::string prefx = "[" + currentDateTime() + "] ";
	std::string filen = file;
	std::replace(filen.begin(), filen.end(), '\\', '/');
	filen = filen.substr(filen.find_last_of('/') + 1);

	(*m_output) << termcolor::green << termcolor::dark;

	(*m_output) << prefx;

	switch (level) {
		case LogLevel::Debug: (*m_output) << termcolor::cyan; break;
		case LogLevel::Info: (*m_output) << termcolor::blue; break;
		case LogLevel::Warning: (*m_output) << termcolor::yellow; break;
		case LogLevel::Error: (*m_output) << termcolor::red; break;
		case LogLevel::Fatal: (*m_output) << termcolor::magenta; break;
	}

	switch (level) {
		case LogLevel::Debug: (*m_output) << "[DBG]"; break;
		case LogLevel::Info: (*m_output) << "[INF]"; break;
		case LogLevel::Warning: (*m_output) << "[WRN]"; break;
		case LogLevel::Error: (*m_output) << "[ERR]"; break;
		case LogLevel::Fatal: (*m_output) << "[FTL]"; break;
	}

	(*m_output) << termcolor::reset;

	(*m_output) << " [" << filen << "(" << function << " @ " << line << ")] " << msg << std::endl;
}

