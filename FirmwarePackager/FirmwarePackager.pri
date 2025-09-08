QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


HEADERS += \
    $$PWD/src/core/Hasher.h \
    $$PWD/src/core/IdGenerator.h \
    $$PWD/src/core/Logger.h \
    $$PWD/src/core/ManifestWriter.h \
    $$PWD/src/core/Packager.h \
    $$PWD/src/core/ProjectModel.h \
    $$PWD/src/core/ProjectSerializer.h \
    $$PWD/src/core/Scanner.h \
    $$PWD/src/core/ScriptWriter.h \
    $$PWD/src/ui/GuiLogger.h \
    $$PWD/src/ui/MainWindow.h \
    $$PWD/src/ui/MappingDialog.h \
    $$PWD/src/ui/ProjectSettingsDialog.h \
    $$PWD/src/ui/BatchEditDialog.h

SOURCES += \
    $$PWD/src/core/Hasher.cpp \
    $$PWD/src/core/IdGenerator.cpp \
    $$PWD/src/core/Logger.cpp \
    $$PWD/src/core/ManifestWriter.cpp \
    $$PWD/src/core/Packager.cpp \
    $$PWD/src/core/ProjectModel.cpp \
    $$PWD/src/core/ProjectSerializer.cpp \
    $$PWD/src/core/Scanner.cpp \
    $$PWD/src/core/ScriptWriter.cpp \
    $$PWD/src/core/core.cpp \
    $$PWD/src/ui/GuiLogger.cpp \
    $$PWD/src/ui/MainWindow.cpp \
    $$PWD/src/ui/MappingDialog.cpp \
    $$PWD/src/ui/ProjectSettingsDialog.cpp \
    $$PWD/src/ui/BatchEditDialog.cpp

# ================= 路径与第三方 =================
isEmpty(LIBARCHIVE_ROOT): LIBARCHIVE_ROOT = $$PWD/third_party/libarchive
isEmpty(OPENSSL_ROOT): OPENSSL_ROOT = $$PWD/third_party/openssl
isEmpty(JSON_INCLUDE_DIRS): JSON_INCLUDE_DIRS = $$PWD/third_party


LIBARCHIVE_INCLUDE_DIR = $$LIBARCHIVE_ROOT/include
LIBARCHIVE_LIB_DIR = $$LIBARCHIVE_ROOT/lib
LIBARCHIVE_BIN_DIR = $$LIBARCHIVE_ROOT/bin


OPENSSL_INCLUDE_DIR = $$OPENSSL_ROOT/include
OPENSSL_LIB_DIR = $$OPENSSL_ROOT/lib
OPENSSL_BIN_DIR = $$OPENSSL_ROOT/bin


INCLUDEPATH += \
    $$PWD/src \
    $$JSON_INCLUDE_DIRS \
    $$LIBARCHIVE_INCLUDE_DIR \
    $$OPENSSL_INCLUDE_DIR


win32 {
    LIBS += \
        $$LIBARCHIVE_LIB_DIR/archive.lib \
        $$LIBARCHIVE_LIB_DIR/zlib.lib \
        $$LIBARCHIVE_LIB_DIR/bz2.lib \
        $$LIBARCHIVE_LIB_DIR/lz4.lib \
        $$LIBARCHIVE_LIB_DIR/zstd.lib \
        $$LIBARCHIVE_LIB_DIR/lzma.lib \
        $$OPENSSL_LIB_DIR/libssl.lib \
        $$OPENSSL_LIB_DIR/libcrypto.lib
}

win32 {
    LIBARCHIVE_BIN_DIR_WIN = $$shell_path($$LIBARCHIVE_BIN_DIR)
    OPENSSL_BIN_DIR_WIN    = $$shell_path($$OPENSSL_BIN_DIR)

    isEmpty(DESTDIR) {
        CONFIG(release, debug|release): BINDIR = $$OUT_PWD/release
        CONFIG(debug,   debug|release): BINDIR = $$OUT_PWD/debug
    } else {
        BINDIR = $$absolute_path($$DESTDIR, $$OUT_PWD)
    }
    BINDIR = $$shell_path($$BINDIR)

    # 不再额外加引号
    copy_runtime.commands = $$QMAKE_COPY_DIR $$LIBARCHIVE_BIN_DIR_WIN\\*.dll $$BINDIR\\ >NUL && \
                            $$QMAKE_COPY_DIR $$OPENSSL_BIN_DIR_WIN\\*.dll $$BINDIR\\ >NUL

    copy_runtime.CONFIG += phony
    QMAKE_EXTRA_TARGETS += copy_runtime
    QMAKE_POST_LINK     += $$copy_runtime.commands
}

# ================= 部署的默认规则 =================
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
