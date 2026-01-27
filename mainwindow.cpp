#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "addfilestoproject.h"
#include "addfiles.h"
#include <QListWidgetItem>
#include <QFont>
#include <QIcon>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <QSettings>
#include <QDebug>
#include <filesystem>
#include <ctime>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    saveDefaultSettings();

    //----Slot-Connection----
    connect(ui->pushButtonCreateProject, &QPushButton::clicked, this, &MainWindow::newProject);
    connect(ui->pushButtonAddFiles, &QPushButton::clicked, this, &MainWindow::openAddFilesWindow);
    connect(ui->projectList, &QListWidget::itemClicked, this, &MainWindow::onProjectItemClicked);
    connect(ui->listWidgetFiles, &QListWidget::itemClicked, this, &MainWindow::onFileClicked);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &MainWindow::turnAddProjectInvisible);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &MainWindow::onOkClicked);




    //----CODE----
    // DB öffnen
    db = nullptr;
    int rc = sqlite3_open("../../identfs.db", &db);
    if(rc) {
        std::cerr << "Database can't be opened: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Database opened successfully!" << std::endl;
    }

    // --- UI-Setup ---
    // Quelle
    turnAddProjectInvisible();
}

MainWindow::~MainWindow()
{
    if(db) sqlite3_close(db);
    delete ui;
}



std::string MainWindow::guidToString(const GUID& guid) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(8) << guid.Data1 << "-"
       << std::setw(4) << guid.Data2 << "-"
       << std::setw(4) << guid.Data3 << "-";
    for(int i=0; i<2; i++) ss << std::setw(2) << static_cast<int>(guid.Data4[i]);
    ss << "-";
    for(int i=2; i<8; i++) ss << std::setw(2) << static_cast<int>(guid.Data4[i]);
    return ss.str();
}

