# Browser Profiler

A tool for profiling energy consumption of web browser during web page loading.

[Design and How to Patch](https://docs.google.com/document/d/1GmUjzytqePOqvnfsUt-sbVPjnJ3iOVsdZCksipiDZso/edit?usp=sharing)

Browser Profiler is designed as a portable library for convenient integration to different web browsers (e.g., Chromium and Firefox). A patch set for integrating Browser Profiler to Chromium 38 is included as an example for the glue code layer. However, it is easy to integrate to Chromium master branch’s HEAD which I'd do upon request.

## Dependency
[Android cpu tools](https://github.com/ducalpha/android_cpu_tools) for determining number of CPU cores and running a synchronization workload.

## Application
This tool is used in paper ["Rethinking Energy-Performance Trade-Off in Mobile Web Page Loading"](http://cps.kaist.ac.kr/papers/com073-buiA.pdf), by Duc Hoang Bui, Yunxin Liu, Hyosu Kim, Insik Shin and Feng Zhao, in Proceedings of the 21st ACM Intl. Conference on Mobile Computing and Networking (MobiCom '15), Paris, France, September 2015.

More information (talk, introduction and demo videos) is available at the paper's [web page](http://cps.kaist.ac.kr/?page=research/eBrowser/contents.html).
