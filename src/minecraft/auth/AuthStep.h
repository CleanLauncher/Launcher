#pragma once
#include <QList>
#include <QObject>

#include "QObjectPtr.h"
#include "minecraft/auth/AccountData.h"

enum class AccountTaskState
{
    STATE_CREATED,
    STATE_WORKING,
    STATE_SUCCEEDED,
    STATE_DISABLED,

    STATE_FAILED_SOFT,

    STATE_FAILED_HARD,

    STATE_FAILED_GONE,

    STATE_OFFLINE

};

class AuthStep : public QObject
{
    Q_OBJECT

public:
    using Ptr = shared_qobject_ptr<AuthStep>;

    explicit AuthStep(AccountData* data) : QObject(nullptr), m_data(data) {};
    virtual ~AuthStep() noexcept = default;

    virtual QString describe() = 0;

public slots:
    virtual void perform() = 0;
    virtual void abort() {}

signals:
    void finished(AccountTaskState resultingState, QString message);

protected:
    AccountData* m_data;
};
