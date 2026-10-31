#include "PluginManager.h"

#include <QDir>
#include <QFileInfo>

void PluginManager::loadFrom(const QString& directory)
{
    QDir dir(directory);

    if (!dir.exists())
        return;

    const QStringList files =
        dir.entryList(QDir::Files);

    for (const QString& fileName : files) {
        const QString path = dir.absoluteFilePath(fileName);

        auto loader =
            std::make_unique<QPluginLoader>(path);

        QObject* instance = loader->instance();

        if (!instance)
            continue;

        auto* plugin =
            qobject_cast<IAnalyzerPlugin*>(instance);

        if (plugin) {
            plugins_.push_back(plugin);
            loaders_.push_back(std::move(loader));
        }
        else {
            loader->unload();
        }
    }
}

QStringList PluginManager::runAll(
    const QString& filePath) const
{
    QStringList result;

    for (auto* plugin : plugins_) {
        if (!plugin)
            continue;

        result << QString("[%1]\n%2")
            .arg(plugin->name(), plugin->analyze(filePath));
    }

    return result;
}

QStringList PluginManager::names() const
{
    QStringList result;

    for (auto* plugin : plugins_) {
        if (plugin)
            result << plugin->name();
    }

    return result;
}