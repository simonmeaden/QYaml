#pragma once

#include <QSyntaxHighlighter>

#include "qyaml_global.h"

class QYAML_SHARED_EXPORT QYamlHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT
public:
  explicit QYamlHighlighter(QObject *parent = nullptr);

protected:
  // QSyntaxHighlighter interface
  void highlightBlock(const QString &text);
};

