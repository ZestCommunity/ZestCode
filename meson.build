# This is the main build script.
# This is where we configure how ZestCode is built.
# It's used in combination with scripts/v5.ini, which tells Meson (the build system)
# what tools to use and what platform the program is built for.

# project configuration
project(
  'zestcode', # project name (zestcode)
  ['c', 'cpp'], # written in both C and C++ (for now)
  version : '0.0.0',
  default_options: [
    'c_std=gnu2x', # latest c standard supported in gcc v13
    'cpp_std=gnu++23', # latest c++ standard supported in gcc v13
    'warning_level=3', # 0, 1, 2, 3, or everything. Higher means more warnings
    'optimization=s', # minimize binary size to minimize upload times. We have a ton of performance overhead
    'force_fallback_for=v5, v5_header'
  ],
)

# this script runs whenever meson is configured.
# it patches the .clangd file so the system includes work.
python = find_program('python3')
run_command(python, 'scripts/patch_clangd.py', meson.get_compiler('c'), check: true)

# optimization flags passed to the compiler
optimization_flags = [
  '-ffunction-sections', # used in combination with --gc-sections to reduce binary size
  '-fdata-sections', # used in combination with --gc-sections to reduce binary size
  '-fno-strict-aliasing', # needed due to bad coding practices in the FreeRTOS source
]

# formatting flags passed to the compiler and linker
formatting_flags = [
  '-fdiagnostics-color', # makes the compiler output easier to read, by using colors!
]
add_global_arguments(formatting_flags, language: 'c')
add_global_arguments(formatting_flags, language: 'cpp')
add_global_link_arguments(formatting_flags, language: 'c')
add_global_link_arguments(formatting_flags, language: 'cpp')

# miscellaneous flags passed to the linker
linker_flags = [
  '-Wl,--gc-sections', # used in combination with -ffunction-sections and -fdata-sections to reduce binary size
  '-Wl,--no-warn-rwx-segments' # silences a warning that does not make a difference for our use case
]

# system libraries we depend on
system_deps = [
  '-nostdlib', # we still need to implement some newlib stubs
  '-lc', # libc
  '-lm', # libm
  '-lgcc', #libgcc
  '-lstdc++', # libstdc++
]
add_global_link_arguments( system_deps, language: 'c')
add_global_link_arguments(system_deps, language: 'cpp')

# configuration of the standard library
stdlib_conf = [
  '-D_POSIX_MONOTONIC_CLOCK', # enable the POSIX monotonic clock
]

# apply all these flags and configs
add_global_arguments(optimization_flags, formatting_flags, stdlib_conf, language: 'c')
add_global_arguments(optimization_flags, formatting_flags, stdlib_conf, language: 'cpp')
add_global_link_arguments(optimization_flags, linker_flags, formatting_flags, system_deps, language: 'c')
add_global_link_arguments(optimization_flags, linker_flags, formatting_flags, system_deps, language: 'cpp')

# source files
source = [
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

# include directories.
# we only specify the top level in order to enforce paths in include directives.
include = include_directories('./include')

# this is what user projects will link against, so we use declare_dependency instead of static_library
zestcode_dep = declare_dependency(
  sources: source,
  include_directories: include,
  dependencies: dependency('v5_header'), # vex sdk header files
)

elf = executable(
  'program.elf',
  sources: './src/main.cpp',
  include_directories: include,
  dependencies: [
    dependency('v5'), # link to the vex sdk
    zestcode_dep.as_link_whole() # link whole so libc symbols are overridden
  ]
)

# meson won't strip the executable, so we have to do it ourselves
objcopy = find_program('arm-none-eabi-objcopy')
custom_target(
  'program.bin',
  output: 'program.bin',
  input: elf,
  build_by_default: true, # otherwise it won't be built
  command: [objcopy, ['-O', 'binary', '-S', '@INPUT@', '@OUTPUT@']],
)