
#ifndef STATISTICS_H
#define STATISTICS_H

#include <QJsonObject>
#include <QString>

namespace SaveStatistics {

enum class Outcome {
    Win,
    Loss,
    Tie
};

bool recordRound(
    QJsonObject& gameData,
    const QString& gameName,
    Outcome outcome,
    QString& error
    );

}

#endif
