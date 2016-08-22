// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#include "browser_profiler_impl.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "browser_profiler_impl_constants.h"
#include "browser_profiler_impl_switches.h"
#include "power_tool_controller.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/macros.h"
#include "base/process/kill.h"
#include "base/process/launch.h"
#include "base/process/process.h"
#include "base/process/process_handle.h"
#include "base/strings/string_util.h"
#include "base/strings/string_split.h"
#include "base/strings/string_number_conversions.h"
#include "base/sys_info.h"
#include "base/threading/platform_thread.h"
#include "base/third_party/android_cpu_tools/src/cpu_configurer/cpu_configurer_switches.h"
#include "base/third_party/android_cpu_tools/src/cpu_info/cpu_info.h"
#include "base/third_party/android_cpu_tools/src/workload_generator/workload_generator.h"
#include "base/time/time.h"


namespace {

const int kNanosecondsPerSecond = 1000000000;

// TODO: read home dir from file or command line
//const char kBrowserProfilerHomeDir[] = "/sdcard/ducalpha/bp/";
const char kBrowserProfilerHomeDir[] = "/data/local/tmp/my_home/android_env/bp/";

const char kBrowserProfilerWritableDir[] = "/sdcard/bp/";

// Prefix file so that files are sorted by experiment time
// Unfortunately, FilePath does not have a convenient way to prefix a filename
void PrefixFile(const base::FilePath& file_path, const std::string& prefix) {
  base::FilePath::StringType new_file_str(file_path.DirName().value());
  new_file_str.push_back(base::FilePath::kSeparators[0]);
  new_file_str.append(prefix + ".").append(file_path.BaseName().value());

  base::FilePath new_file(new_file_str);

  const size_t max_try = 100;
  for (size_t i = 0; base::PathExists(new_file); ++i) {
    new_file = new_file.InsertBeforeExtension(base::UintToString(i));
    if (i >= max_try) {
      LOG(ERROR) << "Cannot find unique name for prefixed experiment result";
      break;
    }
  }

  base::File::Error error;
  if (!base::ReplaceFile(file_path, new_file, &error)) {
    LOG(ERROR) << "Fail to prefix the experiment result file from " << file_path.value()
        << " to " << new_file.value() << ": " << base::File::ErrorToString(error);
  }
}

bool CheckedReadFileToString(const base::FilePath& file_path, std::string* dest) {
  if (!base::ReadFileToString(file_path, dest)) {
    PLOG(ERROR) << "Cannot read file " << file_path.value();
    return false;
  }
  return true;
}

bool CheckedWriteStringToFile(const base::FilePath& file_path, const std::string& str) {
  int written = WriteFile(file_path, str.c_str(), str.length());
  if (written < 0 || str.length() != static_cast<size_t>(written)) {
    PLOG(ERROR) << "Fail writing to file " << file_path.value();
    return false;
  }
  return true;
}

// Execute a shell command like system() in C++
// Because any failure will make an experiment fail,
// use FATAL here
bool ExecuteCommand(const base::CommandLine& cmd) {
  VLOG(1) << "Execute command: " << cmd.GetCommandLineString();

  base::Process process = base::LaunchProcess(cmd, base::LaunchOptions());
  if (!process.IsValid()) {
    LOG(ERROR) << "Cannot run command " << cmd.GetCommandLineString();
    return false;
  }

  int exit_code;
  if (!process.WaitForExit(&exit_code)) {
    LOG(ERROR) << "Cannot get return code for command " << cmd.GetCommandLineString();
    return false;
  }

  if (exit_code != 0) {
    LOG(ERROR) << "Running " << cmd.GetCommandLineString()
        << " failed with code " << exit_code;
    return false;
  }

  return true;
}

bool ExecuteCommand(const std::string& cmd_str) {
  std::vector<std::string> cmd_argv =
      base::SplitString(cmd_str, " ", base::WhitespaceHandling::TRIM_WHITESPACE,
                        base::SplitResult::SPLIT_WANT_NONEMPTY);

  return ExecuteCommand(base::CommandLine(cmd_argv));
}

// Run a command as root
bool ExecuteCommandAsRoot(const std::string& cmd) {
  return ExecuteCommand("su -c " + cmd);
}

bool ExecuteCommandAsRoot(const base::CommandLine& cmd) {
  base::CommandLine su_cmd(cmd);
  su_cmd.PrependWrapper("su -c");
  return ExecuteCommand(su_cmd);
}

bool ExecuteCommandAsRoot(const base::FilePath& cmd) {
  return ExecuteCommandAsRoot(cmd.value());
}

// Use directly clock_gettime
// Don't want to depend on Chromium base::TimeTicks::ToInternalValue() which may change over time
double MonotonicNow() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (double) now.tv_sec + (double) now.tv_nsec / kNanosecondsPerSecond;
}

