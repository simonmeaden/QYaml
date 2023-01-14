#include "qyaml/qyamlhighlighter.h"
#include "qyaml/qyamldocument.h"
#include "qyaml/qyamledit.h"
#include "qyaml/qyamlparser.h"
#include "qyaml/yamlnode.h"
#include "utilities/x11colors.h"

QYamlHighlighter::QYamlHighlighter(QYamlParser* parser, QYamlEdit* parent)
  : QSyntaxHighlighter{ parent->document() }
  , m_parser(parser)
  , m_textColor(QColorConstants::Black)
  , m_backgroundColor(QColorConstants::White)
  , m_mapColor(QColorConstants::X11::LightBlue)
  , m_mapKeyColor(QColorConstants::X11::chocolate)
  , m_mapValueColor(QColorConstants::X11::DarkBlue)
  , m_seqValueColor(QColorConstants::X11::OrangeRed)
  , m_seqColor(QColorConstants::X11::DarkOrange)
  , m_commentColor(QColorConstants::X11::ForestGreen)
  , m_scalarColor(QColorConstants::X11::MediumSeaGreen)
  , m_directiveColor(QColorConstants::X11::peru)
  , m_tagColor(QColorConstants::X11::purple)
  , m_docStartColor(QColorConstants::X11::MediumBlue)
  , m_docEndColor(QColorConstants::X11::MediumBlue)
  , m_errorColor(QColorConstants::X11::red)
{
  m_textFormat.setBackground(m_backgroundColor);
  m_textFormat.setForeground(m_textColor);
  m_mapFormat.setBackground(m_backgroundColor);
  m_mapFormat.setForeground(m_mapColor);
  m_mapKeyFormat.setBackground(m_backgroundColor);
  m_mapKeyFormat.setForeground(m_mapKeyColor);
  m_mapValueFormat.setBackground(m_backgroundColor);
  m_mapValueFormat.setForeground(m_mapValueColor);
  m_seqFormat.setBackground(m_backgroundColor);
  m_seqFormat.setForeground(m_seqColor);
  m_seqValueFormat.setBackground(m_backgroundColor);
  m_seqValueFormat.setForeground(m_seqValueColor);
  m_commentFormat.setBackground(m_backgroundColor);
  m_commentFormat.setForeground(m_commentColor);
  m_scalarFormat.setBackground(m_backgroundColor);
  m_scalarFormat.setForeground(m_scalarColor);
  m_tagFormat.setBackground(m_backgroundColor);
  m_tagFormat.setForeground(m_tagColor);
  m_directiveFormat.setBackground(m_backgroundColor);
  m_directiveFormat.setForeground(m_directiveColor);
  m_docStartFormat.setBackground(m_backgroundColor);
  m_docStartFormat.setForeground(m_docStartColor);
  m_docEndFormat.setBackground(m_backgroundColor);
  m_docEndFormat.setForeground(m_docEndColor);
}

