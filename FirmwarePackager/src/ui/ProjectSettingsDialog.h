#pragma once

#include <QDialog>
#include <QLineEdit>

// Dialog allowing configuration of project root and output directories
class ProjectSettingsDialog : public QDialog {
    Q_OBJECT
public:
    ProjectSettingsDialog(const QString& root, const QString& output,
                         const QString& pkgId, QWidget* parent = nullptr);
    QString rootDir() const;
    QString outputDir() const;
    QString pkgId() const;
private slots:
    void browseRoot();
    void browseOutput();
private:
    QLineEdit* rootEdit;
    QLineEdit* outputEdit;
    QLineEdit* pkgIdEdit;
};

