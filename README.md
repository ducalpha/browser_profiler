## Browser Profiler

A tool for profiling energy consumption of web browser during web page loading.

# Design documentation
https://docs.google.com/document/d/1GmUjzytqePOqvnfsUt-sbVPjnJ3iOVsdZCksipiDZso/edit

# Dependency
Android cpu tools: https://github.com/ducalpha/android_cpu_tools

# Sample patches for integrating with Chromium 38:
For integrating Browser Profiler to Chromium 38, checkout the https://github.com/ducalpha/browser_profiler to base/third_party, then apply the following patches: browser_profiler_for_chromium_38.txt and browser_profiler_for_chromium_38_webkit_patch.txt