void
QYamlHighlighter::highlightBlock(const QString& text)
{
  //  auto doc = m_parser->currentDoc();
  for (auto doc : m_parser->documents()) {
    if (!doc)
      return;

    auto block = currentBlock();
    auto blockStart = block.position();
    auto textLength = text.length();
    auto docStart = doc->startPos();
    auto docEnd = doc->endPos();

    // doc is completely outside block
    if (docEnd < blockStart || docStart >= blockStart + textLength ||
        textLength == 0)
      continue;

    auto nodes = doc->nodes();
    auto size = nodes.size();
    if (nodes.isEmpty())
      return;

    for (auto node : nodes) {
      auto nodeStart = node->startPos();
      auto nodeEnd = node->endPos();

      // node is completely outside block
      if (nodeEnd < blockStart || nodeStart > blockStart + textLength)
        continue;

      // adjust start and end within text.
      nodeStart -= blockStart;
      nodeEnd -= blockStart;
      nodeStart = nodeStart < 0 ? 0 : nodeStart;
      nodeEnd = nodeEnd > textLength ? textLength : nodeEnd;

      switch (node->type()) {
        case YamlNode::Scalar: {
          setScalarFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::Map: {
          setMapFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::MapItem: {
          auto n = qobject_cast<YamlMapItem*>(node);
          auto type = n->data()->type();
          if (n) {
            setMapItemFormat(n, blockStart, text.length());
          }
          break;
        }
        case YamlNode::Sequence: {
          setSequenceFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::Comment: {
          setCommentFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::YamlDirective: {
          setDirectiveFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::TagDirective: {
          setTagFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::Start: {
          setStartTagFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::End: {
          setEndTagFormat(node, blockStart, text.length());
          break;
        }
        case YamlNode::Undefined:
          // never happen
          break;
      }
    }
  }
}

void
QYamlHighlighter::setScalarFormat(YamlNode* node,
                                  int blockStart,
                                  int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlScalar*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      if (n->errors().testFlag(IllegalFirstCharacter)) {
        m_scalarFormat.setUnderlineColor(m_errorColor);
        m_scalarFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        setFormat(formatable.start, formatable.length, m_scalarFormat);
        m_scalarFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        m_scalarFormat.setUnderlineColor(m_backgroundColor);
      } else
        setFormat(formatable.start, formatable.length, m_scalarFormat);
    }
  }
}

void
QYamlHighlighter::setKeyFormat(YamlMapItem* node,
                               int blockStart,
                               int textLength)
{
  FormatSize formatable;
  if (node) {
    if (isFormatable(node->startPos(),
                     node->keyLength(),
                     blockStart,
                     textLength,
                     formatable)) {
      setFormat(formatable.start, formatable.length, m_mapKeyFormat);
    }
  }
}

void
QYamlHighlighter::setCommentFormat(YamlNode* node,
                                   int blockStart,
                                   int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlComment*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      setFormat(formatable.start, formatable.length, m_commentFormat);
    }
  }
}

void
QYamlHighlighter::setDirectiveFormat(YamlNode* node,
                                     int blockStart,
                                     int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlDirective*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      setFormat(formatable.start, formatable.length, m_directiveFormat);
    }
  }
}

void
QYamlHighlighter::setTagFormat(YamlNode* node, int blockStart, int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlTagDirective*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      setFormat(formatable.start, formatable.length, m_tagFormat);
    }
  }
}

void
QYamlHighlighter::setStartTagFormat(YamlNode* node,
                                    int blockStart,
                                    int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlStart*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      setFormat(formatable.start, formatable.length, m_docStartFormat);
    }
  }
}

void
QYamlHighlighter::setEndTagFormat(YamlNode* node,
                                  int blockStart,
                                  int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlEnd*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      setFormat(formatable.start, formatable.length, m_docEndFormat);
    }
  }
}

void
QYamlHighlighter::setMapFormat(YamlNode* node, int blockStart, int textLength)
{

  auto n = qobject_cast<YamlMap*>(node);
  if (n) {
    switch (n->flowType()) {
      case YamlNode::Flow: {
        auto start = n->startPos() - blockStart;
        auto end = n->endPos() - blockStart;
        if (start >= 0 && start < textLength) {
          setFormat(start, 1, m_mapFormat);
        }
        if (end >= 0 && end < textLength) {
          setFormat(end, 1, m_mapFormat);
        }
        break;
      }
      case YamlNode::Block:
        // TODO
        break;
      default:
        break;
    }
  }
}

void
QYamlHighlighter::setMapItemFormat(YamlMapItem* node,
                                   int blockStart,
                                   int textLength)
{
  if (node) {
    auto n = node->data();
    switch (n->type()) {
      case YamlNode::Scalar: {
        setKeyFormat(node, blockStart, textLength);
        setScalarFormat(n, blockStart, textLength);
        break;
      }
      case YamlNode::Map: {
        setKeyFormat(node, blockStart, textLength);
        setMapFormat(n, blockStart, textLength);
        break;
      }
      case YamlNode::MapItem: { // should never happen
        setMapItemFormat(qobject_cast<YamlMapItem*>(n), blockStart, textLength);
        break;
      }
      case YamlNode::Sequence: {
        setKeyFormat(node, blockStart, textLength);
        setSequenceFormat(n, blockStart, textLength);
        break;
      }
      case YamlNode::Comment: { // should never happen
        setCommentFormat(n, blockStart, textLength);
        break;
      }
      default:
        break;
    }
  }
}

