#include "qyaml/yamlnode.h"
#include "utilities/characters.h"

//====================================================================
//=== YamlNode
//====================================================================
YamlNode::YamlNode(QObject* parent)
  : QObject(parent)
  , m_type(Undefined)
{
}

YamlNode*
YamlNode::parent() const
{
  return m_parent;
}

void
YamlNode::setParent(YamlNode* Parent)
{
  m_parent = Parent;
}

int
YamlNode::indent() const
{
  return m_indent;
}

void
YamlNode::setIndent(int indent)
{
  m_indent = indent;
}

int
YamlNode::row() const
{
  return m_row;
}

void
YamlNode::setRow(int row)
{
  m_row = row;
}

int
YamlNode::getColumn() const
{
  return m_column;
}

void
YamlNode::setColumn(int column)
{
  this->m_column = column;
}

const QTextCursor&
YamlNode::start() const
{
  return m_start;
}

int
YamlNode::startPos() const
{
  return m_start.position();
}

void
YamlNode::setStart(const QTextCursor& newStart)
{
  m_start = newStart;
}

int
YamlNode::length() const
{
  return endPos() - startPos();
}

// void YamlNode::setLength(int newLength) { m_length = newLength; }

const YamlErrors&
YamlNode::errors() const
{
  return m_errors;
}

void
YamlNode::setError(const YamlError& error, bool set)
{
  m_errors.setFlag(error, set);
}

void
YamlNode::setErrors(const YamlErrors& newErrors)
{
  m_errors = newErrors;
}

bool
YamlNode::hasErrors()
{
  return m_errors != YamlError::NoErrors;
}

const YamlWarnings&
YamlNode::warnings() const
{
  return m_warnings;
}

void
YamlNode::setWarning(const YamlWarning& warning, bool set)
{
  m_warnings.setFlag(warning, set);
}

void
YamlNode::setWarnings(const YamlWarnings& newWarnings)
{
  m_warnings = newWarnings;
}

bool
YamlNode::hasWarnings()
{
  return m_warnings != YamlWarning::NoWarnings;
}

bool
YamlNode::hasDodgyChar()
{
  if (m_type == Scalar)
    return !m_dodgyChars.isEmpty();
  return false;
}

QMap<QTextCursor, YamlWarning>
YamlNode::dodgyChars() const
{
  return m_dodgyChars;
}

void
YamlNode::addDodgyChar(QTextCursor pos, YamlWarning warning)
{
  if (warning == YamlWarning::TabCharsDiscouraged) {
    m_dodgyChars.insert(pos, warning);
    m_warnings.setFlag(warning, true);
  }
}

QMap<QTextCursor, YamlWarning>::size_type
YamlNode::removeDodgyChar(QTextCursor pos)
{
  if (m_dodgyChars.size() == 1)
    m_warnings.setFlag(YamlWarning::TabCharsDiscouraged, false);
  return m_dodgyChars.remove(pos);
}

YamlNode::Type
YamlNode::type() const
{
  return m_type;
}

YamlNode::FlowType
YamlNode::flowType() const
{
  return m_flowType;
}

void
YamlNode::setFlowType(YamlNode::FlowType flowType)
{
  m_flowType = flowType;
}

QString
YamlNode::toString(const QString& text, FlowType override)
{
  return text.mid(startPos(), length());
}

const QTextCursor&
YamlNode::end() const
{
  return m_end;
}

int
YamlNode::endPos() const
{
  return m_end.position();
}

void
YamlNode::setEnd(const QTextCursor& end)
{
  m_end = end;
}

//====================================================================
//=== YamlMap
//====================================================================
YamlMap::YamlMap(QObject* parent)
  : YamlNode(parent)
{
  m_type = Map;
}

YamlMap::YamlMap(QMap<QString, YamlMapItem*> data, QObject* parent)
  : YamlNode(parent)
  , m_data(data)
{
  m_type = Map;
}

QMap<QString, YamlMapItem*>
YamlMap::data() const
{
  return m_data;
}

void
YamlMap::setData(QMap<QString, YamlMapItem*> data)
{
  m_data = data;
}

bool
YamlMap::insert(const QString& key, YamlMapItem* data)
{
  if (data) {
    //    if (!m_data.contains(key)) {
    m_data.insert(key, data);
    return true;
    //    }
  }
  return false;
}

int
YamlMap::remove(const QString& key)
{
  return m_data.remove(key);
}

YamlMapItem*
YamlMap::value(const QString& key)
{
  return m_data.value(key);
}

bool
YamlMap::contains(const QString& key)
{
  return m_data.contains(key);
}

QString
YamlMap::toString(const QString& text, FlowType override)
{
  QString result = YamlNode::toString(text, override);

  return result;
}

