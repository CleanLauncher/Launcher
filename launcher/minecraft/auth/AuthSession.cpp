#include "AuthSession.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

QString AuthSession::serializeUserProperties()
{
    QJsonObject userAttrs;

    QJsonDocument value(userAttrs);
    return value.toJson(QJsonDocument::Compact);
}

bool AuthSession::MakeOffline(QString offline_playername)
{
    session = "-";
    access_token = "0";
    player_name = offline_playername;
    return true;
}

void AuthSession::MakeDemo(QString name, QString u)
{
    uuid = u;
    session = "-";
    access_token = "0";
    player_name = name;
    launchMode = LaunchMode::Demo;
};
