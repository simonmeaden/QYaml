#include "qyaml/qyamlbuilder.h"
#include "SMLibraries/utilities/characters.h"

#include <JlCompress.h>
#include <cstdlib>

//====================================================================
//=== QYamlBuilder
//====================================================================
QYamlBuilder::QYamlBuilder(QObject* parent)
  : QObject{ parent }
{
}

QString
QYamlBuilder::inlinePrint() const
{
  return QString();
}

QString
QYamlBuilder::prettyPrint() const
{
  return QString();
}

YamlDocument*
QYamlBuilder::parseDocumentStart(struct fy_event* event)
{
  auto document = new YamlDocument(this);
  auto docState = event->document_start.document_state;

  document->setExplicitVersion(fy_document_state_version_explicit(docState));
  auto fyversion = fy_document_state_version(docState);
  if (fyversion) {
    document->setMajorVersion(fyversion->major);
    document->setMinorVersion(fyversion->minor);
  }

  document->setImplicitStart(fy_document_state_start_implicit(docState));
  auto startMark = fy_document_state_start_mark(docState);
  if (startMark) {
    YamlMark mark;
    mark.pos = startMark->input_pos;
    mark.line = startMark->line;
    mark.column = startMark->column;
    document->setStartMark(mark);
  }

  document->setImplicitEnd(fy_document_state_end_implicit(docState));
  auto endMark = fy_document_state_end_mark(docState);
  if (endMark) {
    YamlMark mark;
    mark.pos = endMark->input_pos;
    mark.line = endMark->line;
    mark.column = endMark->column;
    document->setEndMark(mark);
  }

  auto explicitTags = fy_document_state_tags_explicit(docState);
  if (explicitTags) {
    void* prev = nullptr;
    const struct fy_tag* tagp = nullptr;
    if ((tagp = fy_document_state_tag_directive_iterate(docState, &prev)) !=
        NULL) {
      printf(" TDs: [");
      do {
        printf(" \"%s\",\"%s\"", tagp->handle, tagp->prefix);
      } while ((tagp = fy_document_state_tag_directive_iterate(docState,
                                                               &prev)) != NULL);
      printf(" ]");
    }
  }
  return document;
}

YamlMark
QYamlBuilder::startMark()
{
  return m_start;
}

void
QYamlBuilder::setStartMark(YamlMark mark)
{
  m_start = mark;
}

YamlMark
QYamlBuilder::endMark()
{
  return m_end;
}

void
QYamlBuilder::setEndMark(YamlMark mark)
{
  m_end = mark;
}

