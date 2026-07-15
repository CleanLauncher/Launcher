// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2024 Kenneth Chew <79120643+kthchew@users.noreply.github.com>
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

#ifndef FILEACCESS_H
#define FILEACCESS_H

#include <QtCore/QMap>
#include <QtCore/QSet>
Q_FORWARD_DECLARE_OBJC_CLASS(NSData);
Q_FORWARD_DECLARE_OBJC_CLASS(NSURL);
Q_FORWARD_DECLARE_OBJC_CLASS(NSString);
Q_FORWARD_DECLARE_OBJC_CLASS(NSAutoreleasePool);
Q_FORWARD_DECLARE_OBJC_CLASS(NSMutableDictionary);
Q_FORWARD_DECLARE_OBJC_CLASS(NSMutableSet);
class QString;
class QByteArray;
class QUrl;

class SecurityBookmarkFileAccess {
    NSMutableDictionary* m_bookmarks;

    NSMutableDictionary* m_paths;

    NSMutableSet* m_activeURLs;

    bool m_readOnly;

    NSURL* securityScopedBookmarkToNSURL(QByteArray& bookmark, bool& isStale);

   public:
    SecurityBookmarkFileAccess(bool readOnly = false);
    ~SecurityBookmarkFileAccess();

    QByteArray urlToSecurityScopedBookmark(const QUrl& url);

    QByteArray pathToSecurityScopedBookmark(const QString& path);

    QUrl securityScopedBookmarkToURL(QByteArray& bookmark, bool& isStale);

    bool startUsingSecurityScopedBookmark(QByteArray& bookmark, bool& isStale);
    void stopUsingSecurityScopedBookmark(QByteArray& bookmark);

    bool isAccessingPath(const QString& path);
};

#endif
