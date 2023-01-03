#include "qyaml/qyamldocument.h"
#include "qyaml/yamlnode.h"

//====================================================================
//=== QYamlDocument
//====================================================================
QYamlDocument::QYamlDocument(QObject* parent)
  : QObject(parent)
{
}

int
QYamlDocument::majorVersion() const
{
  return m_majorVersion;
}

void
QYamlDocument::setMajorVersion(int Major)
{
  m_majorVersion = Major;
}

int
QYamlDocument::minorVersion() const
{
  return m_minorVersion;
}

void
QYamlDocument::setMinorVersion(int Minor)
{
  m_minorVersion = Minor;
}

bool
QYamlDocument::implicitStart() const
{
  return m_implicitStart;
}

void
QYamlDocument::setImplicitStart(bool ExplicitStart)
{
  m_implicitStart = ExplicitStart;
}

bool
QYamlDocument::isImplicitVersion() const
{
  return m_implicitVersion;
}

void
QYamlDocument::setImplicitVersion(bool ExplicitVersion)
{
  m_implicitVersion = ExplicitVersion;
}

QTextCursor
QYamlDocument::documentStart()
{
  return m_start;
}

void
QYamlDocument::setDocumentStart(QTextCursor mark)
{
  m_start = mark;
}

QTextCursor
QYamlDocument::documentEnd()
{
  return m_end;
}

void
QYamlDocument::setDocumentEnd(QTextCursor mark)
{
  m_end = mark;
}

bool
QYamlDocument::implicitEnd() const
{
  return m_implicitEnd;
}

void
QYamlDocument::setImplicitEnd(bool ImplicitEnd)
{
  m_implicitEnd = ImplicitEnd;
}

bool
QYamlDocument::getExplicitTags() const
{
  return explicitTags;
}

void
QYamlDocument::setExplicitTags(bool ExplicitTags)
{
  explicitTags = ExplicitTags;
}

QList<YamlNode*>
QYamlDocument::data() const
{
  return m_data;
}

YamlNode*
QYamlDocument::data(int index)
{
  return m_data.at(index);
}

bool
QYamlDocument::addData(YamlNode* data)
{
  switch (data->type()) {
    case YamlNode::Comment:
      m_data.append(data);
      m_nodes.insert(data->start(), data);
      return true;
    case YamlNode::Scalar:
      m_data.append(data);
      m_nodes.insert(data->start(), data);
      return true;
    case YamlNode::Sequence: {
      m_data.append(data);
      return addSequenceData(qobject_cast<YamlSequence*>(data));
    }
    case YamlNode::Map: {
      m_data.append(data);
      return addMapData(qobject_cast<YamlMap*>(data));
    }
    default:
      return false;
  }
}

bool
QYamlDocument::addSequenceData(YamlSequence* sequence, YamlMapItem* item)
{
  if (item) // sub sequence in map
    m_nodes.insert(item->start(), item);
  else
    m_nodes.insert(sequence->start(), sequence);

  for (auto data : sequence->data()) {
    switch (data->type()) {
      case YamlNode::Comment:
        //        m_nodes.insert(data->start(), data);
        break;
      case YamlNode::Scalar:
        m_nodes.insert(data->start(), data);
        break;
      case YamlNode::Sequence:
        addSequenceData(qobject_cast<YamlSequence*>(data));
        break;
      case YamlNode::Map:
        addMapData(qobject_cast<YamlMap*>(data));
        break;
      default:
        return false; // should only happen on error.
    }
  }
  return false;
}

bool
QYamlDocument::addMapData(YamlMap* map, YamlMapItem* item)
{
  if (item) // sub map in map
    m_nodes.insert(item->start(), item);
  else
    m_nodes.insert(map->start(), map);

  bool result = true;
  if (map) {
    for (auto key : map->data().keys()) {
      auto i = map->value(key);
      auto data = i->data();
      switch (data->type()) {
        case YamlNode::Comment:
          //          m_nodes.insert(data->start(), data);
          break;
        case YamlNode::Scalar:
          m_nodes.insert(data->start(), i);
          break;
        case YamlNode::Sequence: {
          addSequenceData(qobject_cast<YamlSequence*>(data), i);
          break;
        }
        case YamlNode::Map: {
          addMapData(qobject_cast<YamlMap*>(data), i);
          break;
        }
        case YamlNode::MapItem: {
          bool r = addMapItemData(qobject_cast<YamlMapItem*>(data));
          if (!r)
            result = false;
          break;
        }
        default:
          return false; // should only happen on error.
      }
    }
  }
  return true;
}

bool
QYamlDocument::addMapItemData(YamlMapItem* item)
{
  if (item) {
    auto data = item->data();
    if (data) {
      switch (data->type()) {
        case YamlNode::Comment:
          //          m_nodes.insert(data->start(), data);
          return true;
        case YamlNode::Scalar:
          m_nodes.insert(item->start(), item);
          return true;
        case YamlNode::Sequence: {
          return addSequenceData(qobject_cast<YamlSequence*>(data));
        }
        case YamlNode::Map: {
          return addMapData(qobject_cast<YamlMap*>(data));
        }
        default:
          return false;
      }
    }
  }
  return false;
}

const YamlErrors&
QYamlDocument::errors() const
{
  return m_errors;
}

void
QYamlDocument::setError(const YamlError& error, bool set)
{
  m_errors.setFlag(error, set);
}

void
QYamlDocument::setErrors(const YamlErrors& newErrors)
{
  m_errors = newErrors;
}

const YamlWarnings&
QYamlDocument::warnings() const
{
  return m_warnings;
}

void
QYamlDocument::setWarning(const YamlWarning& warning, bool set)
{
  m_warnings.setFlag(warning, set);
}

void
QYamlDocument::setWarnings(const YamlWarnings& newWarnings)
{
  m_warnings = newWarnings;
}

const QMap<QTextCursor, YamlNode*>&
QYamlDocument::nodes() const
{
  return m_nodes;
}
