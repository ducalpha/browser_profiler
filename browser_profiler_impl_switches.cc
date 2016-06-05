// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#include "browser_profiler_impl_switches.h"

namespace switches {

// Concise name of the configuration of browser
// E.g., Default, EnergySaving
const char kBrowserConfigName[] = "browser-config-name";

// Record network packets (e.g., using tcpdump)
const char kCapturePackets[] = "capture-packets";

// Automatic delete log files after all experiments finish
const char kCleanLogsAfterAll[] = "clean-logs-after-all";

// Clear browser's cache before each experiment
const char kClearCache[] = "clear-cache";

// Clear DNS cache before each experiment
const char kClearDnsCache[] = "clear-dns";

// Disable Browser Profiler
const char kDisableBrowserProfiler[] = "disable-browser-profiler";

// Do Chrome internal tracing
const char kDoChromeTrace[] = "do-chrome-trace";

// Record Ftrace
const char kDoFtrace[] = "do-ftrace";

// Measure Power
const char kMeasurePower[] = "measure-power";

// Monitor cpu utilization
const char kMonitorCpuUtilization[] = "monitor-cpu-utilization";

// Total try number
const char kNumTryPerUrl[] = "num-try-per-url";

// Automatic rsync all logs to the PC after all experiments finish
const char kRsyncLogsAfterAll[] = "rsync-logs-after-all";

// Record screen
const char kScreenRecord[] = "screen-record";

// Test hot page load (with cache, load right after a visit)
const char kTestHotLoad[] = "test-hot-load";

// Wait for user think-time after load event fired before restarting
const char kUserThinkTimeMillis[] = "user-think-time-millis";

}  // namespace switches
