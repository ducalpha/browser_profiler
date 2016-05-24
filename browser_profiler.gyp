# Copyright 2016 Duc Hoang Bui, KAIST. All rights reserved.
# Licensed under MIT ($DUC_LICENSE_URL)

{
  'targets': [
    {
      'target_name': 'browser_profiler',
      'type': 'static_library',
      'toolsets': ['host', 'target'],
      'include_dirs': [
        '../../../',
        '.'
      ],
      'sources': [
        'browser_profiler_impl.cc',
        'browser_profiler_impl.h',
        'browser_profiler_impl_constants.cc',
        'browser_profiler_impl_constants.h',
        'browser_profiler_impl_state.cc',
        'browser_profiler_impl_state.h',
        'browser_profiler_impl_switches.cc',
        'browser_profiler_impl_switches.h',
        'experiment_result.cc',
        'experiment_result.h',
        'power_tool_connection_impl.cc',
        'power_tool_connection_impl.h',
        'power_tool_controller.cc',
        'power_tool_controller.h',
        'public/browser_profiler.cc',
        'public/browser_profiler.h',
        'public/power_tool_connection.cc',
        'public/power_tool_connection.h',
      ],
    },
  ],
}
