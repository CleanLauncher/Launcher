#pragma once

#include "settings/SettingsObject.h"
#include <QString>

class MCEditTool
{
public:
    MCEditTool(SettingsObject* settings);
    void    setPath(QString& path);
    QString path() const;
    bool    check(const QString& toolPath, QString& error);
    QString getProgramPath();

private:
    SettingsObject* m_settings;
};
