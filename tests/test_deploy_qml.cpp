// library includes
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

// local includes
#include "../src/qml.h"
#include "../src/util.h"

namespace bf = boost::filesystem;

namespace linuxdeploy {
    namespace plugin {
        namespace qt {
            namespace test {
                class TestDeployQml : public testing::Test {

                public:
                    boost::filesystem::path appDirPath;
                    boost::filesystem::path projectQmlRoot;
                    boost::filesystem::path defaultQmlImportPath;
                    boost::filesystem::path qtLibexecsPath;

                    void SetUp() override {
                        appDirPath = getTempDirName();
                        projectQmlRoot = appDirPath.string() + "/usr/qml";

                        boost::filesystem::create_directories(projectQmlRoot);
                        boost::filesystem::copy_file(TESTS_DATA_DIR "/qml_project/file.qml",
                            projectQmlRoot.string() + "/file.qml");

                        setenv(ENV_KEY_QML_MODULES_PATHS, TESTS_DATA_DIR, 1);

                        auto qmakeVars = queryQmake(findQmake());
                        defaultQmlImportPath = qmakeVars["QT_INSTALL_QML"];
                        qtLibexecsPath = qmakeVars["QT_INSTALL_LIBEXECS"];
                    }

                    std::string getTempDirName() const {
                        char tmpl[] = "/tmp/linuxdeploy-plugin-qt-unit-tests-appdir-XXXXXX";
                        char* tempDir = mkdtemp(tmpl);

                        std::string name{tempDir};
                        return name;
                    }

                    void TearDown() override {
                        boost::filesystem::remove_all(appDirPath);
                        unsetenv(ENV_KEY_QML_MODULES_PATHS);
                    }
                };

                TEST_F(TestDeployQml, find_qmlimporter_path) {
                    auto result = findQmlImportScanner(qtLibexecsPath);
                    boost::filesystem::path expected = "/usr/bin/qmlimportscanner";

                    ASSERT_FALSE(result.empty());
                    ASSERT_EQ(result.string(), expected.string());
                }

                TEST_F(TestDeployQml, runQmlImportScanner) {
                    auto result = runQmlImportScanner(qtLibexecsPath,
                                                      {projectQmlRoot},
                                                      {TESTS_DATA_DIR, defaultQmlImportPath});
                    ASSERT_FALSE(result.empty());
                    std::cout << result;
                }

                TEST_F(TestDeployQml, run_qmlimportscanner_without_qml_import_paths) {
                    auto result = runQmlImportScanner(qtLibexecsPath,
                                                      {projectQmlRoot},
                                                      {});
                    ASSERT_FALSE(result.empty());
                    std::cout << result;
                }

                // disabled, as Debian's qmlimportscanner does not behave as expected
                /*
                TEST_F(TestDeployQml, getQmlImports) {
                    auto results = getQmlImports(projectQmlRoot, defaultQmlImportPath);
                    ASSERT_FALSE(results.empty());
                    std::cout << "Imported Qml Modules Found:";
                    for (auto result : results) {
                        std::cout << "\n\tName: " << result.name << "\n\tPath: " << result.path << "\n\tRelative path:"
                                  << result.relativePath << std::endl;
                        ASSERT_FALSE(result.path.empty());
                        ASSERT_FALSE(result.relativePath.empty());
                    }
                }
                */

                TEST_F(TestDeployQml, deploy_qml_imports) {
                    linuxdeploy::core::appdir::AppDir appDir(appDirPath);

                    // speed up test runs; we don't check for the copyright files anyway
                    appDir.setDisableCopyrightFilesDeployment(true);

                    deployQml(appDir, defaultQmlImportPath, qtLibexecsPath);
                    appDir.executeDeferredOperations();

                    ASSERT_TRUE(boost::filesystem::exists(projectQmlRoot.string() + "/QtQuick.2"));
                    ASSERT_TRUE(boost::filesystem::exists(projectQmlRoot.string() + "/qml_module"));
                }

                TEST_F(TestDeployQml, getQmlModuleRelativePath) {
                    std::vector<bf::path> qmlModulesImportPaths = {"/usr/lib/x86_64-linux-gnu/qt5/qml", "/usr/lib/",
                                                                   "/usr/lib/qt5.10/qml", TESTS_DATA_DIR};

                    auto rpath = getQmlModuleRelativePath(qmlModulesImportPaths,
                        "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick.2");
                    ASSERT_EQ(rpath, "QtQuick.2");

                    rpath = getQmlModuleRelativePath(qmlModulesImportPaths,
                        "/usr/lib/x86_64-linux-gnu/qt5/qml/org/kde/plasma");
                    ASSERT_EQ(rpath, "org/kde/plasma");

                    rpath = getQmlModuleRelativePath(qmlModulesImportPaths, TESTS_DATA_DIR "/qml_module");
                    ASSERT_EQ(rpath, "qml_module");

                    rpath = getQmlModuleRelativePath(qmlModulesImportPaths, "/qml_module");
                    ASSERT_EQ(rpath, "");
                }
            }
        }
    }
}
