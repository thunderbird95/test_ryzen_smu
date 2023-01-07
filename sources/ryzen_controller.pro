QT       += core gui widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    apu_driver.cpp \
    graphs.cpp \
    main.cpp \
    qcustomplot.cpp \
    ryzen_control.cpp \
    smu.cpp \
    statistic.cpp \
    test_settings.cpp

HEADERS += \
    apu_driver.h \
    definitions.h \
    graphs.h \
    qcustomplot.h \
    ryzen_control.h \
    smu.h \
    statistic.h \
    test_settings.h

FORMS += \
    graphs.ui \
    ryzen_control.ui \
    statistic.ui \
    test_settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/../../../../../../usr/lib64/ -lpci

INCLUDEPATH += $$PWD/../../../../../../usr/include/pci
DEPENDPATH += $$PWD/../../../../../../usr/include/pci

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../../usr/lib64/libpci.a