bool
QYamlBuilder::parse(const char* data)
{
  struct fy_parse_cfg cfg;
  struct fy_diag_cfg dcfg;
  struct fy_parser* parser = NULL;
  struct fy_event* event = NULL;
  const char* text = NULL;
  const char* anchor = NULL;
  size_t anchor_len = 0;

  cfg.search_path = "";
  cfg.flags = fy_parse_cfg_flags(FYPCF_PARSE_COMMENTS | FYPCF_JSON_AUTO |
                                 FYPCF_RESOLVE_DOCUMENT | FYPCF_COLLECT_DIAG);

  fy_diag_cfg_default(&dcfg);
  auto diag = fy_diag_create(&dcfg);
  cfg.diag = diag;

  parser = fy_parser_create(&cfg);
  if (!parser) {
    m_errors.setFlag(FatalParserCreationError, true);
    return false;
  }

  int rc;
  rc = fy_parser_set_string(parser, data, FY_NT);
  if (rc) {
    m_errors.setFlag(FatalLoadStringError, true);
    return false;
  }

  YamlDocument* document = nullptr;
  YamlMap* map = nullptr;
  YamlSequence* sequence = nullptr;
  QString key;

  event = fy_parser_parse(parser);

  while (event != NULL) {
    switch (event->type) {
      case FYET_NONE: {
        break;
      }
      case FYET_STREAM_START: {
        assignStartMark(event->stream_start.stream_start, m_start);
        parseComment(event->stream_start.stream_start);
        break;
      }
      case FYET_STREAM_END: {
        assignEndMark(event->stream_end.stream_end, m_end);
        parseComment(event->stream_end.stream_end);
        break;
      }
      case FYET_DOCUMENT_START: {
        document = parseDocumentStart(event);
        if (document)
          assignStartMark(event->document_start.document_start, document);
        parseComment(event->document_start.document_start);
        break;
      }
      case FYET_DOCUMENT_END: {
        if (document) {
          //          assignEndMark(event->document_end.document_end, document);
          m_documents.append(document);
          document = nullptr;
        }
        parseComment(event->document_end.document_end);
        break;
      }
      case FYET_MAPPING_START: {
        if (event->mapping_start.anchor) {
          anchor = fy_token_get_text(event->mapping_start.anchor, &anchor_len);
        }

        // free up the event and get the next
        fy_parser_event_free(parser, event);
        event = fy_parser_parse(parser);

        // parse and return the map
        auto map = new YamlMap(this);
        assignStartMark(event->mapping_start.mapping_start, map);
        parseMap(event, parser, map);
        parseComment(event->mapping_start.mapping_start);
        break;
      }
      case FYET_MAPPING_END: {
        if (document) {
          document->addData(map);
          map = nullptr;
        }
        parseComment(event->mapping_end.mapping_end);
        break;
      }
      case FYET_SEQUENCE_START: {
        size_t anchor_len = 0;
        if (event->sequence_start.anchor) {
          anchor = fy_token_get_text(event->sequence_start.anchor, &anchor_len);
        }

        // free up the event and get the next
        fy_parser_event_free(parser, event);
        event = fy_parser_parse(parser);

        // parse and return the map
        auto sequence = new YamlSequence(this);
        if (document) {
          assignStartMark(event->sequence_start.sequence_start, sequence);
          document->addData(sequence);
        }
        parseSequence(event, parser, sequence);
        parseComment(event->sequence_start.sequence_start);
        break;
      }
      case FYET_SEQUENCE_END: {
        assignEndMark(event->sequence_end.sequence_end, sequence);
        parseComment(event->sequence_end.sequence_end);

        break;
      }
      case FYET_SCALAR: {
        anchor = fy_token_get_text(event->scalar.anchor, &anchor_len);

        YamlScalar* value;
        if (map) {
          YamlMark keyStart, keyEnd;
          text = fy_token_get_text0(event->scalar.value);
          if (text && key.isEmpty()) {
            auto mark = fy_token_start_mark(event->scalar.value);
            if (mark) {
              keyStart.pos = mark->input_pos;
              keyStart.line = mark->line;
              keyStart.column = mark->column;
            }
            mark = fy_token_end_mark(event->scalar.value);
            if (mark) {
              keyEnd.pos = mark->input_pos;
              keyEnd.line = mark->line;
              keyEnd.column = mark->column;
            }
            anchor = fy_token_get_text(event->scalar.anchor, &anchor_len);
            key = text; // possible mark for key.
          } else {
            value = new YamlScalar(text, this);
            anchor = fy_token_get_text(event->scalar.anchor, &anchor_len);
            assignStartMark(event->scalar.value, value);
            assignEndMark(event->scalar.value, value);
            if (!map->contains(key)) {
              map->insert(key, value, keyStart, keyEnd);
              key.clear();
            } else {
              // TODO invalid key.
              qWarning();
            }
          }
        } else if (sequence) {
          text = fy_token_get_text0(event->scalar.value);
          if (text) {
            value = new YamlScalar(text, this);
            sequence->append(value);
          }
        } else {
          value = new YamlScalar(text, this);
          document->addData(value);
        }

        size_t len = 0;
        text = fy_token_get_text(event->scalar.value, &len);
        if (text && len > 0)
          parseComment(event->scalar.value);

        break;
      }

      case FYET_ALIAS: {
        anchor = fy_token_get_text(event->alias.anchor, &anchor_len);

        break;
      }

      default:
        break;

    } // end switch
    fy_parser_event_free(parser, event);
    event = fy_parser_parse(parser);
  } // end while

  if (document) {
    // This shouldn't happen in a good document.
    m_documents.append(document);
    document = nullptr;
  }

  return true;
}

