#ifndef TOOLS_H
#define TOOLS_H

#include <QString>

//! Parses timestamp
/*!
  Parses given timestamp and returns time in format "W week(s) D day(s) hh:mm:ss"
  \param time timestamp to parse
  \return Returns formatted time
*/
QString formatTimestamp(qint64 time);

QString formatMilliseconds(qint64 msecs);


#endif // TOOLS_H