bool EnsureInitializeCpuInfoCommandLine(const base::FilePath& cpu_info_cmd,
    const base::FilePath& cpu_info_command_line_file) {
  if (!ExecuteCommandAsRoot(cpu_info_cmd.value() + " > " + cpu_info_command_line_file.value())) {
    LOG(ERROR) << "Cannot initialize cpu info command line file";
    return false;
  }

  return true;
}

void AddSwitchToCommandLineString(std::string* command_line,
    const std::string& switch_name) {
  std::string switch_with_prefix("--");
  switch_with_prefix.append(switch_name);

  if (command_line->find(switch_with_prefix) == std::string::npos) {
    command_line->append(" ");
    command_line->append(switch_with_prefix);
  }
}

void RemoveSwitchFromCommandLineString(std::string* command_line,
    const std::string& switch_name) {
  std::string switch_with_prefix("--");
  switch_with_prefix.append(switch_name);

  base::RemoveChars(*command_line, switch_with_prefix, command_line);
}

// Extract host from url which has forms:
// protocol://host:port or host:port
// Chromium's GURL is not available among web browsers
std::string HostInUrl(std::string url) {
  std::string::size_type protocol_slash = url.find("://");
    
  if (protocol_slash != std::string::npos) {
    url = url.substr(protocol_slash + 3);
  }
  
  std::string::size_type host_separator = url.find('/');
  if (host_separator != std::string::npos) {
    url = url.substr(0, host_separator);
  }
  
  std::string::size_type port_separator = url.find(':');
  if (port_separator != std::string::npos) {
    url = url.substr(0, port_separator);
  }

  return url;
}

base::FilePath GenerateBackupFileName(const base::FilePath& original_file) {
  //return original_file.InsertBeforeExtension("bak");
  return base::FilePath(std::string(kBrowserProfilerWritableDir) + original_file.BaseName().value() + "-bak");
}

// Don't want to have dependency on Chromium's base string_number_conversions
// since it would requires LazyInstance for locks of a third_party floating point lib
std::string DoubleToString(double value) {
  std::ostringstream output;
  output << value;
  return output.str();
}

void TrimTrailingNewLine(std::string *str) {
  *str = str->substr(0, str->rfind('\n'));
}

}  // namespace

namespace browser_profiler {

struct BrowserProfilerImpl::Setting {
  Setting();

  bool capture_packets;
  bool do_ftrace;
  bool do_itrace;
  std::string tracing_categories;
  bool measure_power;
  unsigned num_try_per_url;

  bool need_clear_cache;
  bool clear_dns_cache;

  unsigned user_think_time_millis;

  bool rsync_logs_after_all;
  bool clean_logs_after_all;

  bool screen_record;
  bool monitor_cpu_utilization;

  bool test_hot_load;

