project(
  'btrfs-soft-reboot-generator',
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

libeconf = cc.find_library('libeconf')
libmount = cc.find_library('mount')

prefixdir = get_option('prefix')
if not prefixdir.startswith('/')
        error('Prefix is not absolute: "@0@"'.format(prefixdir))
endif
systemgeneratorsdir = prefixdir / 'lib/systemd/system-generators'


btrfs_soft_reboot_generator_c = ['src/btrfs-soft-reboot-generator.c']

executable('btrfs-soft-reboot-generator',
           btrfs_soft_reboot_generator_c,
           dependencies : [libeconf, libmount],
           install : true)

config_h = configure_file(
  output : 'config.h',
  configuration : conf)
