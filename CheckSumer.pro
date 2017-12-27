QT += concurrent widgets

TARGET = QChecksumer
TEMPLATE = app

RC_FILE     += CheckSumer.rc

SOURCES     += main.cpp\
    checksumermain.cpp \
    checksumer.cpp

DISTFILES += \
    QChecksumer.rc

HEADERS     += checksumermain.h \
    checksumer.h

FORMS       += checksumermain.ui

RESOURCES   += \
    image.qrc
