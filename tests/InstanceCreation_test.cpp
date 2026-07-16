#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QTemporaryDir>
#include <QTest>

#include <minecraft/MinecraftInstance.h>
#include <settings/INISettingsObject.h>
#include <settings/SettingsObject.h>

class TestVersion : public BaseVersion {
   public:
    QString m_name;
    QString m_descriptor;

    TestVersion(QString name, QString descriptor) : m_name(std::move(name)), m_descriptor(std::move(descriptor)) {}

    QString descriptor() const override { return m_descriptor; }
    QString name() const override { return m_name; }
    QString typeString() const override { return "Release"; }
};

class InstanceCreationTest : public QObject {
    Q_OBJECT

   private:
    std::unique_ptr<SettingsObject> createGlobalSettings()
    {
        auto settings = std::make_unique<INISettingsObject>("dummy.ini");
        settings->registerSetting("ShowGameTime", false);
        settings->registerSetting("RecordGameTime", false);
        settings->registerSetting("OverrideGameTime", false);
        settings->registerSetting("PreLaunchCommand", QString());
        settings->registerSetting("WrapperCommand", QString());
        settings->registerSetting("PostExitCommand", QString());
        settings->registerSetting("OverrideCommands", false);
        settings->registerSetting("ShowConsole", false);
        settings->registerSetting("AutoCloseConsole", false);
        settings->registerSetting("ShowConsoleOnError", false);
        settings->registerSetting("LogPrePostOutput", false);
        settings->registerSetting("ConsoleMaxLines", 1000);
        settings->registerSetting("ConsoleOverflowStop", 1000000);
        settings->registerSetting("LauncherName", QString("Launcher"));
        settings->registerSetting("Notes", QString());
        settings->registerSetting("LastTimePlayed", QDateTime());
        settings->registerSetting("TotalTimePlayed", 0);
        settings->registerSetting("ConsoleWindowState", QByteArray());
        settings->registerSetting("ConsoleLogGeometry", QByteArray());
        settings->registerSetting("GameTimeLog", QVariantList());
        settings->registerSetting("ManagedPackType", QString());
        settings->registerSetting("ManagedPackName", QString());
        settings->registerSetting("ManagedPackVersion", QString());
        settings->registerSetting("ManagedPackURL", QString());
        settings->registerSetting("ProfilerDisabled", true);
        settings->registerSetting("ProfilerOutputDir", QString());
        settings->registerSetting("JvmArgs", QString());
        settings->registerSetting("JavaPath", QString());
        settings->registerSetting("SkipLauncherCompatibilityCheck", false);
        settings->registerSetting("IgnoreJavaCompatibility", false);
        settings->registerSetting("UseOptimizedJvmArgs", true);
        settings->registerSetting("GarbageCollectorPreset", QString());
        settings->registerSetting("JavaSignature", QString());
        settings->registerSetting("JavaArchitecture", QString());
        settings->registerSetting("JavaRealArchitecture", QString());
        settings->registerSetting("JavaVersion", QString());
        settings->registerSetting("JavaVendor", QString());
        settings->registerSetting("LaunchMaximized", false);
        settings->registerSetting("MinecraftWinWidth", 854);
        settings->registerSetting("MinecraftWinHeight", 480);
        settings->registerSetting("MinMemAlloc", 512);
        settings->registerSetting("MaxMemAlloc", 1024);
        settings->registerSetting("PermGen", 128);
        settings->registerSetting("LowMemWarning", false);
        settings->registerSetting("UseNativeOpenAL", false);
        settings->registerSetting("CustomOpenALPath", QString());
        settings->registerSetting("UseNativeGLFW", false);
        settings->registerSetting("CustomGLFWPath", QString());
        settings->registerSetting("EnableFeralGamemode", false);
        settings->registerSetting("EnableMangoHud", false);
        settings->registerSetting("UseDiscreteGpu", false);
        settings->registerSetting("UseZink", false);
        settings->registerSetting("CloseAfterLaunch", false);
        settings->registerSetting("QuitAfterGameStop", false);
        settings->registerSetting("OnlineFixes", false);
        settings->registerSetting("ElyPatchPreference", QString());
        settings->registerSetting("Env", QVariantMap());
        return settings;
    }

    void registerCommonSettings(INISettingsObject& settings)
    {
        settings.registerSetting("name", QString());
        settings.registerSetting("iconKey", QString());
        settings.registerSetting("version", QString());
        settings.registerSetting("notes", QString());
        settings.registerSetting("lastLaunch", QDateTime());
        settings.registerSetting("loader", QString());
        settings.registerSetting("loaderVersion", QString());
        settings.registerSetting("javaPath", QString());
        settings.registerSetting("minMemory", 0);
        settings.registerSetting("maxMemory", 0);
    }

   private slots:
    void test_inifileCreation()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString iniPath = tempDir.path() + "/instance.cfg";
        INISettingsObject settings(iniPath);
        registerCommonSettings(settings);

        settings.set("name", "Test Instance");
        settings.set("iconKey", "grass");
        settings.set("version", "1.20.1");

