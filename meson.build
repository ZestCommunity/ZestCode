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

# this script runs whenever meson is configured.
# it patches the .clangd file so the system includes work.
arm_none_eabi_gcc = find_program('arm-none-eabi-gcc')
python = find_program('python3')
run_command(python, 'build/patch_clangd.py', arm_none_eabi_gcc, check: true)

# this is what user projects will link against
lib = static_library(
  'libzestcode.a',
  sources: src,
  include_directories: './include/',
  dependencies: dependency('v5_header'), # vex sdk header files
)

elf = executable(
  'program.elf',
  sources: './src/main.cpp',
  include_directories: './include/',
  link_whole: lib, # link_whole to override libc weak symbols
  dependencies: dependency('v5'), # link to the vex sdk
)

# by using find_program, we enable meson to do some checks that it's all working properly.
objcopy = find_program('arm-none-eabi-objcopy')

# meson won't strip the executable, so we have to do it ourselves
custom_target(
  'program.bin',
  output: 'program.bin',
  input: elf,
  build_by_default: true, # otherwise it won't be built
  command: [objcopy, ['-O', 'binary', '-S', '@INPUT@', '@OUTPUT@']],
)