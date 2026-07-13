#pragma once

#include <QList>
#include <QMap>
#include <QTimer>
#include "Component.h"
#include "tasks/Task.h"

class MinecraftInstance;
using ComponentContainer = QList<ComponentPtr>;
using ComponentIndex = QMap<QString, ComponentPtr>;

struct PackProfileData {

    MinecraftInstance* m_instance;

    std::shared_ptr<LaunchProfile> m_profile;

    ComponentContainer components;
    ComponentIndex componentIndex;
    bool dirty = false;
    QTimer m_saveTimer;
    shared_qobject_ptr<ComponentUpdateTask> m_updateTask;
    bool loaded = false;
    bool interactionDisabled = true;
};
