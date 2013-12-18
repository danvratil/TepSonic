/*
 * TEPSONIC
 * Copyright 2013 Daniel Vrátil <dan@progdan.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.
 */

// For random()
#include <cstdlib>
#include <ctime>

#include "constants.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings/settingsdialog.h"
#include "playlist/playlistmodel.h"
#include "bookmarks/bookmarksmanager.h"
#include "abstractplugin.h"
#include "taskmanager.h"
#include "pluginsmanager.h"
#include "tools.h"
#include "supportedformats.h"
#include "metadataeditor.h"
#include "settings.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

#include <QMessageBox>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QDateTime>

#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>

#include "qxtglobalshortcut.h"

#include <QDebug>

Q_DECLARE_METATYPE(QModelIndex)

MainWindow::MainWindow():
    m_metadataEditor(0),
    m_canClose(false)
{
    // Initialize pseudo-random numbers generator
    srand(time(NULL));

    // Create default UI
    m_ui = new Ui::MainWindow();
    m_ui->setupUi(this);

    // Set application icon
    m_appIcon = new QIcon(QLatin1String(":/icons/mainIcon"));
    QApplication::setWindowIcon(*m_appIcon);

    // Create menus
    createMenus();

    // Create tray
    m_trayIcon = new TrayIcon(*m_appIcon, this);
    m_trayIcon->setVisible(true);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setToolTip(tr("Player is stopped"));

    // Create label for displaying playlist length
    m_playlistLengthLabel = new QLabel(this);
    m_ui->statusBar->addPermanentWidget(m_playlistLengthLabel, 0);
    m_playlistLengthLabel->setText(tr("%n track(s)", "", 0).append(QLatin1String(" (00:00)")));

    connect(m_ui->playlistView, SIGNAL(playlistLengthChanged(int,int)),
            this, SLOT(playlistLengthChanged(int,int)));
    connect(m_ui->playlistView, SIGNAL(nowPlayingChanged(QModelIndex)),
            this, SLOT(setCurrentTrack(QModelIndex)));

    // Set playlist browser columns widths and visibility
    const QVariantList playlistColumnsStates = Settings::instance()->playlistColumnsStates();
    const QVariantList playlistColumnsWidths = Settings::instance()->playlistColumnsWidths();
    for (int i = 0; i < playlistColumnsStates.count() - 1; i++) {
        if (playlistColumnsStates.at(i).toBool()) {
            m_ui->playlistView->showColumn(i);
            m_ui->playlistView->setColumnWidth(i, playlistColumnsWidths.at(i).toInt());
            m_ui->menuVisible_columns->actions().at(i)->setChecked(true);
        } else {
            m_ui->playlistView->hideColumn(i);
            m_ui->menuVisible_columns->actions().at(i)->setChecked(false);
        }
    }

    // Restore main window geometry
    restoreGeometry(Settings::instance()->windowGeometry());
    restoreState(Settings::instance()->windowState());
    m_ui->viewsSplitter->restoreState(Settings::instance()->windowSplitterState());

    // Enable or disable collections
    if (Settings::instance()->collectionsEnabled()) {
        setupCollections();
        if (Settings::instance()->collectionsAutoRebuild()) {
            TaskManager::instance()->rebuildCollections();
        }
    } else {
        m_ui->viewsTab->setTabEnabled(0, false);
    }

    // Set up filesystem browser
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setRootPath(QDir::rootPath());
    m_fileSystemModel->setNameFilters(SupportedFormats::extensionsList());
    m_fileSystemModel->setNameFilterDisables(false);
    m_ui->filesystemBrowser->setModel(m_fileSystemModel);
    m_ui->filesystemBrowser->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create seek slider and volume slider
    m_ui->seekSlider->setMediaObject(Player::instance()->mediaObject());
    m_ui->volumeSlider->setAudioOutput(Player::instance()->audioOutput());

    // Load last playlist
    if (Settings::instance()->sessionRestore()) {
        TaskManager::instance()->addFileToPlaylist(QString(_CONFIGDIR).append(QLatin1String("/last.m3u")));
        randomModeChanged(Settings::instance()->playerRandomMode());
        repeatModeChanged(static_cast<Player::RepeatMode>(Settings::instance()->playerRepeatMode()));
    }

    // Create bookmarks manager
    m_bookmarksManager = new BookmarksManager(m_ui);

    // At the very end bind all signals and slots
    bindSignals();
    // Bind global shortcuts
    bindShortcuts();
}

