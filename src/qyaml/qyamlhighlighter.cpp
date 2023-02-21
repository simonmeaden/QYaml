#include "qyaml/qyamlhighlighter.h"
#include "qyaml/qyamldocument.h"
#include "qyaml/qyamledit.h"
#include "qyaml/qyamlparser.h"
#include "qyaml/yamlnode.h"
#include "utilities/ContainerUtil.h"
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
  , m_tagColor(QColorConstants::X11::peru)
  , m_reservedColor(QColorConstants::X11::peru)
  , m_docStartColor(QColorConstants::X11::MediumBlue)
  , m_docEndColor(QColorConstants::X11::LightBlue1)
  , m_errorColor(QColorConstants::X11::red)
  , m_warningColor(QColorConstants::X11::orange)
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
  m_reservedFormat.setBackground(m_backgroundColor);
  m_reservedFormat.setForeground(m_reservedColor);
  m_directiveFormat.setBackground(m_backgroundColor);
  m_directiveFormat.setForeground(m_directiveColor);
  m_docStartFormat.setBackground(m_backgroundColor);
  m_docStartFormat.setForeground(m_docStartColor);
  m_docEndFormat.setBackground(m_backgroundColor);
  m_docEndFormat.setForeground(m_docEndColor);
  m_warningFormat.setBackground(m_warningColor);
  m_warningFormat.setForeground(m_backgroundColor);
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

    for (auto& node : nodes) {
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
          //          if (node->hasErrors() &&
          //              node->errors().testFlag(YamlError::EmptyFlowValue)) {
          //            setScalarFormat(node, blockStart, text.length());
          //            continue;
          //          }
          setScalarFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::Map: {
          setMapFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::MapItem: {
          auto n = qSharedPointerDynamicCast<YamlMapItem>(node);
          //          auto type = n->data()->type();
          if (n) {
            setMapItemFormat(n, blockStart, textLength);
          }
          break;
        }
        case YamlNode::Sequence: {
          setSequenceFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::Comment: {
          setCommentFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::YamlDirective: {
          setDirectiveFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::TagDirective: {
          setTagFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::ReservedDirective: {
          setReservedFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::Start: {
          setStartTagFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::End: {
          setEndTagFormat(node, blockStart, textLength);
          break;
        }
        case YamlNode::Directive:
        case YamlNode::Undefined:
          // never happen
          break;
      }
    }
  }
}

QColor
QYamlHighlighter::reservedColor() const
{
  return m_reservedColor;
}

void
QYamlHighlighter::setReservedColor(const QColor& reservedColor)
{
  m_reservedColor = reservedColor;
}

void
QYamlHighlighter::setScalarFormat(SharedNode node,
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
      } else if (n->errors().testFlag(EmptyFlowValue)) {
        m_scalarFormat.setUnderlineColor(m_errorColor);
        m_scalarFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        setFormat(formatable.start, 1, m_scalarFormat);
        m_scalarFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        m_scalarFormat.setUnderlineColor(m_backgroundColor);
      } else if (n->hasDodgyChar()) {
        auto dodgy = n->dodgyChars();
        setFormat(formatable.start, formatable.length, m_scalarFormat);
        for (auto [key, warning] : asKeyValueRange(dodgy)) {
          auto start = key.position() - blockStart;
          m_scalarFormat.setUnderlineColor(m_warningColor);
          m_scalarFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
          setFormat(start, 1, m_scalarFormat);
          m_scalarFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
          m_scalarFormat.setUnderlineColor(m_backgroundColor);
        }
      } else
        setFormat(formatable.start, formatable.length, m_scalarFormat);
    }
  }
}

void
QYamlHighlighter::setKeyFormat(QSharedPointer<YamlMapItem> node,
                               int blockStart,
                               int nodeLength)
{
  FormatSize formatable;
  if (node) {
    if (isFormatable(node->startPos(),
                     node->keyLength(),
                     blockStart,
                     nodeLength,
                     formatable)) {
      setFormat(formatable.start, formatable.length, m_mapKeyFormat);
    }
  }
}

void
QYamlHighlighter::setCommentFormat(SharedNode node,
                                   int blockStart,
                                   int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlComment*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      if (n->warnings().testFlag(YamlWarning::InvalidSpaceWarning) ||
          n->warnings().testFlag(YamlWarning::IllegalCommentPosition) ||
          n->warnings().testFlag(
            YamlWarning::InvalidMinorVersionWarning)) { // errors override
                                                        // warnings
        m_commentFormat.setUnderlineColor(m_warningColor);
        m_commentFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        setFormat(formatable.start, formatable.length, m_commentFormat);
        m_commentFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        m_commentFormat.setUnderlineColor(m_backgroundColor);
      } else {
        setFormat(formatable.start, formatable.length, m_commentFormat);
      }
    }
  }
}

