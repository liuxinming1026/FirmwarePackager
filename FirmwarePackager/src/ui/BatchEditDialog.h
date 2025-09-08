#pragma once

#include <QDialog>
#include <QLineEdit>

// Dialog for batch editing file entries
class BatchEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit BatchEditDialog(QWidget* parent = nullptr);
    QString prefix() const;
    QString mode() const;
    QString owner() const;
    QString group() const;
private:
    QLineEdit* prefixEdit;
    QLineEdit* modeEdit;
    QLineEdit* ownerEdit;
    QLineEdit* groupEdit;
};