void MainWindow::newProject()
{
    QString projectNameInput = ui->lineEditProjectName->text();
    std::string projectName = projectNameInput.toStdString();
    if(projectName.empty()) return;

    // UUID erzeugen
    GUID guid;
    if (CoCreateGuid(&guid) != S_OK) {
        std::cerr << "Error while creating the uuid!" << std::endl;
        return;
    }
    std::string UUID = guidToString(guid);
    std::cout << "Project-UUID: " << UUID << std::endl;
    std::cout << "Name: " << projectName << std::endl;

    // --- Vault-Ordner erstellen ---
    std::filesystem::path path;
    try {
        // Drive Letter direkt aus QSettings
        QSettings settings("settings.ini", QSettings::IniFormat);
        QString driveLetter = settings.value("user/driveLetter", "C").toString();

        // Pfad zusammensetzen (mit ":" für Windows-Laufwerke)
        path = std::filesystem::path((driveLetter + ":\\").toStdString())
                                     / "IdentFS" / "vault" / UUID;

        // Ordner erstellen
        if (std::filesystem::create_directories(path)) {
            std::cout << "Ordner erstellt: " << path << std::endl;
        } else {
            std::cout << "Ordner existiert bereits oder konnte nicht erstellt werden." << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Fehler: " << e.what() << std::endl;
    }

    //Generate Timestamp
    time_t timestamp;
    time(&timestamp);


    // Insert in DB
    sqlite3_stmt* stmt;
    const char* sqlInsert = "INSERT INTO projects (project_uuid, name, vault_path, create_date, modification_date) VALUES (?, ?, ?, ?, ?);";
    int rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
    if(rc != SQLITE_OK) {
        std::cerr << "Error while preparing: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, UUID.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, projectName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, path.string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, ctime(&timestamp), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, ctime(&timestamp), -1, SQLITE_TRANSIENT);

    if(sqlite3_step(stmt) != SQLITE_DONE)
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;

    sqlite3_finalize(stmt);




    std::cout << "Projekt erfolgreich hinzugefuegt!" << std::endl;
    updateProjectList();

    // LineEdit leeren
    ui->lineEditProjectName->clear();
}

void MainWindow::openAddFilesWindow()
{
    // Modelless-Fenster (kann parallel zu MainWindow benutzt werden)
    addFiles *window = new addFiles(this);  // "this" als Parent optional
    window->show();  // zeigt das Fenster
}

void MainWindow::openAddFilesToProject() {
    addFilesToProject *window = new addFilesToProject(this);  // "this" als Parent optional
    window->show();  // zeigt das Fenster
}

void MainWindow::saveDefaultSettings()
{
    // Pfad + Format bestimmen (INI-Datei im Programmverzeichnis)
    QSettings settings("settings.ini", QSettings::IniFormat);

    // Werte hartcodiert setzen
    settings.setValue("user/name", "User");
    settings.setValue("app/theme", "light");
    settings.setValue("user/driveLetter", "C");

    // Werte können jederzeit wieder ausgelesen werden
    QString name = settings.value("user/name").toString();
    QString theme = settings.value("app/theme").toString();
    QString driveLetter = settings.value("user/driveLetter").toString();

    qDebug() << "Name:" << name << "Theme:" << theme << "Drive-Letter: " << driveLetter;
}

void MainWindow::updateFileList() {
    ui->listWidgetFiles->clear();
    ui->listWidgetFiles->addItem(new QListWidgetItem("Alle Dateien ⤵️"));
    ui->listWidgetFilesInProject->clear();
    ui->listWidgetFilesInProject->addItem(new QListWidgetItem("Projektdateien ⤵️"));

    tempFileUUIDs.clear();
    qDebug() << tempFileUUIDs << " is empty now";
    qDebug() << "Update File List";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT file_uuid, name, last_path, size, active FROM files;", -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "SQLite-Fehler";
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QString uuid = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        QString name = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        QString lastPath = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        QString size = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        int active = sqlite3_column_int(stmt, 4);

        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/icons/unkown_file.png"), name);

        QVariantMap data;
        data["name"] = name;
        data["uuid"] = uuid;
        data["lastPath"] = lastPath;
        data["size"] = size;
        data["active"] = active;

        item->setData(Qt::UserRole, data);
        ui->listWidgetFiles->addItem(item);
    }

    sqlite3_finalize(stmt);
}

void MainWindow::updateProjectList() {

    // QListWidget Item
    ui->projectList->setIconSize(QSize(32, 32));
    QFont font = ui->projectList->font();
    font.setPointSize(14);
    ui->projectList->setFont(font);
    ui->projectList->clear();


    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT project_uuid, name, vault_path, create_date, modification_date FROM projects;", -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "sqLite-Fehler";
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* uuidText = sqlite3_column_text(stmt, 0);
        const unsigned char* nameText = sqlite3_column_text(stmt, 1);
        const unsigned char* vaultText = sqlite3_column_text(stmt, 2);
        const unsigned char* create_dateText = sqlite3_column_text(stmt, 3);
        const unsigned char* modification_dateText = sqlite3_column_text(stmt, 4);

        QString uuid = reinterpret_cast<const char*>(uuidText);
        QString name = reinterpret_cast<const char*>(nameText);
        QString vaultPath = reinterpret_cast<const char*>(vaultText);
        QString create_date = reinterpret_cast<const char*>(create_dateText);
        QString modification_date = reinterpret_cast<const char*>(modification_dateText);

        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/icons/project.png"), name);
        QVariantMap data;
        data["name"] = name;
        data["uuid"] = uuid;
        data["vault_path"] = vaultPath;
        data["createDate"] = create_date;
        data["modification_date"] = modification_date;

        item->setData(Qt::UserRole, data);

        ui->projectList->addItem(item);
    }

    sqlite3_finalize(stmt);
}

void MainWindow::turnAddProjectInvisible() {
    ui->groupNewProject->setVisible(false);
    projectUUID = "";
    projectName = "";
    projectCreated = "";
    projectModificated = "";
    projectVaultPath = "";
    tempFileUUIDs.clear();

    ui->groupNewProject->setStyleSheet("QGroupBox { border: none; }");
    ui->labelProjectName->setText("Willkommen bei identFS!");

    updateFileList();
    updateProjectList();
    qDebug() << tempFileUUIDs << " is empty now";

    qDebug() << "Kein Projekt ausgewählt.";
}

