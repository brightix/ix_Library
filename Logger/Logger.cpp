#include "logger.h"
#include<fstream>

#include <time.h>
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <format>
#include <cstdarg>
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
	/* 打开失败时报错 */
	if (m_fout.fail()) {
		throw logic_error("Open file failed " + m_filename);
	}
	/* 获取当前时间戳 */
	time_t ticks = time(NULL);
	struct tm ptm;
	/* 转换为时间结构 */
	errno_t err = localtime_s(&ptm,&ticks);
	if (err) {
		char errMsg[256]{};
		strerror_s(errMsg,sizeof(errMsg),err);
		printf("Log Error,errno=%d,msg=%s", err, errMsg);
		return;
	}
	/* 格式化时间 */
	char timestamp[32];
	strftime(timestamp,sizeof(timestamp),"%Y-%m-%d %H:%M:%S",&ptm);
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
	}

	m_fout << "\n";
	m_fout.flush();
	if (m_len > m_SetMaxSize) {
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
	m_SetMaxSize = val;
}

void ix::utility::Logger::Rotate()
{
	Close();
	time_t ticks = time(NULL);
	struct tm ptm;
	/* 转换为时间结构 */
	errno_t err = localtime_s(&ptm, &ticks);
	if (err) {
		char errMsg[256]{};
		strerror_s(errMsg, sizeof(errMsg), err);
		printf("Log Error,errno=%d,msg=%s", err, errMsg);
		return;
	}
	/* 格式化时间 */
	char timestamp[32];

	strftime(timestamp, sizeof(timestamp), ".%Y_%m_%d_%H_%M_%S", &ptm);
	size_t dotPos = m_filename.find_last_of('.');
	string base = m_filename.substr(0,dotPos);
	string ext = m_filename.substr(dotPos,m_filename.size());
	string filename = base + timestamp + ext;
	if (rename(m_filename.c_str(),filename.c_str()) != 0) {
		char buffer[256]{};
		strerror_s(buffer, sizeof(buffer), errno);
		throw std::logic_error("rename log file failed: "+ std::string(buffer));
	}
	Open(m_filename);
}

string ix::utility::Logger::PrintErrno(int err)
{
	char errMsg[256]{};
	strerror_s(errMsg, sizeof(errMsg),err);
	return string(errMsg);
}

Logger::Logger() : m_level(Level::DEBUG),m_len(0),m_SetMaxSize(0){

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