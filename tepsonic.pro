TEMPLATE = subdirs
CONFIG += debug

SUBDIRS = src \
  plugins

plugins.depends = src

DISTFILES += LICENSE

