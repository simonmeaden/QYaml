#include "qyaml/qyamldocument.h"
#include "qyaml/yamlnode.h"
#include "utilities/ContainerUtil.h"

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
  return m_directive->major();
}

// void QYamlDocument::setMajorVersion(int Major) {
//   m_majorVersion = Major;
//   m_implicitVersion = false;
// }

int
QYamlDocument::minorVersion() const
{
  return m_directive->minor();
}

// void QYamlDocument::setMinorVersion(int Minor) {
//   m_minorVersion = Minor;
//   m_implicitVersion = false;
// }

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
QYamlDocument::setImplicitVersion(bool implicitVersion)
{
  m_implicitVersion = implicitVersion;
}

QTextCursor
QYamlDocument::start()
{
  return m_start;
}

int
QYamlDocument::startPos()
{
  return m_start.position();
}

void
QYamlDocument::setStart(QTextCursor position, SharedStart start)
{
  m_start = position;
  m_implicitStart = false;
  if (start) {
    m_data.append(start);
    m_nodes.insert(start->start(), start);
  }
}

bool
QYamlDocument::hasStart()
{
  return !m_start.isNull();
}

QTextCursor
QYamlDocument::end()
{
  return m_end;
}

int
QYamlDocument::endPos()
{
  return m_end.position();
}

void
QYamlDocument::setEnd(QTextCursor mark, SharedEnd end)
{
  if (end) {
    m_end = end->end();
    m_data.append(end);
    m_nodes.insert(end->start(), end);
  } else {
    m_end = mark;
    m_implicitEnd = false;
  }
}

bool
QYamlDocument::hasEnd()
{
  return !m_end.isNull();
}

int
QYamlDocument::textLength()
{
  return m_end.position() - m_start.position();
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

QList<SharedNode>
QYamlDocument::nodes() const
{
  return m_data;
}

SharedNode
QYamlDocument::node(int index)
{
  return m_data.at(index);
}

SharedNode
QYamlDocument::node(QTextCursor cursor)
{
  return m_nodes.value(cursor, nullptr);
}

bool
QYamlDocument::addNode(SharedNode data, bool root)
{
  switch (data->type()) {
    case YamlNode::Comment:
      m_data.append(data);
      if (root)
        m_root.append(data);
      m_nodes.insert(data->start(), data);
      return true;
    case YamlNode::Scalar:
      m_data.append(data);
      if (root)
        m_root.append(data);
      m_nodes.insert(data->start(), data);
      return true;
    case YamlNode::Sequence: {
      m_data.append(data);
      if (root)
        m_root.append(data);
      return addSequenceData(qSharedPointerDynamicCast<YamlSequence>(data));
    }
    case YamlNode::Map: {
      m_data.append(data);
      if (root)
        m_root.append(data);
      return addMapData(qSharedPointerDynamicCast<YamlMap>(data));
    }
    default:
      return false;
  }
}

void
QYamlDocument::addDirective(SharedNode directive)
{
  auto yaml = qSharedPointerDynamicCast<YamlYamlDirective>(directive);
  if (yaml) {
    if (m_yaml.isEmpty()) {
      //      directive->setError(YamlError::TooManyYamlDirectivesError, true);
      //    } else {
      m_directive = yaml;
    }
    m_yaml.insert(directive->start(), yaml);
    m_nodes.insert(directive->start(), yaml);
    m_data.append(yaml);
    return;
  }
  auto tag = qSharedPointerDynamicCast<YamlTagDirective>(directive);
  if (tag) {
    m_tags.insert(tag->start(), tag);
    m_nodes.insert(tag->start(), tag);
    m_data.append(tag);
  }
  auto reserved = qSharedPointerDynamicCast<YamlReservedDirective>(directive);
  if (reserved) {
    m_reserved.insert(reserved->start(), reserved);
    m_nodes.insert(reserved->start(), reserved);
    m_data.append(reserved);
  }
}

bool
QYamlDocument::addSequenceData(QSharedPointer<YamlSequence> sequence,
                               QSharedPointer<YamlMapItem> item)
{
  if (item) // sub sequence in map
    m_nodes.insert(item->start(), item);
  else
    m_nodes.insert(sequence->start(), sequence);

  for (auto data : sequence->data()) {
    if (data) {
      switch (data->type()) {
        case YamlNode::Comment:
          //        m_nodes.insert(data->start(), data);
          break;
        case YamlNode::Scalar:
          m_data.append(data);
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
  }
  return false;
}

bool
QYamlDocument::addMapData(QSharedPointer<YamlMap> map,
                          QSharedPointer<YamlMapItem> item)
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
          m_data.append(data);
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
QYamlDocument::addMapItemData(QSharedPointer<YamlMapItem> item)
{
  if (item) {
    auto data = item->data();
    if (data) {
      switch (data->type()) {
        case YamlNode::Comment:
          //          m_nodes.insert(data->start(), data);
          return true;
        case YamlNode::Scalar:
          m_data.append(data);
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

const QMap<QTextCursor, SharedNode>&
QYamlDocument::nodeMap() const
{
  return m_nodes;
}

QMap<QTextCursor, SharedTagDirective>
QYamlDocument::tags() const
{
  return m_tags;
}

void
QYamlDocument::setTags(const QMap<QTextCursor, SharedTagDirective>& tags)
{
  m_tags = tags;
  for (auto [key, tag] : asKeyValueRange(tags)) {
    m_data.append(tag);
    m_nodes.insert(key, tag);
  }
}

void
QYamlDocument::addTag(SharedTagDirective tag)
{
  m_tags.insert(tag->start(), tag);
  m_root.append(tag);
  m_data.append(tag);
  m_nodes.insert(tag->start(), tag);
}

bool
QYamlDocument::hasTag()
{
  return !m_tags.isEmpty();
}

void
QYamlDocument::removeTag(QTextCursor position)
{
  m_tags.remove(position);
}

QMap<QTextCursor, SharedReservedDirective> QYamlDocument::reserved() const
{
  return m_reserved;
}

void QYamlDocument::addReserved(const QMap<QTextCursor, SharedReservedDirective> &reserved)
{
  m_reserved = reserved;
}

bool QYamlDocument::hasReserved()
{
  return !m_reserved.isEmpty();
}

void QYamlDocument::removeReserved(QTextCursor position)
{
  m_reserved.remove(position);
}

SharedYamlDirective
QYamlDocument::getDirective() const
{
  return m_directive;
}

bool
QYamlDocument::hasDirective()
{
  return (m_directive != nullptr);
}

void
QYamlDocument::setDirective(SharedYamlDirective directive)
{
  this->m_directive = directive;
  m_root.append(directive);
  m_data.append(directive);
  m_nodes.insert(directive->start(), m_directive);
}

QTextCursor
QYamlDocument::versionStart() const
{
  return m_directive->start();
}

int
QYamlDocument::versionStartPos() const
{
  return versionStart().position();
}

// void QYamlDocument::setVersionStart(const QTextCursor &versionStart) {
//   m_versionStart = versionStart;
// }

int
QYamlDocument::versionLength() const
{
  return m_directive->length();
}

// void QYamlDocument::setVersionLength(int versionLength) {
//   m_versionLength = versionLength;
// }