MainWindow::~MainWindow()
{
    Settings::instance()->setWindowGeometry(saveGeometry());
    Settings::instance()->setWindowState(saveState());
    Settings::instance()->setWindowSplitterState(m_ui->viewsSplitter->saveState());

    QVariantList playlistColumnsStates;
    QVariantList playlistColumnsWidths;
    for (int i = 0; i < m_ui->playlistView->model()->columnCount(QModelIndex()); i++) {
        // Don't store "isColumnHidden" but "isColumnVisible"
        playlistColumnsStates.append(!m_ui->playlistView->isColumnHidden(i));
        playlistColumnsWidths.append(m_ui->playlistView->columnWidth(i));
    }
    Settings::instance()->setPlaylistColumnsStates(playlistColumnsStates);
    Settings::instance()->setPlaylistColumnsWidths(playlistColumnsWidths);

    if (Settings::instance()->sessionRestore()) {
        Settings::instance()->setPlayerRepeatMode(Player::instance()->repeatMode());
        Settings::instance()->setPlayerRandomMode(Player::instance()->randomMode());
    }

    // Save current playlist to file
    TaskManager::instance()->savePlaylistToFile(QString(_CONFIGDIR).append(QLatin1String("/last.m3u")));

    delete m_bookmarksManager;

    delete m_appIcon;
    delete m_ui;
}

void MainWindow::createMenus()
{
    // Create 'Random mode' submenu
    m_randomPlaybackGroup = new QActionGroup(this);
    m_randomPlaybackGroup->addAction(m_ui->actionRandom_ON);
    m_randomPlaybackGroup->addAction(m_ui->actionRandom_OFF);

    // Create 'Repeat mode' submenu
    m_repeatPlaybackGroup = new QActionGroup(this);
    m_repeatPlaybackGroup->addAction(m_ui->actionRepeat_OFF);
    m_repeatPlaybackGroup->addAction(m_ui->actionRepeat_playlist);
    m_repeatPlaybackGroup->addAction(m_ui->actionRepeat_track);
    m_ui->actionRepeat_OFF->setChecked(true);

    // Create tray menu
    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(m_ui->actionShow_Hide);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_ui->actionPrevious_track);
    m_trayIconMenu->addAction(m_ui->actionPlay_pause);
    m_trayIconMenu->addAction(m_ui->actionStop);
    m_trayIconMenu->addAction(m_ui->actionNext_track);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addMenu(m_ui->menuRepeat);
    m_trayIconMenu->addMenu(m_ui->menuRandom);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_ui->actionQuit_TepSonic);

    // Create playlist popup menu
    m_playlistPopupMenu = new QMenu(this);
    m_playlistPopupMenu->addAction(tr("&Stop after this track"), this, SLOT(setStopTrackClicked()));
    m_playlistPopupMenu->addAction(tr("&Edit metadata"), this, SLOT(showMetadataEditor()));
    m_ui->playlistView->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::bindShortcuts()
{
    QxtGlobalShortcut *sc1 = new QxtGlobalShortcut(this);
    sc1->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutPlayPause()));
    connect(sc1, SIGNAL(activated()),
            this, SLOT(playPause()));

    QxtGlobalShortcut *sc2 = new QxtGlobalShortcut(this);
    sc2->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutStop()));
    connect(sc2, SIGNAL(activated()),
            Player::instance(), SLOT(stop()));

    QxtGlobalShortcut *sc3 = new QxtGlobalShortcut(this);
    sc3->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutPreviousTrack()));
    connect(sc3, SIGNAL(activated()),
            m_ui->playlistView, SLOT(selectPreviousTrack()));

    QxtGlobalShortcut *sc4 = new QxtGlobalShortcut(this);
    sc4->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutNextTrack()));
    connect(sc4, SIGNAL(activated()),
            m_ui->playlistView, SLOT(selectNextTrack()));

    QxtGlobalShortcut *sc5 = new QxtGlobalShortcut(this);
    sc5->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutToggleWindow()));
    connect(sc5, SIGNAL(activated()),
            this, SLOT(showHideWindow()));
}

