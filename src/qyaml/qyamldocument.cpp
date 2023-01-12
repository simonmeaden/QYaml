#include "qyaml/qyamldocument.h"
#include "qyaml/yamlnode.h"
#include "utilities/ContainerUtil.h"

//====================================================================
//=== QYamlDocument
//====================================================================
QYamlDocument::QYamlDocument(QObject *parent) : QObject(parent) {}

int QYamlDocument::majorVersion() const { return m_directive->major(); }

// void QYamlDocument::setMajorVersion(int Major) {
//   m_majorVersion = Major;
//   m_implicitVersion = false;
// }

int QYamlDocument::minorVersion() const { return m_directive->minor(); }

// void QYamlDocument::setMinorVersion(int Minor) {
//   m_minorVersion = Minor;
//   m_implicitVersion = false;
// }

bool QYamlDocument::implicitStart() const { return m_implicitStart; }

void QYamlDocument::setImplicitStart(bool ExplicitStart) {
  m_implicitStart = ExplicitStart;
}

bool QYamlDocument::isImplicitVersion() const { return m_implicitVersion; }

void QYamlDocument::setImplicitVersion(bool implicitVersion) {
  m_implicitVersion = implicitVersion;
}

QTextCursor QYamlDocument::start() { return m_start; }

int QYamlDocument::startPos() { return m_start.position(); }

void QYamlDocument::setStart(QTextCursor position, YamlStart *start) {
  m_start = position;
  m_implicitStart = false;
  if (start) {
    m_nodes.insert(start->start(), start);
  }
}

QTextCursor QYamlDocument::end() { return m_end; }

int QYamlDocument::endPos() { return m_end.position(); }

void QYamlDocument::setEnd(QTextCursor mark, YamlEnd *end) {
  if (end) {
    m_end = end->end();
    m_nodes.insert(end->start(), end);
  } else {
    m_end = mark;
    m_implicitEnd = false;
  }
}

int QYamlDocument::textLength() {
  return m_end.position() - m_start.position();
}

bool QYamlDocument::implicitEnd() const { return m_implicitEnd; }

void QYamlDocument::setImplicitEnd(bool ImplicitEnd) {
  m_implicitEnd = ImplicitEnd;
}

bool QYamlDocument::getExplicitTags() const { return explicitTags; }

void QYamlDocument::setExplicitTags(bool ExplicitTags) {
  explicitTags = ExplicitTags;
}

QList<YamlNode *> QYamlDocument::data() const { return m_data; }

YamlNode *QYamlDocument::data(int index) { return m_data.at(index); }

bool QYamlDocument::addData(YamlNode *data) {
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
    return addSequenceData(qobject_cast<YamlSequence *>(data));
  }
  case YamlNode::Map: {
    m_data.append(data);
    return addMapData(qobject_cast<YamlMap *>(data));
  }
  default:
    return false;
  }
}

bool QYamlDocument::addSequenceData(YamlSequence *sequence, YamlMapItem *item) {
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
      addSequenceData(qobject_cast<YamlSequence *>(data));
      break;
    case YamlNode::Map:
      addMapData(qobject_cast<YamlMap *>(data));
      break;
    default:
      return false; // should only happen on error.
    }
  }
  return false;
}

bool QYamlDocument::addMapData(YamlMap *map, YamlMapItem *item) {
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
        addSequenceData(qobject_cast<YamlSequence *>(data), i);
        break;
      }
      case YamlNode::Map: {
        addMapData(qobject_cast<YamlMap *>(data), i);
        break;
      }
      case YamlNode::MapItem: {
        bool r = addMapItemData(qobject_cast<YamlMapItem *>(data));
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

bool QYamlDocument::addMapItemData(YamlMapItem *item) {
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
        return addSequenceData(qobject_cast<YamlSequence *>(data));
      }
      case YamlNode::Map: {
        return addMapData(qobject_cast<YamlMap *>(data));
      }
      default:
        return false;
      }
    }
  }
  return false;
}

const YamlErrors &QYamlDocument::errors() const { return m_errors; }

void QYamlDocument::setError(const YamlError &error, bool set) {
  m_errors.setFlag(error, set);
}

void QYamlDocument::setErrors(const YamlErrors &newErrors) {
  m_errors = newErrors;
}

const YamlWarnings &QYamlDocument::warnings() const { return m_warnings; }

void QYamlDocument::setWarning(const YamlWarning &warning, bool set) {
  m_warnings.setFlag(warning, set);
}

void QYamlDocument::setWarnings(const YamlWarnings &newWarnings) {
  m_warnings = newWarnings;
}

const QMap<QTextCursor, YamlNode *> &QYamlDocument::nodes() const {
  return m_nodes;
}

QMap<QTextCursor, YamlTagDirective *> QYamlDocument::tags() const {
  return m_tags;
}

void QYamlDocument::setTags(const QMap<QTextCursor, YamlTagDirective *> &tags) {
  m_tags = tags;
  for (auto [key, tag] : asKeyValueRange(tags)) {
    m_nodes.insert(key, tag);
  }
}

void QYamlDocument::addTag(QTextCursor position, YamlTagDirective *tag) {
  m_tags.insert(position, tag);
  m_nodes.insert(position, tag);
}

void QYamlDocument::removeTag(QTextCursor position) { m_tags.remove(position); }

YamlDirective *QYamlDocument::getDirective() const { return m_directive; }

void QYamlDocument::setDirective(YamlDirective *directive) {
  this->m_directive = directive;
  m_nodes.insert(directive->start(), m_directive);
}

QTextCursor QYamlDocument::versionStart() const { return m_directive->start(); }

int QYamlDocument::versionStartPos() const { return versionStart().position(); }

// void QYamlDocument::setVersionStart(const QTextCursor &versionStart) {
//   m_versionStart = versionStart;
// }

int QYamlDocument::versionLength() const { return m_directive->length(); }

// void QYamlDocument::setVersionLength(int versionLength) {
//   m_versionLength = versionLength;
// }
