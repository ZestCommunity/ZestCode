project(
  'zestcode',
  ['c', 'cpp'],
  version : '0.0.0'
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
  './src/system/xilinx_vectors.s',
  './src/devices/screen.c',
  './src/devices/battery.cpp',
  './src/devices/vdml_usd.c',
  './src/devices/vdml.c',
  './src/devices/registry.c',
  './src/devices/battery.c',
  './src/devices/competition.cpp',
  './src/common/gid.c',
  './src/common/set.c',
  './src/common/linkedlist.c',
  './src/common/string2.c',
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
  './src/rtos/portASM.S',
]

libzestcode = static_library(
  'zestcode',
  sources: src,
  include_directories: './include/',
  dependencies: dependency('v5_header'),
)

elf = executable(
  meson.project_name() + '.elf',
  sources: './src/main.cpp',
  include_directories: './include/',
  link_whole: [libzestcode],
  dependencies: dependency('v5'),
)

objcopy = find_program('arm-none-eabi-objcopy')

# meson won't strip the executable, so we have to do it ourselves
custom_target(
  meson.project_name() + '.bin',
  output: meson.project_name() + '.bin',
  input: elf,
  build_by_default: true,
  command: [objcopy, ['-O', 'binary', '-S', '@INPUT@', '@OUTPUT@']],
)