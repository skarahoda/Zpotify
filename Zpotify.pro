#-------------------------------------------------
#
# Project created by QtCreator 2014-07-18T11:52:11
#
#-------------------------------------------------

QT       += core gui\
            multimedia \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets \
                                        multimediawidgets

TARGET = Zpotify
TEMPLATE = app


SOURCES +=  main.cpp\
            mainwindow.cpp \
            playercontrols.cpp

HEADERS  += mainwindow.h \
            playercontrols.h

FORMS    += mainwindow.ui
