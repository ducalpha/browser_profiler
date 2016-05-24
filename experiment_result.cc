// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT ($LICENSE_URL)

#include "experiment_result.h"

#include "base/macros.h"
#include "base/logging.h"
#include "base/files/file_util.h"

namespace browser_profiler {

//static
const char* ExperimentResult::kBrowserConfigNameKey = "Browser Config Name";
//static
const char* ExperimentResult::kCommandLineKey = "Command Line";
//static
const char* ExperimentResult::kLogPrefixKey = "Log Prefix";
//static
const char* ExperimentResult::kHostKey = "Host";
//static
const char* ExperimentResult::kUrlKey = "URL";
//static
const char* ExperimentResult::kLoadStartTimeKey = "Load Start Time (s)";
//static
const char* ExperimentResult::kLoadEndTimeKey = "Load End Time (s)";
//static
const char* ExperimentResult::kPageLoadTimeKey = "Page Load Time (s)";
//static
const char* ExperimentResult::kSyncWorkloadEndTimeKey = "Sync Workload End Time (s)";
//static
const char* ExperimentResult::kUserThinkTimeKey = "User Think Time (ms)";

//static
const char* ExperimentResult::kResultLineFields[] = {
  kBrowserConfigNameKey,
  kCommandLineKey,
  kLogPrefixKey,
  kHostKey,
  kUrlKey,
  kLoadStartTimeKey,
  kLoadEndTimeKey,
  kPageLoadTimeKey,
  kSyncWorkloadEndTimeKey,
  kUserThinkTimeKey 
};

// Default values: empty
ExperimentResult::ExperimentResult() {
  for (size_t i = 0; i < arraysize(kResultLineFields); ++i) {
    key_value_map_[kResultLineFields[i]] = "";
  }
}

std::string ExperimentResult::LogHeaderLine() {
  // Java: return TextUtils.join("\t", mResultLineFields);
  std::string header_line;

  for (size_t i = 0; i < arraysize(kResultLineFields); ++i) {
    if (i > 0)
      header_line += '\t';
    header_line += kResultLineFields[i];
  }

  return header_line;
}

void ExperimentResult::Put(const std::string& key, const std::string& value) {
  key_value_map_[key] = value;
}

std::string ExperimentResult::LogLine() {
  std::string log_line;

  for (size_t i = 0; i < arraysize(kResultLineFields); ++i) {
    if (i > 0)
      log_line += '\t';
    log_line += key_value_map_[kResultLineFields[i]];
  }

  return log_line;
}

void ExperimentResult::WriteToFile(const base::FilePath& filepath, bool include_header) {
  if (include_header) {
    std::string header = LogHeaderLine() + "\n";
    if (WriteFile(filepath, header.c_str(), header.length()) == -1) {
      LOG(FATAL) << "Cannot write result header at " << filepath.value();
    }
  }

  std::string result_line = LogLine() + "\n";
  if (AppendToFile(filepath, result_line.c_str(), result_line.length()) == -1) {
      LOG(FATAL) << "Cannot write experiment result at " << filepath.value();
  }
}

/*
std::string ExperimentResult::ToJson() {
  std::string json_output = "{";

  for (size_t i = 0; i < arraysize(kResultLineFields); ++i) {
    std::string key = kResultLineFields[i];
    if (i > 0)
      json_output += ",";
    json_output += "\"" + key + "\" : \"" + key_value_map_[key] + "\"";
  }

  json_output += "}";

  return json_output;
}*/

}  // namespace browser_profiler
