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
  return column;
}

void
YamlNode::setColumn(int column)
{
  this->column = column;
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
  return m_length;
}

void
YamlNode::setLength(int newLength)
{
  m_length = newLength;
}

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
    m_style = DOUBLE_QUOTED;
  else if (m_data.startsWith(Characters::SINGLEQUOTE) &&
           m_data.endsWith(Characters::SINGLEQUOTE))
    m_style = SINGLE_QUOTED;
  else
    m_style = PLAIN;
}

YamlScalar::Style
YamlScalar::style() const
{
  return m_style;
}

int
YamlScalar::length() const
{
  return m_data.length();
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
YamlComment::data() const
{
  return m_data;
}

//====================================================================
//=== YamlDirective
//====================================================================
YamlDirective::YamlDirective(int major, int minor, QObject* parent)
  : YamlNode(parent)
  , m_major(major)
  , m_minor(minor)
{
}

int
YamlDirective::major() const
{
  return m_major;
}

int
YamlDirective::minor() const
{
  return m_minor;
}

bool
YamlDirective::isValid()
{
  return (m_major == 1 && (m_minor >= 0 && m_minor <= 3));
}
