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
#include "trayicon.h"

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
    connect(m_trayIcon, &TrayIcon::activated,
            this, &MainWindow::trayClicked);


    // Create label for displaying playlist length
    m_playlistLengthLabel = new QLabel(this);
    m_ui->statusBar->addPermanentWidget(m_playlistLengthLabel, 0);
    m_playlistLengthLabel->setText(tr("%n track(s)", "", 0).append(QLatin1String(" (00:00)")));

    connect(m_ui->playlistView, &PlaylistView::playlistLengthChanged,
            this, &MainWindow::playlistLengthChanged);
    connect(m_ui->playlistView, &PlaylistView::nowPlayingChanged,
            this, &MainWindow::setCurrentTrack);

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
    connect(sc1, &QxtGlobalShortcut::activated,
            this, &MainWindow::playPause);

    QxtGlobalShortcut *sc2 = new QxtGlobalShortcut(this);
    sc2->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutStop()));
    connect(sc2, &QxtGlobalShortcut::activated,
            Player::instance(), &Player::stop);

    QxtGlobalShortcut *sc3 = new QxtGlobalShortcut(this);
    sc3->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutPreviousTrack()));
    connect(sc3, &QxtGlobalShortcut::activated,
            m_ui->playlistView, &PlaylistView::selectPreviousTrack);

    QxtGlobalShortcut *sc4 = new QxtGlobalShortcut(this);
    sc4->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutNextTrack()));
    connect(sc4, &QxtGlobalShortcut::activated,
            m_ui->playlistView, &PlaylistView::selectNextTrack);

    QxtGlobalShortcut *sc5 = new QxtGlobalShortcut(this);
    sc5->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutToggleWindow()));
    connect(sc5, &QxtGlobalShortcut::activated,
            [=]() { trayClicked(QSystemTrayIcon::Trigger); });
}

