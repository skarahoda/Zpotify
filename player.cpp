
#include "player.h"

#include "playercontrols.h"
#include "playlistmodel.h"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoProbe>
#include <QMediaMetaData>
#include <QtWidgets>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <stdio.h>
#include <qtextstream.h>
#include <QSettings>

Player::Player(QWidget *parent)
    : QWidget(parent)
    , fileWidget(0)
    , coverLabel(0)
    , slider(0)
{
    //TO BE filled from settings
    ip_addr  = "192.168.2.62";
    sql_user = "listen";
    sql_pw   = "1234";
    sql_db   = "ampache";
    //TO BE filled from settings



    m_sSettingsFile = QApplication::applicationDirPath()+ "settings.ini";
    //loadSettings(); //fill ip, sql user, db and pw here
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


    //    //Cpp file:
    //    rclickAction = new QAction(tr("&Add all"), this);
    //    rclickAction->setStatusTip(tr("Options"));
    //    connect(rclickAction, SIGNAL(triggered()), this, SLOT(clearList()));

    //    // Then add it to your treeWidget:
    //        fileWidget->addAction(rclickAction);
    //          fileWidget->setContextMenuPolicy(Qt::ActionsContextMenu);


    playlistModel = new PlaylistModel(this);
    playlistModel->setPlaylist(playlist);
    //! [2]

    menuBar = new QMenuBar(this);

    connectAct = new QAction("&Connect",menuBar);
    connectAct->setShortcut(tr("Ctrl+C"));
    connectAct->setStatusTip(tr("Ctrl+C"));
    menuBar->addAction(connectAct);

    connect(connectAct,SIGNAL(triggered()),this,SLOT(clearList()));

    clearAct = new QAction("Clear",menuBar);
    clearAct->setShortcut(Qt::Key_Delete);
    clearAct->setStatusTip(tr("Delete"));
    menuBar->addAction(clearAct);
    //connectMenu->
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

    getSQL();
}

Player::~Player()
{
}

/*
 * Load settings from the hardcoded .ini file
 */
void Player::loadSettings(){
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    QString sText = settings.value("text", "").toString();
    //TODO : LOAD value from sText
}

/*
 * Save settings to the hardcoded .ini file
 */
void Player::saveSettings(){
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    //TODO : fill sText
    QString sText = "";
    settings.setValue("text", sText);
}

int Player::getSQL(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(ip_addr);
    db.setDatabaseName(sql_db);
    db.setUserName(sql_user);
    db.setPassword(sql_pw);
    if (!db.open())
    {
        QTextStream(stdout) << "User    : " + db.userName()     << endl;
        QTextStream(stdout) << "Password: " + db.password()     << endl;
        QTextStream(stdout) << "Database: " + db.databaseName() << endl;
        QTextStream(stdout) << "HostName: " + db.hostName()     << endl;
        QTextStream(stdout) << ""                               << endl;
        QTextStream(stdout) << db.lastError().text() << endl;
    }

    QSqlQuery query;
    QString qur = "UPDATE   ampache.session SET  expire =  1999999999 WHERE  session.id =  6931";
    query.exec(qur);
    qur = "SELECT artist.name, album.name, song.title, song.id from song inner join album on album.id=song.album inner join artist on artist.id=song.artist order by artist.name, album.name, song.track";

    query.exec(qur);
    while(query.next()) {
        addSong(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString());
    }
    return 1;
}

void Player::addToPlaylist(QTreeWidgetItem* current)
{
    QTreeWidgetItem *albumItem;
    QTreeWidgetItem *songItem;
    switch (current->text(1).toInt()) {
    case -2:
        //        clearList();
        for (int i = 0; i < current->childCount(); i++) {
            albumItem = current->child(i);
            for (int j = 0; j < albumItem->childCount(); j++) {
                songItem = albumItem->child(j);
                QString stringURL = "http://"+ip_addr+"/ampache/play/index.php?ssid=6931&type=song&oid="+songItem->text(1)+"&uid=2&name=/" + songItem->text(0);
                QUrl url(stringURL);
                playlistModel->songName=songItem->text(0);
                playlist->addMedia(url);
            }
        }
        break;
    case -1:
        //        clearList();
        for (int i = 0; i < current->childCount(); i++) {
            songItem = current->child(i);
            QString stringURL = "http://"+ip_addr+"/ampache/play/index.php?ssid=6931&type=song&oid="+songItem->text(1)+"&uid=2&name=/" + songItem->text(0);
            QUrl url(stringURL);
            playlistModel->songName=songItem->text(0);
            playlist->addMedia(url);
        }
        break;
    default:
        QString stringURL = "http://"+ip_addr+"/ampache/play/index.php?ssid=6931&type=song&oid="+current->text(1)+"&uid=2&name=/" + current->text(0);
        QUrl url(stringURL);
        playlistModel->songName=current->text(0);
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
