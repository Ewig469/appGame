#include "logger.h"
#include <iostream>
#include <ostream>
#include <sstream>
#include <chrono>
#include <format>

// Terminal text colors, source: https://stackoverflow.com/a/9158263
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace preset {

// init statics
LogLevel Logger::max_level_ = INFO;
std::ostream* Logger::out_ = &std::cout;
std::ostream* Logger::err_ = &std::cerr;
bool Logger::color_support_ = true;

void Logger::error(std::string msg){
  log(msg, ERRORS);
}
void Logger::warning(std::string msg){
  log(msg, WARNINGS);
}
void Logger::info(std::string msg){
  log(msg, INFO);
  
}
void Logger::debug(std::string msg){
  log(msg, DEBUG);
}
void Logger::log(std::string msg, LogLevel level){
  if (!should_log(level)) return;

  std::stringstream s = {};

    { // scope the namespace renaming because I don't want that as a universal thing in the function
    namespace ch = std::chrono;
    auto now = ch::system_clock::now();
    auto local_time = ch::zoned_time{ch::current_zone(), now};
    s << std::format("({:%T}) ", ch::floor<ch::seconds>(local_time.get_local_time()));
    }
       
  if (color_support_){
    switch (level) {
    case DEBUG:    s <<               "[DEBUG]   ";          break;
    case INFO:     s << BLUE <<       "[INFO]    " << RESET; break;
    case WARNINGS: s << BOLDYELLOW << "[WARNING] " << RESET; break;
    case ERRORS:   s << BOLDRED <<    "[ERROR]   " << RESET; break;
    }
  } else {
    switch (level) {
    case DEBUG:    s << "[DEBUG]   ";  break;
    case INFO:     s << "[INFO]    "; break;
    case WARNINGS: s << "[WARNING] "; break;
    case ERRORS:   s << "[ERROR]   "; break;
    }
  }

  s << msg;

  if (level == ERRORS){
    *err_ << s.str() << std::endl;
    return;
  }
  *out_ << s.str() << std::endl;
}

void Logger::set_max_level(LogLevel level){
  max_level_ = level;
}

bool Logger::should_log(LogLevel level){
  return level >= max_level_;
}

void Logger::set_print_stream(std::ostream* out, std::ostream* err, bool color_support){
  color_support_ = color_support;
  out_ = out;
  err_ = err;
}

} //namespace