void MainWindow::bindSignals()
{
    // Tray icon
    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));

    // Playlist stuff
    connect(m_ui->playlistView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(setCurrentTrack(QModelIndex)));
    connect(m_ui->clearPlaylistSearch, SIGNAL(clicked()),
            this, SLOT(clearPlaylistSearch()));
    connect(m_ui->playlistView->header(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showPlaylistHeaderContextMenu(QPoint)));
    connect(m_ui->playlistSearchEdit, SIGNAL(textChanged(QString)),
            m_ui->playlistView, SLOT(setFilter(QString)));
    connect(m_ui->playlistView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showPlaylistContextMenu(QPoint)));

    // Filesystem browser
    connect(m_ui->fsbBackBtn, SIGNAL(clicked()),
            m_ui->filesystemBrowser, SLOT(goBack()));
    connect(m_ui->fsbFwBtn, SIGNAL(clicked()),
            m_ui->filesystemBrowser, SLOT(goForward()));
    connect(m_ui->fsbGoHomeBtn, SIGNAL(clicked()),
            m_ui->filesystemBrowser, SLOT(goHome()));
    connect(m_ui->fsbCdUpBtn, SIGNAL(clicked()),
            m_ui->filesystemBrowser, SLOT(cdUp()));
    connect(m_ui->fsbPath, SIGNAL(textChanged(QString)),
            m_ui->filesystemBrowser, SLOT(goToDir(QString)));
    connect(m_ui->filesystemBrowser, SIGNAL(pathChanged(QString)),
            m_ui->fsbPath, SLOT(setText(QString)));
    connect(m_ui->filesystemBrowser, SIGNAL(addTrackToPlaylist(QString)),
            TaskManager::instance(), SLOT(addFileToPlaylist(QString)));
    connect(m_ui->filesystemBrowser, SIGNAL(disableBack(bool)),
            m_ui->fsbBackBtn, SLOT(setDisabled(bool)));
    connect(m_ui->filesystemBrowser, SIGNAL(disableForward(bool)),
            m_ui->fsbFwBtn, SLOT(setDisabled(bool)));
    connect(m_ui->filesystemBrowser, SIGNAL(disableCdUp(bool)),
            m_ui->fsbCdUpBtn, SLOT(setDisabled(bool)));
    connect(m_ui->fsbBookmarksBtn, SIGNAL(toggled(bool)),
            m_bookmarksManager, SLOT(toggleBookmarks()));
    connect(m_ui->filesystemBrowser, SIGNAL(addBookmark(QString)),
            m_bookmarksManager, SLOT(showAddBookmarkDialog(QString)));


    // Menu 'Player'
    connect(m_ui->actionNext_track, SIGNAL(triggered(bool)),
            m_ui->playlistView, SLOT(selectNextTrack()));
    connect(m_ui->actionPrevious_track, SIGNAL(triggered(bool)),
            m_ui->playlistView, SLOT(selectPreviousTrack()));
    connect(m_ui->actionPlay_pause, SIGNAL(triggered(bool)),
            this, SLOT(playPause()));
    connect(m_ui->actionStop, SIGNAL(triggered(bool)),
            Player::instance(), SLOT(stop()));

    // Menu 'Player -> Repeat mode'
    connect(m_ui->actionRepeat_OFF, SIGNAL(triggered(bool)),
            this, SLOT(setRepeatModeOff()));
    connect(m_ui->actionRepeat_playlist, SIGNAL(triggered(bool)),
            this, SLOT(setRepeatModeAll()));
    connect(m_ui->actionRepeat_track, SIGNAL(triggered(bool)),
            this, SLOT(setRepeatModeTrack()));

    // Menu 'Player -> Random mode'
    connect(m_ui->actionRandom_ON, SIGNAL(triggered(bool)),
            this, SLOT(setRandomModeOn()));
    connect(m_ui->actionRandom_OFF, SIGNAL(triggered(bool)),
            this, SLOT(setRandomModeOff()));

    // Menu 'Playlist'
    connect(m_ui->actionClear_playlist, SIGNAL(triggered(bool)),
            m_ui->playlistView, SLOT(clearPlaylist()));
    connect(m_ui->actionSave_playlist, SIGNAL(triggered(bool)),
            this, SLOT(savePlaylist()));
    connect(m_ui->actionShuffle_playlist, SIGNAL(triggered(bool)),
            m_ui->playlistView, SLOT(shuffle()));

    // Menu 'TepSonic'
    connect(m_ui->actionSettings, SIGNAL(triggered(bool)),
            this, SLOT(openSettingsDialog()));
    connect(m_ui->actionShow_Hide, SIGNAL(triggered(bool)),
            this, SLOT(showHideWindow()));
    connect(m_ui->actionQuit_TepSonic, SIGNAL(triggered(bool)),
            this, SLOT(quitApp()));

    // Menu 'Help
    connect(m_ui->actionReport_a_bug, SIGNAL(triggered(bool)),
            this, SLOT(reportBug()));
    connect(m_ui->actionAbout_TepSonic, SIGNAL(triggered(bool)),
            this, SLOT(aboutTepSonic()));
    connect(m_ui->actionAbout_Qt, SIGNAL(triggered(bool)),
            this, SLOT(aboutQt()));
    connect(m_ui->actionSupported_formats, SIGNAL(triggered(bool)),
            this, SLOT(showSupportedFormats()));

    // Player object
    connect(Player::instance(), SIGNAL(trackFinished()),
            this, SLOT(updatePlayerTrack()));
    connect(Player::instance(), SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(playerStatusChanged(Phonon::State, Phonon::State)));
    connect(Player::instance(), SIGNAL(repeatModeChanged(Player::RepeatMode)),
            this, SLOT(repeatModeChanged(Player::RepeatMode)));
    connect(Player::instance(), SIGNAL(randomModeChanged(bool)),
            this, SLOT(randomModeChanged(bool)));
    connect(Player::instance(), SIGNAL(trackPositionChanged(qint64)),
            this, SLOT(playerPosChanged(qint64)));

    // Connect UI buttons to their equivalents in the main menu
    connect(m_ui->clearPlaylistButton, SIGNAL(clicked()),
            m_ui->actionClear_playlist, SLOT(trigger()));
    connect(m_ui->previousTrackButton, SIGNAL(clicked()),
            m_ui->actionPrevious_track, SLOT(trigger()));
    connect(m_ui->playPauseButton, SIGNAL(clicked()),
            m_ui->actionPlay_pause, SLOT(trigger()));
    connect(m_ui->stopButton, SIGNAL(clicked()),
            m_ui->actionStop, SLOT(trigger()));
    connect(m_ui->nextTrackButton, SIGNAL(clicked()),
            m_ui->actionNext_track, SLOT(trigger()));
    connect(m_ui->shufflePlaylistButton, SIGNAL(clicked()),
            m_ui->actionShuffle_playlist, SLOT(trigger()));


    // Connect individual PlaylistBrowser columns' visibility state with QActions in ui->menuVisiblem_columns
    m_playlistVisibleColumnContextMenuMapper = new QSignalMapper(this);
    connect(m_ui->actionFilename, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionTrack, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionInterpret, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionTrackname, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionAlbum, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionGenre, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionYear, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionLength, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_ui->actionBitrate, SIGNAL(toggled(bool)),
            m_playlistVisibleColumnContextMenuMapper, SLOT(map()));
    connect(m_playlistVisibleColumnContextMenuMapper, SIGNAL(mapped(int)),
            this, SLOT(togglePlaylistColumnVisibility(int)));
    // Map an identifier to each caller
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionFilename, PlaylistModel::FilenameColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionTrack, PlaylistModel::TrackColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionInterpret, PlaylistModel::InterpretColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionTrackname, PlaylistModel::TracknameColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionAlbum, PlaylistModel::AlbumColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionGenre, PlaylistModel::GenreColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionYear, PlaylistModel::YearColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionLength, PlaylistModel::LengthColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionBitrate, PlaylistModel::BitrateColumn);


    // Task manager
    connect(m_ui->playlistView, SIGNAL(addedFiles(QStringList, int)),
            TaskManager::instance(), SLOT(addFilesToPlaylist(QStringList, int)));
    connect(TaskManager::instance(), SIGNAL(taskStarted(QString)),
            m_ui->statusBar, SLOT(showWorkingBar(QString)));
    connect(TaskManager::instance(), SIGNAL(taskDone()),
            m_ui->statusBar, SLOT(cancelAction()));

    // Plugins
    connect(PluginsManager::instance(), SIGNAL(pluginsLoaded()),
            this, SLOT(setupPluginsUIs()));
    connect(this, SIGNAL(settingsAccepted()),
            PluginsManager::instance(), SIGNAL(settingsAccepted()));
}

