// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher: Minecraft Launcher
 *  Copyright (C) 2023 flowln <flowlnlnln@gmail.com>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (c) 2026 Trial97 <alexandru.tripon97@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "Version.h"

#include <QDebug>
#include <QRegularExpressionMatch>
#include <QUrl>
#include <compare>

QDebug operator<<(QDebug debug, const Version& v)
{
    const QDebugStateSaver saver(debug);

    debug.nospace() << "Version{ string: " << v.toString() << ", sections: [ ";

    bool first = true;
    for (const auto& s : v.m_sections) {
        if (!first) {
            debug.nospace() << ", ";
        }
        debug.nospace() << s.value;
        first = false;
    }

    debug.nospace() << " ]" << " }";

    return debug;
}

std::strong_ordering Version::Section::operator<=>(const Section& other) const
{
    if (this->t == Type::Numeric && other.t == Type::Numeric) {
        auto aLen = this->value.size();
        if (aLen != other.value.size()) {
            return aLen <=> other.value.size();
        }

        auto cmp = QString::compare(this->value, other.value);
        if (cmp < 0) {
            return std::strong_ordering::less;
        }
        if (cmp > 0) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    if (this->t == Type::Null) {
        if (other.t == Type::PreRelease) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::less;
    }
    if (other.t == Type::Null) {
        if (this->t == Type::PreRelease) {
            return std::strong_ordering::less;
        }
        return std::strong_ordering::greater;
    }

    auto minLen = qMin(this->value.size(), other.value.size());
    for (int i = 0; i < minLen; i++) {
        auto a = this->value.at(i);
        auto b = other.value.at(i);
        if (a != b) {
            return a.unicode() <=> b.unicode();
        }
    }

    return this->value.size() <=> other.value.size();
}

namespace
{
void removeLeadingZeros(QString& s)
{
    s.remove(0, std::distance(s.begin(), std::ranges::find_if_not(s, [](QChar c) { return c == '0'; })));
}
}  // namespace

void Version::parse()
{
    auto len = m_string.size();
    for (int i = 0; i < len;) {
        Section cur(Section::Type::Textual);
        auto    c = m_string.at(i);
        if (c == '+') {
            break;
        }

        if (c == '-' || c == ' ') {
            cur.value += c;
            i++;

            if (i < len && !m_string.at(i).isDigit()) {
                cur.t = Section::Type::PreRelease;
            }
        } else if (c.isDigit()) {
            cur.t = Section::Type::Numeric;
        }
        for (; i < len; i++) {
            auto r = m_string.at(i);
            if ((r.isDigit() != (cur.t == Section::Type::Numeric))

                || (r == ' ' && cur.t == Section::Type::Numeric)

                || (r == '-' && cur.t != Section::Type::PreRelease)

                || r == '+') {
                break;
            }

            cur.value += r;
        }
        if (!cur.value.isEmpty()) {
            if (cur.t == Section::Type::Numeric) {
                removeLeadingZeros(cur.value);
            }
            m_sections.append(cur);
        }
    }
}

std::strong_ordering Version::operator<=>(const Version& other) const
{
    const auto size = qMax(m_sections.size(), other.m_sections.size());
    for (int i = 0; i < size; ++i) {
        auto sec1 = (i >= m_sections.size()) ? Section() : m_sections.at(i);
        auto sec2 = (i >= other.m_sections.size()) ? Section() : other.m_sections.at(i);

        if (auto cmp = sec1 <=> sec2; cmp != std::strong_ordering::equal) {
            return cmp;
        }
    }
    return std::strong_ordering::equal;
}
