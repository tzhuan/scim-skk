AM_CPPFLAGS = \
	-DSCIM_SKK_LOCALEDIR=\"$(datadir)/locale\" \
	-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\"
INCLUDES	= \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src 

noinst_HEADERS = \
	conv_table.h \
	$(HELP_FILE) \
	cdb.h \
	scim_skk_intl.h \
	scim_skk_config.h \
	scim_skk_lookup_table.h \
	scim_skk_automaton.h \
	scim_skk_history.h \
	scim_skk_imengine.h \
	scim_skk_dictionary.h \
	scim_skk_core.h \
	scim_skk_keybind.h \
	scim_skk_prefs.h

moduledir = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES = skk.la

skk_la_SOURCES = \
	cdb.cpp \
	scim_skk_config.cpp \
	scim_skk_lookup_table.cpp \
	scim_skk_history.cpp \
	scim_skk_imengine.cpp \
	scim_skk_automaton.cpp \
	scim_skk_dictionary.cpp \
	scim_skk_core.cpp \
	scim_skk_keybind.cpp \
	conv_table.cpp

skk_la_CFLAGS = @SCIM_CFLAGS@  @SCIM_DEBUG_FLAGS@
skk_la_CXXFLAGS = @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

skk_la_LDFLAGS = \
	-avoid-version \
	-module \
	-rpath $(moduledir) \
	@SCIM_LIBS@


if SCIM_BUILD_SETUP
SETUP_MODULE = skk-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

skk_imengine_setup_la_SOURCES = \
	scim_skk_setup_dictionaries.cpp \
	scim_skk_imengine_setup.cpp


skk_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

skk_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

skk_imengine_setup_la_LDFLAGS = -avoid-version \
                                -rpath $(setupmoduledir) \
                                -module \
                                @LIBTOOL_EXPORT_OPTIONS@ \
                                @SCIM_GTKUTILS_LIBS@ \
                                @INTLLIBS@ \
                                @SCIM_LIBS@


HELP_SCRIPT		= ./help_message_conv.awk
HELP_FILE               = scim_skk_help_message.txt

$(HELP_FILE):
	$(HELP_SCRIPT) $(HELP_FILE).in > $(HELP_FILE)

scim_skk_imengine.cpp: $(HELP_FILE)