void MainWindow::bindSignals()
{
    // Playlist stuff
    connect(m_ui->clearPlaylistSearch, &QPushButton::clicked,
            [=]() { clearPlaylistSearch(); });
    connect(m_ui->playlistView->header(), &QHeaderView::customContextMenuRequested,
            this, &MainWindow::showPlaylistHeaderContextMenu);
    connect(m_ui->playlistSearchEdit, &QLineEdit::textChanged,
            m_ui->playlistView, &PlaylistView::setFilter);
    connect(m_ui->playlistView, &PlaylistView::customContextMenuRequested,
            this, &MainWindow::showPlaylistContextMenu);

    // Filesystem browser
    connect(m_ui->fsbBackBtn, &QPushButton::clicked,
            m_ui->filesystemBrowser, &FileSystemBrowser::goBack);
    connect(m_ui->fsbFwBtn, &QPushButton::clicked,
            m_ui->filesystemBrowser, &FileSystemBrowser::goForward);
    connect(m_ui->fsbGoHomeBtn, &QPushButton::clicked,
            m_ui->filesystemBrowser, &FileSystemBrowser::goHome);
    connect(m_ui->fsbCdUpBtn, &QPushButton::clicked,
            m_ui->filesystemBrowser, &FileSystemBrowser::cdUp);
    connect(m_ui->fsbPath, &QLineEdit::textChanged,
            m_ui->filesystemBrowser, &FileSystemBrowser::goToDir);
    connect(m_ui->filesystemBrowser, &FileSystemBrowser::pathChanged,
            m_ui->fsbPath, &QLineEdit::setText);
    connect(m_ui->filesystemBrowser, &FileSystemBrowser::addTrackToPlaylist,
            [=](const QString &file) { TaskManager::instance()->addFileToPlaylist(file); });
    connect(m_ui->filesystemBrowser, &FileSystemBrowser::disableBack,
            m_ui->fsbBackBtn, &QWidget::setDisabled);
    connect(m_ui->filesystemBrowser, &FileSystemBrowser::disableForward,
            m_ui->fsbFwBtn, &QWidget::setDisabled);
    connect(m_ui->filesystemBrowser, &FileSystemBrowser::disableCdUp,
            m_ui->fsbCdUpBtn, &QWidget::setDisabled);
    connect(m_ui->fsbBookmarksBtn, &QPushButton::toggled,
            m_bookmarksManager, &BookmarksManager::toggleBookmarks);
    connect(m_ui->filesystemBrowser, &FileSystemBrowser::addBookmark,
            m_bookmarksManager, &BookmarksManager::showAddBookmarkDialog);


    // Menu 'Player'
    connect(m_ui->actionNext_track, &QAction::triggered,
            m_ui->playlistView, &PlaylistView::selectNextTrack);
    connect(m_ui->actionPrevious_track, &QAction::triggered,
            m_ui->playlistView, &PlaylistView::selectPreviousTrack);
    connect(m_ui->actionPlay_pause, &QAction::triggered,
            this, &MainWindow::playPause);
    connect(m_ui->actionStop, &QAction::triggered,
            Player::instance(), &Player::stop);

    // Menu 'Player -> Repeat mode'
    connect(m_ui->actionRepeat_OFF, &QAction::triggered,
            [=]() { Player::instance()->setRepeatMode(Player::RepeatOff); });
    connect(m_ui->actionRepeat_playlist, &QAction::triggered,
            [=]() { Player::instance()->setRepeatMode(Player::RepeatAll); });
    connect(m_ui->actionRepeat_track, &QAction::triggered,
            [=]() { Player::instance()->setRepeatMode(Player::Player::RepeatTrack); });

    // Menu 'Player -> Random mode'
    connect(m_ui->actionRandom_ON, &QAction::triggered,
            [=]() { Player::instance()->setRandomMode(true); });
    connect(m_ui->actionRandom_OFF, &QAction::triggered,
            [=]() { Player::instance()->setRandomMode(false); });

    // Menu 'Playlist'
    connect(m_ui->actionClear_playlist, &QAction::triggered,
            m_ui->playlistView, &PlaylistView::clearSelection);
    connect(m_ui->actionSave_playlist, &QAction::triggered,
            this, &MainWindow::savePlaylist);
    connect(m_ui->actionShuffle_playlist, &QAction::triggered,
            m_ui->playlistView, &PlaylistView::shuffle);

    // Menu 'TepSonic'
    connect(m_ui->actionSettings, &QAction::triggered,
            this, &MainWindow::openSettingsDialog);
    connect(m_ui->actionShow_Hide, &QAction::triggered,
            [=]() { trayClicked(QSystemTrayIcon::Trigger); });
    connect(m_ui->actionQuit_TepSonic, &QAction::triggered,
            [=]() { m_canClose = true; close(); });

    // Menu 'Help
    connect(m_ui->actionReport_a_bug, &QAction::triggered,
            this, &MainWindow::reportBug);
    connect(m_ui->actionAbout_TepSonic, &QAction::triggered,
            this, &MainWindow::aboutTepSonic);
    connect(m_ui->actionAbout_Qt, &QAction::triggered,
            [=]() { QMessageBox::aboutQt(this, tr("About Qt")); });
    connect(m_ui->actionSupported_formats, &QAction::triggered,
            this, &MainWindow::showSupportedFormats);

    // Player object
    connect(Player::instance(), static_cast<void (Player::*)()>(&Player::trackFinished),
            this, &MainWindow::updatePlayerTrack);
    connect(Player::instance(), &Player::stateChanged,
            this, &MainWindow::playerStatusChanged);
    connect(Player::instance(), &Player::repeatModeChanged,
            this, &MainWindow::repeatModeChanged);
    connect(Player::instance(), &Player::randomModeChanged,
            this, &MainWindow::randomModeChanged);
    connect(Player::instance(), &Player::trackPositionChanged,
            this, &MainWindow::playerPosChanged);

    // Connect UI buttons to their equivalents in the main menu
    connect(m_ui->clearPlaylistButton, &QPushButton::clicked,
            m_ui->actionClear_playlist, &QAction::trigger);
    connect(m_ui->previousTrackButton, &QPushButton::clicked,
            m_ui->actionPrevious_track, &QAction::trigger);
    connect(m_ui->playPauseButton, &QPushButton::clicked,
            m_ui->actionPlay_pause, &QAction::trigger);
    connect(m_ui->stopButton, &QPushButton::clicked,
            m_ui->actionStop, &QAction::trigger);
    connect(m_ui->nextTrackButton, &QPushButton::clicked,
            m_ui->actionNext_track, &QAction::trigger);
    connect(m_ui->shufflePlaylistButton, &QPushButton::clicked,
            m_ui->actionShuffle_playlist, &QAction::trigger);


    // Connect individual PlaylistBrowser columns' visibility state with QActions in ui->menuVisiblem_columns
    m_playlistVisibleColumnContextMenuMapper = new QSignalMapper(this);
    connect(m_ui->actionFilename, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionTrack, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionInterpret, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionTrackname, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionAlbum, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionGenre, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionYear, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionLength, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_ui->actionBitrate, &QAction::toggled,
            [=](bool) { m_playlistVisibleColumnContextMenuMapper->map(); });
    connect(m_playlistVisibleColumnContextMenuMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped),
            [=](int column) { m_ui->playlistView->setColumnHidden(column, (! m_ui->playlistView->isColumnHidden(column))); });

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

    // Plugins
    connect(PluginsManager::instance(), &PluginsManager::pluginsLoaded,
            this, &MainWindow::setupPluginsUIs);
    connect(this, &MainWindow::settingsAccepted,
            PluginsManager::instance(), &PluginsManager::settingsAccepted);
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
    connect(settingsDlg, &SettingsDialog::accepted,
            this, &MainWindow::settingsAccepted);
    connect(settingsDlg, &SettingsDialog::accepted,
            this, &MainWindow::settingsDialogAccepted);
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
        connect(m_metadataEditor, &MetadataEditor::accepted,
                this, &MainWindow::metadataEditorAccepted);
        connect(m_metadataEditor, &MetadataEditor::rejected,
                m_metadataEditor, &MetadataEditor::deleteLater);
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
