# This is the main build script.
# This is where we configure how ZestCode is built.
# It's used in combination with scripts/v5.ini, which tells Meson (the build system)
# what tools to use and what platform the program is built for.

# project configuration
project(
  'zestcode', # project name (zestcode)
  ['c', 'cpp'], # written in both C and C++ (for now)
  version : '0.0.0',
  meson_version: '>= 1.2.0', # we use splitlines which was introduced in meson v1.2.0
  default_options: [
    'c_std=gnu2x', # latest c standard supported in gcc v13
    'cpp_std=gnu++23', # latest c++ standard supported in gcc v13
    'warning_level=3', # 0, 1, 2, 3, or everything. Higher means more warnings
    'optimization=s', # minimize binary size to minimize upload times. We have a ton of performance overhead
    'debug=true', # debug symbols are not included in the final .bin file, so they don't increase upload time at all
    'force_fallback_for=v5, v5_header'
  ],
)

# patches the .clangd file so the system includes work.
# runs whenever meson is configured
python = find_program('python3')
patch_clangd = files('scripts' / 'patch_clangd.py')
run_command(python, patch_clangd, meson.get_compiler('c'), check: true)

# possible source file extensions
file_exts = [
  '*.s', # assembly
  '*.S', # assembly
  '*.c', # C
  '*.h', # C
  '*.cpp', # C++
  '*.hpp' # C++
]
get_files = files('scripts' / 'get_files.py')
# get all source files
get_files_result = run_command(python, get_files, 'src', file_exts, check: true)
source = get_files_result.stdout().strip().splitlines()

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

# include directories.
# we only specify the top level in order to enforce paths in include directives.
include = include_directories('./include')

# this is what user projects will link against, so we use declare_dependency instead of static_library
zestcode_dep = declare_dependency(
  sources: source,
  include_directories: include,
  dependencies: dependency('v5_header'),
)

# TODO: figure out a better way to do this in a future Tests PR
elf = executable(
  'program.elf',
  sources: './tests/examples/basic.cpp',
  include_directories: include,
  dependencies: [
    dependency('v5'),
    zestcode_dep.as_link_whole() # link whole so libc symbols are overridden
  ]
)

# meson won't create the binary file for uploading, so we have to do it ourselves
objcopy = find_program('arm-none-eabi-objcopy')
custom_target(
  'program.bin',
  output: 'program.bin',
  input: elf,
  build_by_default: true, # otherwise it won't be built
  command: [objcopy, ['-O', 'binary', '-S', '@INPUT@', '@OUTPUT@']],
)
