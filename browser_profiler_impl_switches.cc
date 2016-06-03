// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#include "browser_profiler_impl_switches.h"

namespace switches {

// Disable Browser Profiler
const char kDisableBrowserProfiler[] = "disable-browser-profiler";

// Total try number
const char kNumTryPerUrl[] = "num-try-per-url";

// Do Ftrace
const char kDoFtrace[] = "do-ftrace";

// Do event trace
const char kDoChromeTrace[] = "do-chrome-trace";

// Measure Power
const char kMeasurePower[] = "measure-power";

// Do tcpdump
const char kCapturePackets[] = "capture-packets";

// Clear DNS cache before experiment
const char kClearDnsCache[] = "clear-dns";

// Wait for user think-time after load event fired before restart
// Specified in seconds by now
const char kUserThinkTimeMillis[] = "user-think-time-millis";

// Automatic rsync all logs to the PC after all experiments finish
const char kRsyncLogsAfterAll[] = "rsync-logs-after-all";

// Automatic delete log files after all experiments finish
// Only in effect when rsync-logs-after-all
const char kCleanLogsAfterAll[] = "clean-logs-after-all";

// Record screen
const char kScreenRecord[] = "screen-record";

// Monitor cpu utilization
const char kMonitorCpuUtilization[] = "monitor-cpu-utilization";

// Concise name of the configuration of browser
// E.g., Default, EnergySaving
const char kBrowserConfigName[] = "browser-config-name";

// Clear cache
// Copied from native code
const char kClearCache[] = "clear-cache";

// Test hot page load (with cache, load right after a visit)
const char kTestHotLoad[] = "test-hot-load";
}  // namespace switches