void MainWindow::setupPluginsUIs()
{
    PluginsManager *manager = PluginsManager::instance();
    // Let plugins install their menus
    manager->installMenus(m_trayIconMenu, AbstractPlugin::TrayMenu);
    manager->installMenus(m_playlistPopupMenu, AbstractPlugin::PlaylistPopup);

    // Let plugins install their tabs
    manager->installPanes(m_ui->mainTabWidget);
}

void MainWindow::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/* Show "About Tepsonic" dialog */
void MainWindow::aboutTepSonic()
{
    QMessageBox aboutDlg;

    QStringList developers;
    developers << QString::fromUtf8("Daniel Vrátil");
    QStringList artwork;
    artwork << QString::fromUtf8("Matěj Zvěřina")
            << QString::fromUtf8("Michael Ruml");

    const QString str = tr("<h1>%1</h1>"
                  "Version %2"
                  "<p><a href=\"http://danvratil.github.io/TepSonic/\">http://danvratil.github.io/TepSonic/</a></p>"
                  "<p>This program is free software; you can redistribute it and/or modify it under the terms of "
                  "the GNU General Public License as published by the Free Software Foundation; either version "
                  "2 of the License, or (at your option) any later version.</p>"
                  "<h2>Developers:</h2><p>%3</p>"
                  "<h2>Artwork:</h2><p>%4</p>"
                  "<p>&copy; 2009 - 2013 <a href=\"mailto:dan@progdan.cz\">Daniel Vrátil</a></p>")
                  .arg(QApplication::applicationName(),
                       QApplication::applicationVersion(),
                       developers.join(QLatin1String(", ")),
                       artwork.join(QLatin1String(", ")));
    aboutDlg.about(this, tr("About TepSonic"), str);
}

