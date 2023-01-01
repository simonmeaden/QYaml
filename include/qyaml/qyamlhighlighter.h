#pragma once

#include <QColor>
#include <QSyntaxHighlighter>

#include "qyaml_global.h"

class QYamlParser;
class QYamlEdit;
class YamlNode;

class QYAML_SHARED_EXPORT QYamlHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

  struct FormatSize
  {
    int start = -1;
    int length = 0;
  };

public:
  explicit QYamlHighlighter(QYamlParser* parser, QYamlEdit* parent);

  const QColor& backgroundColor() const;
  void setBackgroundColor(const QColor& backgroundColor);

  const QColor& textColor() const;
  void setTextColor(const QColor& textColor);

  const QColor& mapKeyColor() const;
  void setMapKeyColor(const QColor& mapKeyColor);

  const QColor& mapValueColor() const;
  void setMapValueColor(const QColor& mapValueColor);

  const QColor& seqColor() const;
  void setSeqColor(const QColor& seqColor);

  const QColor& seqValueColor() const;
  void setSeqValueColor(const QColor& seqValueColor);

  const QColor& commentColor() const;
  void setCommentColor(const QColor& commentColor);

  const QColor &mapColor() const;
  void setMapColor(const QColor &mapColor);

  protected:
private:
  // QSyntaxHighlighter interface
  void highlightBlock(const QString& text);
  QYamlParser* m_parser;

  QColor m_backgroundColor;
  QColor m_textColor;
  QColor m_mapColor;
  QColor m_mapKeyColor;
  QColor m_mapValueColor;
  QColor m_seqColor;
  QColor m_seqValueColor;
  QColor m_scalarColor;
  QColor m_commentColor;

  QTextCharFormat m_textFormat;
  QTextCharFormat m_mapFormat;
  QTextCharFormat m_mapKeyFormat;
  QTextCharFormat m_mapValueFormat;
  QTextCharFormat m_seqFormat;
  QTextCharFormat m_seqValueFormat;
  QTextCharFormat m_commentFormat;
  QTextCharFormat m_scalarFormat;

  bool isFormatable(int nodeStart,
                    int nodeLength,
                    int blockStart,
                    int textLength,
                    FormatSize& result);
  void setScalarFormat(YamlNode *node, int blockStart, int textLength);
  void setCommentFormat(YamlNode *node, int blockStart, int textLength);
  void setMapFormat(YamlNode *node, int blockStart, int textLength);
  void setSequenceFormat(YamlNode *node, int blockStart, int textLength);
};
