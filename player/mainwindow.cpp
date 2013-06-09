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
#include "playlist/playlistitemdelegate.h"
#include "playlist/playlistproxymodel.h"
#include "playlist/playlistmodel.h"
#include "playlist/playlistitem.h"
#include "collections/collectionproxymodel.h"
#include "collections/collectionmodel.h"
#include "collections/collectionitem.h"
#include "collections/collectionitemdelegate.h"
#include "bookmarks/bookmarksmanager.h"
#include "abstractplugin.h"
#include "taskmanager.h"
#include "pluginsmanager.h"
#include "tools.h"
#include "supportedformats.h"
#include "metadataeditor.h"

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

MainWindow::MainWindow():
    m_collectionModel(0),
    m_collectionProxyModel(0),
    m_metadataEditor(0),
    m_canClose(false)
{
    m_settings = new QSettings(_CONFIGDIR + QDir::separator() + "main.conf", QSettings::IniFormat);

    // Initialize pseudo-random numbers generator
    srand(time(NULL));

    // Create default UI
    m_ui = new Ui::MainWindow();
    m_ui->setupUi(this);

    // Set application icon
    m_appIcon = new QIcon(":/icons/mainIcon");
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
    m_playlistLengthLabel->setText(tr("%n track(s)", "", 0).append(" (00:00)"));

    // Set up playlist browser
    QStringList headers = QStringList() << tr("Filename")
                          << tr("Track")
                          << tr("Interpret")
                          << tr("Track name")
                          << tr("Album")
                          << tr("Genre")
                          << tr("Year")
                          << tr("Length")
                          << tr("Bitrate")
                          << tr("Order");

    m_playlistProxyModel = new PlaylistProxyModel(this);
    m_playlistModel = new PlaylistModel(this, headers, m_playlistProxyModel);

    m_playlistProxyModel->setSourceModel(m_playlistModel);
    m_playlistProxyModel->setDynamicSortFilter(false);

    connect(m_playlistModel, SIGNAL(playlistLengthChanged(int, int)),
            this, SLOT(playlistLengthChanged(int, int)));

    m_playlistItemDelegate = new PlaylistItemDelegate(this, m_playlistModel, m_ui->playlistBrowser, m_playlistProxyModel);

    m_ui->playlistBrowser->setModel(m_playlistProxyModel);
    m_ui->playlistBrowser->setItemDelegate(m_playlistItemDelegate);
    m_ui->playlistBrowser->setDropIndicatorShown(true);
    m_ui->playlistBrowser->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui->playlistBrowser->setAlternatingRowColors(true);
    m_ui->playlistBrowser->sortByColumn(-1);
    // Hide the first column (with filename) and the last one (with order number)
    m_ui->playlistBrowser->hideColumn(PlaylistBrowser::FilenameColumn);
    m_ui->playlistBrowser->hideColumn(PlaylistBrowser::RandomOrderColumn);

    // Set playlist browser columns widths and visibility
    const QVariantList playlistColumnsStates(m_settings->value("Window/PlaylistColumnsStates").toList());
    const QVariantList playlistColumnsWidths(m_settings->value("Window/PlaylistColumnsWidths").toList());
    for (int i = 0; i < playlistColumnsStates.count() - 1; i++) {
        if (playlistColumnsStates.at(i).toBool()) {
            m_ui->playlistBrowser->showColumn(i);
            m_ui->playlistBrowser->setColumnWidth(i, playlistColumnsWidths.at(i).toInt());
            m_ui->menuVisible_columns->actions().at(i)->setChecked(true);
        } else {
            m_ui->playlistBrowser->hideColumn(i);
            m_ui->menuVisible_columns->actions().at(i)->setChecked(false);
        }
    }

    // Restore main window geometry
    restoreGeometry(m_settings->value("Window/Geometry", saveGeometry()).toByteArray());
    restoreState(m_settings->value("Window/State", saveState()).toByteArray());
    m_ui->viewsSplitter->restoreState(m_settings->value("Window/ViewsSplit").toByteArray());

    // Set up task manager
    m_taskManager = new TaskManager(m_playlistModel, m_collectionModel);

    // Enable or disable collections
    if (m_settings->value("Collections/EnableCollections", true).toBool() == true) {
        setupCollections();
        if (m_settings->value("Collections/AutoRebuildAfterStart", false).toBool() == true) {
            m_taskManager->rebuildCollections();
        }
    } else {
        m_ui->viewsTab->setTabEnabled(0, false);
    }

    // Set up filesystem browser
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setRootPath(QDir::rootPath());
    m_fileSystemModel->setNameFilters(SupportedFormats::getExtensionList());
    m_fileSystemModel->setNameFilterDisables(false);
    m_ui->filesystemBrowser->setModel(m_fileSystemModel);
    m_ui->filesystemBrowser->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create seek slider and volume slider
    m_ui->seekSlider->setMediaObject(Player::instance()->mediaObject());
    m_ui->volumeSlider->setAudioOutput(Player::instance()->audioOutput());

    // Load last playlist
    if (m_settings->value("Preferences/RestoreSession", true).toBool()) {
        m_taskManager->addFileToPlaylist(QString(_CONFIGDIR).append("/last.m3u"));
        randomModeChanged(m_settings->value("LastSession/RandomMode", false).toBool());
        repeatModeChanged(Player::RepeatMode(m_settings->value("LastSession/RepeatMode", 0).toInt()));
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
    m_settings->setValue("Window/Geometry", saveGeometry());
    m_settings->setValue("Window/State", saveState());
    m_settings->setValue("Window/ViewsSplit", m_ui->viewsSplitter->saveState());
    QList<QVariant> playlistColumnsStates;
    QList<QVariant> playlistColumnsWidths;
    for (int i = 0; i < m_ui->playlistBrowser->model()->columnCount(QModelIndex()); i++) {
        // Don't store "isColumnHidden" but "isColumnVisible"
        playlistColumnsStates.append(!m_ui->playlistBrowser->isColumnHidden(i));
        playlistColumnsWidths.append(m_ui->playlistBrowser->columnWidth(i));
    }
    m_settings->setValue("Window/PlaylistColumnsStates", playlistColumnsStates);
    m_settings->setValue("Window/PlaylistColumnsWidths", playlistColumnsWidths);

    if (m_settings->value("Preferences/RestoreSession", true).toBool()) {
        m_settings->setValue("LastSession/RepeatMode", int(Player::instance()->repeatMode()));
        m_settings->setValue("LastSession/RandomMode", Player::instance()->randomMode());
    }

    // Save current playlist to file
    m_taskManager->savePlaylistToFile(QString(_CONFIGDIR).append("/last.m3u"));

    delete m_bookmarksManager;

    qDebug() << "Waiting for taskManager to finish...";
    delete m_taskManager;

    delete m_playlistProxyModel;
    delete m_playlistModel;

    delete m_collectionProxyModel;
    delete m_collectionModel;

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

    // Create collections popup menu
    m_collectionsPopupMenu = new QMenu(this);
    m_collectionsPopupMenu->addAction(tr("&Expand all"), m_ui->collectionBrowser, SLOT(expandAll()));
    m_collectionsPopupMenu->addAction(tr("&Collapse all"), m_ui->collectionBrowser, SLOT(collapseAll()));
    m_collectionsPopupMenu->addSeparator();
    m_collectionsPopupMenu->addAction(tr("&Delete file from disk"), this, SLOT(removeFileFromDisk()));
    m_ui->collectionBrowser->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create playlist popup menu
    m_playlistPopupMenu = new QMenu(this);
    m_playlistPopupMenu->addAction(tr("&Stop after this track"), m_ui->playlistBrowser, SLOT(setStopTrack()));
    m_playlistPopupMenu->addAction(tr("&Edit metadata"), this, SLOT(showMetadataEditor()));
    m_ui->playlistBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::bindShortcuts()
{
    m_settings->beginGroup("Shortcuts");

    QxtGlobalShortcut *sc1 = new QxtGlobalShortcut(this);
    sc1->setShortcut(QKeySequence::fromString(m_settings->value("PlayPause", "Meta+P").toString()));
    connect(sc1, SIGNAL(activated()),
            this, SLOT(playPause()));

    QxtGlobalShortcut *sc2 = new QxtGlobalShortcut(this);
    sc2->setShortcut(QKeySequence::fromString(m_settings->value("Stop", "Meta+S").toString()));
    connect(sc2, SIGNAL(activated()),
            this, SLOT(stopPlayer()));

    QxtGlobalShortcut *sc3 = new QxtGlobalShortcut(this);
    sc3->setShortcut(QKeySequence::fromString(m_settings->value("PrevTrack", "Meta+B").toString()));
    connect(sc3, SIGNAL(activated()),
            this, SLOT(previousTrack()));

    QxtGlobalShortcut *sc4 = new QxtGlobalShortcut(this);
    sc4->setShortcut(QKeySequence::fromString(m_settings->value("NextTrack", "Meta+N").toString()));
    connect(sc4, SIGNAL(activated()),
            this, SLOT(nextTrack()));

    QxtGlobalShortcut *sc5 = new QxtGlobalShortcut(this);
    sc5->setShortcut(QKeySequence::fromString(m_settings->value("ShowHideWin", "Meta+H").toString()));
    connect(sc5, SIGNAL(activated()),
            this, SLOT(showHideWindow()));

    m_settings->endGroup();
}

void MainWindow::bindSignals()
{
    // Tray icon
    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
    connect(m_trayIcon, SIGNAL(mouseWheelScrolled(int)),
            this, SLOT(trayIconMouseWheelScrolled(int)));

    // Playlist stuff
    connect(m_ui->playlistBrowser, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(playlistBrowserDoubleClick(QModelIndex)));
    connect(m_ui->clearPlaylistSearch, SIGNAL(clicked()),
            this, SLOT(clearPlaylistSearch()));
    connect(m_ui->playlistBrowser->header(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showPlaylistHeaderContextMenu(QPoint)));
    connect(m_ui->playlistSearchEdit, SIGNAL(textChanged(QString)),
            m_playlistProxyModel, SLOT(setFilterRegExp(QString)));
    connect(m_ui->playlistBrowser, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showPlaylistContextMenu(QPoint)));
    connect(m_ui->playlistBrowser, SIGNAL(setTrack(int)),
            this, SLOT(setTrack(int)));

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
            m_taskManager, SLOT(addFileToPlaylist(QString)));
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
            this, SLOT(nextTrack()));
    connect(m_ui->actionPrevious_track, SIGNAL(triggered(bool)),
            this, SLOT(previousTrack()));
    connect(m_ui->actionPlay_pause, SIGNAL(triggered(bool)),
            this, SLOT(playPause()));
    connect(m_ui->actionStop, SIGNAL(triggered(bool)),
            this, SLOT(stopPlayer()));

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
            this, SLOT(clearPlaylist()));
    connect(m_ui->actionSave_playlist, SIGNAL(triggered(bool)),
            this, SLOT(savePlaylist()));
    connect(m_ui->actionShuffle_playlist, SIGNAL(triggered(bool)),
            m_ui->playlistBrowser, SLOT(shuffle()));

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
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionFilename, PlaylistBrowser::FilenameColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionTrack, PlaylistBrowser::TrackColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionInterpret, PlaylistBrowser::InterpretColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionTrackname, PlaylistBrowser::TracknameColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionAlbum, PlaylistBrowser::AlbumColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionGenre, PlaylistBrowser::GenreColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionYear, PlaylistBrowser::YearColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionLength, PlaylistBrowser::LengthColumn);
    m_playlistVisibleColumnContextMenuMapper->setMapping(m_ui->actionBitrate, PlaylistBrowser::BitrateColumn);


    // Task manager
    connect(m_ui->playlistBrowser, SIGNAL(addedFiles(QStringList, int)),
            m_taskManager, SLOT(addFilesToPlaylist(QStringList, int)));
    connect(m_taskManager, SIGNAL(collectionsPopulated()),
            this, SLOT(fixCollectionProxyModel()));
    connect(m_taskManager, SIGNAL(taskStarted(QString)),
            m_ui->statusBar, SLOT(showWorkingBar(QString)));
    connect(m_taskManager, SIGNAL(taskDone()),
            m_ui->statusBar, SLOT(cancelAction()));
    // This refreshes the filter when an item is added to the playlist so the item appears immediately
    connect(m_taskManager, SIGNAL(playlistPopulated()),
            m_playlistProxyModel, SLOT(invalidate()));
    connect(m_taskManager, SIGNAL(insertItemToPlaylist(Player::MetaData, int)),
            m_playlistModel, SLOT(insertItem(Player::MetaData, int)));

    connect(m_taskManager, SIGNAL(collectionsRebuilt()),
            m_taskManager, SLOT(populateCollections()));

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
    if (m_collectionModel) {
        manager->installMenus(m_collectionsPopupMenu, AbstractPlugin::CollectionsPopup);
    }

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
    developers << "Dan Vrátil";
    QStringList artwork;
    artwork << "Matěj Zvěřina"
            << "Michael Ruml";

    QString str = "<h1>" + QApplication::applicationName() + "</h1>"
                  + tr("Version %1").arg(QApplication::applicationVersion()) +
                  "<p><a href=\"http://danvratil.github.io/TepSonic/\">http://danvratil.github.io/TepSonic/</a></p>"
                  "<p>This program is free software; you can redistribute it and/or modify it under the terms of "
                  "the GNU General Public License as published by the Free Software Foundation; either version "
                  "2 of the License, or (at your option) any later version.</p>"
                  "<h2>" + tr("Developers") + ":</h2>"
                  "<p>" + developers.join(", ") + "</p>"
                  "<h2>" + tr("Artwork") + ":</h2>"
                  "<p>" + artwork.join(", ") + "</p>"
                  "<p>&copy; 2009 - 2013 <a href=\"mailto:dan@progdan.cz\">Dan Vrátil</a></p>";
    aboutDlg.about(this, tr("About TepSonic"), str.toAscii());
}

