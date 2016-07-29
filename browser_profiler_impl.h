// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#ifndef BROWSER_PROFILER_BROWSER_PROFILER_IMPL_H_
#define BROWSER_PROFILER_BROWSER_PROFILER_IMPL_H_

#include "public/browser_profiler.h"
#include "public/internal_tracing_controller.h"

#include "browser_profiler_impl_constants.h"
#include "browser_profiler_impl_state.h"
#include "experiment_result.h"
#include "power_tool_controller.h"

#include "base/command_line.h"
#include "base/files/file_path.h"

#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
#include <memory>
#else
#include "base/memory/scoped_ptr.h"
#endif

#include "base/compiler_specific.h"

namespace browser_profiler {

/**
 * Profile the browser
 * Setup environment, cycle browser through experiments, and collect experiment results
 *
 * For each experiment_cmd_line
 *   For each url in url_list
 *     For num_try_per_url
 *       Prepare experiment
 *       Perform experiment (loading)
 *       Post process experiment
 *
 * If experiment_cmd_line is not provided, just use the command line
 * Restarts the browser for each experiment
 * State presevered across experiments
 * Tolerate browser crashes
 *
 */
class BrowserProfilerImpl : public BrowserProfiler {
 public:
  BrowserProfilerImpl(BrowserProfilerClient* client);

  // Need to know cpu_info_command_line_file to initialize it if needed
  virtual void Initialize(const base::FilePath& browser_command_line_file,
      const base::FilePath& cpu_info_command_line_file) override;

  virtual bool Prepare(std::string *experiment_url) override;

  virtual bool PostProcess(const std::string& url,
      double navigation_start_monotonic_time, double load_event_end_monotonic_time) override;

  virtual void ClearCacheIfNeeded(const base::FilePath& cache_path) override;

  virtual void DelayedTaskCallback() override;

  virtual void OnInternalTracingStopped() override;

 private:
  void PostProcessInternal();
  void PostProcessInternalSecondHalf();
  void StartTracers();
  void StopTracers();
  void StopTracersSecondHalf();
  void RestartBrowser();
  void ConsolidateExperimentResult(const std::string& url,
        double navigation_start_monotonic_time, double load_event_end_monotonic_time);

  void ManipulateCommandLineForHotPageLoad();
  void ReplaceCurrentWithNextExperimentCommandLine();
  void PostProcessAfterAllExperiments();

  // Generate ID of an experiment result
  // ID includes the time stamp (to seconds) of the start of an experiment
  // and model name
  std::string GenerateExperimentId(const std::string& current_experiment_id);

  void StartPowerSampling();
  void StopPowerSampling();
  void UpdateExperimentIndexAndCommandLine();
  void BackupCurrentCommandLine();
  void RestoreBackupCommandLine();
  std::string BrowserCommandLine();
  void StartInternalTracing();
  void InitializeCpuSetupCommands();

  void StartFtrace();
  void StopFtrace(const std::string& output_prefix);
  void StartScreenRecord(const std::string& output_prefix);
  void StopScreenRecord();
  void StartCpuUtilizationMonitor(const std::string& output_prefix);
  void StopCpuUtilizationMonitor();
  void StartCapturePackets(const std::string& prefix);
  void StopCapturePackets();
  void ClearDnsCache();

  struct Setting;

#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
  std::unique_ptr<Setting> setting_;
	std::unique_ptr<PowerToolController> power_tool_controller_;
#else
  scoped_ptr<Setting> setting_;
	scoped_ptr<PowerToolController> power_tool_controller_;
#endif

	BrowserProfilerImplState state_;

	ExperimentResult experiment_result_;

  base::FilePath browser_command_line_file_;

  BrowserProfilerImplConstants constants_;

  base::CommandLine default_cpu_setup_command_;
  base::CommandLine sync_workload_cpu_setup_command_;

  std::string experiment_id_;

  std::shared_ptr<InternalTracingController> internal_tracing_controller_;
  
  bool chrome_tracing_started_;

  // whether or not the Prepare() is executed
  // E.g., at start up , PostProcess() will be called but not Prepare()
  bool prepared_;
  DISALLOW_COPY_AND_ASSIGN(BrowserProfilerImpl);
};

} // namespace browser_profiler 
#endif // BROWSER_PROFILER_BROWSER_PROFILER_IMPL_H_