//====================================================================
//=== YamlMapItem
//====================================================================
YamlMapItem::YamlMapItem(QObject* parent)
  : YamlNode(parent)
{
  m_type = MapItem;
}

YamlMapItem::YamlMapItem(const QString& key, YamlNode* data, QObject* parent)
  : YamlNode(parent)
  , m_key{ key }
  , m_data{ data }
{
  m_type = MapItem;
}

const QString&
YamlMapItem::key() const
{
  return m_key;
}

void
YamlMapItem::setKey(const QString& key)
{
  m_key = key;
}

int
YamlMapItem::keyLength() const
{
  return m_key.length();
}

YamlNode*
YamlMapItem::data() const
{
  return m_data;
}

void
YamlMapItem::setData(YamlNode* data)
{
  m_data = data;
}

QString
YamlMapItem::toString(const QString& text, FlowType override)
{
  QString result = YamlNode::toString(text, override);

  return result;
}

// const QTextCursor&
// YamlMapItem::keyStart() const
//{
//   return m_keyStart;
// }

// int YamlMapItem::keyStartPos() {
//   return m_keyStart.position();
// }

// void
// YamlMapItem::setKeyStart(const QTextCursor& keyStart)
//{
//   m_keyStart = keyStart;
// }

// const QTextCursor &YamlMapItem::dataStart() const
//{
//   return m_dataStart;
// }

// void YamlMapItem::setDataStart(const QTextCursor &dataStart)
//{
//   m_dataStart = dataStart;
// }

//====================================================================
//=== YamlSequence
//====================================================================
YamlSequence::YamlSequence(QObject* parent)
  : YamlNode(parent)
{
  m_type = Sequence;
}

YamlSequence::YamlSequence(QVector<YamlNode*> sequence, QObject* parent)
  : YamlNode(parent)
  , m_data(sequence)
{
  m_type = Sequence;
}

QVector<YamlNode*>
YamlSequence::data() const
{
  return m_data;
}

void
YamlSequence::setData(QVector<YamlNode*> data)
{
  m_data = data;
}

bool
YamlSequence::append(YamlNode* data)
{
  if (!m_data.contains(data)) {
    m_data.append(data);
    return true;
  }
  return false;
}

void
YamlSequence::remove(int index)
{
  m_data.remove(index);
}

int
YamlSequence::indexOf(YamlNode* node)
{
  return m_data.indexOf(node);
}

QString
YamlSequence::toString(const QString& text, FlowType override)
{
  QString result = YamlNode::toString(text, override);

  return result;
}

// void YamlSequence::setEnd(const QTextCursor &end)
//{

//}

//====================================================================
//=== YamlScalar
//====================================================================
YamlScalar::YamlScalar(QObject* parent)
  : YamlNode(parent)
{
  m_type = Scalar;
}

YamlScalar::YamlScalar(QString value, QObject* parent)
  : YamlNode(parent)
{
  m_type = Scalar;
  setData(value);
}

void
YamlScalar::setData(const QString& data)
{
  m_data = data.trimmed();
  if (m_data.startsWith(Characters::QUOTATION) &&
      m_data.endsWith(Characters::QUOTATION))
    m_style = DOUBLEQUOTED;
  else if (m_data.startsWith(Characters::SINGLEQUOTE) &&
           m_data.endsWith(Characters::SINGLEQUOTE))
    m_style = SINGLEQUOTED;
  else
    m_style = PLAIN;

  if (data.contains(Characters::NEWLINE)) {
    m_multiline = true;
  }
}

YamlScalar::Style
YamlScalar::style() const
{
  return m_style;
}

void
YamlScalar::setStyle(Style style)
{
  m_style = style;
}

int
YamlScalar::length() const
{
  return m_data.length();
}

bool
YamlScalar::multiline() const
{
  return m_multiline;
}

QString
YamlScalar::toFlowScalar(const QString& text)
{
  QString result;
  auto splits = text.split(Characters::NEWLINE, Qt::KeepEmptyParts);
  for (auto i = 0; i < splits.size(); i++) {
    auto s = splits.at(i);
    // in this case ignore start/end spaces and replace newlines with a space.
    s = s.trimmed();
    if (i == 0 && s.isEmpty()) {
      // empty before text ignore.
      continue;
    } else {
      if (!result.isEmpty()) {
        if (s.isEmpty()) {
          result += Characters::NEWLINE;
          continue;
        } else {
          if (i < splits.size() && !result.endsWith(Characters::NEWLINE)) {
            result += Characters::SPACE;
          }
        }
      }
      result += s;
    }
  }
  return result;
}

