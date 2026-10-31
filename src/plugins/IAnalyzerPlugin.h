#pragma once
#include <QtPlugin>
#include <QString>

class IAnalyzerPlugin {
public:
    virtual ~IAnalyzerPlugin() = default;
    virtual QString name() const = 0;
    virtual QString analyze(const QString& filePath) = 0;
};

#define BinaryInspectorPlugin_iid "org.binaryinspector.IAnalyzerPlugin/1.0"
Q_DECLARE_INTERFACE(IAnalyzerPlugin, BinaryInspectorPlugin_iid)