void MainWindow::showSupportedFormats()
{
    QMessageBox msBox(tr("Supported formats"),
                      tr("On this computer TepSonic can play following audio files:") + "\n\n" + SupportedFormats::getExtensionList().join(", "),
                      QMessageBox::Information,
                      QMessageBox::Ok,
                      0, 0);
    msBox.exec();
}

void MainWindow::reportBug()
{
    QDesktopServices::openUrl(QUrl("https://github.com/danvratil/TepSonic/issues", QUrl::TolerantMode));
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
    connect(settingsDlg, SIGNAL(rebuildCollections()), m_taskManager, SLOT(rebuildCollections()));
    connect(settingsDlg, SIGNAL(outputDeviceChanged()), Player::instance(), SLOT(setDefaultOutputDevice()));
    connect(settingsDlg, SIGNAL(accepted()), this, SIGNAL(settingsAccepted()));
    connect(settingsDlg, SIGNAL(accepted()), this, SLOT(settingsDialogAccepted()));
    settingsDlg->exec();

}

void MainWindow::settingsDialogAccepted()
{
    if (m_settings->value("Collections/EnableCollections").toBool()) {
        /* When collections were enabled in preferences but were not created during program startup,
           create them now otherwise we can expect that collectionBrowser is already displayed and
           working so no action is needed */
        if (m_collectionModel == NULL) {
            setupCollections();
        }
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
        nextTrack();
    }
}

