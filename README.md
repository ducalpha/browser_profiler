# Browser Profiler

A tool for profiling energy consumption of web browser during web page loading.

[Design and How to Patch](https://docs.google.com/document/d/1GmUjzytqePOqvnfsUt-sbVPjnJ3iOVsdZCksipiDZso/edit?usp=sharing)

Browser Profiler is designed as a portable library for convenient integration to different web browsers (e.g., Chromium and Firefox). A patch set for integrating Browser Profiler to Chromium 38 is included as an example for the glue code layer. However, it is easy to integrate to Chromium master branch’s HEAD which I'd do upon request.

## Dependency
[Android cpu tools](https://github.com/ducalpha/android_cpu_tools) for determining number of CPU cores and running a synchronization workload.
