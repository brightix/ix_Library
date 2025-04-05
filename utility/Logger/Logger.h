#pragma once
#include <string>
#include <iostream>
#include <fstream>

#define debug(format,...) \
	ix::utility::Logger::Instance().Log(ix::utility::Logger::Level::DEBUG,__FILE__,__LINE__,format,##__VA_ARGS__)
#define info(format,...) \
	ix::utility::Logger::Instance().Log(ix::utility::Logger::Level::INFO,__FILE__,__LINE__,format,##__VA_ARGS__)
#define warn(format,...) \
	ix::utility::Logger::Instance().Log(ix::utility::Logger::Level::WARN,__FILE__,__LINE__,format,##__VA_ARGS__)
#define error(format,...) \
	ix::utility::Logger::Instance().Log(ix::utility::Logger::Level::ERR,__FILE__,__LINE__,format,##__VA_ARGS__)
#define fatal(format,...) \
	ix::utility::Logger::Instance().Log(ix::utility::Logger::Level::FATAL,__FILE__,__LINE__,format,##__VA_ARGS__)

namespace ix {
namespace utility {
class Logger 
{
public:
	enum Level
	{
		DEBUG,
		INFO,
		WARN,
		ERR,
		FATAL,
		LEVEL_COUNT
	};
	void Log(Level level,const char* file,int line,const char* format,...);
	static Logger& Instance();
	void Open(const std::string& filename);
	void Close();
	void SetLevel(Level level);
	void SetMaxSize(int val);
	void Rotate();
	std::string PrintErrno(int err);
	Logger(Logger&) = delete;
	Logger& operator=(Logger&) = delete;
private:
	Logger();
	~Logger();
	std::string m_filename;
	std::ofstream m_fout;
	Level m_level;
	int m_len;
	int m_maxSize;
	static const char* s_level[LEVEL_COUNT];
};
}
}