void MainWindow::showSupportedFormats()
{
    QMessageBox msBox(tr("Supported formats"),
                      tr("On this computer TepSonic can play following audio files:") + QLatin1String("\n\n") + SupportedFormats::extensionsList().join(QLatin1String(", ")),
                      QMessageBox::Information,
                      QMessageBox::Ok,
                      0, 0);
    msBox.exec();
}

void MainWindow::reportBug()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("https://github.com/danvratil/TepSonic/issues"), QUrl::TolerantMode));
}

void MainWindow::trayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (this->isHidden()) {
            this->show();
        } else {
            this->hide();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // If tray icon is visible then hide the window and ignore this event
    if (m_trayIcon->isVisible() && (m_canClose == false)) {
        this->hide();
        event->ignore();
    }
}

void MainWindow::openSettingsDialog()
{
    // Show preferences dialog
    SettingsDialog *settingsDlg = new SettingsDialog(this);
    connect(settingsDlg, SIGNAL(rebuildCollections()), TaskManager::instance(), SLOT(rebuildCollections()));
    connect(settingsDlg, SIGNAL(outputDeviceChanged()), Player::instance(), SLOT(setDefaultOutputDevice()));
    connect(settingsDlg, SIGNAL(accepted()), this, SIGNAL(settingsAccepted()));
    connect(settingsDlg, SIGNAL(accepted()), this, SLOT(settingsDialogAccepted()));
    settingsDlg->exec();
}

