QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../src/jsonbuild.cpp \
    ../src/jsonparse.cpp \
    chat.cpp \
    client.cpp \
    filewidgetitem.cpp \
    main.cpp

HEADERS += \
    ../src/jsonbuild.h \
    ../src/jsoncommands.h \
    ../src/jsonparse.h \
    chat.h \
    client.h \
    filewidgetitem.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    chat.ui

RESOURCES += \
    Resource.qrc
