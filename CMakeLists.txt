project (BROWSER_PROFILER CXX)

set (CMAKE_CXX_FLAGS "-std=c++11")

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

file (GLOB BROWSER_PROFILER_SRC 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc" 
  "${CMAKE_CURRENT_SOURCE_DIR}/public/*.cc")

add_library (browser_profiler STATIC ${BROWSER_PROFILER_SRC})
target_link_libraries (browser_profiler base-chromium)