QString
YamlScalar::toBlockScalar(const QString& text)
{
  // TODO block scalars.
  return QString();
}

QString
YamlScalar::toString(const QString& text, FlowType override)
{
  QString result = YamlNode::toString(text, override);

  if (override) {
    switch (override) {
      case Flow:
        if (m_multiline) {
          return toFlowScalar(result);
        } else {
          return result;
        }
        break;

      case Block:

        break;

      case NoFlowType:
        // This should never happen.
        break;
    }
  } else {
    switch (m_flowType) {
      case Flow:
        if (m_multiline) {
          return toFlowScalar(result);
        } else {
          return result;
        }
        break;

      case Block:
        break;

      case NoFlowType:
        // This should never happen.
        break;
    }
  }

  return result;
}

QString
YamlScalar::data() const
{
  return m_data;
}

//====================================================================
//=== YamlComment
//====================================================================
YamlComment::YamlComment(QObject* parent)
  : YamlNode(parent)
{
  m_type = Comment;
}

YamlComment::YamlComment(QString value, QObject* parent)
  : YamlNode(parent)
  , m_data(value)
{
  m_type = Comment;
}

void
YamlComment::append(QChar c)
{
  m_data += c;
}

void
YamlComment::setData(const QString& data)
{
  m_data = data;
}

int
YamlComment::length() const
{
  return m_data.length();
}

QString
YamlComment::toString(const QString& text, FlowType override)
{
  QString result;

  return result;
}

QString
YamlComment::data() const
{
  return m_data;
}

//====================================================================
//=== YamlDirective
//====================================================================
YamlDirective::YamlDirective(QObject* parent)
  : YamlNode(parent)
{
  m_type = YamlNode::YamlDirective;
}

YamlDirective::YamlDirective(int major, int minor, QObject* parent)
  : YamlNode(parent)
  , m_major(major)
  , m_minor(minor)
{
  m_type = YamlNode::YamlDirective;
}

int
YamlDirective::major() const
{
  return m_major;
}

void
YamlDirective::setMajor(int major)
{
  m_major = major;
}

int
YamlDirective::minor() const
{
  return m_minor;
}

void
YamlDirective::setMinor(int minor)
{
  m_minor = minor;
}

bool
YamlDirective::isValid()
{
  return (m_major == 1 && (m_minor >= 0 && m_minor <= 3));
}

QTextCursor
YamlDirective::versionStart() const
{
  return m_versionStart;
}

int
YamlDirective::versionStartPos() const
{
  return m_versionStart.position();
}

void
YamlDirective::setVersionStart(const QTextCursor& versionStart)
{
  m_versionStart = versionStart;
}

//====================================================================
//=== YamlTagDirective
//====================================================================
YamlTagDirective::YamlTagDirective(QObject* parent)
  : YamlNode(parent)
{
  m_type = TagDirective;
}

YamlTagDirective::YamlTagDirective(const QString& handle,
                                   const QString& value,
                                   QObject* parent)
  : YamlNode(parent)
  , m_value(value)
  , m_handle(handle)
{
  m_type = TagDirective;
}

bool
YamlTagDirective::isValid()
{
  if (m_handle.isEmpty() || m_value.isEmpty())
    return false;
  return true;
}

QString
YamlTagDirective::value() const
{
  return m_value;
}

void
YamlTagDirective::setValue(const QString& value)
{
  m_value = value;
}

void
YamlTagDirective::setValueStart(QTextCursor cursor)
{
  m_valueStart = cursor;
}

QTextCursor
YamlTagDirective::valueStart()
{
  return m_valueStart;
}

int
YamlTagDirective::valueStartPos()
{
  return m_valueStart.position();
}

QString
YamlTagDirective::handle() const
{
  return m_handle;
}

void
YamlTagDirective::setHandle(const QString& id)
{
  m_handle = id;
}

void
YamlTagDirective::setHandleStart(QTextCursor cursor)
{
  m_handleStart = cursor;
}

QTextCursor
YamlTagDirective::handleStart()
{
  return m_handleStart;
}

int
YamlTagDirective::handleStartPos()
{
  return m_handleStart.position();
}

YamlTagDirective::TagHandleType
YamlTagDirective::handleType() const
{
  return m_handleType;
}

void
YamlTagDirective::setHandleType(TagHandleType handleType)
{
  m_handleType = handleType;
}

//====================================================================
//=== YamlStart
//====================================================================
YamlStart::YamlStart(QObject* parent)
  : YamlNode(parent)
{
  m_type = Start;
}

//====================================================================
//=== YamlEnd
//====================================================================
YamlEnd::YamlEnd(QObject* parent)
  : YamlNode(parent)
{
  m_type = End;
}
