#pragma once

#include "IAnalyzerPlugin.h"

#include <QObject>
#include <QPluginLoader>
#include <QStringList>

#include <memory>
#include <vector>

class PluginManager {
public:
    void loadFrom(const QString& directory);
    QStringList runAll(const QString& filePath) const;
    QStringList names() const;

private:
    std::vector<std::unique_ptr<QPluginLoader>> loaders_;
    QList<IAnalyzerPlugin*> plugins_;
};