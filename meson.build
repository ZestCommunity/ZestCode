project('zestcode', 'c', 'cpp',
  version : '0.1',
  default_options : ['warning_level=10', 'c_std=gnu2x', 'cpp_std=c++20'])

fw_dir = 'firmware' 
eo = [fw_dir / 'libc.a', fw_dir / 'libm.a', fw_dir / 'libc.a', fw_dir / 'libv5rt/sdk/vexv5/libv5rt.a' ]


add_project_arguments(['-mcpu=cortex-a9', '-mfpu=neon-fp16', '-mfloat-abi=hard', '-Os', '-g', ], language: ['c', 'cpp'])
add_project_arguments(['-D_POSIX_THREADS', '-D_UNIX98_THREAD_MUTEX_ATTRIBUTES', '-D_POSIX_TIMERS', '-D_POSIX_MONOTONIC_CLOCK'], language: ['c', 'cpp'])


add_project_arguments(['-ffunction-sections', '-fdata-sections', '-fdiagnostics-color', '-funwind-tables'], language: ['c', 'cpp'])

add_project_arguments(['-Wno-psabi', '-Wall', '-Wpedantic'], language: ['c', 'cpp'])

add_project_link_arguments(['-nostdlib'], language: ['c', 'cpp'])
add_project_link_arguments([
    '-T../@0@/v5.ld'.format(fw_dir),
    '-T../@0@/v5-common.ld'.format(fw_dir),
    '-Wl,--no-warn-rwx-segments',
    '-Wl,--gc-sections',
    '-Wl,--start-group',
    '-Wl,-lgcc',
    '-Wl,-lstdc++',
    '-Wl,--end-group',
  ],language: ['c', 'cpp'])


src = [
  './src/system/rtos_hooks.c',
  './src/system/user_functions.c',
  './src/system/cpp_support.cpp',
  './src/system/startup.cpp',
  './src/system/mlock.c',
  './src/system/hot.c',
  './src/system/dev/file_system_stubs.c',
  './src/system/dev/dev_driver.c',
  './src/system/dev/usd_driver.c',
  './src/system/dev/ser_daemon.c',
  './src/system/dev/ser_driver.c',
  './src/system/dev/vfs.c',
  './src/system/unwind.c',
  './src/system/envlock.c',
  './src/system/stubs.cpp',
  './src/system/system_daemon.c',
  './src/devices/vdml_link.c',
  './src/devices/vdml_link.cpp',
  './src/devices/screen.cpp',
  './src/devices/vdml_rotation.c',
  './src/devices/vdml_device.cpp',
  './src/devices/vdml_imu.c',
  './src/devices/vdml_serial.cpp',
  './src/devices/vdml_motors.cpp',
  './src/devices/vdml_distance.cpp',
  './src/devices/vdml_rotation.cpp',
  './src/devices/vdml_serial.c',
  './src/devices/vdml_adi.c',
  './src/devices/vdml_vision.cpp',
  './src/devices/controller.c',
  './src/devices/screen.c',
  './src/devices/battery.cpp',
  './src/devices/vdml_gps.c',
  './src/devices/vdml_adi.cpp',
  './src/devices/vdml_distance.c',
  './src/devices/vdml_usd.c',
  './src/devices/vdml_motors.c',
  './src/devices/vdml.c',
  './src/devices/vdml_motorgroup.cpp',
  './src/devices/controller.cpp',
  './src/devices/vdml_ai_vision.c',
  './src/devices/registry.c',
  './src/devices/battery.c',
  './src/devices/vdml_ai_vision.cpp',
  './src/devices/vdml_optical.c',
  './src/devices/vdml_device.c',
  './src/devices/vdml_vision.c',
  './src/devices/vdml_ext_adi.c',
  './src/devices/vdml_imu.cpp',
  './src/devices/vdml_usd.cpp',
  './src/devices/vdml_optical.cpp',
  './src/devices/vdml_gps.cpp',
  './src/main.cpp',
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
fs = import('fs')

inc_dirs = [include_directories('include', './firmware/libv5rt/sdk/vexv5/patched_include')]

# TODO: make more robust (only add include dir for source files in that dir and don't replace all instances of src with include)
foreach src_file : src
  inc_dir = fs.parent(src_file).replace('src', 'include')
  if fs.exists(inc_dir)
    inc_dirs += inc_dir
  endif
endforeach

exe = executable('zestcode', 
  src,
  objects : eo,
  install : true,
  include_directories : inc_dirs
  )

test('basic', exe)