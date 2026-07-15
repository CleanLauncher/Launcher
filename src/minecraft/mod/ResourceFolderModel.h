#pragma once

#include <QAbstractListModel>
#include <QAction>
#include <QDir>
#include <QFileSystemWatcher>
#include <QHeaderView>
#include <QMutex>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include "Resource.h"

#include "BaseInstance.h"

#include "tasks/ConcurrentTask.h"
#include "tasks/Task.h"

class QSortFilterProxyModel;

#define RESOURCE_HELPERS(T)                                         \
    T& at(int index)                                                \
    {                                                               \
        return *static_cast<T*>(m_resources[index].get());          \
    }                                                               \
    const T& at(int index) const                                    \
    {                                                               \
        return *static_cast<const T*>(m_resources.at(index).get()); \
    }                                                               \
    QList<T*> selected##T##s(const QModelIndexList& indexes)        \
    {                                                               \
        QList<T*> result;                                           \
        for (const QModelIndex& index : indexes) {                  \
            if (index.column() != 0)                                \
                continue;                                           \
                                                                    \
            result.append(&at(index.row()));                        \
        }                                                           \
        return result;                                              \
    }                                                               \
    QList<T*> all##T##s()                                           \
    {                                                               \
        QList<T*> result;                                           \
        result.reserve(m_resources.size());                         \
                                                                    \
        for (const Resource::Ptr& resource : m_resources)           \
            result.append(static_cast<T*>(resource.get()));         \
                                                                    \
        return result;                                              \
    }

class ResourceFolderModel : public QAbstractListModel {
    Q_OBJECT
   public:
    ResourceFolderModel(const QDir& dir, BaseInstance* instance, bool isIndexed, bool createDir, QObject* parent = nullptr);
    ~ResourceFolderModel() override;

    virtual QString id() const { return "resource"; }

    bool startWatching(const QStringList& paths);

    bool stopWatching(const QStringList& paths);

    virtual bool startWatching() { return startWatching({ indexDir().absolutePath(), m_dir.absolutePath() }); }
    virtual bool stopWatching() { return stopWatching({ indexDir().absolutePath(), m_dir.absolutePath() }); }

    virtual QDir indexDir() const { return { QString("%1/.index").arg(dir().absolutePath()) }; }

    virtual bool installResource(QString path);

    virtual void installResourceWithFlameMetadata(const QString& path, ModPlatform::IndexedVersion& vers);

    virtual bool uninstallResource(const QString& fileName, bool preserveMetadata = false);
    virtual bool deleteResources(const QModelIndexList&);
    virtual void deleteMetadata(const QModelIndexList&);

    virtual bool setResourceEnabled(const QModelIndexList& indexes, EnableAction action);

    virtual bool update();

    virtual void resolveResource(Resource::Ptr res);

    qsizetype size() const { return m_resources.size(); }
    [[nodiscard]] bool empty() const { return size() == 0; }

    Resource& at(int index) { return *m_resources[index].get(); }
    const Resource& at(int index) const { return *m_resources.at(index).get(); }
    QList<Resource*> selectedResources(const QModelIndexList& indexes);
    QList<Resource*> allResources();

    Resource::Ptr find(QString id);

    const QDir& dir() const { return m_dir; }

    bool hasPendingParseTasks() const;

    enum Columns : std::uint8_t { ActiveColumn = 0, NameColumn, DateColumn, ProviderColumn, SizeColumn, FileNameColumn, NumColumns };

    QStringList columnNames(bool translated = true) const { return translated ? m_columnNamesTranslated : m_columnNames; }

    int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : static_cast<int>(size()); }
    int columnCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : NumColumns; }

    Qt::DropActions supportedDropActions() const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QStringList mimeTypes() const override;
    [[nodiscard]] bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    [[nodiscard]] bool validateIndex(const QModelIndex& index) const;

    QBrush rowBackground(int row) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setupHeaderAction(QAction* act, int column);
    void saveColumns(QTreeView* tree);
    void loadColumns(QTreeView* tree);
    QMenu* createHeaderContextMenu(QTreeView* tree);

    QSortFilterProxyModel* createFilterProxyModel(QObject* parent = nullptr);

    SortType columnToSortKey(size_t column) const;
    QList<QHeaderView::ResizeMode> columnResizeModes() const { return m_columnResizeModes; }

    class ProxyModel : public QSortFilterProxyModel {
       public:
        explicit ProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

       protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const override;
    };

    QString instDirPath() const;
    BaseInstance* instance() const { return m_instance; }

   signals:
    void updateFinished();
    void parseFinished();

   protected:
    [[nodiscard]] virtual Task* createPreUpdateTask() { return nullptr; }

    [[nodiscard]] Task* createUpdateTask();

    [[nodiscard]] virtual Resource* createResource(const QFileInfo& info) { return new Resource(info); }

    [[nodiscard]] virtual Task* createParseTask(Resource&) { return nullptr; }

    void applyUpdates(QSet<QString>& currentSet, QSet<QString>& newSet, QMap<QString, Resource::Ptr>& newResources);

   protected slots:
    void directoryChanged(const QString&);

    virtual void onUpdateSucceeded();
    virtual void onUpdateFailed() {}

    virtual void onParseSucceeded(int ticket, const QString& resourceId);
    virtual void onParseFailed(int ticket, const QString& resourceId);

   protected:
    QList<SortType> m_columnSortKeys = { SortType::Enabled,  SortType::Name, SortType::Date,
                                         SortType::Provider, SortType::Size, SortType::Filename };
    QStringList m_columnNames = { "Enable", "Name", "Last Modified", "Provider", "Size", "File Name" };
    QStringList m_columnNamesTranslated = { tr("Enable"), tr("Name"), tr("Last Modified"), tr("Provider"), tr("Size"), tr("File Name") };
    QList<QHeaderView::ResizeMode> m_columnResizeModes = { QHeaderView::Interactive, QHeaderView::Stretch,     QHeaderView::Interactive,
                                                           QHeaderView::Interactive, QHeaderView::Interactive, QHeaderView::Interactive };
    QList<bool> m_columnsHideable = { false, false, true, true, true, true };

    QDir m_dir;
    BaseInstance* m_instance;
    QFileSystemWatcher m_watcher;
    bool m_isWatching = false;

    bool m_isIndexed;
    bool m_firstFolderLoad = true;

    Task::Ptr m_currentUpdateTask = nullptr;
    bool m_scheduledUpdate = false;

    QList<Resource::Ptr> m_resources;

    QMap<QString, int> m_resourcesIndex;

    ConcurrentTask m_resourceResolver;
    bool m_resourceResolverRunning = false;

    QMap<int, Task::Ptr> m_activeParseTasks;
    std::atomic<int> m_nextResolutionTicket = 0;
};
