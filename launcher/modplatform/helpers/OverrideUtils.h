#pragma once

#include <QString>

namespace Override {

void createOverrides(const QString& name, const QString& parent_folder, const QString& override_path);

QStringList readOverrides(const QString& name, const QString& parent_folder);

}  // namespace Override
