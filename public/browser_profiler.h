// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT ($LICENSE_URL)

#ifndef BROWSER_PROFILER_PUBLIC_BROWSER_PROFILER_H_
#define BROWSER_PROFILER_PUBLIC_BROWSER_PROFILER_H_

#include <string>

#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
#include <memory>
#else
#include "base/memory/scoped_ptr.h"
#endif
#include "base/files/file_path.h"

namespace browser_profiler {
// Assumption: Profiler assumes information of cpu cores is loaded in command line
// It will try to initialize cpu-info command line
// However, it is ProfilerClient that read the cpu info into the browser command line


class BrowserProfilerClient;

// Services provided by a profiler
// The implementer must call Prepare(), PostProcess() and ClearCacheIfNeeded() in
// appropriate points in browser code
// Besides, the implementer must make the start up url of the browser to
// load BrowserProfiler's Prepare() url
// On Chromium at startup loading, PostProcess will be called without Prepare() for the startup page
class BrowserProfiler {
 public:
  BrowserProfiler(BrowserProfilerClient* client);

  // Must call this before calling other functions
  virtual void Initialize(const base::FilePath& browser_command_line_file,
    const base::FilePath& cpu_info_command_line_file) = 0;

  // Prepare environment experiments
  // Must be called right before web page loading started
  // output the url to experiment
  // return true on success, false on failure
  virtual bool Prepare(std::string *experiment_url) = 0;

  // Do post processing after an experiment
  // return true on success, false on failure
  virtual bool PostProcess(const std::string& url,
      double navigation_start_monotonic_time, double load_event_end_monotonic_time) = 0;

  // Delete all the cache, given a cache path and enabled argument switch
  // Must be called before the cache is initialized
  // It is the responsibility
  virtual void ClearCacheIfNeeded(const base::FilePath& cache_path) = 0;

  // Passing a method pointer is not trivial in C++98
  // TODO: change to std::function in C++11 for more flexibility
  virtual void DelayedTaskCallback() = 0;

  // For scoped_ptr
#if !(defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900)
  virtual ~BrowserProfiler() {}
#endif

 protected:
#if defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900
	std::unique_ptr<BrowserProfilerClient> client_;
#else
	scoped_ptr<BrowserProfilerClient> client_;
#endif
};


// Interface for the client of a profiler
class BrowserProfilerClient {
 public:
  // This will call Profiler::DelayedCallback after delay_millis
  // TODO: change to std::function in C++11
  typedef void (BrowserProfiler::*DelayedTaskCallback)();
  virtual void DoDelayedTask(DelayedTaskCallback delayed_callback, long delay_millis) = 0;

  virtual void RestartBrowser() = 0;

  // Do some action after finishing all experiments
  // E.g., stop restarting web browser
  virtual void FinishAllExperiments() = 0;

  // Close the currently active shell
  virtual void CloseActiveShell() = 0;

  // For scoped_ptr
#if !(defined(COMPILER_GCC) && __cplusplus >= 201103L && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40900)
  virtual ~BrowserProfilerClient() {}
#endif
};

} // namespace browser_profiler 

#endif // BROWSER_PROFILER_PUBLIC_BROWSER_PROFILER_H_