void MainWindow::settingsDialogAccepted()
{
    if (Settings::instance()->collectionsEnabled()) {
        setupCollections();
    } else {
        destroyCollections();
    }
}

void MainWindow::updatePlayerTrack()
{
    Player *player = Player::instance();
    if (player->repeatMode() == Player::RepeatTrack) {
        player->setTrack(player->currentSource().fileName());
        player->play();
    } else {
        m_ui->playlistView->selectNextTrack();
    }
}

void MainWindow::setCurrentTrack(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    m_ui->playlistView->setNowPlaying(index);

    const QModelIndex sibling = index.sibling(index.row(), PlaylistModel::FilenameColumn);
    const QString filename = sibling.data().toString();
    Player::instance()->setTrack(filename, true);
}

void MainWindow::playerStatusChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);

    const Player::MetaData metadata = Player::instance()->currentMetaData();

    QString playing;
    if (metadata.title.isEmpty()) {
        playing = QFileInfo(metadata.filename).fileName();
    } else {
        playing = metadata.title;
    }
    if (!metadata.artist.isEmpty()) {
        playing = metadata.artist + QLatin1String(" - ") + playing;
    }

    switch (newState) {
    case Phonon::PlayingState:
        m_ui->playPauseButton->setIcon(QIcon(QLatin1String(":/icons/pause")));
        m_ui->actionPlay_pause->setIcon(QIcon(QLatin1String(":/icons/pause")));
        m_ui->stopButton->setEnabled(true);
        m_ui->actionStop->setEnabled(true);
        m_ui->trackTitleLabel->setText(playing);
        m_ui->playbackTimeLabel->setText(QLatin1String("00:00:00"));
        m_trayIcon->setToolTip(tr("Playing: ") + playing);
        break;
    case Phonon::PausedState:
        m_ui->playPauseButton->setIcon(QIcon(QLatin1String(":/icons/start")));
        m_ui->actionPlay_pause->setIcon(QIcon(QLatin1String(":/icons/start")));
        m_ui->stopButton->setEnabled(true);
        m_ui->actionStop->setEnabled(true);
        m_ui->trackTitleLabel->setText(tr("%1 [paused]").arg(m_ui->trackTitleLabel->text()));
        m_trayIcon->setToolTip(tr("%1 [paused]").arg(m_trayIcon->toolTip()));
        break;
    case Phonon::StoppedState:
        m_ui->playPauseButton->setIcon(QIcon(QLatin1String(":/icons/start")));
        m_ui->actionPlay_pause->setIcon(QIcon(QLatin1String(":/icons/start")));
        m_ui->stopButton->setEnabled(false);
        m_ui->actionStop->setEnabled(false);
        m_ui->trackTitleLabel->setText(tr("Player is stopped"));
        m_ui->playbackTimeLabel->setText(tr("Stopped"));
        m_trayIcon->setToolTip(tr("Player is stopped"));
        break;
    case Phonon::ErrorState:
        m_ui->statusBar->showMessage(Player::instance()->errorString(), 5000);
        break;
    case Phonon::LoadingState:
        //todo
        break;
    case Phonon::BufferingState:
        //todo
        break;
    }
}

