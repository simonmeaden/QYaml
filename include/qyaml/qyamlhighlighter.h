#pragma once

#include <QColor>
#include <QSyntaxHighlighter>

#include "qyaml/yamlnode.h"
#include "qyaml_global.h"

class QYamlParser;
class QYamlEdit;
class YamlNode;
class YamlMapItem;

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

  const QColor& mapColor() const;
  void setMapColor(const QColor& mapColor);

  QColor tagColor() const;
  void setTagColor(const QColor& tagColor);

  QColor reservedColor() const;
  void setReservedColor(const QColor &reservedColor);

  QColor directiveColor() const;
  void setDirectiveColor(const QColor& directiveColor);

  QColor docStartColor() const;
  void setDocStartColor(const QColor& docStartColor);

  QColor docEndColor() const;
  void setDocEndColor(const QColor& docEndColor);

  QColor errorColor() const;
  void setErrorColor(const QColor& errorColor);

  QColor warningColor() const;
  void setWarningColor(const QColor& warningColor);

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
  QColor m_directiveColor;
  QColor m_tagColor;
  QColor m_reservedColor;
  QColor m_commentColor;
  QColor m_docStartColor;
  QColor m_docEndColor;
  QColor m_errorColor;
  QColor m_warningColor;

  QTextCharFormat m_textFormat;
  QTextCharFormat m_mapFormat;
  QTextCharFormat m_mapKeyFormat;
  QTextCharFormat m_mapValueFormat;
  QTextCharFormat m_seqFormat;
  QTextCharFormat m_seqValueFormat;
  QTextCharFormat m_commentFormat;
  QTextCharFormat m_scalarFormat;
  QTextCharFormat m_directiveFormat;
  QTextCharFormat m_tagFormat;
  QTextCharFormat m_reservedFormat;
  QTextCharFormat m_docStartFormat;
  QTextCharFormat m_docEndFormat;
  QTextCharFormat m_warningFormat;

  bool isFormatable(int nodeStart,
                    int nodeLength,
                    int blockStart,
                    int textLength,
                    FormatSize& result);
  void setScalarFormat(SharedNode node, int blockStart, int textLength);
  void setKeyFormat(QSharedPointer<YamlMapItem> node,
                    int blockStart,
                    int nodeLength);
  void setCommentFormat(SharedNode node, int blockStart, int textLength);
  void setDirectiveFormat(SharedNode node, int blockStart, int textLength);
  void setTagFormat(SharedNode node, int blockStart, int textLength);
  void setReservedFormat(SharedNode node, int blockStart, int textLength);
  void setMapFormat(SharedNode node, int blockStart, int textLength);
  void setMapItemFormat(QSharedPointer<YamlMapItem> node,
                        int blockStart,
                        int textLength);
  void setSequenceFormat(SharedNode node, int blockStart, int textLength);
  void setStartTagFormat(SharedNode node, int blockStart, int textLength);
  void setEndTagFormat(SharedNode node, int blockStart, int textLength);
};
