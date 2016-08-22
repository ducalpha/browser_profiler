// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#include "public/browser_profiler.h"

#include <unistd.h>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"

namespace browser_profiler {

BrowserProfiler::BrowserProfiler(BrowserProfilerClient* client)
  : client_(client) {
}

// static
void BrowserProfiler::ClearCacheIfNeeded(const base::FilePath& cache_path) {
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch("clear-cache"))
    return;

  if (!base::DeleteFile(cache_path, /* recursive */ true)) {
    LOG(ERROR) << "Unable to delete cache folder";
    return;
  }

  // Sync to disk to prevent noise to the experiment
  sync();

  VLOG(1) << "Deleted cache directory at " << cache_path.value();
}

} // namespace browser_profiler 
