#pragma once

#include <QString>
#include <compare>

struct MessageLevel
{
    enum class Enum
    {
        Unknown,
        StdOut,
        StdErr,
        Launcher,
        Trace,
        Debug,
        Info,
        Message,
        Warning,
        Error,
        Fatal,
    };
    using enum Enum;
    constexpr MessageLevel(Enum e = Unknown) : m_type(e) {}
    static MessageLevel  fromName(const QString& type);
    static MessageLevel  fromQtMsgType(const QtMsgType& type);
    static MessageLevel  fromLine(const QString& line);
    inline bool          isValid() const { return m_type != Unknown; }
    std::strong_ordering operator<=>(const MessageLevel& other) const = default;
    std::strong_ordering operator<=>(const MessageLevel::Enum& other) const { return m_type <=> other; }
    explicit             operator int() const { return static_cast<int>(m_type); }
    explicit             operator MessageLevel::Enum() { return m_type; }

    static MessageLevel takeFromLine(QString& line);

    static MessageLevel takeFromLauncherLine(QString& line);

private:
    Enum m_type;
};
