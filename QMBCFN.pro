CONFIG += qt

QT += svg

HEADERS += qmbcfn.h \
           mainwindow.h \
           settingsmap.h \
           mainwidget.h \
           functionwidget.h \
           paramderivswidget.h \
           constantswidget.h \
           chisqrextratermwidget.h \
           fitsettingswidget.h \
           line_edit_delegate.h \
           key_press_filter.h \
           file_open_filter.h \
           convert_qstring.h \
           abstract_global_model.h \
           parse_model.h \
           abstract_prior.h \
           gaussian_prior.h \
           fitter.h \
           parser.h \
           chisqr_extra_term.h \
           fit_thread.h \
           fit_dialog.h \
           bootstrap_thread.h \
           bootstrap_dialog.h \
           spacing.h

SOURCES += qmbcfn.cpp \
           main.cpp \
           mainwindow.cpp \
           mbf_load_save.cpp \
           settingsmap.cpp \
           mainwidget.cpp \
           functionwidget.cpp \
           paramderivswidget.cpp \
           constantswidget.cpp \
           chisqrextratermwidget.cpp \
           fitsettingswidget.cpp \
           line_edit_delegate.cpp \
           key_press_filter.cpp \
           file_open_filter.cpp \
           convert_qstring.cpp \
           parse_model.cpp \
           gaussian_prior.cpp \
           fitter.cpp \
           parser.cpp \
           chisqr_extra_term.cpp \
           fit_thread.cpp \
           fit_dialog.cpp \
           bootstrap_thread.cpp \
           bootstrap_dialog.cpp

RESOURCES += QMBCFN.qrc

LIBS += -lgsl -lgslcblas -lm

QMAKE_CXXFLAGS += -funswitch-loops -fgcse-after-reload