void MainWindow::playlistBrowserDoubleClick(const QModelIndex &index)
{
    const QModelIndex mappedIndex = m_playlistProxyModel->mapToSource(index);
    // Play item on row double click
    m_playlistModel->setCurrentItem(m_playlistModel->index(mappedIndex.row(), 0, QModelIndex()));
    setTrack(mappedIndex.row());
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
        playing = metadata.artist + " - " + playing;
    }

    switch (newState) {
    case Phonon::PlayingState:
        m_ui->playPauseButton->setIcon(QIcon(":/icons/pause"));
        m_ui->actionPlay_pause->setIcon(QIcon(":/icons/pause"));
        m_ui->stopButton->setEnabled(true);
        m_ui->actionStop->setEnabled(true);
        m_ui->trackTitleLabel->setText(playing);
        m_ui->playbackTimeLabel->setText("00:00:00");
        m_trayIcon->setToolTip(tr("Playing: ") + playing);
        break;
    case Phonon::PausedState:
        m_ui->playPauseButton->setIcon(QIcon(":/icons/start"));
        m_ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
        m_ui->stopButton->setEnabled(true);
        m_ui->actionStop->setEnabled(true);
        m_ui->trackTitleLabel->setText(tr("%1 [paused]").arg(m_ui->trackTitleLabel->text()));
        m_trayIcon->setToolTip(tr("%1 [paused]").arg(m_trayIcon->toolTip()));
        break;
    case Phonon::StoppedState:
        m_ui->playPauseButton->setIcon(QIcon(":/icons/start"));
        m_ui->actionPlay_pause->setIcon(QIcon(":/icons/start"));
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

void MainWindow::previousTrack()
{
    // We must remap the current item to get it's real position in current filter
    const QModelIndex currentItem = m_playlistProxyModel->mapFromSource(m_playlistModel->currentItem());
    if ((currentItem.row() > 0) && (m_playlistModel->rowCount(QModelIndex()) > 0)) {
        m_playlistModel->setCurrentItem(m_playlistModel->previousItem());
        setTrack(m_playlistModel->currentItem().row());
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
        if ((player->currentSource().fileName().isEmpty()) &&
                (m_playlistModel->rowCount() > 0)) {
            m_playlistModel->setCurrentItem(m_playlistModel->index(0, 0, QModelIndex()));
            setTrack(0);
        }
        player->play();
    }
}

void MainWindow::nextTrack()
{
    // unmapped!!! index
    const QModelIndex currentItem = m_playlistModel->currentItem();

    // If the track we just played was "stop-on-this" track then stop playback
    const QModelIndex stopTrack = m_playlistModel->getStopTrack();
    if (stopTrack.isValid() && stopTrack.row() == currentItem.row()) {
        stopPlayer();
        return;
    }

    Player *player = Player::instance();
    // 1) Random playback?
    if (player->randomMode()) {
        int row = rand() % m_playlistModel->rowCount(QModelIndex());
        m_playlistModel->setCurrentItem(m_playlistModel->index(row, 0, QModelIndex()));
        // 2) Not last item?
    } else if (m_playlistModel->nextItem().isValid()) {
        m_playlistModel->setCurrentItem(m_playlistModel->nextItem());
        // 3) Repeat all playlist?
    } else if (player->repeatMode() == Player::RepeatAll) {
        m_playlistModel->setCurrentItem(m_playlistModel->index(0, 0, QModelIndex()));
        // 4) Stop, there's nothing else to play
    } else {
        return;
    }

    setTrack(m_playlistModel->currentItem().row());
}

void MainWindow::addPlaylistItem(const QString &filename)
{
    m_taskManager->addFileToPlaylist(filename);
}

void MainWindow::showPlaylistHeaderContextMenu(const QPoint &pos)
{
    m_ui->menuVisible_columns->popup(m_ui->playlistBrowser->header()->mapToGlobal(pos));
}

void MainWindow::togglePlaylistColumnVisibility(int column)
{
    m_ui->playlistBrowser->setColumnHidden(column, (! m_ui->playlistBrowser->isColumnHidden(column)));
}

void MainWindow::savePlaylist()
{
    QString filename;
    filename = QFileDialog::getSaveFileName(this,
                                            tr("Save playlist to..."),
                                            QString(),
                                            tr("M3U Playlist (*.m3u)"));
    m_taskManager->savePlaylistToFile(filename);
}

void MainWindow::playlistLengthChanged(int totalLength, int tracksCount)
{
    QString time = formatTimestamp(totalLength);
    m_playlistLengthLabel->setText(tr("%n track(s)", "", tracksCount).append(" (" + time + ")"));
}

void MainWindow::clearPlaylistSearch()
{
    m_ui->playlistSearchEdit->setText("");
}

void MainWindow::clearCollectionSearch()
{
    m_ui->colectionSearchEdit->setText("");
}

void MainWindow::showError(const QString &error)
{
    m_ui->statusBar->showMessage(error, 5000);
}

void MainWindow::fixCollectionProxyModel()
{
    // Workaround for QTBUG 7585
    // http://bugreports.qt.nokia.com/browse/QTBUG-7585
    /*m_collectionProxyModel->invalidate();
    m_collectionProxyModel->setFilterRegExp("");*/
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

void MainWindow::collectionBrowserDoubleClick(const QModelIndex &index)
{
    //QModelIndex mappedIndex = m_collectionProxyModel->mapToSource(index);
    const QModelIndex mappedIndex = index;
    const QString file = mappedIndex.sibling(index.row(), 1).data().toString();

    if (!file.isEmpty()) {
        m_taskManager->addFileToPlaylist(file);
    }
}

void MainWindow::trayIconMouseWheelScrolled(int delta)
{
    Player *player = Player::instance();
    if (delta > 0) {
        player->audioOutput()->setMuted(false);
        if (player->audioOutput()->volume() < 1) {
            player->audioOutput()->setVolume(player->audioOutput()->volume() + 0.1);
        } else {
            player->audioOutput()->setVolume(1);
        }
    } else {
        // not a typo
        if (player->audioOutput()->volume() > 0.01) {
            player->audioOutput()->setVolume(player->audioOutput()->volume() - 0.1);
        } else {
            player->audioOutput()->setMuted(true);
        }
    }
}

void MainWindow::showCollectionsContextMenu(const QPoint &pos)
{
    m_collectionsPopupMenu->popup(m_ui->collectionBrowser->mapToGlobal(pos));
}

void MainWindow::showPlaylistContextMenu(const QPoint &pos)
{
    const bool valid = (m_ui->playlistBrowser->currentIndex().isValid() ||
                  m_ui->playlistBrowser->indexAt(pos).isValid());

    for (int i = 0; i < m_playlistPopupMenu->actions().count(); i++)
        m_playlistPopupMenu->actions().at(i)->setEnabled(valid);

    // Store pointer to playlist item's index
    QModelIndex *index;
    if (!m_ui->playlistBrowser->indexAt(pos).isValid())
        index = new QModelIndex(m_ui->playlistBrowser->currentIndex());
    else
        index = new QModelIndex(m_ui->playlistBrowser->indexAt(pos));

    const QVariant pitem = qVariantFromValue((void *) index);
    m_playlistPopupMenu->setProperty("playlistItem", pitem);

    m_playlistPopupMenu->popup(m_ui->playlistBrowser->mapToGlobal(pos));
}


void MainWindow::removeFileFromDisk()
{
    qDebug() << m_ui->collectionBrowser->mapFromGlobal(m_collectionsPopupMenu->pos());
    const QModelIndex item = m_ui->collectionBrowser->indexAt(m_ui->collectionBrowser->mapFromGlobal(m_collectionsPopupMenu->pos()));
    const QString itemName = item.data().toString();
    const QString file = item.sibling(item.row(), 1).data().toString();

    QString question;
    if (file.isEmpty()) {
        question = QString(tr("Are you sure you want to remove all content of %1 from hard disk?").arg(itemName));
    } else {
        question = QString(tr("Are you sure you want to remove track %1 (%2) from hard disk?").arg(itemName, file));
    }

    if (QMessageBox::question(this,
                              tr("Confirm removal"),
                              question,
                              QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::Yes) {


        if (file.isEmpty()) {
            QStringList files = m_collectionModel->getItemChildrenTracks(item);
            for (int i = 0; i < files.count(); i++) {
                // Remove file
                QFile::remove(files.at(i));
                // And save only path instead of filepath
                files[i] = QFileInfo(files.at(i)).absolutePath();
            }
            // Remove duplicate paths from list
            files = files.toSet().toList();
            // Rebuild collections in all paths that were affected
            for (int i = 0; i < files.count(); i++) {
                m_taskManager->rebuildCollections(files.at(i));
            }
        } else {
            // Remove the file
            QFile::remove(file);
            // Rebuild collections in the file's path
            m_taskManager->rebuildCollections(QFileInfo(file).absolutePath());
        }
    }
}


void MainWindow::setupCollections()
{
    // Not translatable
    QStringList headers;
    headers = QStringList() << "title" << "filename" << "data1" << "data2";
    m_collectionModel = new CollectionModel(headers, this);

    /*m_collectionProxyModel = new CollectionProxyModel(this);
    m_collectionProxyModel->setSourceModel(m_collectionModel);
    m_collectionProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_collectionProxyModel->setFilterKeyColumn(0);
    m_collectionProxyModel->setDynamicSortFilter(true);*/

    //m_collectionItemDelegate = new CollectionItemDelegate(this, m_collectionProxyModel);
    m_collectionItemDelegate = new CollectionItemDelegate(this);

    m_ui->collectionBrowser->setModel(m_collectionModel);
    m_ui->collectionBrowser->setItemDelegate(m_collectionItemDelegate);
    m_ui->collectionBrowser->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_ui->collectionBrowser->setDragEnabled(true);
    m_ui->collectionBrowser->setDropIndicatorShown(true);
    m_ui->collectionBrowser->setAlternatingRowColors(true);
    m_ui->collectionBrowser->setRootIsDecorated(true);
    // Hide the last three columns that cotain filename and additional data
    m_ui->collectionBrowser->hideColumn(1);
    m_ui->collectionBrowser->hideColumn(2);
    m_ui->collectionBrowser->hideColumn(3);
    // Hide the header
    m_ui->collectionBrowser->header()->setHidden(true);

    connect(m_ui->collectionBrowser, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showCollectionsContextMenu(QPoint)));
    connect(m_ui->collectionBrowser, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(collectionBrowserDoubleClick(QModelIndex)));
    connect(m_taskManager, SIGNAL(clearCollectionModel()),
            m_collectionModel, SLOT(clear()));
    connect(m_taskManager, SIGNAL(insertItemToCollections(QModelIndex, QString, QString, QString, QString, QModelIndex *)),
            m_collectionModel, SLOT(addItem(QModelIndex, QString, QString, QString, QString, QModelIndex *)));


    // Since we disabled the Proxy model, no search inputs are needed
    m_ui->label_2->hide();
    m_ui->colectionSearchEdit->hide();
    m_ui->clearCollectionSearch->hide();

    m_ui->viewsTab->setTabEnabled(0, true);

    m_taskManager->populateCollections();
}

void MainWindow::destroyCollections()
{
    m_ui->collectionBrowser->setModel(NULL);
    delete m_collectionProxyModel;
    m_collectionProxyModel = NULL;
    delete m_collectionModel;
    m_collectionModel = NULL;

    m_ui->viewsTab->setTabEnabled(0, false);
}

void MainWindow::setTrack(int row)
{
    QModelIndex currentItem = m_playlistModel->currentItem();
    QModelIndex index = m_playlistModel->index(currentItem.row(), 0, QModelIndex());
    QString filename = m_playlistModel->data(index, 0).toString();
    Player::instance()->setTrack(filename, true);
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

    const QModelIndex currentIndex = m_ui->playlistBrowser->currentIndex();
    const QModelIndex mappedIndex = m_playlistProxyModel->mapToSource(currentIndex);
    PlaylistItem *item = m_playlistModel->getItem(mappedIndex);

    m_metadataEditor->setFilename(item->data(PlaylistBrowser::FilenameColumn).toString());
    m_metadataEditor->setTrackTitle(item->data(PlaylistBrowser::TracknameColumn).toString());
    m_metadataEditor->setAlbum(item->data(PlaylistBrowser::AlbumColumn).toString());
    m_metadataEditor->setArtist(item->data(PlaylistBrowser::InterpretColumn).toString());
    m_metadataEditor->setGenre(item->data(PlaylistBrowser::GenreColumn).toString());
    m_metadataEditor->setYear(item->data(PlaylistBrowser::YearColumn).toInt());
    m_metadataEditor->setTrackNumber(item->data(PlaylistBrowser::TrackColumn).toInt());

    m_metadataEditor->exec();
}

void MainWindow::metadataEditorAccepted()
{
    if (!m_metadataEditor) {
        return;
    }

    TagLib::FileRef f(m_metadataEditor->filename().toLocal8Bit().constData());
    // FileRef::isNull() is not enough sometimes. Let's check the tag() too...
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

    const QModelIndex currentIndex = m_ui->playlistBrowser->currentIndex();
    const QModelIndex mappedIndex = m_playlistProxyModel->mapToSource(currentIndex);
    PlaylistItem *item = m_playlistModel->getItem(mappedIndex);

    item->setData(PlaylistBrowser::TracknameColumn, m_metadataEditor->trackTitle());
    item->setData(PlaylistBrowser::AlbumColumn, m_metadataEditor->album());
    item->setData(PlaylistBrowser::InterpretColumn, m_metadataEditor->artist());
    item->setData(PlaylistBrowser::GenreColumn, m_metadataEditor->genre());
    item->setData(PlaylistBrowser::YearColumn, m_metadataEditor->year());
    item->setData(PlaylistBrowser::TrackColumn, m_metadataEditor->trackNumber());

    m_taskManager->rebuildCollections(m_metadataEditor->filename());

    delete m_metadataEditor;
}
