project(
  'dualsensectl',
  'c',
  version: '0.7.0',
  license: 'GPLv2',
  meson_version: '>=0.58.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    ],
  )

cc = meson.get_compiler('c')
extra_args = ['-DDUALSENSECTL_VERSION="@0@"'.format(meson.project_version())]
deps = []

if host_machine.system() == 'darwin'
  # macOS: use generic hidapi (IOKit backend), no udev
  hidapi = dependency('hidapi')
  deps += hidapi

  # Link IOKit and CoreFoundation frameworks for the monitor command
  iokit = dependency('appleframeworks', modules: ['IOKit', 'CoreFoundation'])
  deps += iokit
else
  # Linux: use hidapi-hidraw backend + libudev
  extra_args += '-Wl,--exclude-libs=ALL'
  hidapi_hidraw = dependency('hidapi-hidraw')
  udev = dependency('libudev')
  deps += [hidapi_hidraw, udev]
endif

add_project_arguments(extra_args, language: 'c')

executable(
  'dualsensectl',
  ['main.c'],
  dependencies: deps,
  install: true,
  )