YamlSequence*
QYamlBuilder::parseSequence(struct fy_event* event,
                            struct fy_parser* parser,
                            YamlSequence* sequence)
{
  const char* text = NULL;
  QString key;
  YamlNode* value;

  while (event != NULL) {
    switch (event->type) {
      case FYET_MAPPING_START: {
        // free up the event and get the next
        fy_parser_event_free(parser, event);
        event = fy_parser_parse(parser);

        auto subMap = new YamlMap(this);
        assignStartMark(event->mapping_start.mapping_start, subMap);
        sequence->append(subMap);
        break;
      }
        //    case FYET_MAPPING_END:
        //      not needed here
        //      break;
      case FYET_SEQUENCE_START: {
        // free up the event and get the next
        fy_parser_event_free(parser, event);
        event = fy_parser_parse(parser);

        auto subSequence = new YamlSequence(this);
        assignStartMark(event->sequence_start.sequence_start, sequence);
        parseSequence(event, parser, subSequence);
        sequence->append(subSequence);
        break;
      }
      case FYET_SEQUENCE_END: {
        return sequence;
      }
      case FYET_SCALAR: {
        if (sequence) {
          text = fy_token_get_text0(event->scalar.value);
          if (text) {
            value = new YamlScalar(text, this);
            sequence->append(value);
          }
        }
        break;
      }
      default:
        break;
    }
    fy_parser_event_free(parser, event);
    event = fy_parser_parse(parser);
  }

  return sequence;
}

YamlMap*
QYamlBuilder::parseMap(fy_event* event, fy_parser* parser, YamlMap* map)
{
  const char* text = NULL;
  QString key;
  YamlMark keyStart, keyEnd;

  while (event != NULL) {
    switch (event->type) {
      case FYET_MAPPING_START: {
        // free up the event and get the next
        fy_parser_event_free(parser, event);
        event = fy_parser_parse(parser);

        auto subMap = new YamlMap(this);
        assignStartMark(event->mapping_start.mapping_start, subMap);
        parseMap(event, parser, map);
        if (!key.isEmpty()) {
          map->insert(key, subMap);
          key.clear();
        }
        break;
      }
      case FYET_MAPPING_END: {
        assignEndMark(event->mapping_end.mapping_end, map);
        return map;
      }
      case FYET_SEQUENCE_START: {
        // free up the event and get the next
        fy_parser_event_free(parser, event);
        event = fy_parser_parse(parser);

        auto subSequence = new YamlSequence(this);
        assignStartMark(event->sequence_start.sequence_start, subSequence);
        parseSequence(event, parser, subSequence);
        if (!key.isEmpty()) {
          map->insert(key, subSequence);
          key.clear();
        }
        break;
      }
      case FYET_SCALAR: {
        if (map) {
          YamlScalar* value;
          text = fy_token_get_text0(event->scalar.value);
          if (text && key.isEmpty()) {
            assignStartMark(event->scalar.value, keyStart);
            assignEndMark(event->scalar.value, keyEnd);
            key = text;
          } else {
            if (!map->contains(key)) {
              value = new YamlScalar(text, this);
              assignStartMark(event->scalar.value, value);
              assignEndMark(event->scalar.value, value);
              map->insert(key, value, keyStart, keyEnd);
              key.clear();
              keyStart.clear();
              keyEnd.clear();
            } else {
              // TODO invalid key.
              qWarning();
            }
          }
        } /* else {
           value = new YamlScalar(text, this);
           map->insert(key, value);
         }*/
        break;
      }
      default:
        break;
    }
    fy_parser_event_free(parser, event);
    event = fy_parser_parse(parser);
  }

  return map;
}

void
QYamlBuilder::assignStartMark(struct fy_token* token, Markable* markable)
{
  if (markable) {
    auto mark = fy_token_start_mark(token);
    if (mark) {
      YamlMark startMark;
      startMark.pos = mark->input_pos;
      startMark.line = mark->line;
      startMark.column = mark->column;
      markable->setStartMark(startMark);
    }
  }
}

void
QYamlBuilder::assignStartMark(struct fy_token* token,
                              KeyMarkable* markable,
                              QString key)
{
  if (markable) {
    auto mark = fy_token_start_mark(token);
    if (mark) {
      YamlMark startMark;
      startMark.pos = mark->input_pos;
      startMark.line = mark->line;
      startMark.column = mark->column;
      markable->setStartMark(key, startMark);
    }
  }
}

void
QYamlBuilder::assignStartMark(fy_token* token, YamlMark& yamlmark)
{
  auto mark = fy_token_start_mark(token);
  if (mark) {
    yamlmark.pos = mark->input_pos;
    yamlmark.line = mark->line;
    yamlmark.column = mark->column;
  }
}

