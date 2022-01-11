// library includes
#include <boost/filesystem.hpp>
#include <linuxdeploy/core/appdir.h>

#pragma once

typedef struct {
    std::string name;
    boost::filesystem::path path;
    boost::filesystem::path relativePath;
} QmlModuleImport;

static const char* const ENV_KEY_QML_MODULES_PATHS = "QML_MODULES_PATHS";
static const char* const ENV_KEY_QML_SOURCES_PATHS = "QML_SOURCES_PATHS";

struct QmlImportScannerError : public std::runtime_error {
    explicit QmlImportScannerError(const std::string& message) : runtime_error(message) {}
};

// deploys QML files into AppDir
void deployQml(linuxdeploy::core::appdir::AppDir &appDir,
               const boost::filesystem::path &installQmlPath,
               const boost::filesystem::path &qtLibexecsPath);

boost::filesystem::path findQmlImportScanner(const boost::filesystem::path &qtLibexecsPath);

std::string runQmlImportScanner(const boost::filesystem::path &qtLibexecsPath,
                                const std::vector<boost::filesystem::path> &sourcesPaths,
                                const std::vector<boost::filesystem::path>& qmlImportPaths);

boost::filesystem::path getQmlModuleRelativePath(const std::vector<boost::filesystem::path>& qmlModulesImportPaths,
                                                 const boost::filesystem::path& qmlModulePath);

std::vector<QmlModuleImport> getQmlImports(const boost::filesystem::path& projectRootPath,
                                           const boost::filesystem::path& installQmlPath,
                                           const boost::filesystem::path &qtLibexecsPath);
