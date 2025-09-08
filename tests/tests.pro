QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

include(../FirmwarePackager/FirmwarePackager.pri)

INCLUDEPATH += \
  ../FirmwarePackager

HEADERS += \
    # testnetworkmanager/tst_testnetworkmanager.h \
    # testsqlite/tst_testsqlite.h \
    # testudpprobe/ts_testudpprobe.h

SOURCES += \
    # main.cpp \
    # testnetworkmanager/tst_testnetworkmanager.cpp \
    # testsqlite/tst_testsqlite.cpp \
 \    # testudpprobe/ts_testudpprobe.cpp
    main.cpp