void
QYamlHighlighter::setDirectiveFormat(SharedNode node,
                                     int blockStart,
                                     int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlYamlDirective*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      if (n->errors().testFlag(YamlError::TooManyYamlDirectivesError)) {
        m_directiveFormat.setUnderlineColor(m_errorColor);
        m_directiveFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        setFormat(formatable.start, formatable.length, m_directiveFormat);
        m_directiveFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        m_directiveFormat.setUnderlineColor(m_backgroundColor);
      } else if (n->warnings().testFlag(YamlWarning::InvalidSpaceWarning) ||
                 n->warnings().testFlag(
                   YamlWarning::
                     InvalidMinorVersionWarning)) { // errors override warnings
        m_directiveFormat.setUnderlineColor(m_warningColor);
        m_directiveFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        setFormat(formatable.start, formatable.length, m_tagFormat);
        m_directiveFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        m_directiveFormat.setUnderlineColor(m_backgroundColor);
      } else {
        setFormat(formatable.start, formatable.length, m_directiveFormat);
      }
    }
  }
}

void
QYamlHighlighter::setTagFormat(SharedNode node, int blockStart, int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlTagDirective*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      if (n->warnings().testFlag(InvalidSpaceWarning) ||
          n->warnings().testFlag(YamlWarning::IllegalCommentPosition)) {
        m_tagFormat.setUnderlineColor(m_warningColor);
        m_tagFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        setFormat(formatable.start, formatable.length, m_tagFormat);
        m_tagFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        m_tagFormat.setUnderlineColor(m_backgroundColor);
      } else {
        setFormat(formatable.start, formatable.length, m_tagFormat);
      }
    }
  }
}

void
QYamlHighlighter::setReservedFormat(SharedNode node,
                                    int blockStart,
                                    int textLength)
{
  FormatSize formatable;
  auto n = qobject_cast<YamlReservedDirective*>(node);
  if (n) {
    if (isFormatable(
          n->startPos(), n->length(), blockStart, textLength, formatable)) {
      if (n->warnings().testFlag(ReservedDirectiveWarning) ||
          n->warnings().testFlag(InvalidSpaceWarning)) {
        m_reservedFormat.setUnderlineColor(m_warningColor);
        m_reservedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        setFormat(formatable.start, formatable.length, m_reservedFormat);
        m_reservedFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        m_reservedFormat.setUnderlineColor(m_backgroundColor);
      } else {
        setFormat(formatable.start, formatable.length, m_reservedFormat);
      }
    }
  }
}

void
QYamlHighlighter::setStartTagFormat(SharedNode node,
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
QYamlHighlighter::setEndTagFormat(SharedNode node,
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
QYamlHighlighter::setMapFormat(SharedNode node, int blockStart, int textLength)
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
QYamlHighlighter::setMapItemFormat(QSharedPointer<YamlMapItem> node,
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
        setMapItemFormat(
          qSharedPointerDynamicCast<YamlMapItem>(n), blockStart, textLength);
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
QYamlHighlighter::setSequenceFormat(SharedNode node,
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
    for (auto& child : n->data()) {
      if (child) {
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
    result.start = 0;
  else {
    result.start = nodeStart - blockStart;
    result.start = (result.start < 0 ? 0 : result.start);
  }

  if (end > textend)
    result.length = textLength;
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

QColor
QYamlHighlighter::warningColor() const
{
  return m_warningColor;
}

void
QYamlHighlighter::setWarningColor(const QColor& warningColor)
{
  m_warningColor = warningColor;
}

QColor
QYamlHighlighter::errorColor() const
{
  return m_errorColor;
}

void
QYamlHighlighter::setErrorColor(const QColor& errorColor)
{
  m_errorColor = errorColor;
}
