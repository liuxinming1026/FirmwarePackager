#pragma once

#include <QDialog>
#include <QLineEdit>
#include "src/core/ProjectModel.h"

// Dialog for editing mapping information of a FileEntry
class MappingDialog : public QDialog {
    Q_OBJECT
public:
    explicit MappingDialog(core::FileEntry& entry, QWidget* parent = nullptr);
protected:
    void accept() override;
private:
    core::FileEntry& fileEntry;
    QLineEdit* idEdit;
};

