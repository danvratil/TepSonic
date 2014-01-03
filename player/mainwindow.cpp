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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings/settingsdialog.h"
#include "taskmanager.h"
#include "metadataeditor.h"

#include <core/constants.h>
#include <core/actionmanager.h>
#include <core/playlist.h>
#include <core/abstractplugin.h>
#include <core/pluginsmanager.h>
#include <core/utils.h>
#include <core/supportedformats.h>
#include <core/settings.h>
#include <core/collectionmodel.h>
#include <core/player.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

// For random()
#include <cstdlib>
#include <ctime>

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
#include <QSignalMapper>
#include <QActionGroup>
#include <QDebug>

#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>

#include "qxtglobalshortcut.h"

Q_DECLARE_METATYPE(QModelIndex)

using namespace TepSonic;

MainWindow::MainWindow():
    m_canClose(false)
{
    // Initialize pseudo-random numbers generator
    srand(time(NULL));

    // Create default UI
    m_ui = new Ui::MainWindow();
    m_ui->setupUi(this);

    // Create menus
    createMenus();

    // Create label for displaying playlist length
    m_playlistLengthLabel = new QLabel(this);
    m_ui->statusBar->addPermanentWidget(m_playlistLengthLabel, 0);
    m_playlistLengthLabel->setText(tr("%n track(s)", "", 0).append(QLatin1String(" (00:00)")));

    // Set playlist browser columns widths and visibility
    const QVariantList playlistColumnsStates = Settings::instance()->playlistColumnsStates();
    const QVariantList playlistColumnsWidths = Settings::instance()->playlistColumnsWidths();
    QMenu *menu = ActionManager::instance()->menu(QStringLiteral("PlaylistVisibleColumns"));
    for (int i = 0; i < playlistColumnsStates.count() - 1; i++) {
        if (playlistColumnsStates.at(i).toBool()) {
            m_ui->playlistView->showColumn(i);
            m_ui->playlistView->setColumnWidth(i, playlistColumnsWidths.at(i).toInt());
            menu->actions().at(i)->setChecked(true);
        } else {
            m_ui->playlistView->hideColumn(i);
            menu->actions().at(i)->setChecked(false);
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

    // Create seek slider and volume slider
    m_ui->seekSlider->setMediaObject(Player::instance()->mediaObject());
    m_ui->volumeSlider->setAudioOutput(Player::instance()->audioOutput());

    // Load last playlist
    if (Settings::instance()->sessionRestore()) {
        Player::instance()->playlist()->loadPlaylist(XdgConfigDir + QLatin1String("/last.m3u"));
        Player::instance()->setRandomMode(Settings::instance()->playerRandomMode());
        Player::instance()->setRepeatMode(static_cast<Player::RepeatMode>(Settings::instance()->playerRepeatMode()));
    }

   // At the very end bind all signals and slots
    bindSignals();
    // Bind global shortcuts
    bindShortcuts();

    // The playlist can be populated before the MainWindow is constructed
    onPlaylistLengthChanged(Player::instance()->playlist()->totalLength(),
                            Player::instance()->playlist()->rowCount());
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
    Player::instance()->playlist()->savePlaylist(XdgConfigDir + QLatin1String("/last.m3u"));

    delete m_ui;
}

void MainWindow::createMenus()
{
    addAction(m_ui->menuTepSonic, QStringLiteral("Settings"),
              this, &MainWindow::openSettingsDialog);
    m_ui->menuTepSonic->addSeparator();
    addAction(m_ui->menuTepSonic, QStringLiteral("ToggleWindow"),
              this, &MainWindow::toggleWindowVisible);
    m_ui->menuTepSonic->addSeparator();
    addAction(m_ui->menuTepSonic, QStringLiteral("Quit"),
              [=]() { m_canClose = true; close(); });

    addAction(m_ui->menuPlaylist, QStringLiteral("PlaylistClear"),
              Player::instance()->playlist(), &Playlist::clear);
    addAction(m_ui->menuPlaylist, QStringLiteral("PlaylistShuffle"),
              Player::instance()->playlist(), &Playlist::shuffle);
    m_ui->menuPlaylist->addSeparator();
    addAction(m_ui->menuPlaylist, QStringLiteral("PlaylistSave"),
              this, &MainWindow::savePlaylist);

    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped),
            [=](int column) { m_ui->playlistView->setColumnHidden(column, (!m_ui->playlistView->isColumnHidden(column))); });
    QMenu *menu = m_ui->menuPlaylist->addMenu(tr("&Visible Columns"));
    for (int i = 0; i < m_ui->playlistView->header()->count(); ++i) {
        QAction *action = ActionManager::instance()->addAction(
                QString::fromLatin1("Column%1").arg(i),
                m_ui->playlistView->model()->headerData(i, Qt::Horizontal).toString());
        action->setCheckable(true);
        connect(action, &QAction::triggered,
                signalMapper, static_cast<void (QSignalMapper::*)(void)>(&QSignalMapper::map));
        signalMapper->setMapping(action, i);
        menu->addAction(action);
    }
    ActionManager::instance()->addMenu(QStringLiteral("PlaylistVisibleColumns"), menu);

    addAction(m_ui->menuPlayer, QStringLiteral("PlayerPreviousTrack"),
              Player::instance(), &Player::previous);
    addAction(m_ui->menuPlayer, QStringLiteral("PlayerPlayPause"),
              this, &MainWindow::playPause);
    addAction(m_ui->menuPlayer, QStringLiteral("PlayerStop"),
              Player::instance(), &Player::stop);
    addAction(m_ui->menuPlayer, QStringLiteral("PlayerNextTrack"),
              Player::instance(), &Player::next);
    m_ui->menuPlayer->addSeparator();
    m_ui->menuPlayer->addMenu(ActionManager::instance()->menu(QStringLiteral("PlayerRandomSubmenu")));
    m_ui->menuPlayer->addMenu(ActionManager::instance()->menu(QStringLiteral("PlayerRepeatSubmenu")));
    ActionManager::instance()->action(QStringLiteral("PlayerRandomOff"))->setChecked(true);
    ActionManager::instance()->action(QStringLiteral("PlayerRepeatOff"))->setChecked(true);

    addAction(m_ui->menuHelp, QStringLiteral("AboutTepSonic"),
              this, &MainWindow::aboutTepSonic);
    addAction(m_ui->menuHelp, QStringLiteral("AboutQt"),
              [=]() { QMessageBox::aboutQt(this, tr("About Qt")); });
    m_ui->menuHelp->addSeparator();
    addAction(m_ui->menuHelp, QStringLiteral("SupportedFormats"),
              this, &MainWindow::showSupportedFormats);
    m_ui->menuHelp->addSeparator();
    addAction(m_ui->menuHelp, QStringLiteral("ReportBug"),
              this, &MainWindow::reportBug);

    // FIXME: Move the entire metadata handling to PlaylistView, including this piece of code
    menu = new QMenu(this);
    menu->addAction(tr("&Stop after this track"), this, SLOT(setStopTrackClicked()));
    menu->addAction(tr("&Edit metadata"), this, SLOT(showMetadataEditor()));
    ActionManager::instance()->addMenu(QStringLiteral("PlaylistContextMenu"), menu);
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
            Player::instance(), &Player::next);

    QxtGlobalShortcut *sc4 = new QxtGlobalShortcut(this);
    sc4->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutNextTrack()));
    connect(sc4, &QxtGlobalShortcut::activated,
            Player::instance(), &Player::previous);

    QxtGlobalShortcut *sc5 = new QxtGlobalShortcut(this);
    sc5->setShortcut(QKeySequence::fromString(Settings::instance()->shortcutToggleWindow()));
    connect(sc5, &QxtGlobalShortcut::activated,
            this, &MainWindow::toggleWindowVisible);
}

