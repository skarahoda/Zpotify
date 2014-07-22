/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "player.h"

#include "playercontrols.h"
#include "playlistmodel.h"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoProbe>
#include <QMediaMetaData>
#include <QtWidgets>

Player::Player(QWidget *parent)
    : QWidget(parent)
    , fileWidget(0)
    , coverLabel(0)
    , slider(0)
{
    setWindowTitle(tr("Zpotify") + QChar(0x2122));
//! [create-objs]
    player = new QMediaPlayer(this);
    // owned by PlaylistModel
    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);
//! [create-objs]

    connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(playlist, SIGNAL(currentIndexChanged(int)), SLOT(playlistPositionChanged(int)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));

//! [2]
    fileWidget = new QTreeWidget(this);
    fileWidget->setColumnCount(1);
    fileWidget->header()->hide();
    connect(fileWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(addToPlaylist(QTreeWidgetItem*)));

    playlistModel = new PlaylistModel(this);
    playlistModel->setPlaylist(playlist);
//! [2]

    menuBar = new QMenuBar(this);

    connectAct = new QAction("&Connect",menuBar);
    connectAct->setShortcut(tr("Ctrl+C"));
    connectAct->setStatusTip(tr("Ctrl+C"));
    menuBar->addAction(connectAct);

    clearAct = new QAction("Clear",menuBar);
    clearAct->setShortcut(Qt::Key_Delete);
    clearAct->setStatusTip(tr("Delete"));
    menuBar->addAction(clearAct);
    connect(clearAct,SIGNAL(triggered()),this,SLOT(clearList()));

    playlistView = new QListView(this);
    playlistView->setModel(playlistModel);
    playlistView->setCurrentIndex(playlistModel->index(playlist->currentIndex(), 0));

    connect(playlistView, SIGNAL(activated(QModelIndex)), this, SLOT(jump(QModelIndex)));

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, player->duration() / 1000);

    labelDuration = new QLabel(this);
    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));

    PlayerControls *controls = new PlayerControls(this);
    controls->setState(player->state());
    controls->setVolume(player->volume());
    controls->setMuted(controls->isMuted());

    connect(controls, SIGNAL(play()), player, SLOT(play()));
    connect(controls, SIGNAL(pause()), player, SLOT(pause()));
    connect(controls, SIGNAL(stop()), player, SLOT(stop()));
    connect(controls, SIGNAL(next()), playlist, SLOT(next()));
    connect(controls, SIGNAL(previous()), this, SLOT(previousClicked()));
    connect(controls, SIGNAL(changeVolume(int)), player, SLOT(setVolume(int)));
    connect(controls, SIGNAL(changeMuting(bool)), player, SLOT(setMuted(bool)));

    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)),
            controls, SLOT(setState(QMediaPlayer::State)));
    connect(player, SIGNAL(volumeChanged(int)), controls, SLOT(setVolume(int)));
    connect(player, SIGNAL(mutedChanged(bool)), controls, SLOT(setMuted(bool)));

    QBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addWidget(fileWidget, 1);
    displayLayout->addWidget(playlistView, 1);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addStretch(1);
    controlLayout->addWidget(controls);
    controlLayout->addStretch(1);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(menuBar);
    layout->addLayout(displayLayout);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(slider);
    hLayout->addWidget(labelDuration);
    layout->addLayout(hLayout);
    layout->addLayout(controlLayout);

    setLayout(layout);

    if (!player->isAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));

        controls->setEnabled(false);
        playlistView->setEnabled(false);
    }

    addSong(tr("The Black Keys"),tr("El Camino"),tr("Lonely Boy"),tr("1"));
    addSong(tr("The Black Keys"),tr("El Camino"),tr("Dead and Gone"),tr("2"));
    addSong(tr("The Black Keys"),tr("Turn Blue"),tr("In Time"),tr("3"));
    addSong(tr("The Black Keys"),tr("Turn Blue"),tr("Fever"),tr("4"));
}

Player::~Player()
{
}