void MainWindow::playPause()
{
    Player *player = Player::instance();
    if (player->playerState() == Phonon::PlayingState) {
        player->pause();
    } else {
        /* When the source is empty there are some files in playlist, select the
           first row and load it as current source */
        if ((player->currentSource().fileName().isEmpty()) && (m_ui->playlistView->model()->rowCount() > 0)) {
            const QModelIndex nowPlaying = m_ui->playlistView->model()->index(0, 0);
            setCurrentTrack(nowPlaying);
        }

        player->play();
    }
}

void MainWindow::addPlaylistItem(const QString &filename)
{
    TaskManager::instance()->addFileToPlaylist(filename);
}

void MainWindow::showPlaylistHeaderContextMenu(const QPoint &pos)
{
    m_ui->menuVisible_columns->popup(m_ui->playlistView->header()->mapToGlobal(pos));
}

void MainWindow::togglePlaylistColumnVisibility(int column)
{
    m_ui->playlistView->setColumnHidden(column, (! m_ui->playlistView->isColumnHidden(column)));
}

void MainWindow::savePlaylist()
{
    QString filename;
    filename = QFileDialog::getSaveFileName(this,
                                            tr("Save playlist to..."),
                                            QString(),
                                            tr("M3U Playlist (*.m3u)"));
    TaskManager::instance()->savePlaylistToFile(filename);
}

void MainWindow::playlistLengthChanged(int totalLength, int tracksCount)
{
    QString time = formatTimestamp(totalLength);
    m_playlistLengthLabel->setText(tr("%n track(s)", "", tracksCount).append(QLatin1String(" (") + time + QLatin1String(")")));
}

void MainWindow::clearPlaylistSearch()
{
    m_ui->playlistSearchEdit->setText(QString());
}

void MainWindow::clearCollectionSearch()
{
    m_ui->colectionSearchEdit->setText(QString());
}

void MainWindow::showError(const QString &error)
{
    m_ui->statusBar->showMessage(error, 5000);
}

void MainWindow::repeatModeChanged(Player::RepeatMode newMode)
{
    switch (newMode) {
    case Player::RepeatOff:
        m_ui->actionRepeat_OFF->setChecked(true);
        break;
    case Player::RepeatTrack:
        m_ui->actionRepeat_track->setChecked(true);
        break;
    case Player::RepeatAll:
        m_ui->actionRepeat_playlist->setChecked(true);
        break;
    }
}

void MainWindow::randomModeChanged(bool newMode)
{
    if (newMode == true) {
        m_ui->actionRandom_ON->setChecked(true);
    } else {
        m_ui->actionRandom_OFF->setChecked(true);
    }
}

void MainWindow::playerPosChanged(qint64 newPos)
{
    m_ui->playbackTimeLabel->setText(formatMilliseconds(newPos, true));
}

void MainWindow::showPlaylistContextMenu(const QPoint &pos)
{
    const bool valid = (m_ui->playlistView->currentIndex().isValid() ||
                  m_ui->playlistView->indexAt(pos).isValid());

    for (int i = 0; i < m_playlistPopupMenu->actions().count(); i++) {
        m_playlistPopupMenu->actions().at(i)->setEnabled(valid);
    }

    m_playlistPopupMenu->popup(m_ui->playlistView->mapToGlobal(pos));
}

void MainWindow::setupCollections()
{
    m_ui->collectionView->enableCollections();

    // Since we disabled the Proxy model, no search inputs are needed
    m_ui->label_2->hide();
    m_ui->colectionSearchEdit->hide();
    m_ui->clearCollectionSearch->hide();

    m_ui->viewsTab->setTabEnabled(0, true);
}

void MainWindow::destroyCollections()
{
    m_ui->collectionView->disableCollections();
    m_ui->viewsTab->setTabEnabled(0, false);
}

