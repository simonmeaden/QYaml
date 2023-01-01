#pragma once

#include <QObject>

#include "qyaml_global.h"

class QYAML_SHARED_EXPORT QYamlBuilder : public QObject
{
    Q_OBJECT
public:
    explicit QYamlBuilder(QObject *parent = nullptr);

signals:

};

