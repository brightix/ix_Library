#include "Logger.h"
#include<fstream>

#include <ctime>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <format>
#include <cstdarg>
#include <filesystem>

using namespace ix::utility;
using namespace std;
const char* Logger::s_level[LEVEL_COUNT] = {
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"FATAL"
};

void ix::utility::Logger::Log(Level level, const char* file, int line, const char* fmt, ...)
{
	if (m_level > level) {
		return;
	}

	if (m_fout.fail()) {
		throw logic_error("Open file failed " + m_filename);
	}

	time_t ticks = time(NULL);

	if(ticks == (time_t)-1){
		PrintErrno("Failed to get time");
	}

	struct tm* ptm = localtime(&ticks);
	if (ptm == nullptr) {
		PrintErrno("Log Error");
		return;
	}
	char timestamp[32];
	strftime(timestamp,sizeof(timestamp),"%Y-%m-%d %H:%M:%S",ptm);

	// 简化文件路径
	file = filesystem::path(file).filename().c_str();

	string buffer;
	buffer = std::format("{} [{}] {}:{}  ", timestamp, s_level[level], file, line);
	m_len += static_cast<int>(buffer.size());
	m_fout << buffer;

	va_list arg_ptr;
	va_start(arg_ptr,fmt);
	int size = vsnprintf(NULL,0,fmt,arg_ptr);
	va_end(arg_ptr);
	if (size > 0) {
		char* content = new char[size + 1];
		va_start(arg_ptr,fmt);
		vsnprintf(content,size+1,fmt,arg_ptr);
		va_end(arg_ptr);
		m_len += size;
		m_fout << content;

		// 如果启用了打印日志到控制台
		if (printOnConsole)
		{
			cout << buffer << content << endl;
		}
	}
	m_fout << "\n";
	m_fout.flush();
	if (m_len > m_maxSize) {
		Rotate();
	}
}

void ix::utility::Logger::Open(const std::string& filename)
{
	m_filename = filename;
	m_fout.open(filename,ios::app);
	if (m_fout.fail()) {
		throw logic_error("Open file failed");
	}
	m_fout.seekp(0,ios::end);
	m_len = m_fout.tellp();
}

void ix::utility::Logger::Close()
{
	m_fout.close();
}

void Logger::SetLevel(Level level)
{
	m_level = level;
}

void ix::utility::Logger::SetMaxSize(int val)
{
	m_maxSize = val;
}

void ix::utility::Logger::Rotate()
{
	Close();
	time_t ticks = time(NULL);
    if(ticks == static_cast<time_t>(-1)){
        PrintErrno("Failed to get time");
    }

	struct tm* ptm = localtime(&ticks);
	if (ptm == nullptr) {
       	PrintErrno("Log Error");
		return;
	}

	char timestamp[32];

	strftime(timestamp, sizeof(timestamp), ".%Y_%m_%d_%H_%M_%S", ptm);
	size_t dotPos = m_filename.find_last_of('.');
	string base = m_filename.substr(0,dotPos);
	string ext = m_filename.substr(dotPos,m_filename.size());
	string filename = base + timestamp + ext;
	if (rename(m_filename.c_str(),filename.c_str()) != 0) {
		throw std::logic_error("rename log file failed: " + string(strerror(errno)));
	}
	Open(m_filename);
}

string Logger::PrintErrno(string_view errorName)
{
	return string(errorName) + ", errno: " + to_string(errno) +  ",errMsg: " + strerror(errno);
}

void Logger::print_log_on_console()
{
	printOnConsole = true;
}




Logger::Logger() : m_level(Level::DEBUG),m_len(0),m_maxSize(4096){

}

Logger::~Logger()
{
	Close();
}

Logger& Logger::Instance()
{
	static Logger instance;
	return instance;
}