TEMPLATE = app
TARGET = player

INCLUDEPATH += /usr/include/mysql

QT += network \
      xml \
      multimedia \
      multimediawidgets \
      widgets \
      sql

HEADERS = \
    player.h \
    playercontrols.h \
    playlistmodel.h \
    settingsdialog.h
SOURCES = main.cpp \
    player.cpp \
    playercontrols.cpp \
    playlistmodel.cpp \
    settingsdialog.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/multimediawidgets/player
INSTALLS += target
