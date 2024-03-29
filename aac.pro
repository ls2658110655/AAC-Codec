#-------------------------------------------------
#
# Project created by QtCreator 2018-11-29T09:13:59
#
#-------------------------------------------------

QT       += core gui
QT       += network #SQL
QT       += multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = aac
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        codec.cpp

HEADERS  += codec.h

FORMS    += codec.ui

INCLUDEPATH += $$PWD/ff4.0.2x32/include \
                $$PWD/sdl/include

LIBS    += $$PWD/ff4.0.2x32/lib/avcodec.lib \
            $$PWD/ff4.0.2x32/lib/avdevice.lib \
            $$PWD/ff4.0.2x32/lib/avfilter.lib \
            $$PWD/ff4.0.2x32/lib/avformat.lib \
            $$PWD/ff4.0.2x32/lib/avutil.lib \
            $$PWD/ff4.0.2x32/lib/postproc.lib \
            $$PWD/ff4.0.2x32/lib/swresample.lib \
            $$PWD/ff4.0.2x32/lib/swscale.lib
#            $$PWD/opencv3.4.2/build/bin \
            $$PWD/sdl/lib/libSDL2.a \
#            $$PWD/sdl/lib/libSDL2_test.a \
#            $$PWD/sdl/lib/libSDL2main.a \
#            $$PWD/sdl/lib/libSDL2.dll.a
