glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')

cairo_prefix = dependency('cairo-gobject').get_pkgconfig_variable('prefix')
cairo_docpath = join_paths(cairo_prefix, 'share', 'gtk-doc', 'html', 'cairo')

docpath = join_paths(gtk_datadir, 'gtk-doc', 'html')

version_conf = configuration_data()
version_conf.set('GTK_VERSION', meson.project_version())

subdir('gdk')
subdir('gsk')
