#include "GuiLogger.h"

GuiLogger::GuiLogger(QPlainTextEdit* widget) : logWidget(widget) {}

void GuiLogger::info(const std::string& msg) {
    if (logWidget) {
        logWidget->appendPlainText(QString::fromStdString(msg));
    }
}

void GuiLogger::error(const std::string& msg) {
    if (logWidget) {
        logWidget->appendPlainText(QString::fromStdString(msg));
    }
}