        INISettingsObject loaded(iniPath);
        registerCommonSettings(loaded);
        QCOMPARE(loaded.get("name").toString(), QString("Test Instance"));
        QCOMPARE(loaded.get("iconKey").toString(), QString("grass"));
        QCOMPARE(loaded.get("version").toString(), QString("1.20.1"));
    }

    void test_inifilePersistence()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString iniPath = tempDir.path() + "/instance.cfg";

        {
            INISettingsObject settings(iniPath);
            registerCommonSettings(settings);
            settings.set("name", "Persistent Instance");
            settings.set("iconKey", "diamond");
            settings.set("version", "1.19.4");
            settings.set("notes", "This is a test note");
            settings.set("lastLaunch", QDateTime::currentDateTime());
        }

        INISettingsObject loaded(iniPath);
        registerCommonSettings(loaded);
        QCOMPARE(loaded.get("name").toString(), QString("Persistent Instance"));
        QCOMPARE(loaded.get("iconKey").toString(), QString("diamond"));
        QCOMPARE(loaded.get("version").toString(), QString("1.19.4"));
        QCOMPARE(loaded.get("notes").toString(), QString("This is a test note"));
    }

    void test_inifileMultipleValues()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString iniPath = tempDir.path() + "/instance.cfg";
        INISettingsObject settings(iniPath);
        registerCommonSettings(settings);

        settings.set("name", "Multi Value");
        settings.set("iconKey", "forge");
        settings.set("version", "1.20.1");
        settings.set("loader", "net.minecraftforge");
        settings.set("loaderVersion", "47.2.0");
        settings.set("javaPath", "/usr/lib/jvm/java-17");
        settings.set("minMemory", 2048);
        settings.set("maxMemory", 4096);

        INISettingsObject loaded(iniPath);
        registerCommonSettings(loaded);
        QCOMPARE(loaded.get("name").toString(), QString("Multi Value"));
        QCOMPARE(loaded.get("iconKey").toString(), QString("forge"));
        QCOMPARE(loaded.get("version").toString(), QString("1.20.1"));
        QCOMPARE(loaded.get("loader").toString(), QString("net.minecraftforge"));
        QCOMPARE(loaded.get("loaderVersion").toString(), QString("47.2.0"));
        QCOMPARE(loaded.get("javaPath").toString(), QString("/usr/lib/jvm/java-17"));
        QCOMPARE(loaded.get("minMemory").toInt(), 2048);
        QCOMPARE(loaded.get("maxMemory").toInt(), 4096);
    }

    void test_inifileOverwrite()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString iniPath = tempDir.path() + "/instance.cfg";
        INISettingsObject settings(iniPath);
        registerCommonSettings(settings);

        settings.set("name", "First Value");
        settings.set("name", "Second Value");

        INISettingsObject loaded(iniPath);
        registerCommonSettings(loaded);
        QCOMPARE(loaded.get("name").toString(), QString("Second Value"));
    }

    void test_inifileDelete()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString iniPath = tempDir.path() + "/instance.cfg";
        INISettingsObject settings(iniPath);
        registerCommonSettings(settings);

        settings.set("name", "Delete Test");
        INISettingsObject loaded(iniPath);
        registerCommonSettings(loaded);
        QCOMPARE(loaded.get("name").toString(), QString("Delete Test"));
    }

    void test_globalSettingsSetup()
    {
        auto settings = createGlobalSettings();

        QVERIFY(settings->contains("ShowGameTime"));
        QVERIFY(settings->contains("RecordGameTime"));
        QVERIFY(settings->contains("PreLaunchCommand"));
        QVERIFY(settings->contains("JavaPath"));
        QVERIFY(settings->contains("JvmArgs"));
        QVERIFY(settings->contains("MinMemAlloc"));
        QVERIFY(settings->contains("MaxMemAlloc"));
        QVERIFY(settings->contains("ShowConsole"));
        QVERIFY(settings->contains("AutoCloseConsole"));
        QVERIFY(settings->contains("LaunchMaximized"));
        QVERIFY(settings->contains("MinecraftWinWidth"));
        QVERIFY(settings->contains("MinecraftWinHeight"));
        QVERIFY(settings->contains("UseNativeOpenAL"));
        QVERIFY(settings->contains("UseNativeGLFW"));
        QVERIFY(settings->contains("EnableFeralGamemode"));
        QVERIFY(settings->contains("CloseAfterLaunch"));
        QVERIFY(settings->contains("QuitAfterGameStop"));
    }

    void test_versionCreation()
    {
        auto version = std::make_shared<TestVersion>("1.20.1", "1.20.1");
        QCOMPARE(version->name(), QString("1.20.1"));
        QCOMPARE(version->descriptor(), QString("1.20.1"));
        QCOMPARE(version->typeString(), QString("Release"));
    }

    void test_versionComparison()
    {
        auto v1 = std::make_shared<TestVersion>("1.17.1", "1.17.1");
        auto v2 = std::make_shared<TestVersion>("1.20.1", "1.20.1");

        QVERIFY(*v1 < *v2);
        QVERIFY(*v2 > *v1);
    }

    void test_multipleVersions()
    {
        QList<BaseVersion::Ptr> versions;
        versions.append(std::make_shared<TestVersion>("1.17.1", "1.17.1"));
        versions.append(std::make_shared<TestVersion>("1.18.2", "1.18.2"));
        versions.append(std::make_shared<TestVersion>("1.19.4", "1.19.4"));
        versions.append(std::make_shared<TestVersion>("1.20.1", "1.20.1"));

        QCOMPARE(versions.size(), 4);
        QCOMPARE(versions[0]->name(), QString("1.17.1"));
        QCOMPARE(versions[3]->name(), QString("1.20.1"));
    }

    void test_directoryCreation()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QDir dir(tempDir.path());
        QVERIFY(dir.mkdir("instances"));
        QVERIFY(dir.cd("instances"));
        QVERIFY(dir.mkdir("TestInstance"));
        QVERIFY(dir.cd("TestInstance"));
        QVERIFY(dir.exists());
    }
};

QTEST_GUILESS_MAIN(InstanceCreationTest)

#include "InstanceCreation_test.moc"