void
QYamlHighlighter::setSequenceFormat(YamlNode* node,
                                    int blockStart,
                                    int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlSequence*>(node);
  if (n) {
    switch (n->flowType()) {
      case YamlNode::Flow: {
        auto start = n->startPos() - blockStart;
        auto end = n->endPos() - blockStart;
        if (start >= 0 && start < textLength) {
          setFormat(start, 1, m_seqFormat);
        }
        if (end >= 0 && end < textLength) {
          setFormat(end, 1, m_seqFormat);
        }
        break;
      }
      case YamlNode::Block:
        // TODO
        break;
      default:
        break;
    }
    for (auto child : n->data()) {
      switch (child->type()) {
        case YamlNode::Scalar: {
          setScalarFormat(n, blockStart, textLength);
          break;
        }
        case YamlNode::Map: {
          setMapFormat(child, blockStart, textLength);
          break;
        }
        case YamlNode::Sequence: {
          setSequenceFormat(child, blockStart, textLength);
          break;
        }
        case YamlNode::Comment: {
          setCommentFormat(node, blockStart, textLength);
          break;
        }
        default:
          break;
      }
    }
  }
}

bool
QYamlHighlighter::isFormatable(int nodeStart,
                               int nodeLength,
                               int blockStart,
                               int textLength,
                               FormatSize& result)
{
  //  auto offsetstart = start + blockStart;
  auto end = nodeStart + nodeLength;
  auto textend = blockStart + textLength;

  if (end < blockStart || nodeStart > textend)
    return false;

  if (nodeStart < blockStart)
    result.start = blockStart;
  else {
    result.start = nodeStart - blockStart;
    result.start = (result.start < 0 ? 0 : result.start);
  }

  if (end > textend)
    result.length = blockStart + textLength - nodeStart;
  else
    result.length = nodeLength;

  return true;
}

const QColor&
QYamlHighlighter::mapColor() const
{
  return m_mapColor;
}

void
QYamlHighlighter::setMapColor(const QColor& mapColor)
{
  m_mapColor = mapColor;
}

const QColor&
QYamlHighlighter::commentColor() const
{
  return m_commentColor;
}

void
QYamlHighlighter::setCommentColor(const QColor& commentColor)
{
  m_commentColor = commentColor;
}

const QColor&
QYamlHighlighter::seqValueColor() const
{
  return m_seqValueColor;
}

void
QYamlHighlighter::setSeqValueColor(const QColor& seqValueColor)
{
  m_seqValueColor = seqValueColor;
}

const QColor&
QYamlHighlighter::seqColor() const
{
  return m_seqColor;
}

void
QYamlHighlighter::setSeqColor(const QColor& seqColor)
{
  m_seqColor = seqColor;
}

const QColor&
QYamlHighlighter::mapValueColor() const
{
  return m_mapValueColor;
}

void
QYamlHighlighter::setMapValueColor(const QColor& mapValueColor)
{
  m_mapValueColor = mapValueColor;
}

const QColor&
QYamlHighlighter::mapKeyColor() const
{
  return m_mapKeyColor;
}

void
QYamlHighlighter::setMapKeyColor(const QColor& mapKeyColor)
{
  m_mapKeyColor = mapKeyColor;
}

const QColor&
QYamlHighlighter::textColor() const
{
  return m_textColor;
}

void
QYamlHighlighter::setTextColor(const QColor& textColor)
{
  m_textColor = textColor;
}

const QColor&
QYamlHighlighter::backgroundColor() const
{
  return m_backgroundColor;
}

void
QYamlHighlighter::setBackgroundColor(const QColor& backgroundColor)
{
  m_backgroundColor = backgroundColor;
}

QColor
QYamlHighlighter::docEndColor() const
{
  return m_docEndColor;
}

void
QYamlHighlighter::setDocEndColor(const QColor& docEndColor)
{
  m_docEndColor = docEndColor;
}

QColor
QYamlHighlighter::docStartColor() const
{
  return m_docStartColor;
}

void
QYamlHighlighter::setDocStartColor(const QColor& docStartColor)
{
  m_docStartColor = docStartColor;
}

QColor
QYamlHighlighter::directiveColor() const
{
  return m_directiveColor;
}

void
QYamlHighlighter::setDirectiveColor(const QColor& directiveColor)
{
  m_directiveColor = directiveColor;
}

QColor
QYamlHighlighter::tagColor() const
{
  return m_tagColor;
}

void
QYamlHighlighter::setTagColor(const QColor& tagColor)
{
  m_tagColor = tagColor;
}
