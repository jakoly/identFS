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


    //----CODE----
    // DB öffnen
    db = nullptr;
    int rc = sqlite3_open("../../identfs.db", &db);
    if(rc) {
        std::cerr << "Database can't be opened: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Database opened successfully!" << std::endl;
    }


    updateProjectList();
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
    settings.setValue("user/name", "Jakob");
    settings.setValue("app/theme", "light");
    settings.setValue("user/driveLetter", "C");

    // Werte können jederzeit wieder ausgelesen werden
    QString name = settings.value("user/name").toString();
    QString theme = settings.value("app/theme").toString();
    QString driveLetter = settings.value("user/driveLetter").toString();

    qDebug() << "Name:" << name << "Theme:" << theme << "Drive-Letter: " << driveLetter;
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
        // Fehlerbehandlung
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

void MainWindow::onProjectItemClicked(QListWidgetItem *item)
{
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

    qDebug() << "Item geklickt: " << projectName << " UUID:" << uuid;

    openAddFilesToProject();
}
