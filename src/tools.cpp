#include <QString>
#include <QObject>

#include "tools.h"

QString formatTimestamp(qint64 time)
{
    int weeks = time / 604800;
    int days = (time - (weeks*604800))/ 86400;
    int hours = (time - (weeks*604800) - (days*86400))/ 3600;
    int mins = (time - (weeks*604800) - (days*86400) - (hours*3600))/60;
    int secs = time - (weeks*604800) - (days*86400) - (hours*3600) - (mins*60);

    QString sWeeks;
    QString sDays;
    QString sHours;
    QString sMins;
    QString sSecs;

    if (weeks > 0) {
        sWeeks = QObject::tr("%n week(s)","",weeks).append(" ");
    }
    if (days > 0) {
        sDays = QObject::tr("%n day(s)","",days).append(" ");
    }

    if (hours<10) {
        sHours = QString("0").append(QString::number(hours));
    } else {
        sHours = QString::number(hours);
    }
    if (hours == 0) sHours = QString("00");
    if (mins<10) {
        sMins = QString("0").append(QString::number(mins));
    } else {
        sMins = QString::number(mins);
    }
    if (secs<10) {
        sSecs = QString("0").append(QString::number(secs));
    } else {
        sSecs = QString::number(secs);
    }

    return sWeeks+sDays+sHours+":"+sMins+":"+sSecs;
}

QString formatMilliseconds(qint64 msecs)
{
    int secs = (int)(msecs/1000);
    int hours = secs/3600;
    int mins = (secs - (hours*3600))/60;
    secs = secs - (hours*3600) - (mins*60);

    QString sHours;
    QString sMins;
    QString sSecs;

    if (hours<10) {
        sHours = QString("0").append(QString::number(hours));
    } else {
        sHours = QString::number(hours);
    }
    if (hours == 0) sHours = QString("00");
    if (mins<10) {
        sMins = QString("0").append(QString::number(mins));
    } else {
        sMins = QString::number(mins);
    }
    if (secs<10) {
        sSecs = QString("0").append(QString::number(secs));
    } else {
        sSecs = QString::number(secs);
    }

    return sHours+":"+sMins+":"+sSecs;
}
