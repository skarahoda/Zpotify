TEMPLATE = app
TARGET = player

QT += network \
      xml \
      multimedia \
      multimediawidgets \
      widgets \
      sql

HEADERS = \
    player.h \
    playercontrols.h \
    playlistmodel.h
SOURCES = main.cpp \
    player.cpp \
    playercontrols.cpp \
    playlistmodel.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/multimediawidgets/player
INSTALLS += target
