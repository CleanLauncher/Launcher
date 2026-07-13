#pragma once

#include <QFuture>
#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>

class McClient : public QObject {
    Q_OBJECT

   public:
    explicit McClient(QObject* parent, QString domain, QString ip, uint16_t port);

    void getStatusData();

   signals:
    void succeeded(QJsonObject data);
    void failed(QString error);
    void finished();

   private:
    static uint8_t readByte(QByteArray& data);
    static int readVarInt(QByteArray& data);
    static void writeUInt16(QByteArray& data, uint16_t value);
    static void writeString(QByteArray& data, const QString& value);
    static void writeVarInt(QByteArray& data, int value);

   private:
    void sendRequest();

    void readRawResponse();
    void parseResponse();
    void writePacketToSocket(QByteArray& data);

    void emitFail(const QString& error);
    void emitSucceed(QJsonObject data);

   private:
    enum class ResponseReadState : uint8_t {
        Waiting,
        GotLength,
        Finished
    };

    QString m_domain;
    QString m_ip;
    uint16_t m_port;
    QTcpSocket m_socket;

    ResponseReadState m_responseReadState = ResponseReadState::Waiting;
    int32_t m_wantedRespLength = 0;
    QByteArray m_resp;
};
