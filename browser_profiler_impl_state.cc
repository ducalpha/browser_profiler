// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT ($LICENSE_URL)

#include "browser_profiler_impl_state.h"

#include <sstream>

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/string_split.h"

namespace {

bool ReadExperimentCommandLines(
      const base::FilePath& experiment_command_lines_file,
      std::vector<std::string> *experiment_command_lines) {
  if (!base::PathExists(experiment_command_lines_file)) {
    LOG(ERROR) << "Experiment command line file not exist at: " << experiment_command_lines_file.value();
    return false;
  }

  // More memory consumption, O(n), but reduce disk I/O by reading once
  std::string lines;
  if (!base::ReadFileToString(experiment_command_lines_file, &lines)) {
    LOG(FATAL) << "Failed to read experiment command line file";
    return false;
  }

  std::istringstream all_cmdlines(lines);
  std::string cmdline;
  while (getline(all_cmdlines, cmdline)) {
    // Only get the command line with prefix "chrome"
    if (StartsWithASCII(cmdline, "chrome ", /*case-sensitive*/ true)) {
      experiment_command_lines->push_back(cmdline);
    }
  }

  return true;
}

bool ReadExperimentUrlList(const base::FilePath& url_list_file,
      std::vector<std::string> *url_list) {
  std::string lines; 
  if (!base::ReadFileToString(url_list_file, &lines)) {
    LOG(FATAL) << "Cannot read url list at " << url_list_file.value();
    return false;
  }

  std::vector<std::string> list;
  base::SplitString(lines, '\n', &list);

  url_list->clear();
  for (size_t i = 0; i < list.size(); ++i) {
    std::string url = list[i];
    // Skip url starting with #
    if (!url.empty() && !StartsWithASCII(url, "#", true)) {
      VLOG(1) << "Read url: " << url;

      // Add http by default, if the url is just a host name
      if (!StartsWithASCII(url, "http", true))
        url = "http://" + url;

      url_list->push_back(url);
    }
  }
  return true;
}

}  // namespace


namespace browser_profiler {

BrowserProfilerImplState::BrowserProfilerImplState() {
  Reset();
}

void BrowserProfilerImplState::Reset() {
  experiment_command_line_index = 0;
  current_url_index = 0;
  current_url_try_done = 0;

  experiment_command_lines.clear();
  experiment_urls.clear();

  started = false;
  all_experiments_finished = false;
  start_new_experiments = false;
  last_experiment_id = "last_experiment_id";
}

void BrowserProfilerImplState::Initialize(const base::FilePath& experiment_command_lines_file,
      const base::FilePath& url_list_file) {
  // Reset values after loading from file
  Reset();

  if (!ReadExperimentCommandLines(experiment_command_lines_file,
          &experiment_command_lines)) {
    LOG(ERROR) << "Fail to read experiment command lines";
  }

  if (!ReadExperimentUrlList(url_list_file, &experiment_urls)) {
    LOG(FATAL) << "Fail to read experiment url list";
  }
}

// No exception allowed in Chromium so we need to check each write
#define STREAM_WRITELN(stream, variable) \
  do { \
    stream << variable << std::endl; \
    if (!stream.good()) { \
      LOG(FATAL) << "Error writing string to stream"; \
      return false; \
    } \
 } while (0);

// To save development time, use the simplest way: hardcode a format
// In the future, may use json Chromium's pickle (with crc32 check)
bool BrowserProfilerImplState::SaveToFile(const base::FilePath& file_name) {
  std::ostringstream output;

  STREAM_WRITELN(output, experiment_command_line_index);
  STREAM_WRITELN(output, current_url_index);
  STREAM_WRITELN(output, current_url_try_done);
  STREAM_WRITELN(output, experiment_command_lines.size());

  for (size_t i = 0; i < experiment_command_lines.size(); ++i) {
    STREAM_WRITELN(output, experiment_command_lines[i]);
  }

  STREAM_WRITELN(output, experiment_urls.size());
  for (size_t i = 0; i < experiment_urls.size(); ++i) {
    STREAM_WRITELN(output, experiment_urls[i]);
  }

  STREAM_WRITELN(output, started);
  STREAM_WRITELN(output, all_experiments_finished);
  STREAM_WRITELN(output, start_new_experiments);
  STREAM_WRITELN(output, last_experiment_id);

  std::string output_str = output.str();
  return WriteFile(file_name, output_str.c_str(), output_str.length());
}

// No exception allowed in Chromium so we need to check each read
// file_name and stream_str must match ones in LoadFromFile
#define STREAM_READ(stream, variable) \
  do { \
    stream >> variable; \
    if (!stream.good()) { \
      LOG(FATAL) << "Error parsing string from state file (" << file_name.value() \
          << "): " << state_str; \
      return false; \
    } \
 } while (0);

// Load in the inverse order of saving
bool BrowserProfilerImplState::LoadFromFile(const base::FilePath& file_name) {
  if (!base::PathExists(file_name)) {
    LOG(ERROR) << "State file not exist at: " << file_name.value();
    return false;
  }

  std::string state_str;
  if (!base::ReadFileToString(file_name, &state_str)) {
    LOG(FATAL) << "Cannot read state file: " << file_name.value();
    return false;
  }

  std::istringstream input(state_str);

  STREAM_READ(input, experiment_command_line_index);
  STREAM_READ(input, current_url_index);
  STREAM_READ(input, current_url_try_done);

  size_t experiment_command_lines_size;
  STREAM_READ(input, experiment_command_lines_size);
  LOG(INFO) << "Experiment command lines size: " << experiment_command_lines_size;
  experiment_command_lines.resize(experiment_command_lines_size);

  // Skip all new lines, otherwise getline will have unexpected behavior
  input.ignore(1000, '\n');

  for (size_t i = 0; i < experiment_command_lines_size; ++i) {
    std::string cmdline;
    if (!std::getline(input, cmdline)) {
      LOG(FATAL) << "Cannot read experiment command line";
    }
    LOG(INFO) << "Read command line: " << cmdline;
    experiment_command_lines[i] = cmdline;
  }

  size_t experiment_urls_size;
  STREAM_READ(input, experiment_urls_size);
  experiment_urls.resize(experiment_urls_size);
  for (size_t i = 0; i < experiment_urls_size; ++i) {
    STREAM_READ(input, experiment_urls[i]);
  }

  STREAM_READ(input, started);
  STREAM_READ(input, all_experiments_finished);
  STREAM_READ(input, start_new_experiments);
  STREAM_READ(input, last_experiment_id);

  return true;
}

} // namespace browser_profiler 
