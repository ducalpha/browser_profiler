// Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
// Licensed under MIT (https://github.com/ducalpha/browser_profiler/blob/master/LICENSE)

#include "public/browser_profiler.h"

namespace browser_profiler {

BrowserProfiler::BrowserProfiler(BrowserProfilerClient* client)
  : client_(client) {
}

} // namespace browser_profiler 
