kde4_header()

add_subdirectory( pics ) 
add_subdirectory( examples ) 

include_directories( ${CMAKE_SOURCE_DIR}/kwordquiz/icons ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kwordquiz_SRCS 
   kwordquizprefs.cpp 
   kwordquiz.cpp 
   main.cpp 
   qaview.cpp 
   flashview.cpp 
   multipleview.cpp 
   wqquiz.cpp 
   wqscore.cpp 
   prefeditor.cpp 
   prefquiz.cpp 
   dlgsort.cpp 
   dlgrc.cpp 
   dlgspecchar.cpp 
   dlglanguage.cpp 
   wqprintdialogpage.cpp 
   prefcharacter.cpp 
   wqundo.cpp 
   prefcardappearance.cpp 
   kwqnewstuff.cpp 
   kwqnewstuff.h 
   kwqtablemodel.cpp 
   kwqtableview.cpp 
   kwqtabledelegate.cpp
   )

kde4_automoc(${kwordquiz_SRCS})

kde4_add_ui_files(kwordquiz_SRCS 
   dlglanguagebase.ui 
   flashviewbase.ui 
   multipleviewbase.ui 
   qaviewbase.ui 
   prefeditorbase.ui 
   prefquizbase.ui 
   dlgsortbase.ui 
   dlgrcbase.ui 
   prefcharacterbase.ui 
   prefcardappearancebase.ui )

kde4_add_kcfg_files(kwordquiz_SRCS prefs.kcfgc )

kde4_add_executable(kwordquiz ${kwordquiz_SRCS})

target_link_libraries(kwordquiz  ${QT_AND_KDECORE_LIBS} kdeeducore kdeui kdeprint knewstuff )

install_targets(/bin kwordquiz )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/Edutainment/Miscellaneous FILES kwordquiz.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES kwordquiz.kcfg )
install_files( ${CONFIG_INSTALL_DIR} FILES kwordquizrc )
install_files( ${DATA_INSTALL_DIR}/kwordquiz FILES kwordquizui.rc eventsrc )
install_files( ${MIME_INSTALL_DIR}/application FILES x-kwordquiz.desktop )

kde4_footer()



#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#bin_PROGRAMS = kwordquiz
#
### INCLUDES were found outside kdevelop specific part
#
#kwordquiz_SOURCES = kwordquizprefs.cpp kwordquiz.cpp main.cpp \
#	dlglanguagebase.ui flashviewbase.ui multipleviewbase.ui qaviewbase.ui qaview.cpp \
#	flashview.cpp multipleview.cpp wqquiz.cpp wqscore.cpp prefeditorbase.ui \
#	prefquizbase.ui prefeditor.cpp prefquiz.cpp dlgsortbase.ui dlgsort.cpp dlgrcbase.ui \
#	dlgrc.cpp dlgspecchar.cpp dlglanguage.cpp wqprintdialogpage.cpp prefcharacter.cpp \
#	prefcharacterbase.ui wqundo.cpp prefs.kcfgc prefcardappearance.cpp prefcardappearancebase.ui \
#	kwqnewstuff.cpp kwqnewstuff.h kwqtablemodel.cpp kwqtableview.cpp
#kwordquiz_LDADD = ../../libkdeedu/kdeeducore/libkdeeducore.la $(LIB_QT) \
#	$(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIBSOCKET) $(LIB_KDEPRINT) -lknewstuff
#
#
#EXTRA_DIST = main.cpp kwordquiz.cpp kwordquiz.h kwordquizdoc.cpp kwordquizdoc.h kwordquizui.rc kwordquiz.desktop kwordquizprefs.cpp kwordquizprefs.h
#
#kde_kcfg_DATA = kwordquiz.kcfg
#kde_conf_DATA = kwordquizrc
#mime_DATA = x-kwordquiz.desktop
#mimedir = $(kde_mimedir)/application
#
######## kdevelop will overwrite this part!!! (end)############
## These paths are KDE specific. Use them:
## kde_appsdir         Where your application's menu entry (.desktop) should go to.
## kde_icondir         Where your icon should go to - better use KDE_ICON.
## kde_sounddir        Where your sounds should go to.
## kde_htmldir         Where your docs should go to. (contains lang subdirs)
## kde_datadir         Where you install application data. (Use a subdir)
## kde_locale          Where translation files should go to. (contains lang subdirs)
## kde_cgidir          Where cgi-bin executables should go to.
## kde_confdir         Where config files should go to (system-wide ones with default values).
## kde_mimedir         Where mimetypes .desktop files should go to.
## kde_servicesdir     Where services .desktop files should go to.
## kde_servicetypesdir Where servicetypes .desktop files should go to.
## kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
## kde_wallpaperdir    Where general wallpapers should go to.
## kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
## kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
## kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
## kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
## kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
## kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).
#
## set the include path for X, qt and KDE
#INCLUDES = -I$(top_builddir)/libkdeedu/kdeeducore \
#	-I$(srcdir)/../../libkdeedu/kdeeducore -I$(top_srcdir)/kwordquiz/icons $(all_includes)
#SUBDIRS = . pics examples
#METASOURCES = AUTO
#
## the library search path.
#kwordquiz_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#rcdir = $(kde_datadir)/kwordquiz
#rc_DATA = kwordquizui.rc eventsrc
#
#apps_DATA = kwordquiz.desktop
#appsdir = $(kde_appsdir)/Edutainment/Miscellaneous
#
#messages: rc.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	 $(XGETTEXT) $$LIST -o $(podir)/kwordquiz.pot; \
#	fi
#
#
#noinst_HEADERS = qaview.h flashview.h multipleview.h wqquiz.h wqlistitem.h \
#	wqscore.h prefeditor.h prefquiz.h dlgsort.h dlgrc.h dlgspecchar.h dlglanguage.h \
#	dlglanguage.h wqprintdialogpage.h prefcharacter.h wqundo.h prefcardappearance.h \
#	kwqtablemodel.h kwqtableview.h