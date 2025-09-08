#pragma once

#include <QPlainTextEdit>
#include "src/core/Logger.h"

// Simple GUI logger that writes log messages to a QPlainTextEdit widget
class GuiLogger : public core::ILogger {
public:
    explicit GuiLogger(QPlainTextEdit* widget);
    void info(const std::string& msg) override;
    void error(const std::string& msg) override;
private:
    QPlainTextEdit* logWidget;
};