void
QYamlBuilder::assignEndMark(struct fy_token* token,
                            KeyMarkable* markable,
                            QString key)
{
  if (markable) {
    auto mark = fy_token_end_mark(token);
    if (mark) {
      YamlMark endMark;
      endMark.pos = mark->input_pos;
      endMark.line = mark->line;
      endMark.column = mark->column;
      markable->setEndMark(key, endMark);
    }
  }
}

void
QYamlBuilder::assignEndMark(struct fy_token* token, Markable* markable)
{
  if (markable) {
    auto mark = fy_token_end_mark(token);
    if (mark) {
      YamlMark endMark;
      endMark.pos = mark->input_pos;
      endMark.line = mark->line;
      endMark.column = mark->column;
      markable->setEndMark(endMark);
    }
  }
}

void
QYamlBuilder::assignEndMark(fy_token* token, YamlMark& yamlmark)
{
  auto mark = fy_token_end_mark(token);
  if (mark) {
    yamlmark.pos = mark->input_pos;
    yamlmark.line = mark->line;
    yamlmark.column = mark->column;
  }
}

YamlComment*
QYamlBuilder::parseComment(struct fy_token* token)
{
  char buf[4096];
  const char* str;

  if (!token)
    return nullptr;

  // can't iterate over enums so cheat a bit, there are only three values
  for (int i = 0; i < 3; i++) {
    auto placement = fy_comment_placement(i);
    str = fy_token_get_comment(token, buf, sizeof(buf), placement);
    if (str) {
      QString string = str;
      auto comment = new YamlComment(string, this);
      return comment;
    }
    continue;
  }

  return nullptr;
}

bool
QYamlBuilder::parse(const QString& text)
{
  m_text = text;
  const char* cText;
  auto s = text.toStdString();
  cText = s.c_str();

  auto result = parse(cText);
  return result;
}

const QString
QYamlBuilder::filename() const
{
  return m_filename;
}

bool
QYamlBuilder::loadFile(const QString& filename)
{
  m_filename = filename;
  QFile file(m_filename);
  if (file.open(QIODevice::ReadOnly)) {
    QString text = file.readAll();
    return parse(text);
  }
  return false;
}

bool
QYamlBuilder::loadFromZip(const QString& zipFile, const QString& href)
{
  m_filename = href;
  m_zipFile = zipFile;
  auto fileName = JlCompress::extractFile(zipFile, href);
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly)) {
    QString text = file.readAll();
    return parse(text);
  }
  return false;
}

QList<YamlDocument*>
QYamlBuilder::documents() const
{
  return m_documents;
}

QString
QYamlBuilder::text() const
{
  return m_text;
}

void
QYamlBuilder::setDocuments(QList<YamlDocument*> root)
{
  m_documents = root;
}

void
QYamlBuilder::append(YamlDocument* document)
{
  m_documents.append(document);
}

bool
QYamlBuilder::isMultiDocument()
{
  return (m_documents.size() > 1);
}

bool
QYamlBuilder::isEmpty()
{
  return m_documents.empty();
}

int
QYamlBuilder::count()
{
  return m_documents.size();
}

QList<YamlDocument*>::iterator
QYamlBuilder::begin()
{
  return m_documents.begin();
}

QList<YamlDocument*>::const_iterator
QYamlBuilder::constBegin()
{
  return m_documents.constBegin();
}

QList<YamlDocument*>::iterator
QYamlBuilder::end()
{
  return m_documents.end();
}

QList<YamlDocument*>::const_iterator
QYamlBuilder::constEnd()
{
  return m_documents.constEnd();
}

//====================================================================
//=== YamlDocument
//====================================================================
YamlDocument::YamlDocument(QObject* parent)
  : QObject(parent)
{
}

int
YamlDocument::majorVersion() const
{
  return m_majorVersion;
}

void
YamlDocument::setMajorVersion(int Major)
{
  m_majorVersion = Major;
}

int
YamlDocument::minorVersion() const
{
  return m_minorVersion;
}

void
YamlDocument::setMinorVersion(int Minor)
{
  m_minorVersion = Minor;
}

bool
YamlDocument::implicitStart() const
{
  return m_implicitStart;
}

void
YamlDocument::setImplicitStart(bool ExplicitStart)
{
  m_implicitStart = ExplicitStart;
}

