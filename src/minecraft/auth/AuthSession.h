#pragma once

#include <QString>
#include <memory>

#include "LaunchMode.h"

class MinecraftAccount;

struct AuthSession
{
    bool MakeOffline(QString offline_playername);
    void MakeDemo(QString name, QString uuid);

    QString serializeUserProperties();

    QString session;

    QString access_token;

    QString player_name;

    QString uuid;

    QString user_type;

    LaunchMode launchMode;

    bool wantsElyPatch = false;
};

using AuthSessionPtr = std::shared_ptr<AuthSession>;
