// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#include "browser_profiler_impl_constants.h"

namespace {

// Input and temporary files
const char kTmpDirName[] = "tmp/";

// Output directory
const char kOutDirName[] = "out/";

// Scripts and executables
const char kBinDirName[] = "bin/";

} // namespace

namespace browser_profiler {

BrowserProfilerImplConstants::BrowserProfilerImplConstants(const base::FilePath& home_dir, const base::FilePath& writable_dir)
  : kBpHome(home_dir), 
    kBpTmpDir(writable_dir.Append(kTmpDirName)),
    kClearDnsCacheCommand("ndc resolver flushdefaultif"),
    kExperimentResultBaseName("experiment_result.log"),
    kFtraceBaseName("ftrace.dat"),
    kItraceBaseName("itrace.json"),
    kPcapBaseName("pcap"),
    kBlankPageUrl("about:blank") {
    kBpStateFile = kBpTmpDir.Append(std::string("browser-profiler-state"));
    kPowerToolServerConfigFile = kBpTmpDir.Append("power-tool-server-config");
    kExperimentCommandLineFile = kBpTmpDir.Append("experiment-command-lines");
    kBpUrlListFile = kBpTmpDir.Append("bp-url-list");
    kBpOutDir = writable_dir.Append(kOutDirName);
    kExperimentResultFile = kBpOutDir.Append(kExperimentResultBaseName);
    kBinDir = kBpHome.Append(kBinDirName);
    kStartFtraceScript = kBinDir.Append("start-ftrace.sh");
    kStopFtraceScript = kBinDir.Append("stop-ftrace.sh");
    kSyncOutToPcScript = kBinDir.Append("sync_out_to_pc.sh");
    kCleanLogsScript = kBinDir.Append("clean-logs.sh");
    kCpuConfigurerExecutable = kBinDir.Append("cpu_configurer");
    kCpuInfoExecutable = kBinDir.Append("cpu_info");
    kStartScreenRecordScript = kBinDir.Append("record_screen_start.sh");
    kStopScreenRecordScript = kBinDir.Append("record_screen_stop.sh");
    kStartCpuUtilizationMonitorScript = kBinDir.Append("monitor_cpu_util_start.sh");
    kStopCpuUtilizationMonitorScript = kBinDir.Append("monitor_cpu_util_stop.sh");
    kStartCapturePacketsScript = kBinDir.Append("capture-packets.sh");
    kStopCapturePacketsScript = kBinDir.Append("capture-packets-stop.sh");
}

} // namespace browser_profiler 
