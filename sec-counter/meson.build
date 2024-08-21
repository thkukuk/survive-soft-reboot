project(
  'sec-counter',
  'c',
  meson_version : '>= 0.61.0',
  default_options : [
                  'prefix=/usr',
                  'sysconfdir=/etc',
                  'localstatedir=/var',
                  'buildtype=debugoptimized',
  		  'default_library=shared',
		  'b_pie=true',
                  'b_lto=true',
		  'warning_level=2',],
  license : ['Apache-2.0',],
  version : '0.1.0',
)

conf = configuration_data()
conf.set_quoted('PROJECT_VERSION', meson.project_version())

cc = meson.get_compiler('c')

add_project_arguments(['-D_GNU_SOURCE=1',
		       '-DXTSTRINGDEFINES',
		       '-D_FORTIFY_SOURCE=2'], language : 'c')

possible_cc_flags = [
		  '-fstack-protector-strong',
		  '-funwind-tables',
		  '-fasynchronous-unwind-tables',
		  '-fstack-clash-protection',
		  '-Werror=return-type',
		  '-Wbad-function-cast',
		  '-Wcast-align',
		  '-Wcast-qual',
                  '-Wformat-security',
		  '-Winline',
		  '-Wmissing-declarations',
		  '-Wmissing-prototypes',
		  '-Wnested-externs',
		  '-Wshadow',
		  '-Wstrict-prototypes',
		  '-Wundef',
		  ]
add_project_arguments(cc.get_supported_arguments(possible_cc_flags), language : 'c')

libsystemd = cc.find_library('libsystemd')

prefixdir = get_option('prefix')
if not prefixdir.startswith('/')
        error('Prefix is not absolute: "@0@"'.format(prefixdir))
endif
seccounterdir = prefixdir / 'libexec/sec-counter'
systemunitdir = prefixdir / 'lib/systemd/system'

sec_counter_c = ['src/sec-counter.c']

executable('sec-counter',
           sec_counter_c,
           dependencies : [libsystemd],
           install_dir : seccounterdir,
           install : true)

subdir('units')
