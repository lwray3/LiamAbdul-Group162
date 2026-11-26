#include <QApplication>
#include "ui/mainwindow.h"

#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QJsonObject>
#include <QMessageBox>
#include <QDebug>
#include "storage/storage.h"
#include "storage/statistics.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("MirageGroup162");
    a.setApplicationName("MirageCasino");

    QString saveDirectory =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation
            );

    if (saveDirectory.isEmpty()) {
        QMessageBox::critical(
            nullptr, "Storage Error",
            "Cannot determine the save directory."
            );
        return 1;
    }

    QString savePath =
        QDir(saveDirectory).filePath("mirage_save.json");

    JsonFileStorage storage(savePath);

    QJsonObject gameData;
    QString error;

    auto status = storage.load(gameData, error);

    if (status == JsonFileStorage::LoadStatus::Success) {

        qDebug() << "Existing save loaded successfully.";

    }
    else if (status == JsonFileStorage::LoadStatus::NotFound) {

        qDebug() << "No save found. Loading default data.";

        JsonFileStorage defaultStorage(
            ":/data/default.json"
            );

        auto defaultStatus =
            defaultStorage.load(gameData, error);

        if (defaultStatus !=
            JsonFileStorage::LoadStatus::Success) {

            QMessageBox::critical(
                nullptr, "Storage Error",
                "Cannot load default data: " + error
                );
            return 1;
        }

        if (!storage.save(gameData, error)) {
            QMessageBox::critical(
                nullptr, "Storage Error",
                "Cannot create save file: " + error
                );
            return 1;
        }

        qDebug() << "Initial save created.";
    }
    else {
        QMessageBox::critical(
            nullptr, "Storage Error",
            "Cannot load the existing save file: " + error
            );
        return 1;
    }

    qDebug() << "Save location:" << savePath;


    MainWindow w;

    auto saveRound = [&storage, &gameData, &w](
                         const QString& gameName,
                         SaveStatistics::Outcome outcome
                         ) {
        QJsonObject updatedData = gameData;
        QString saveError;

        bool recorded = SaveStatistics::recordRound(
            updatedData,
            gameName,
            outcome,
            saveError
            );

        if (!recorded) {
            QMessageBox::warning(
                &w,
                "Statistics Error",
                "Could not update " + gameName +
                    " statistics: " + saveError
                );
            return;
        }

        if (!storage.save(updatedData, saveError)) {
            QMessageBox::warning(
                &w,
                "Save Error",
                "Could not save " + gameName +
                    " statistics: " + saveError
                );
            return;
        }

        gameData = updatedData;

        QJsonObject stats =
            gameData["statistics"].toObject()
                [gameName].toObject();

        qDebug() << gameName << "round saved!";
        qDebug() << "Total rounds:"
                 << stats["rounds_played"].toInt();
        qDebug() << "Wins:"
                 << stats["wins"].toInt();
        qDebug() << "Losses:"
                 << stats["losses"].toInt();
        qDebug() << "Ties:"
                 << stats["ties"].toInt();
    };

    QObject::connect(
        &w,
        &MainWindow::baccaratRoundFinished,
        &w,
        [&saveRound](int playerScore, int bankerScore) {

            SaveStatistics::Outcome outcome;

            if (playerScore > bankerScore) {
                outcome = SaveStatistics::Outcome::Win;
            }
            else if (bankerScore > playerScore) {
                outcome = SaveStatistics::Outcome::Loss;
            }
            else {
                outcome = SaveStatistics::Outcome::Tie;
            }

            saveRound("baccarat", outcome);
        }
        );

    auto savePlayerResult = [&saveRound](
                                const QString& gameName,
                                bool playerWon,
                                bool tied
                                ) {
        SaveStatistics::Outcome outcome;

        if (tied) {
            outcome = SaveStatistics::Outcome::Tie;
        }
        else if (playerWon) {
            outcome = SaveStatistics::Outcome::Win;
        }
        else {
            outcome = SaveStatistics::Outcome::Loss;
        }

        saveRound(gameName, outcome);
    };

    QObject::connect(
        &w,
        &MainWindow::blackjackRoundFinished,
        &w,
        [&savePlayerResult](bool playerWon, bool tied) {
            savePlayerResult(
                "blackjack",
                playerWon,
                tied
                );
        }
        );

    QObject::connect(
        &w,
        &MainWindow::pokerRoundFinished,
        &w,
        [&savePlayerResult](bool playerWon, bool tied) {
            savePlayerResult(
                "poker",
                playerWon,
                tied
                );
        }
        );

    w.show();

    return a.exec();
}
