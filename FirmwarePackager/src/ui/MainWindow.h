#pragma once

#include <QMainWindow>
#include <QTableView>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QStandardItemModel>
#include <memory>
#include "src/core/Packager.h"
#include "src/core/ProjectModel.h"
#include "src/core/ProjectSerializer.h"
#include "GuiLogger.h"
#include <filesystem>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void newProject();
    void openProject();
    void saveProject();
    void openRoot();
    void buildPackage();
    void openSettings();
    void editMapping();
    void addFile();
    void addFolder();
    void batchEdit();
    void previewScript();

private:
    void populateTable(const core::Project& project);
    core::Project buildProject(const std::filesystem::path& root,
                               const core::Scanner::PathList& exclusions = {});

    QTableView* tableView;
    QStandardItemModel* model;
    QPlainTextEdit* logPane;
    QLineEdit* rootEdit;
    QLineEdit* outputEdit;

    GuiLogger* guiLogger;
    core::Scanner scanner;
    core::Hasher hasher;
    core::ManifestWriter manifest;
    core::IdGenerator idGen;
    core::ScriptWriter script;
    core::ProjectSerializer serializer;
    std::unique_ptr<core::Packager> packager;
    std::filesystem::path tplRoot;
    core::Project currentProject;
};

