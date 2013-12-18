
option(WITH_LASTFMSCROBBLER_PLUGIN "Enable Last.fm scrobbler plugin" ON)
if (WITH_LASTFMSCROBBLER_PLUGIN)
  add_subdirectory(LastFmScrobbler)
endif ()


option(WITH_LYRICS_PLUGIN "Enable Lyrics plugin" ON)
if (WITH_LYRICS_PLUGIN)
  add_subdirectory(lyrics)
endif ()


option(WITH_MPRIS2_PLUGIN "Enable MPRIS2 plugin" ON)
if (WITH_MPRIS2_PLUGIN)
  add_subdirectory(mpris2)
endif ()
