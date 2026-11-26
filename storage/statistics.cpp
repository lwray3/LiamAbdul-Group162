
#include "statistics.h"

#include <QJsonValue>
#include <limits>

namespace SaveStatistics {

static bool incrementCounter(
    QJsonObject& object,
    const QString& key
    ) {
    QJsonValue value = object.value(key);

    if (!value.isDouble()) {
        return false;
    }

    int count = value.toInt(-1);

    if (count < 0 ||
        count == std::numeric_limits<int>::max()) {
        return false;
    }

    object[key] = count + 1;
    return true;
}

bool recordRound(
    QJsonObject& gameData,
    const QString& gameName,
    Outcome outcome,
    QString& error
    ) {
    error.clear();

    // Only accept the three supported games
    if (gameName != "blackjack" &&
        gameName != "poker" &&
        gameName != "baccarat") {

        error = "Unknown game: " + gameName;
        return false;
    }

    if (!gameData.value("statistics").isObject()) {
        error = "Missing statistics object.";
        return false;
    }

    QJsonObject statistics =
        gameData.value("statistics").toObject();

    if (!statistics.value(gameName).isObject()) {
        error = "Missing statistics for " + gameName;
        return false;
    }

    QJsonObject gameStats =
        statistics.value(gameName).toObject();

    QString resultKey;

    switch (outcome) {
    case Outcome::Win:
        resultKey = "wins";
        break;
    case Outcome::Loss:
        resultKey = "losses";
        break;
    case Outcome::Tie:
        resultKey = "ties";
        break;
    }

    // Increase completed round count
    if (!incrementCounter(gameStats, "rounds_played")) {
        error = "Invalid rounds_played counter.";
        return false;
    }

    // Increase the appropriate result counter
    if (!incrementCounter(gameStats, resultKey)) {
        error = "Invalid " + resultKey + " counter.";
        return false;
    }

    statistics[gameName] = gameStats;
    gameData["statistics"] = statistics;

    return true;
}

}
