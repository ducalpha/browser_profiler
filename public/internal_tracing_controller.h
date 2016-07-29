// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#ifndef BROWSER_PROFILER_PUBLIC_INTERNAL_TRACING_CONTROLLER_H_
#define BROWSER_PROFILER_PUBLIC_INTERNAL_TRACING_CONTROLLER_H_

#include "browser_profiler.h"

#include <string>

#include "base/files/file_path.h"

namespace browser_profiler {

class InternalTracingController {
 public:
  // Start internal tracing (e.g., about:tracing in Chrome)
  // Return true if started (TODO: guarantee the tracing has started on all child processes?)
  virtual bool StartTracing(BrowserProfiler *browser_profiler,
                                    const std::string& tracing_categories,
                                    const std::string& tracing_options) { return false; }

  // Stop internal tracing asynchronously, it will call OnInternalTracingStopped on BrowserProfiler
  virtual bool StopTracing(const base::FilePath& trace_file) { return false; }
};

} // namespace browser_profiler 

#endif // BROWSER_PROFILER_PUBLIC_INTERNAL_TRACING_CONTROLLER_H_
