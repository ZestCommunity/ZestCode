project('zestcode', 'c', 'cpp',
  version : '0.0.0',
  default_options : ['warning_level=0', 'c_std=gnu2x', 'cpp_std=c++23'])

  # Compiler flags for C and C++
add_project_arguments(
  '-mcpu=cortex-a9',
  '-mfpu=neon-fp16',
  '-mfloat-abi=hard',
  '-ffunction-sections',
  '-fdata-sections',
  '-fdiagnostics-color',
  '-funwind-tables',
  '-Wno-psabi',
  '-D_POSIX_THREADS',
  '-D_UNIX98_THREAD_MUTEX_ATTRIBUTES',
  '-D_POSIX_TIMERS',
  '-D_POSIX_MONOTONIC_CLOCK',
  language : ['c', 'cpp']
)

# Linker flags
add_project_link_arguments(
  '-nostdlib',
  '-Wl,--gc-sections',
  '-Wl,--start-group',
  '-lgcc',
  '-lstdc++',
  '-Wl,--end-group',
  '-Wl,-T./firmware/v5-common.ld',
  '-Wl,--no-warn-rwx-segments',
  language : ['c', 'cpp']
)


src = [
  './src/system/rtos_hooks.c',
  './src/system/user_functions.c',
  './src/system/cpp_support.cpp',
  './src/system/startup.cpp',
  './src/system/mlock.c',
  './src/system/dev/file_system_stubs.c',
  './src/system/dev/usd_driver.c',
  './src/system/dev/ser_daemon.c',
  './src/system/dev/ser_driver.c',
  './src/system/dev/vfs.c',
  './src/system/unwind.c',
  './src/system/envlock.c',
  './src/system/stubs.cpp',
  './src/system/system_daemon.c',
  './src/devices/screen.c',
  './src/devices/battery.cpp',
  './src/devices/vdml_usd.c',
  './src/devices/vdml.c',
  './src/devices/registry.c',
  './src/devices/battery.c',
  './src/common/gid.c',
  './src/common/set.c',
  './src/common/linkedlist.c',
  './src/common/string.c',
  './src/common/cobs.c',
  './src/rtos/queue.c',
  './src/rtos/semphr.c',
  './src/rtos/list.c',
  './src/rtos/task_notify_when_deleting.c',
  './src/rtos/tasks.c',
  './src/rtos/port.c',
  './src/rtos/rtos.cpp',
  './src/rtos/timers.c',
  './src/rtos/stream_buffer.c',
  './src/rtos/heap_4.c'
]

include = [
  './include/',
  './include/common/',
  './include/pros/',
  './include/rtos/',
  './include/system/',
  './include/vdml',
  'firmware/libv5rt/sdk/vexv5/include/',
]

lib_zestcode = static_library(
  'zestcode',
  sources: src,
  include_directories: include,
)

project_src = [
    './src/main.cpp'
]

v5rt_dep = declare_dependency(
  link_args: './firmware/libv5rt/sdk/vexv5/libv5rt.a',
  include_directories: include_directories('firmware/libv5rt/sdk/vexv5/include/')
)

executable('zestcode', 
  sources : project_src,
  link_with : lib_zestcode,
  include_directories: include,
  install : true,
  dependencies: v5rt_dep
)