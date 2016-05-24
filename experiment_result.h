// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT ($LICENSE_URL)

#ifndef BROWSER_PROFILER_EXPERIMENT_RESULT_H_
#define BROWSER_PROFILER_EXPERIMENT_RESULT_H_

#include <string>

#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
#include <unordered_map>
#else
// use 'map' as it is not very performance critical and don't want to use tr1::
#include <map>
#endif

#include "base/files/file_path.h"

namespace browser_profiler {

// A map of key/value for log lines
// Can convert to json or to a tab-separated log line
// Because all values are string, create json without any library dependency
// (It would be much easier in Java, by using JsonObject)
class ExperimentResult {
 public:
  static const char* kBrowserConfigNameKey;
  static const char* kCommandLineKey;
  static const char* kLogPrefixKey;
  static const char* kHostKey;
  static const char* kUrlKey;
  static const char* kLoadStartTimeKey;
  static const char* kLoadEndTimeKey;
  static const char* kPageLoadTimeKey;
  static const char* kSyncWorkloadEndTimeKey;
  static const char* kUserThinkTimeKey;

  // This array retains the order of fields in the experiment result log file
  // Use array for easy initialization in C++98
  static const char* kResultLineFields[];

  ExperimentResult();

  void Put(const std::string& key, const std::string& value);

  // Return tab-separated keys as a log header line
  std::string LogHeaderLine();

  // Return tab-separated values as a log line
  std::string LogLine();

  // Will write header first, then the log line if include_header is true
  void WriteToFile(const base::FilePath& filepath, bool include_header);

 private:
#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
  std::unordered_map<std::string, std::string> key_value_map_;
#else
  std::map<std::string, std::string> key_value_map_;
#endif
};

}  // namespace browser_profiler

#endif  // BROWSER_PROFILER_EXPERIMENT_RESULT_H_
