#pragma once

#include "BaseVersion.h"
#include "InstanceTask.h"
#include "minecraft/MinecraftInstance.h"

class InstanceCreationTask : public InstanceTask {
    Q_OBJECT
   public:
    InstanceCreationTask() = default;
    virtual ~InstanceCreationTask() = default;

    bool abort() override;

   protected:
    void executeTask() final override;

    virtual bool updateInstance() { return false; };

    virtual std::unique_ptr<MinecraftInstance> createInstance() { return nullptr; }

    QString getError() const { return m_error_message; }

   protected:
    void setError(const QString& message) { m_error_message = message; };
    void scheduleToDelete(QWidget* parent, const QDir& dir, const QString& path, bool checkDisabled = false);

   protected:
    bool m_abort = false;

    QStringList m_filesToRemove;
    ShouldDeleteSaves m_shouldDeleteSaves;

   private:
    QString m_error_message;
    std::unique_ptr<MinecraftInstance> m_instance;
    Task::Ptr m_gameFilesTask;
};
