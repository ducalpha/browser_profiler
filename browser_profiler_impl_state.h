// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#ifndef BROWSER_PROFILER_BROWSER_PROFILER_IMPL_STATE_H_
#define BROWSER_PROFILER_BROWSER_PROFILER_IMPL_STATE_H_
#include <string>
#include <vector>

#include "base/files/file_path.h"

namespace browser_profiler {

// Serializable state of BrowserProfilerImpl
struct BrowserProfilerImplState {
  BrowserProfilerImplState();

  // Reset state
  void Reset();

  // Reset and read command lines and url list
  void Initialize(const base::FilePath& experiment_command_lines_file,
      const base::FilePath& url_list_file);

  // Save to a file
  // Return true if succeed
  bool SaveToFile(const base::FilePath& file_name);

  // Load from a file
  // Return true if succeed
  bool LoadFromFile(const base::FilePath& file_name);

  void Initialize(const base::FilePath& experiment_command_lines_file,
        const std::string& url_list_file);

  size_t experiment_command_line_index;
  size_t current_url_index;
  size_t current_url_try_done; // count after experiment done

  std::vector<std::string> experiment_command_lines;
  std::vector<std::string> experiment_urls;

  bool started;
  bool all_experiments_finished;
  bool start_new_experiments;

  // Store last experiment id to prefix the experiment summary file
  std::string last_experiment_id;
};

} // namespace browser_profiler 
#endif // BROWSER_PROFILER_BROWSER_PROFILER_IMPL_STATE_H_