void MainWindow::bindSignals()
{
    // Playlist stuff
    connect(m_ui->clearPlaylistSearch, &QPushButton::clicked,
            [=]() { clearPlaylistSearch(); });
    connect(m_ui->playlistSearchEdit, &QLineEdit::textChanged,
            m_ui->playlistView, &PlaylistView::setFilter);

    // Filesystem browser
    connect(m_ui->fsbTab, &FileSystemWidget::trackSelected,
            [=](const QString &filePath) { Player::instance()->playlist()->insert(QStringList() << filePath); });

    // Player object
    connect(Player::instance(), &Player::trackChanged,
            this, &MainWindow::onPlayerTrackChanged);
    connect(Player::instance(), &Player::stateChanged,
            this, &MainWindow::onPlayerStateChanged);
    connect(Player::instance(), &Player::trackPositionChanged,
            this, &MainWindow::onPlayerPositionChanged);
    connect(Player::instance()->playlist(), &Playlist::playlistLengthChanged,
            this, &MainWindow::onPlaylistLengthChanged);

    // Connect UI buttons to their equivalents in the main menu
    connect(m_ui->clearPlaylistButton, &QPushButton::clicked,
            ActionManager::instance()->action(QStringLiteral("PlaylistClear")), &QAction::trigger);
    connect(m_ui->previousTrackButton, &QPushButton::clicked,
            ActionManager::instance()->action(QStringLiteral("PlayerPreviousTrack")), &QAction::trigger);
    connect(m_ui->playPauseButton, &QPushButton::clicked,
            ActionManager::instance()->action(QStringLiteral("PlayerPlayPause")), &QAction::trigger);
    connect(m_ui->stopButton, &QPushButton::clicked,
            ActionManager::instance()->action(QStringLiteral("PlayerStop")), &QAction::trigger);
    connect(m_ui->nextTrackButton, &QPushButton::clicked,
            ActionManager::instance()->action(QStringLiteral("PlayerNextTrack")), &QAction::trigger);
    connect(m_ui->shufflePlaylistButton, &QPushButton::clicked,
            ActionManager::instance()->action(QStringLiteral("PlaylistShuffle")), &QAction::trigger);


    //Plugins
    connect(PluginsManager::instance(), &PluginsManager::pluginsLoaded,
            this, &MainWindow::setupPluginsUIs);
    connect(this, &MainWindow::settingsAccepted,
            PluginsManager::instance(), &PluginsManager::settingsAccepted);
}

