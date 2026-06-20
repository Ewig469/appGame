#pragma once

#include <ostream>
#include <string>

namespace preset {

/// Available log levels.
enum LogLevel {
  DEBUG,
  INFO,
  WARNINGS,
  ERRORS,
};

/// Log information and errors at several levels. Only levels severer than the max log level get logged.
class Logger {
public:
  /// Set max level. Default: `LogLevel::INFO`
  static void set_max_level(LogLevel level);
  /// Log an error.
  static void error(std::string msg);
  /// Log a warning.
  static void warning(std::string msg);
  /// Log information.
  static void info(std::string msg);
  /// Log debugging information.
  static void debug(std::string msg);
  /// Return whether a level should be logged.
  static bool should_log(LogLevel level);
  /// Change print stream settings, specifically: standard stream, error stream and whether to use colors. Used to dump logs to a file or a string stream.
  static void set_print_stream(std::ostream* out, std::ostream* err, bool color_support);
private:
  static LogLevel max_level_;
  /// Stream for standard output.
  static std::ostream* out_;
  /// Stream for error output.
  static std::ostream* err_;
  static bool color_support_;
  /// Manually log at a certain level.
  static void log(std::string msg, LogLevel level);
};


} //namespace