bool
YamlDocument::explicitVersion() const
{
  return m_explicitVersion;
}

void
YamlDocument::setExplicitVersion(bool ExplicitVersion)
{
  m_explicitVersion = ExplicitVersion;
}

YamlMark
YamlDocument::startMark()
{
  return m_start;
}

void
YamlDocument::setStartMark(YamlMark mark)
{
  m_start = mark;
}

YamlMark
YamlDocument::endMark()
{
  return m_end;
}

void
YamlDocument::setEndMark(YamlMark mark)
{
  m_end = mark;
}

bool
YamlDocument::implicitEnd() const
{
  return m_implicitEnd;
}

void
YamlDocument::setImplicitEnd(bool ImplicitEnd)
{
  m_implicitEnd = ImplicitEnd;
}

bool
YamlDocument::getExplicitTags() const
{
  return explicitTags;
}

void
YamlDocument::setExplicitTags(bool ExplicitTags)
{
  explicitTags = ExplicitTags;
}

QList<YamlNode*>
YamlDocument::data() const
{
  return m_data;
}

YamlNode
YamlDocument::data(int index) const
{
  return m_data.at(index);
}

void
YamlDocument::addData(YamlNode* data)
{
  m_data.append(data);
}

//====================================================================
//=== Markable
//====================================================================
YamlMark
Markable::startMark()
{
  return m_start;
}

void
Markable::setStartMark(YamlMark mark)
{
  m_start = mark;
}

YamlMark
Markable::endMark()
{
  return m_end;
}

void
Markable::setEndMark(YamlMark mark)
{
  m_end = mark;
}

//====================================================================
//=== KeyMarkable
//====================================================================
YamlMark
KeyMarkable::startMark(const QString& key)
{
  return m_marks.value(key).first;
}

void
KeyMarkable::setStartMark(const QString& key, YamlMark mark)
{
  m_marks.value(key).first = mark;
}

YamlMark
KeyMarkable::endMark(const QString& key)
{
  return m_marks.value(key).second;
}

void
KeyMarkable::setEndMark(const QString& key, YamlMark mark)
{
  m_marks.value(key).second = mark;
}

//====================================================================
//=== YamlNode
//====================================================================
YamlNode::YamlNode(QObject* parent)
  : QObject(parent)
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

//====================================================================
//=== YamlMap
//====================================================================
YamlMap::YamlMap(QObject* parent)
  : YamlNode(parent)
{
}

YamlMap::YamlMap(QMap<QString, YamlNode*> data, QObject* parent)
  : YamlNode(parent)
  , m_data(data)
{
}

QMap<QString, YamlNode*>
YamlMap::data() const
{
  return m_data;
}

void
YamlMap::setData(QMap<QString, YamlNode*> data)
{
  m_data = data;
}

bool
YamlMap::insert(const QString& key, YamlNode* data)
{
  if (data) {
    if (!m_data.contains(key)) {
      m_data.insert(key, data);
      return true;
    }
  }
  return false;
}

bool
YamlMap::insert(const QString& key,
                YamlNode* data,
                YamlMark start,
                YamlMark end)
{
  if (data) {
    insert(key, data);
    if (!m_marks.contains(key)) {
      m_marks.insert(key, qMakePair(start, end));
      return true;
    }
  }
  return false;
}

int
YamlMap::remove(const QString& key)
{
  return m_data.remove(key);
}

YamlNode*
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
//=== YamlSequence
//====================================================================
YamlSequence::YamlSequence(QObject* parent)
  : YamlNode(parent)
{
}

YamlSequence::YamlSequence(QVector<YamlNode*> sequence, QObject* parent)
  : YamlNode(parent)
  , m_data(sequence)
{
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

//====================================================================
//=== YamlScalar
//====================================================================
YamlScalar::YamlScalar(QObject* parent)
  : YamlNode(parent)
{
}

YamlScalar::YamlScalar(QString value, QObject* parent)
  : YamlNode(parent)
  , m_data(value)
{
}

void
YamlScalar::setData(const QString& data)
{
  m_data = data;
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
}

YamlComment::YamlComment(QString value, QObject* parent)
  : YamlNode(parent)
  , m_data(value)
{
}

void
YamlComment::setData(const QString& data)
{
  m_data = data;
}

QString
YamlComment::data() const
{
  return m_data;
}
