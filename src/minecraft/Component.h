#pragma once

#include "ProblemProvider.h"
#include "QObjectPtr.h"
#include "meta/JsonFormat.h"
#include "modplatform/ModIndex.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QList>
#include <memory>
#include <optional>
#include <variant>

class PackProfile;
class LaunchProfile;
namespace Meta
{
class Version;
class VersionList;
}  // namespace Meta

class VersionFile;

struct UpdateActionChangeVersion
{
    QString targetVersion;
};
struct UpdateActionLatestRecommendedCompatible
{
    QString parentUid;
    QString parentName;

    QString version;
};
struct UpdateActionRemove
{};
struct UpdateActionImportantChanged
{
    QString oldVersion;
};

using UpdateActionNone = std::monostate;

using UpdateAction = std::variant<UpdateActionNone,
                                  UpdateActionChangeVersion,
                                  UpdateActionLatestRecommendedCompatible,
                                  UpdateActionRemove,
                                  UpdateActionImportantChanged>;

struct ModloaderMapEntry
{
    ModPlatform::ModLoaderType type;
    QStringList                knownConflictingComponents;
};

class Component : public QObject, public ProblemProvider
{
    Q_OBJECT
public:
    Component(PackProfile* parent, const QString& uid);

    Component(PackProfile* parent, const QString& uid, std::shared_ptr<VersionFile> file);

    virtual ~Component() {}

    static const QMap<QString, ModloaderMapEntry> KNOWN_MODLOADERS;

    void applyTo(LaunchProfile* profile);

    bool isEnabled();
    bool setEnabled(bool state);
    bool canBeDisabled();

    bool        isMoveable();
    bool        isCustomizable();
    bool        isRevertible();
    bool        isRemovable();
    bool        isCustom();
    bool        isVersionChangeable(bool wait = true);
    bool        isKnownModloader();
    QStringList knownConflictingComponents();

    void setOrder(int order);
    int  getOrder();

    QString                        getID();
    QString                        getName();
    QString                        getVersion();
    std::shared_ptr<Meta::Version> getMeta();
    QDateTime                      getReleaseDateTime();

    QString getFilename();

    std::shared_ptr<class VersionFile>       getVersionFile() const;
    std::shared_ptr<class Meta::VersionList> getVersionList() const;

    void setImportant(bool state);

    const QList<PatchProblem> getProblems() const override;
    ProblemSeverity           getProblemSeverity() const override;
    void                      addComponentProblem(ProblemSeverity severity, const QString& description);
    void                      resetComponentProblems();

    void setVersion(const QString& version);
    bool customize();
    bool revert();

    void updateCachedData();

    void waitLoadMeta();

    void         setUpdateAction(const UpdateAction& action);
    void         clearUpdateAction();
    UpdateAction getUpdateAction();

signals:
    void dataChanged();

public:
    PackProfile* m_parent;

    QString m_uid;

    QString m_version;

    bool m_dependencyOnly = false;

    bool m_important = false;

    bool m_disabled = false;

    QString m_cachedName;

    QString m_cachedVersion;

    Meta::RequireSet m_cachedRequires;
    Meta::RequireSet m_cachedConflicts;

    bool m_cachedVolatile = false;

    bool m_orderOverride = false;
    int  m_order         = 0;

    std::shared_ptr<Meta::Version> m_metaVersion;
    std::shared_ptr<VersionFile>   m_file;
    bool                           m_loaded = false;

private:
    QList<PatchProblem> m_componentProblems;
    ProblemSeverity     m_componentProblemSeverity = ProblemSeverity::None;
    UpdateAction        m_updateAction             = UpdateAction{UpdateActionNone{}};
};

using ComponentPtr = shared_qobject_ptr<Component>;
