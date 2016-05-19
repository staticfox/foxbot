# plugins automake file
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS  = -module -export-dynamic -avoid-version -no-undefined -shared
AM_LDFLAGS += -export-symbols-regex _fox_module
LIBS += $(top_srcdir)/src/libfoxbot.a

plugin_libsdir = $(PLUIGIN_DIR)

plugin_libs_LTLIBRARIES = echo_test.la
