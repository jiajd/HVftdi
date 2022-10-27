QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FT2232Drv.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CDM/ftd2xx.h \
    FT2232Drv.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/CDM/i386/ -lftd2xx
#else:win32:CONFIG(release, debug|release): LIBS += -L$$PWD/CDM/amd64/ -lftd2xx

LIBS += -L$$PWD/CDM/i386/ -lftd2xx