  std::string browser_config_name;
};

BrowserProfilerImpl::BrowserProfilerImpl(BrowserProfilerClient* client)
  : BrowserProfiler(client),
    constants_(base::FilePath(kBrowserProfilerHomeDir), base::FilePath(kBrowserProfilerWritableDir)),
    default_cpu_setup_command_(constants_.kCpuConfigurerExecutable),
    sync_workload_cpu_setup_command_(constants_.kCpuConfigurerExecutable),
    prepared_(false) {
  chrome_tracing_started_ = false;
}

void BrowserProfilerImpl::Initialize(const base::FilePath& browser_command_line_file,
    const base::FilePath& cpu_info_command_line_file) {
  browser_command_line_file_ = browser_command_line_file;

  // Try to load state from the state file first
  if (!state_.LoadFromFile(constants_.kBpStateFile) ||
        state_.start_new_experiments) {

		if (!base::PathExists(cpu_info_command_line_file)) {

      // Initialize cpu info if needed
      if (!EnsureInitializeCpuInfoCommandLine(constants_.kCpuInfoExecutable, cpu_info_command_line_file)) {
        LOG(FATAL) << "Fail to initialize cpu info file";
      } else {
        // Restart web browser so that Browser Profiler Client
        // can load it to the command line
        RestartBrowser();  
      }
    }

    VLOG(1) << "Initialize state";
    state_.Initialize(constants_.kExperimentCommandLineFile, constants_.kBpUrlListFile);
  }

  // Always re-read settings from the command line
  setting_.reset(new Setting());

  InitializeCpuSetupCommands();
}

bool BrowserProfilerImpl::Prepare(std::string *experiment_url) {
  VLOG(1) << "Prepare";

  prepared_ = true;

  if (state_.all_experiments_finished) {
    // Put Content Shell to a blank state
    *experiment_url = constants_.kBlankPageUrl;

    return true;
  }

  // First time to do the experiment with a list of command lines
  if (!state_.started && !state_.experiment_command_lines.empty()) {
    VLOG(1) << "First experiment, restart";
    // Restart content shell to use the next command line list
    state_.started = true;

    BackupCurrentCommandLine();
    ReplaceCurrentWithNextExperimentCommandLine();

    if (!state_.SaveToFile(constants_.kBpStateFile))
      LOG(FATAL) << "Cannot save browser profiler state to file at " << constants_.kBpStateFile.value();
    RestartBrowser();
    return false;
  }

  state_.started = true;

  // Reset to default power management which may have been changed due to other experiments
  ExecuteCommandAsRoot(default_cpu_setup_command_);

  if (setting_->measure_power) {
    // Use Delegation/Factory method design pattern when there is another power tool controller
    power_tool_controller_.reset(
        new PowerToolController(constants_.kPowerToolServerConfigFile));

    // Connect here, at preparation step to avoid delay later
    power_tool_controller_->Connect();
  }

  StartTracers();

  *experiment_url = state_.experiment_urls[state_.current_url_index];
  VLOG(1) << "Experiment url: " << *experiment_url;

  experiment_id_ = GenerateExperimentId(*experiment_url);
  VLOG(1) << "Generated experiment id: " << experiment_id_;

  return true;
}

bool BrowserProfilerImpl::PostProcess(const std::string& url, double navigation_start_monotonic_time,
    double load_event_end_monotonic_time) {
  VLOG(1) << "PostProcess";

  // Use started flag to avoid unexpected wake up
  if (!prepared_ || !state_.started) {
    return false;
  }

  // Do not restart further if all experiments in this experiment set finished
  if (state_.all_experiments_finished) {
    PostProcessAfterAllExperiments();
    return true;
  }

  // Do not write to disk at this time to avoid noise to the experiment
  ConsolidateExperimentResult(url, navigation_start_monotonic_time, load_event_end_monotonic_time);

  if (setting_->user_think_time_millis > 0) {
    // Pass pointer of the super class method DelayedTaskCallback which is overriden by this
    // base::Bind() is not used here for portability with browsers other than Chromium
    // TODO: use more flexible way call the delayed task call back (eg. std::function in C++11)
    client_->DoDelayedTask(&BrowserProfiler::DelayedTaskCallback, setting_->user_think_time_millis);
  } else {
    PostProcessInternal();
  }

  return true;
}

void BrowserProfilerImpl::DelayedTaskCallback() {
  PostProcessInternal();
}

void BrowserProfilerImpl::PostProcessInternal() {
  // Write the experiment result here, after all, to avoid noise to the experiment
  bool first_experiment = state_.current_url_try_done == 0 &&
                          state_.current_url_index == 0 &&
                          state_.experiment_command_line_index == 0;
  experiment_result_.WriteToFile(constants_.kExperimentResultFile, first_experiment);

  StopTracers();
}


void BrowserProfilerImpl::PostProcessInternalSecondHalf() {
  // Update experiment index only when experiment is successful
  UpdateExperimentIndexAndCommandLine();

  state_.last_experiment_id = experiment_id_;
  LOG(INFO) << "Last experiment id: " << state_.last_experiment_id;
  if (!state_.SaveToFile(constants_.kBpStateFile))
    LOG(FATAL) << "Cannot save browser profiler state to file";

  RestartBrowser();
}

void BrowserProfilerImpl::StartTracers() {
  if (setting_->clear_dns_cache)
    ClearDnsCache();

  // Might break app --> start this first, if it breaks, other things have not been started
  if (setting_->measure_power) {
    StartPowerSampling();
  }

  // don't want to include screen record into ftrace
  if (setting_->screen_record)
    StartScreenRecord(experiment_id_);

  if (setting_->monitor_cpu_utilization)
    StartCpuUtilizationMonitor(experiment_id_);

  if (setting_->do_ftrace)
    StartFtrace();

  if (setting_->do_itrace)
    StartInternalTracing();

  if (setting_->capture_packets)
    StartCapturePackets(experiment_id_);
}

void BrowserProfilerImpl::StopTracers() {
  if (setting_->do_itrace && chrome_tracing_started_
      && internal_tracing_controller_ != nullptr) {
    VLOG(0) << "Stop ChromeTracing";

    base::FilePath output_file(
        constants_.kBpOutDir.Append(experiment_id_).value() + "." + constants_.kItraceBaseName);

    // We call internal tracing and ChromeTracing interchangebly
    // if ETracingAsync is OK, onTracingStopped will be called after done
    if (!internal_tracing_controller_->StopTracing(output_file))
      StopTracersSecondHalf(); // fallback to normal flow if internal tracing not supported
  } else {
    StopTracersSecondHalf();
  }
}

void BrowserProfilerImpl::OnInternalTracingStopped() {
  StopTracersSecondHalf();
}

void BrowserProfilerImpl::StopTracersSecondHalf() {
  if (setting_->measure_power) {
    client_->CloseActiveShell(); // reduce power noise

    // wait 500ms for other threads to finish
    // hotfix: sleep on Java layer instead of here?
    // base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(500));

    StopPowerSampling();

    // set back to default power management in case we test other things
    ExecuteCommandAsRoot(default_cpu_setup_command_);
  }

  // don't want to include screen record into ftrace
  if (setting_->screen_record)
    StopScreenRecord();

  if (setting_->monitor_cpu_utilization)
    StopCpuUtilizationMonitor();

  if (setting_->do_ftrace)
    StopFtrace(experiment_id_); // synchronous Ftrace stop

  if (setting_->capture_packets)
    StopCapturePackets();

  PostProcessInternalSecondHalf();
}

void BrowserProfilerImpl::RestartBrowser() {
  const int max_restart_trial = 10;

  for (int restart_trial = 0; restart_trial < max_restart_trial; ++restart_trial) {
    client_->RestartBrowser();
    // wait 5 seconds, if still running, try to restart once more
    // TODO: use better interface than a timeout
    base::PlatformThread::Sleep(base::TimeDelta::FromSeconds(5));
    ++restart_trial;
  }

  LOG(FATAL) << "Cannot restart browser";
}

// Does not include sync workload end time which is used for power tool controller server only
void BrowserProfilerImpl::ConsolidateExperimentResult(const std::string& url,
      double navigation_start_monotonic_time, double load_event_end_monotonic_time) {
  experiment_result_.Put(ExperimentResult::kBrowserConfigNameKey, setting_->browser_config_name);
  experiment_result_.Put(ExperimentResult::kCommandLineKey, BrowserCommandLine());
  experiment_result_.Put(ExperimentResult::kLogPrefixKey, experiment_id_);

  experiment_result_.Put(ExperimentResult::kHostKey, HostInUrl(url));
  experiment_result_.Put(ExperimentResult::kUrlKey, url);

  experiment_result_.Put(ExperimentResult::kLoadStartTimeKey,
      DoubleToString(navigation_start_monotonic_time));
  experiment_result_.Put(ExperimentResult::kLoadEndTimeKey,
      DoubleToString(load_event_end_monotonic_time));

  experiment_result_.Put(ExperimentResult::kPageLoadTimeKey,
      DoubleToString(load_event_end_monotonic_time - navigation_start_monotonic_time));
  experiment_result_.Put(ExperimentResult::kUserThinkTimeKey,
      base::UintToString(setting_->user_think_time_millis));
}

/* Assume the command line has "--clear-cache --test-host-load"
 * The first experiment will have --clear-cache and --clear-dns-cache
 * Since the second experiment, no --clear-cache and --clear-dns-cache*/
void BrowserProfilerImpl::ManipulateCommandLineForHotPageLoad() {
  /* Care only the first two experiment */
  if (state_.current_url_try_done > 1)
    return;

  const std::string cold_load_switches[] = {
      switches::kClearCache, switches::kClearDnsCache
  };

  std::string new_command_line;
  
  if (!CheckedReadFileToString(browser_command_line_file_, &new_command_line)) {
    LOG(FATAL) << "Failed to read current command line file at "
        << browser_command_line_file_.value();
    return;
  }

  if (state_.current_url_try_done == 0) {
    for (size_t i = 0; i < arraysize(cold_load_switches); ++i) {
      AddSwitchToCommandLineString(&new_command_line, cold_load_switches[i]);
    }
  } else if (state_.current_url_try_done == 1) {
    for (size_t i = 0; i < arraysize(cold_load_switches); ++i) {
      RemoveSwitchFromCommandLineString(&new_command_line, cold_load_switches[i]);
    }
  }

  /* Write to the command line file */
  if (!CheckedWriteStringToFile(browser_command_line_file_, new_command_line)) {
    LOG(FATAL) << "Failed to manipulate command line for hot page load";
  }
}

// Write next experiment command line to the browser's command line
void BrowserProfilerImpl::ReplaceCurrentWithNextExperimentCommandLine() {
  std::string cmd = "su -c echo '" + state_.experiment_command_lines[state_.experiment_command_line_index] +
      "' > " + browser_command_line_file_.value();
  // ExecuteCommandAsRoot does not work well, maybe because of the ' sign in the command
  // E.g., The command line becomes su -c --easure-power --v=0' echo 'chrome
  if (system(cmd.c_str()) < 0) {
    LOG(FATAL) << "Writing next command line failed";
  }
  std::string change_permission = "chmod 666 " + browser_command_line_file_.value();
  if (!ExecuteCommandAsRoot(change_permission)) {
    LOG(FATAL) << "Writing next command line failed";
  }
}

void BrowserProfilerImpl::PostProcessAfterAllExperiments() {
  VLOG(1) << "PostProcessAfterAllexperiments";
  PrefixFile(constants_.kExperimentResultFile, state_.last_experiment_id);

  if (setting_->measure_power) {
    // Create a new connection when all experiments finished
    power_tool_controller_.reset(
        new PowerToolController(constants_.kPowerToolServerConfigFile));
    power_tool_controller_->Connect();

    if (!power_tool_controller_->FinishAllExp()) {
      LOG(FATAL) << "Cannot send finish all experiments command";
    }

    // Disconnect the connection to the server
    power_tool_controller_.reset();
  }

  if (setting_->rsync_logs_after_all) {
    ExecuteCommandAsRoot(constants_.kSyncOutToPcScript);

    if (setting_->clean_logs_after_all)
      ExecuteCommandAsRoot(constants_.kCleanLogsScript);
  }

  // Reset no_further_experiment to restart experiment process
  // and do new experiments
  state_.start_new_experiments = true;
  if (!state_.SaveToFile(constants_.kBpStateFile))
    LOG(FATAL) << "Cannot save browser profiler state to file";

  // Restore the original command line file if needed
  if (!state_.experiment_command_lines.empty()) {
    RestoreBackupCommandLine();
  }

  client_->FinishAllExperiments();
}

std::string BrowserProfilerImpl::GenerateExperimentId(
    const std::string& current_experiment_url) {
  const size_t kTimeSize = 64;
  char now_formatted_str[kTimeSize] = "";
  std::time_t t = std::time(NULL);

  // month-day_hour.minute.second{a.m.,p.m.}
  if (!std::strftime(now_formatted_str, kTimeSize, "%m-%d_%I.%M.%S%p", std::localtime(&t))) {
    LOG(ERROR) << "Cannot get formatted now";
  }

  std::string hardware_model_name;
#if defined(OS_ANDROID)
  hardware_model_name = base::SysInfo::HardwareModelName(); 
#endif

  return setting_->browser_config_name + "." + HostInUrl(current_experiment_url) +
       "." + now_formatted_str + "." + hardware_model_name;
}

void BrowserProfilerImpl::StartPowerSampling() {
  if (!power_tool_controller_->StartSampling()) {
    LOG(FATAL) << "Cannot start sampling power"; 
  }
}

void BrowserProfilerImpl::StopPowerSampling() {
  ExecuteCommandAsRoot(sync_workload_cpu_setup_command_);

  // Run a single thread (avoid thread migration issues)
  // on max core id (typically a big core)
  // Run a 0.9 sec workload Exynos 5422
  android_cpu_tools::WorkloadGenerator::RunWorkload(
      std::vector<size_t>(1, android_cpu_tools::CommandLineCpuInfo::MaxCoreId()),
      10000);

  std::ostringstream sync_workload_end_time;
  sync_workload_end_time << std::fixed << std::setprecision(6)
      << MonotonicNow();

  experiment_result_.Put(
      ExperimentResult::kSyncWorkloadEndTimeKey, sync_workload_end_time.str());

  ExecuteCommandAsRoot(default_cpu_setup_command_);

  if (!power_tool_controller_->StopSampling(
        experiment_result_.LogHeaderLine(), experiment_result_.LogLine())) {
    LOG(FATAL) << "Cannot stop sampling power"; 
  }
}

// Update experiment indexes
// If new command line is reached, replace current command line with it
void BrowserProfilerImpl::UpdateExperimentIndexAndCommandLine() {
  VLOG(2) << "Current url try done: " << state_.current_url_try_done;
  VLOG(2) << "Current url index: " << state_.current_url_index;
  VLOG(2) << "Experiment command line index: " << state_.experiment_command_line_index;

  bool use_next_command_line = false;

  ++state_.current_url_try_done;

  if (state_.current_url_try_done >= setting_->num_try_per_url) {
    ++state_.current_url_index;
    state_.current_url_try_done = 0;

    if (state_.current_url_index >= state_.experiment_urls.size()) {
      ++state_.experiment_command_line_index;
      state_.current_url_index = 0;

      // When there is no experiment_urls, '>' will occur
      if (state_.experiment_command_line_index >= state_.experiment_command_lines.size()) {
        state_.all_experiments_finished = true;
      } else {
        use_next_command_line = true;
      }
    }
  }

  if (use_next_command_line)
    ReplaceCurrentWithNextExperimentCommandLine();

  /* Add/remove options to/from current command line for next expr */
  if (setting_->test_hot_load)
    ManipulateCommandLineForHotPageLoad();
}

void BrowserProfilerImpl::BackupCurrentCommandLine() {
  if (!base::CopyFile(browser_command_line_file_,
          GenerateBackupFileName(browser_command_line_file_))) {
    LOG(ERROR) << "Cannot backup current command line";
  }
}

void BrowserProfilerImpl::RestoreBackupCommandLine() {
    if (!base::CopyFile(GenerateBackupFileName(browser_command_line_file_),
            browser_command_line_file_)) {
      LOG(ERROR) << "Cannot restore original command line";
    }
}

std::string BrowserProfilerImpl::BrowserCommandLine() {
  std::string current_command_line_;
  if (!CheckedReadFileToString(browser_command_line_file_, &current_command_line_)) {
    LOG(FATAL) << "Cannot read current command line";
  }

  TrimTrailingNewLine(&current_command_line_);

  return current_command_line_;
}

void BrowserProfilerImpl::StartInternalTracing() {
  VLOG(0) << "Start internal tracing";

  internal_tracing_controller_ = client_->GetInternalTracingControllerInstance();
  if (internal_tracing_controller_  == nullptr)
    LOG(FATAL) << "Fail to initialize internal tracing controller";

  if (internal_tracing_controller_ == nullptr
      || !internal_tracing_controller_->StartTracing(this, setting_->tracing_categories,
        "record-as-much-as-possible")) {
    LOG(ERROR) << "Failed to start ChromeTracing";
  } else {
    chrome_tracing_started_ = true;
  }
}

void BrowserProfilerImpl::InitializeCpuSetupCommands() {
  std::string num_cores =
      base::IntToString(android_cpu_tools::CommandLineCpuInfo::MaxCoreId() - android_cpu_tools::CommandLineCpuInfo::MinCoreId() + 1);

  default_cpu_setup_command_.AppendSwitchASCII(
      switches::kAutoHotplugType, android_cpu_tools::CommandLineCpuInfo::AutoHotplug());
  default_cpu_setup_command_.AppendSwitchASCII(
      switches::kSetAutoHotplug, switches::kOn);
  default_cpu_setup_command_.AppendSwitchASCII(
      switches::kSetNumOnlineCores, num_cores);
  default_cpu_setup_command_.AppendSwitchASCII(
      switches::kSetGovernor, android_cpu_tools::CommandLineCpuInfo::FirstFreqGovernor());
  default_cpu_setup_command_.AppendSwitchASCII(
      switches::kMinFreq, base::UintToString(android_cpu_tools::CommandLineCpuInfo::MinFreq()));
  default_cpu_setup_command_.AppendSwitchASCII(
      switches::kMaxFreq, base::UintToString(android_cpu_tools::CommandLineCpuInfo::MaxFreq()));

  VLOG(1) << "Default cpu setup command: "
      << default_cpu_setup_command_.GetCommandLineString();

  // Turn on all cores and put them at max freq
  sync_workload_cpu_setup_command_.AppendSwitchASCII(
      switches::kAutoHotplugType, android_cpu_tools::CommandLineCpuInfo::AutoHotplug());
  sync_workload_cpu_setup_command_.AppendSwitchASCII(
      switches::kSetAutoHotplug, switches::kOff);
  sync_workload_cpu_setup_command_.AppendSwitchASCII(
      switches::kSetNumOnlineCores, num_cores);
  sync_workload_cpu_setup_command_.AppendSwitchASCII(
      switches::kSetGovernor, "performance");
  sync_workload_cpu_setup_command_.AppendSwitchASCII(
      switches::kMaxFreq, base::UintToString(android_cpu_tools::CommandLineCpuInfo::MaxFreq()));

  VLOG(1) << "Sync workload cpu setup command: "
      << default_cpu_setup_command_.GetCommandLineString();
}

void BrowserProfilerImpl::StartFtrace() {
  ExecuteCommandAsRoot(constants_.kStartFtraceScript);
}

void BrowserProfilerImpl::StopFtrace(const std::string& output_prefix) {
  std::string cmd(constants_.kStopFtraceScript.value());
  cmd.append(" " + output_prefix);
  ExecuteCommandAsRoot(cmd);
}

void BrowserProfilerImpl::StartScreenRecord(const std::string& output_prefix) {
  ExecuteCommandAsRoot(constants_.kStartScreenRecordScript.value() + " " + output_prefix);
}

void BrowserProfilerImpl::StopScreenRecord() {
  ExecuteCommandAsRoot(constants_.kStopScreenRecordScript);
}

void BrowserProfilerImpl::StartCpuUtilizationMonitor(const std::string& output_prefix) {
  ExecuteCommandAsRoot(constants_.kStartCpuUtilizationMonitorScript.value() + " " + output_prefix);
}

void BrowserProfilerImpl::StopCpuUtilizationMonitor() {
  ExecuteCommandAsRoot(constants_.kStopCpuUtilizationMonitorScript);
}

void BrowserProfilerImpl::StartCapturePackets(const std::string& prefix) {
  std::string pcap_file(constants_.kBpOutDir.value());
  pcap_file.append(prefix + ".");
  pcap_file.append(constants_.kPcapBaseName);

  std::string capture_packets_cmd(constants_.kStartCapturePacketsScript.value());
  capture_packets_cmd.append(" " + pcap_file);
  ExecuteCommandAsRoot(capture_packets_cmd);
}

void BrowserProfilerImpl::StopCapturePackets() {
  ExecuteCommandAsRoot(constants_.kStopCapturePacketsScript);
}

void BrowserProfilerImpl::ClearDnsCache() {
  ExecuteCommandAsRoot(constants_.kClearDnsCacheCommand);
}

BrowserProfilerImpl::Setting::Setting()
  : capture_packets(false),
    do_ftrace(false),
    do_itrace(false),
    tracing_categories("*"),
    measure_power(false),
    num_try_per_url(5),
    need_clear_cache(false),
    clear_dns_cache(false),
    user_think_time_millis(0),
    rsync_logs_after_all(false),
    clean_logs_after_all(false),
    screen_record(false),
    monitor_cpu_utilization(false),
    test_hot_load(false),
    browser_config_name("UnknownConfig") {
  const base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();

  std::string num_try_str = command_line.GetSwitchValueASCII(switches::kNumTryPerUrl);
  if (!num_try_str.empty()) {
    if (!base::StringToUint(num_try_str, &num_try_per_url)) { 
      LOG(ERROR) << "Cannot parse switch " << switches::kNumTryPerUrl << ": " << num_try_str;
    }
  }

  do_itrace = command_line.HasSwitch(switches::kDoItrace);
  if (do_itrace)
    tracing_categories = command_line.GetSwitchValueASCII(switches::kDoItrace);

  std::string utt_str = command_line.GetSwitchValueASCII(switches::kUserThinkTimeMillis);
  if (!utt_str.empty()) {
    if (!base::StringToUint(utt_str, &user_think_time_millis)) { 
      LOG(ERROR) << "Cannot parse switch " << switches::kUserThinkTimeMillis << ": " << utt_str;
    }
  }

  capture_packets = command_line.HasSwitch(switches::kCapturePackets);
  do_ftrace = command_line.HasSwitch(switches::kDoFtrace);
  measure_power = command_line.HasSwitch(switches::kMeasurePower);
  need_clear_cache = command_line.HasSwitch(switches::kClearCache);
  clear_dns_cache = command_line.HasSwitch(switches::kClearDnsCache);
  rsync_logs_after_all = command_line.HasSwitch(switches::kRsyncLogsAfterAll);
  clean_logs_after_all = command_line.HasSwitch(switches::kCleanLogsAfterAll);
  screen_record = command_line.HasSwitch(switches::kScreenRecord);
  monitor_cpu_utilization = command_line.HasSwitch(switches::kMonitorCpuUtilization);
  test_hot_load = command_line.HasSwitch(switches::kTestHotLoad);

  if (command_line.HasSwitch(switches::kBrowserConfigName))
    browser_config_name = command_line.GetSwitchValueASCII(switches::kBrowserConfigName);
}

}  // namespace browser_profiler
