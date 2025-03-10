# main project config
# here, we specify the name, what programming languages we use,
# the project version, and some default options like the warning level
# and c/c++ standards.
project('zestcode', ['c', 'cpp'],
  version : '0.0.0',
  default_options : ['warning_level=0', 'c_std=gnu2x', 'cpp_std=gnu++23'])

# global defines to configure standard library
stdlib_conf = [
  '-D_POSIX_THREADS',
  '-D_UNIX98_THREAD_MUTEX_ATTRIBUTES',
  '-D_POSIX_TIMERS',
  '-D_POSIX_MONOTONIC_CLOCK'
]

add_global_arguments(meson.get_external_property('arch_flags'), language: ['c', 'cpp'])
add_global_arguments(stdlib_conf, language: ['c', 'cpp'])

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
  './src/rtos/heap_4.c',
  './src/main.cpp'
]

inc = [
  './include/',
  './include/common/',
  './include/pros/',
  './include/rtos/',
  './include/system/',
  './include/vdml',
]

v5dep = dependency('v5')

executable(
  'zestcode',
  sources: src,
  include_directories: inc,
  link_args: '-nostdlib',
  dependencies: v5dep
)