void MainWindow::showMetadataEditor()
{
    if (!m_metadataEditor) {
        m_metadataEditor = new MetadataEditor();
        connect(m_metadataEditor, SIGNAL(accepted()),
                this, SLOT(metadataEditorAccepted()));
        connect(m_metadataEditor, SIGNAL(rejected()),
                m_metadataEditor, SLOT(deleteLater()));
    }

    const QModelIndex currentIndex = m_ui->playlistView->currentIndex();
    QModelIndex idx = currentIndex.sibling(currentIndex.row(), PlaylistModel::FilenameColumn);
    m_metadataEditor->setFilename(idx.data().toString());
    idx = idx.sibling(idx.row(), PlaylistModel::TracknameColumn);
    m_metadataEditor->setTrackTitle(idx.data().toString());
    idx = idx.sibling(idx.row(), PlaylistModel::AlbumColumn);
    m_metadataEditor->setAlbum(idx.data().toString());
    idx = idx.sibling(idx.row(), PlaylistModel::InterpretColumn);
    m_metadataEditor->setArtist(idx.data().toString());
    idx = idx.sibling(idx.row(), PlaylistModel::GenreColumn);
    m_metadataEditor->setGenre(idx.data().toString());
    idx = idx.sibling(idx.row(), PlaylistModel::YearColumn);
    m_metadataEditor->setYear(idx.data().toInt());
    idx = idx.sibling(idx.row(), PlaylistModel::TrackColumn);
    m_metadataEditor->setTrackNumber(idx.data().toInt());

    m_metadataEditor->exec();
}

void MainWindow::metadataEditorAccepted()
{
    if (!m_metadataEditor) {
        return;
    }

    TagLib::FileRef f(m_metadataEditor->filename().toLocal8Bit().constData());
//     // FileRef::isNull() is not enough sometimes. Let's check the tag() too...
    if (f.isNull() || !f.tag()) {
        return;
    }

    f.tag()->setTrack(m_metadataEditor->trackNumber());
    f.tag()->setArtist(m_metadataEditor->artist().toLocal8Bit().constData());
    f.tag()->setAlbum(m_metadataEditor->album().toLocal8Bit().constData());
    f.tag()->setTitle(m_metadataEditor->trackTitle().toLocal8Bit().constData());
    f.tag()->setYear(m_metadataEditor->year());
    f.tag()->setGenre(m_metadataEditor->genre().toLocal8Bit().constData());
    f.save();

    QAbstractItemModel *model = m_ui->playlistView->model();
    const QModelIndex currentIndex = m_ui->playlistView->currentIndex();
    QModelIndex idx = currentIndex.sibling(currentIndex.row(), PlaylistModel::TracknameColumn);
    model->setData(idx, m_metadataEditor->trackTitle());
    idx = idx.sibling(idx.row(), PlaylistModel::AlbumColumn);
    model->setData(idx, m_metadataEditor->album());
    idx = idx.sibling(idx.row(), PlaylistModel::InterpretColumn);
    model->setData(idx, m_metadataEditor->artist());
    idx = idx.sibling(idx.row(), PlaylistModel::GenreColumn);
    model->setData(idx, m_metadataEditor->genre());
    idx = idx.sibling(idx.row(), PlaylistModel::YearColumn);
    model->setData(idx, m_metadataEditor->year());
    idx = idx.sibling(idx.row(), PlaylistModel::TrackColumn);
    model->setData(idx, m_metadataEditor->trackNumber());

    TaskManager::instance()->rebuildCollections(m_metadataEditor->filename());

    delete m_metadataEditor;
}

void MainWindow::setStopTrackClicked()
{
    const QModelIndex index = m_ui->playlistView->currentIndex();
    if (index.row() == m_ui->playlistView->stopTrack().row()) {
        m_ui->playlistView->clearStopTrack();
    } else {
        m_ui->playlistView->setStopTrack(index);
    }
}