void MainWindow::onProjectItemClicked(QListWidgetItem *item)
{
    updateFileList();

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString name = data["name"].toString();
    QString uuid = data["uuid"].toString();
    QString vaultPath = data["vaultPath"].toString();
    QString createDate = data["createDate"].toString();
    QString modification_date = data["modification_date"].toString();

    projectUUID = uuid;
    projectName = name;
    projectCreated = createDate;
    projectModificated = modification_date;
    projectVaultPath = vaultPath;

    loadProjectFiles(projectUUID);

    qDebug() << "Item geklickt: " << projectName << " UUID:" << uuid;

    ui->groupNewProject->setVisible(true);

    ui->labelProjectName->setText(QString("%1").arg(projectName));
    ui->labelCreated->setText(QString("Projekt erstellt:    %1").arg(projectCreated));
    ui->labelLastChange->setText(QString("Projekt geändert:     %1").arg(projectModificated));
}

void MainWindow::onFileClicked(QListWidgetItem *item) {
    QListWidgetItem *newItem = item->clone();
    ui->listWidgetFilesInProject->addItem(newItem);

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString uuid = data["uuid"].toString();
    tempFileUUIDs.append(uuid);
    qDebug() << tempFileUUIDs;
}

void MainWindow::onOkClicked() {
    const char* sqlInsert =
        "INSERT INTO file_to_project (file_uuid, project_uuid) VALUES (?, ?);";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
        return;
    }

    for (const QString &fileUUID : tempFileUUIDs) {
        QByteArray fileUuidUtf8 = fileUUID.toUtf8();
        QByteArray projectUuidUtf8 = projectUUID.toUtf8();

        sqlite3_bind_text(stmt, 1, fileUuidUtf8.constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, projectUuidUtf8.constData(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            qDebug() << "Insert failed:" << sqlite3_errmsg(db);
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        qDebug() << fileUUID << " was added to " << projectUUID;
    }

    sqlite3_finalize(stmt);

    QMessageBox::information(
        this,
        "Dateien wurden hinzugefügt",
        "Die Dateien wurden erfolgreich zum Projekt hinzugefügt"
    );

    turnAddProjectInvisible();
}

void MainWindow::loadProjectFiles(const QString &projectUuid) {
    ui->listWidgetFilesInProject->clear(); // vorher leeren

    // 1 Alle file_uuid für das Projekt aus file_to_project holen
    QStringList fileUUIDs;

    const char* sqlFilesToProject = "SELECT file_uuid FROM file_to_project WHERE project_uuid = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlFilesToProject, -1, &stmt, nullptr) != SQLITE_OK) {
        qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, projectUuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QString fileUuid = QString::fromUtf8(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))
            );
        fileUUIDs.append(fileUuid);
    }

    sqlite3_finalize(stmt);

    // 2️⃣ Für jede file_uuid den Namen aus files holen und Item hinzufügen
    const char* sqlFileName = "SELECT name FROM files WHERE file_uuid = ?;";

    for (const QString &fileUuid : fileUUIDs) {
        sqlite3_stmt* stmtName = nullptr;
        if (sqlite3_prepare_v2(db, sqlFileName, -1, &stmtName, nullptr) != SQLITE_OK) {
            qDebug() << "Prepare failed:" << sqlite3_errmsg(db);
            continue;
        }

        sqlite3_bind_text(stmtName, 1, fileUuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmtName) == SQLITE_ROW) {
            QString fileName = QString::fromUtf8(
                reinterpret_cast<const char*>(sqlite3_column_text(stmtName, 0))
                );

            // Item erstellen und zum ListWidget hinzufügen
            QListWidgetItem *item = new QListWidgetItem(fileName);
            ui->listWidgetFilesInProject->addItem(item);
        }

        sqlite3_finalize(stmtName);
    }
}
