// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#ifndef BROWSER_PROFILER_BROWSER_PROFILER_IMPL_CONSTANTS_H_
#define BROWSER_PROFILER_BROWSER_PROFILER_IMPL_CONSTANTS_H_

#include <string>

#include "base/files/file_path.h"

namespace browser_profiler {

struct BrowserProfilerImplConstants {
  BrowserProfilerImplConstants(const base::FilePath& home_dir, const base::FilePath& writable_dir);

  base::FilePath kBpHome;

  // Only out/ and tmp/ are accessible from apps
  base::FilePath kBpTmpDir;
  base::FilePath kBpStateFile;
  base::FilePath kPowerToolServerConfigFile;
  base::FilePath kExperimentCommandLineFile;
  base::FilePath kBpUrlListFile;

  base::FilePath kBpOutDir;
  base::FilePath kExperimentResultFile;

  base::FilePath kBinDir;
  base::FilePath kStartFtraceScript;
  base::FilePath kStopFtraceScript;
  base::FilePath kSyncOutToPcScript;
  base::FilePath kCleanLogsScript;
  base::FilePath kCpuConfigurerExecutable;
  base::FilePath kCpuInfoExecutable;
  base::FilePath kStartScreenRecordScript;
  base::FilePath kStopScreenRecordScript;
  base::FilePath kStartCpuUtilizationMonitorScript;
  base::FilePath kStopCpuUtilizationMonitorScript;
  base::FilePath kStartCapturePacketsScript;
  base::FilePath kStopCapturePacketsScript;

  std::string kClearDnsCacheCommand;

  std::string kExperimentResultBaseName;
  std::string kFtraceBaseName;
  std::string kItraceBaseName;
  std::string kPcapBaseName;

  std::string kBlankPageUrl;
};

} // namespace browser_profiler 

#endif // BROWSER_PROFILER_BROWSER_PROFILER_IMPL_CONSTANTS_H_