void MainWindow::setupPluginsUIs()
{
    PluginsManager *manager = PluginsManager::instance();
    // Let plugins install their menus
    manager->installMenus(ActionManager::instance()->menu(QStringLiteral("TrayIconMenu")), AbstractPlugin::TrayMenu);
    manager->installMenus(ActionManager::instance()->menu(QStringLiteral("PlaylistContextMenu")), AbstractPlugin::PlaylistPopup);

    // Let plugins install their tabs
    manager->installPanes(m_ui->mainTabWidget);
}

void MainWindow::toggleWindowVisible()
{
    if (isVisible()) {
        hide();
    } else {
        show();
    }
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    // If tray icon is visible then hide the window and ignore this event
    if (!m_canClose) {
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

void MainWindow::onPlayerStateChanged(Phonon::State newState, Phonon::State oldState)
{
    Q_UNUSED(oldState);

    const MetaData metadata = m_currentMetaData;

    QString playing;
    if (metadata.title().isEmpty()) {
        playing = QFileInfo(metadata.fileName()).fileName();
    } else {
        playing = metadata.title();
    }
    if (!metadata.artist().isEmpty()) {
        playing = metadata.artist() + QLatin1String(" - ") + playing;
    }

    switch (newState) {
    case Phonon::PlayingState:
        m_ui->playPauseButton->setIcon(QIcon(QLatin1String(":/icons/pause")));
        ActionManager::instance()->action(QStringLiteral("PlayerPlayPause"))->setIcon(QIcon(QLatin1String(":/icons/pause")));
        m_ui->stopButton->setEnabled(true);
        ActionManager::instance()->action(QStringLiteral("PlayerStop"))->setEnabled(true);
        m_ui->trackTitleLabel->setText(playing);
        m_ui->playbackTimeLabel->setText(QLatin1String("00:00:00"));
        break;
    case Phonon::PausedState:
        m_ui->playPauseButton->setIcon(QIcon(QLatin1String(":/icons/start")));
        ActionManager::instance()->action(QStringLiteral("PlayerPlayPause"))->setIcon(QIcon(QLatin1String(":/icons/start")));
        m_ui->stopButton->setEnabled(true);
        ActionManager::instance()->action(QStringLiteral("PlayerStop"))->setEnabled(true);
        m_ui->trackTitleLabel->setText(tr("%1 [paused]").arg(m_ui->trackTitleLabel->text()));
        break;
    case Phonon::StoppedState:
        m_ui->playPauseButton->setIcon(QIcon(QLatin1String(":/icons/start")));
        ActionManager::instance()->action(QStringLiteral("PlayerPlayPause"))->setIcon(QIcon(QLatin1String(":/icons/start")));
        m_ui->stopButton->setEnabled(false);
        ActionManager::instance()->action(QStringLiteral("PlayerStop"))->setEnabled(false);
        m_ui->trackTitleLabel->setText(tr("Player is stopped"));
        m_ui->playbackTimeLabel->setText(tr("Stopped"));
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
        player->play();
    }
}

void MainWindow::onPlayerPositionChanged(qint64 newPos)
{
    m_ui->playbackTimeLabel->setText(Utils::formatMilliseconds(newPos, true));
}

void MainWindow::onPlaylistLengthChanged(int totalLength, int tracksCount)
{
    const QString time = Utils::formatTimestamp(totalLength);
    m_playlistLengthLabel->setText(tr("%n track(s)", "", tracksCount).append(QLatin1String(" (") + time + QLatin1String(")")));
}

void MainWindow::onPlayerTrackChanged()
{
    const int index = Player::instance()->currentTrack();
    if (index > -1) {
        m_currentMetaData = Player::instance()->playlist()->track(index);
    } else {
        m_currentMetaData = MetaData();
    }
}

void MainWindow::savePlaylist()
{
    QString filename;
    filename = QFileDialog::getSaveFileName(this,
                                            tr("Save playlist to..."),
                                            QString(),
                                            tr("M3U Playlist (*.m3u)"));
    Player::instance()->playlist()->savePlaylist(filename);
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

void MainWindow::setupCollections()
{
    m_ui->collectionView->enableCollections();

    connect(m_ui->collectionView, &CollectionView::doubleClicked,
            this, &MainWindow::onCollectionViewDoubleClicked);

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
    QPointer<MetadataEditor> editor(new MetadataEditor());
    const QModelIndex currentIndex = m_ui->playlistView->currentIndex();
    const MetaData metaData = Player::instance()->playlist()->track(currentIndex.row());
    editor->setMetaData(metaData);

    int result = editor->exec();
    if (result == QDialog::Rejected) {
        if (editor) {
            editor->deleteLater();
        }
        return;
    }

    if (!editor) {
        return;
    }

    const MetaData newMetaData = editor->metaData();
    TagLib::FileRef f(newMetaData.fileName().toLocal8Bit().constData());
//     // FileRef::isNull() is not enough sometimes. Let's check the tag() too...
    if (f.isNull() || !f.tag()) {
        return;
    }

    f.tag()->setTrack(newMetaData.trackNumber());
    f.tag()->setArtist(newMetaData.artist().toLocal8Bit().constData());
    f.tag()->setAlbum(newMetaData.album().toLocal8Bit().constData());
    f.tag()->setTitle(newMetaData.title().toLocal8Bit().constData());
    f.tag()->setYear(newMetaData.year());
    f.tag()->setGenre(newMetaData.genre().toLocal8Bit().constData());
    f.save();

    Player::instance()->playlist()->setTrack(currentIndex.row(), newMetaData);
    TaskManager::instance()->rebuildCollections(newMetaData.fileName());

    delete editor;
}

void MainWindow::setStopTrackClicked()
{
    const QModelIndex index = m_ui->playlistView->currentIndex();
    if (index.row() == Player::instance()->stopTrack()) {
        Player::instance()->setStopTrack(-1);
    } else {
        Player::instance()->setStopTrack(index.row());
    }
}

void MainWindow::onCollectionViewDoubleClicked(const QModelIndex& index)
{
    // FIXME: Share MetaData
    const QString file = index.data(CollectionModel::FilePathRole).toString();
    if (!file.isEmpty()) {
        Player::instance()->playlist()->insert(QStringList() << file);
    }
}