void Player::addToPlaylist(QTreeWidgetItem* current)
{
    QTreeWidgetItem *albumItem;
    QTreeWidgetItem *songItem;
    switch (current->text(1).toInt()) {
    case -2:
        for (int i = 0; i < current->childCount(); i++) {
            albumItem = current->child(i);
            for (int j = 0; j < albumItem->childCount(); j++) {
                songItem = albumItem->child(j);
                QString stringURL = songItem->text(0);
                QUrl url(stringURL);
                playlist->addMedia(url);
            }
        }
        break;
    case -1:
        for (int i = 0; i < current->childCount(); i++) {
            songItem = current->child(i);
            QString stringURL = songItem->text(0);
            QUrl url(stringURL);
            playlist->addMedia(url);
        }
        break;
    default:
        QString stringURL = current->text(0);
        QUrl url(stringURL);
        playlist->addMedia(url);
        break;
    }
}

void Player::durationChanged(qint64 duration)
{
    this->duration = duration/1000;
    slider->setMaximum(duration / 1000);
}

void Player::positionChanged(qint64 progress)
{
    if (!slider->isSliderDown()) {
        slider->setValue(progress / 1000);
    }
    updateDurationInfo(progress / 1000);
}

void Player::metaDataChanged()
{
    if (player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                .arg(player->metaData(QMediaMetaData::AlbumArtist).toString())
                .arg(player->metaData(QMediaMetaData::Title).toString()));

        if (coverLabel) {
            QUrl url = player->metaData(QMediaMetaData::CoverArtUrlLarge).value<QUrl>();

            coverLabel->setPixmap(!url.isEmpty()
                    ? QPixmap(url.toString())
                    : QPixmap());
        }
    }
}

void Player::previousClicked()
{
    // Go to previous track if we are within the first 5 seconds of playback
    // Otherwise, seek to the beginning.
    if(player->position() <= 5000)
        playlist->previous();
    else
        player->setPosition(0);
}

void Player::jump(const QModelIndex &index)
{
    if (index.isValid()) {
        playlist->setCurrentIndex(index.row());
        player->play();
    }
}

void Player::playlistPositionChanged(int currentItem)
{
    playlistView->setCurrentIndex(playlistModel->index(currentItem, 0));
}

void Player::seek(int seconds)
{
    player->setPosition(seconds * 1000);
}

void Player::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);

    // handle status message
    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Media Stalled"));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage();
        break;
    }
}

void Player::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if (status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void Player::bufferingProgress(int progress)
{
    setStatusInfo(tr("Buffering %4%").arg(progress));
}

void Player::setTrackInfo(const QString &info)
{
    trackInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void Player::setStatusInfo(const QString &info)
{
    statusInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void Player::displayErrorMessage()
{
    setStatusInfo(player->errorString());
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || duration) {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
        QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
        QString format = "mm:ss";
        if (duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    labelDuration->setText(tStr);
}

QTreeWidgetItem * Player::addArtist(const QString &artistName)
{
    QTreeWidgetItem *artist = new QTreeWidgetItem(fileWidget);
    artist->setText(0, artistName);
    artist->setText(1,tr("-2"));
    return artist;
}

void Player::addSong(const QString &artistName, const QString &albumName, const QString &songName, const QString &ID)
{
    //find the artist
    QList<QTreeWidgetItem *> artistList = (fileWidget->findItems(artistName, Qt::MatchExactly, 0));
    QTreeWidgetItem * artist;
    //if the artist is not in the tree then add to the tree
    if (artistList.empty())
        artist = addArtist(artistName);
    else
        artist = artistList[0];

    //find the album
    QTreeWidgetItem * album;
    for (int i=0; i < artist->childCount(); i++) {
        album = artist->child(i);
        if(album->text(0) == albumName)
        {
            QTreeWidgetItem *song = new QTreeWidgetItem(album);
            song->setText(0,songName);
            song->setText(1,ID);
            return;
        }
    }
    //if the album is not in the tree then add to tree
    album = addAlbum(artist,albumName);
    QTreeWidgetItem *song = new QTreeWidgetItem(album);
    song->setText(0,songName);
    song->setText(1,ID);
    return;
}

QTreeWidgetItem * Player::addAlbum(QTreeWidgetItem * artist, const QString &albumName)
{
    QTreeWidgetItem *album = new QTreeWidgetItem(artist);
    album->setText(0,albumName);
    album->setText(1,tr("-1"));
    return album;
}

void Player::clearList()
{
    if(!playlist->clear())
        QMessageBox::warning(this, tr("List Error"),tr("QMediaPlaylist couldn't clear the list"));
}
