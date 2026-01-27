#include "addfiles.h"
#include "ui_addfiles.h"
#include "tools.h"
#include <QListWidgetItem>
#include <QFont>
#include <QIcon>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <QSettings>
#include <QDebug>
#include <QTextStream>
#include <QUuid>
#include <QString>
#include <fstream>
#include <string>
using namespace std;


addFiles::addFiles(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addFiles)
{
    ui->setupUi(this);

    connect(ui->pushButtonAddFiles, &QPushButton::clicked, this, &addFiles::selectFiles);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &addFiles::onOkClicked);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &addFiles::onCancelClicked);




    //open DB
    db = nullptr;
    int rc = sqlite3_open("../../identfs.db", &db);
    if(rc) {
        std::cerr << "Database can't be opened: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Database opened successfully!" << std::endl;
    }
}

addFiles::~addFiles()
{
    delete ui;
}

float addFiles::getFileSize(const std::string &filename)
{
    std::ifstream in(filename.c_str(), std::ios::binary);
    if (!in) return 0; // Datei konnte nicht geöffnet werden

    in.seekg(0, std::ios::end);        // ans Ende der Datei springen
    std::streampos size = in.tellg();  // Position = Dateigröße in Bytes
    return static_cast<float>(size / 1024); // in KB umrechnen
}

void addFiles::selectFiles()
{
    files = QFileDialog::getOpenFileNames(
        this,
        "Dateien auswählen",
        "",  // Startordner
        "Alle Dateien (*);;Bilder (*.png *.jpg *.bmp)"
        );
    if (!files.isEmpty()) {
        for (const QString &filePath : files) {
            QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/icons/unkown_file.png"), filePath);
            ui->listWidgetFiles->addItem(item);
            qDebug() << "Ausgewählte Datei:" << filePath;
        }
    }
}

void addFiles::onOkClicked() {
    qDebug() << "OK gedrueckt!";

    if (!files.isEmpty()) {
        for (const QString &filePath : files) {
            // --- UUID erzeugen ---
            QUuid uuid = QUuid::createUuid();           // Qt erzeugt UUID
            QString uuidText = uuid.toString();         // String für DB und ADS

            int fileSize = getFileSize(filePath.toStdString());

            // --- Insert in DB ---
            sqlite3_stmt* stmt;
            const char* sqlInsert = "INSERT INTO files (file_uuid, last_path, size, active) VALUES (?, ?, ?, 1);";
            int rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Error while preparing statement: " << sqlite3_errmsg(db) << std::endl;
                continue;
            }

            sqlite3_bind_text(stmt, 1, uuidText.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, filePath.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 3, fileSize);

            if (sqlite3_step(stmt) != SQLITE_DONE)
                std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;

            sqlite3_finalize(stmt);

            // --- UUID in eigenem ADS speichern ---
            QString uuidStream = filePath + ":IdentFS_UUID";  // ADS-Stream
            QFile file(uuidStream);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "UUID=" << uuidText;
                file.close();
                qDebug() << "UUID geschrieben:" << uuidText;
            } else {
                qDebug() << "Fehler beim Öffnen des ADS für" << filePath;
            }

            std::cout << "Datei erfolgreich hinzugefügt: " << filePath.toStdString() << std::endl;
            QString uuidReaded = readUuidFromFile(filePath);
            qDebug() << "Ausgelesene UUID:" << uuidReaded;
        }
    }

    close();
}

void addFiles::onCancelClicked() {
    close();
}

QString addFiles::readUuidFromFile(const QString &filePath) {
    QString adsPath = filePath + ":IdentFS_UUID";  // ADS-Pfad
    QFile file(adsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Fehler beim Öffnen des ADS für" << filePath;
        return QString();
    }

    QTextStream in(&file);
    QString line = in.readLine();
    file.close();

    // Annahme: Zeile hat Format "UUID=<uuid>"
    if (line.startsWith("UUID=")) {
        return line.mid(5); // alles nach "UUID="
    }

    return QString();
}
