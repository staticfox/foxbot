# plugins automake file
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS  = -module -export-dynamic -avoid-version -no-undefined -shared
AM_LDFLAGS += -export-symbols-regex fox_plugin
LIBS += $(top_srcdir)/src/libfoxbot.la

plugin_libsdir = $(PLUGIN_DIR)

plugin_libs_LTLIBRARIES = \
                        echo_test.la \
                        plugin_manager.la
