#include "qyaml/qyamledit.h"
#include "lnplaintext/hoverwidget.h"
#include "markdown/markdowntools.h"
#include "qyaml/qyamlhighlighter.h"
#include "qyaml/qyamlparser.h"
#include "utilities/characters.h"

#include <JlCompress.h>

//====================================================================
//=== QYamlEdit
//====================================================================
QYamlEdit::QYamlEdit(QWidget* parent)
  : QLNPlainTextEdit(parent)
  , m_parser(new QYamlParser(document(), this))
  , m_highlighter(new QYamlHighlighter(m_parser, this))
{
  connect(m_parser,
          &QYamlParser::parseComplete,
          m_highlighter,
          &QYamlHighlighter::rehighlight);
}

const QString
QYamlEdit::filename() const
{
  return m_filename;
}

void
QYamlEdit::loadFile(const QString& filename)
{
  m_filename = filename;
  QFile file(m_filename);
  if (file.open(QIODevice::ReadOnly)) {
    auto text = file.readAll();
    setText(text);
  }
}

void
QYamlEdit::loadFromZip(const QString& zipFile, const QString& href)
{
  m_filename = href;
  m_zipFile = zipFile;
  auto fileName = JlCompress::extractFile(zipFile, href);
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly)) {
    auto text = file.readAll();
    setText(text);
  }
}

void
QYamlEdit::setText(const QString& text)
{
  disconnect(QLNPlainTextEdit::document(),
             &QTextDocument::contentsChange,
             this,
             &QYamlEdit::textHasChanged);
  QPlainTextEdit::setPlainText(text);
  m_parser->parse(text);
  connect(QLNPlainTextEdit::document(),
          &QTextDocument::contentsChange,
          this,
          &QYamlEdit::textHasChanged);
}

void
QYamlEdit::hoverEnter(QHoverEvent* event)
{
}

void
QYamlEdit::hoverLeave(QHoverEvent* event)
{
  QLNPlainTextEdit::hoverLeave(event);
}

bool
QYamlEdit::isInText(const QPoint& pos)
{
  // Unfortunately cursorForPosition() returns the cursor for the end of line
  // even if the mouse pointer is beyond the end of the text as the text
  // block takes the entire width of the document. So we have to check for
  // the mouse position being inside the text first.
  auto x = pos.x();
  auto y = pos.y();
  auto contained = false;
  auto layout = m_document->documentLayout();
  auto fm = fontMetrics();
  auto block = firstVisibleBlock();
  auto blockNumber = block.blockNumber();
  auto geom = blockBoundingGeometry(block);
  auto top = qRound(geom.translated(contentOffset()).top());
  auto left = qRound(geom.left());
  auto bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && block.isVisible()) {
    auto text = block.text();
    auto length = fm.horizontalAdvance(text);
    auto right = left + length;
    if (x >= left && x < right && y >= top && y <= bottom) {
      contained = true;
      break;
    }
    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }

  return contained;
}

void
QYamlEdit::createHoverWidget(QPoint pos, QString text, QString title)
{
  m_hoverWidget = new HoverWidget(title, text, HoverWidget::Markdown, this);
  connect(
    m_hoverWidget, &HoverWidget::finished, this, &QYamlEdit::killHoverWidget);
  auto size = m_hoverWidget->sizeHint();
  m_hoverWidget->setGeometry(pos.x(), pos.y(), size.width(), size.height());
  m_hoverWidget->show(m_hoverTime);
}

void
QYamlEdit::hoverMove(QHoverEvent* event)
{
  auto pos = event->position().toPoint();

  if (!isInText(pos))
    return;

  // Then we can use the cursor.
  auto cursor = cursorForPosition(pos);
  auto node = m_parser->nodeAt(cursor);
  if (node) {
    QStringList errors, warnings;
    auto hasErrors = node->hasErrors();
    auto hasWarnings = node->hasWarnings();
    QString title;
    QString text;

    if (hasErrors && hasWarnings) {
      title = tr("The node has the following errors and warnings.");
    } else if (hasErrors) {
      title = tr("The node has the following errors.");
    } else if (hasWarnings) {
      title = tr("The node has the following warnings.");
    } else {
      return; // no errors or warnings
    }

    if (hasErrors) {
      errors = YamlNode::errorText(node->errors());
      for (auto& t : errors) {
        Markdown::startMDListItem(text);
        Markdown::startMDStrongEmphasis(text);
        text += t;
        Markdown::endMDStrongEmphasis(text);
        text += Characters::NEWLINE;
      }
    }
    if (hasWarnings) {
      warnings = YamlNode::warningText(node->warnings());
      for (auto& t : warnings) {
        Markdown::startMDListItem(text);
        Markdown::startMDEmphasis(text); // emphasis
        text += t;
        Markdown::endMDEmphasis(text); // kill the emphasis
        text += Characters::NEWLINE;
      }
    }

    if (!m_hoverWidget) {
      createHoverWidget(pos, text, title);
      m_hoverNode = node;
      return;
    }

    if (m_hoverNode && node != m_hoverNode) {
      if (m_hoverWidget) {
        m_hoverWidget->destroy();
        m_hoverWidget = nullptr;
        createHoverWidget(pos, text, title);
      } else {
        createHoverWidget(pos, text, title);
        m_hoverNode = node;
      }
    }
  }
}

int
QYamlEdit::hoverTime() const
{
  return m_hoverTime;
}

void
QYamlEdit::setHoverTime(int hoverTime)
{
  m_hoverTime = hoverTime;
}

void
QYamlEdit::textHasChanged(int position, int charsRemoved, int charsAdded)
{
}

void
QYamlEdit::killHoverWidget()
{
  if (m_hoverWidget) {
    m_hoverWidget->close();
    m_hoverWidget->deleteLater();
    m_hoverWidget = nullptr;
  }
}
