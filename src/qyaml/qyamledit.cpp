#include "qyaml/qyamledit.h"
#include "qyaml/qyamlparser.h"
#include "qyaml/qyamlhighlighter.h"

#include <JlCompress.h>

QYamlEdit::QYamlEdit(QWidget* parent)
  : LNPlainTextEdit(parent)
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
  disconnect(LNPlainTextEdit::document(),
             &QTextDocument::contentsChange,
             this,
             &QYamlEdit::textHasChanged);
  QPlainTextEdit::setPlainText(text);
  m_parser->parse(text);
  connect(LNPlainTextEdit::document(),
          &QTextDocument::contentsChange,
          this,
          &QYamlEdit::textHasChanged);
}

void
QYamlEdit::textHasChanged(int position, int charsRemoved, int charsAdded)
{
}
