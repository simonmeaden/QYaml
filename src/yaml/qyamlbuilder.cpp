#include "SMLibraries/widgets/yaml/yamlbuilder.h"
#include "SMLibraries/utilities/characters.h"

#include <JlCompress.h>
#include <cstdlib>

//====================================================================
//=== YamlBuilder
//====================================================================
YamlBuilder::YamlBuilder(QObject* parent)
  : QObject{ parent }
{
}

QString
YamlBuilder::inlinePrint() const
{
  return QString();
}

QString
YamlBuilder::prettyPrint() const
{
  return QString();
}

// YamlNode*
// YamlBuilder::parseMap(YAML::Node node)
//{
//   auto map = new YamlMap(this);
//   for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
//     auto n = (*it);
//     QString key;
//     if (n.first.IsScalar()) {
//       key = QString::fromStdString(n.first.Scalar());
//     }
//     auto value = parseNode(n.second);
//     map->insert(key, value);
//     //    switch (n.second.Type()) {
//     //      case YAML::NodeType::Scalar: {
//     //        auto value = parseScalar(n.second);
//     //        map->insert(key, value);
//     //        break;
//     //      }
//     //      case YAML::NodeType::Sequence: {
//     //        auto value = parseSequence(n.second);
//     //        map->insert(key, value);
//     //        break;
//     //      }
//     //      case YAML::NodeType::Map: {
//     //        auto value = parseMap(n.second);
//     //        map->insert(key, value);
//     //        break;
//     //      }
//     //      case YAML::NodeType::Undefined:
//     //        break; // TODO indicate error
//     //      case YAML::NodeType::Null:
//     //        break; // TODO indicate error
//     //    }
//   }

//  return map;
//}

// YamlNode*
// YamlBuilder::parseSequence(YAML::Node node)
//{
//   auto seq = new YamlSequence(this);
//   for (std::size_t i = 0; i < node.size(); i++) {
//     auto n = node[i];
//     auto value = parseNode(n);
//     seq->append(value);
//     //    switch (n.Type()) {
//     //      case YAML::NodeType::Scalar: {
//     //        auto value = parseScalar(n);
//     //        seq->append(value);
//     //        break;
//     //      }
//     //      case YAML::NodeType::Sequence: {
//     //        auto value = parseSequence(n);
//     //        seq->append(value);
//     //        break;
//     //      }
//     //      case YAML::NodeType::Map: {
//     //        auto value = parseMap(n);
//     //        seq->append(value);
//     //        break;
//     //      }
//     //      case YAML::NodeType::Undefined:
//     //        return nullptr; // TODO indicate error
//     //      case YAML::NodeType::Null:
//     //        return nullptr; // TODO indicate error
//     //    }
//   }
//   return seq;
// }

// YamlNode*
// YamlBuilder::parseComment(YAML::Node node)
//{
//   return new YamlComment(this);
// }

// YamlNode*
// YamlBuilder::parseScalar(YAML::Node node)
//{
//   auto scalar = node.Scalar();
//   auto tag = node.Tag();
//   auto yamlNode = new YamlScalar(
//     QString::fromStdString(scalar), QString::fromStdString(tag), this);
//   return yamlNode;
// }

// YamlNode*
// YamlBuilder::parseNode(YAML::Node node)
//{
//   YamlNode* value;
//   switch (node.Type()) {
//     case YAML::NodeType::Scalar: {
//       value = parseScalar(node);
//       value->setMark(node.Mark());
//       break;
//     }
//     case YAML::NodeType::Sequence: {
//       value = parseSequence(node);
//       value->setMark(node.Mark());
//       break;
//     }
//     case YAML::NodeType::Map: {
//       value = parseMap(node);
//       value->setMark(node.Mark());
//       break;
//     }
//     case YAML::NodeType::Undefined:
//       return nullptr; // TODO indicate error
//     case YAML::NodeType::Null:
//       return nullptr; // TODO indicate error
//   }
//   return value;
// }

YamlDocument*
YamlBuilder::parseDocumentStart(struct fy_event* event)
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
    document->startMark().pos = startMark->input_pos;
    document->startMark().line = startMark->line;
    document->startMark().column = startMark->column;
  }

  document->setImplicitEnd(fy_document_state_end_implicit(docState));
  auto endMark = fy_document_state_end_mark(docState);
  if (endMark) {
    document->endMark().pos = endMark->input_pos;
    document->endMark().line = endMark->line;
    document->endMark().column = endMark->column;
  }

  auto explicitTags = fy_document_state_tags_explicit(docState);
  document.setexp if (explicitTags)
  {
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

bool
YamlBuilder::parse(const char* text)
{
  struct fy_parse_cfg cfg;
  struct fy_diag_cfg dcfg;
  struct fy_parser* parser = nullptr;
  struct fy_emitter* emitter = nullptr;
  struct fy_document* doc = nullptr;
  struct fy_event* event = nullptr;
  struct fy_path_component *parent, *last;

  cfg.search_path = "";
  cfg.flags = fy_parse_cfg_flags(0 | FYPCF_PARSE_COMMENTS | FYPCF_JSON_AUTO |
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
  rc = fy_parser_set_string(parser, text, FY_NT);
  if (rc) {
    m_errors.setFlag(FatalLoadStringError, true);
    return false;
  }

  YamlDocument* document = nullptr;



//  event = fy_parser_parse(parser);
//  while (event != NULL) {
//    switch (event->type) {
//      case FYET_NONE: {
//        break;
//      }
//      case FYET_STREAM_START: {
//        break;
//      }
//      case FYET_STREAM_END: {
//        break;
//      }
//      case FYET_DOCUMENT_START: {
//        document = parseDocumentStart(event);
//        break;
//      }
//      case FYET_DOCUMENT_END: {
//        if (document) {
//          m_documents.append(document);
//          document = nullptr;
//        }
//        break;
//      }
//      case FYET_MAPPING_START: {
//        if (document) {
//          const char* anchor = NULL;
//          const char* tag = NULL;
//          //          const char* text = NULL;
//          size_t anchor_len = 0, tag_len = 0, text_len = 0;
//          enum fy_scalar_style style;

//          auto a = event->mapping_start.anchor;
//          if (a) {
//            anchor =
//              fy_token_get_text(event->mapping_start.anchor, &anchor_len);
//            qWarning();
//          }

//          auto t = event->mapping_start.tag;
//          if (t) {
//            tag = fy_token_get_text(event->mapping_start.tag, &tag_len);
//            qWarning();
//          }

//          fy_node_style node_style = fy_event_get_node_style(event);
//          switch (node_style) {
//            case FYNS_ANY:
//              break;
//            case FYNS_FLOW:
//              break;
//            case FYNS_BLOCK:
//              break;
//            case FYNS_PLAIN:
//              break;
//            case FYNS_SINGLE_QUOTED:
//              break;
//            case FYNS_DOUBLE_QUOTED:
//              break;
//            case FYNS_LITERAL:
//              break;
//            case FYNS_FOLDED:
//              break;
//            case FYNS_ALIAS:
//              break;
//          }

//          if (anchor) {
//            printf(" &%.*s", (int)anchor_len, anchor);
//          }
//          if (tag) {
//            printf(" <%.*s>", (int)tag_len, tag);
//          }
//        }
//        break;
//      }
//      case FYET_MAPPING_END: {
//        break;
//      }
//      case FYET_SEQUENCE_START: {
//        break;
//      }
//      case FYET_SEQUENCE_END: {
//        break;
//      }
//      case FYET_SCALAR: {
//        const char* anchor = NULL;
//        const char* tag = NULL;
//        const char* text = NULL;
//        size_t anchor_len = 0, tag_len = 0, text_len = 0;
//        enum fy_scalar_style style;

//        auto aToken = event->scalar.anchor;
//        if (aToken)
//          anchor = fy_token_get_text(event->scalar.anchor, &anchor_len);

//        auto tToken = event->scalar.tag;
//        if (tToken)
//          tag = fy_token_get_text(event->scalar.tag, &tag_len);

//        style = fy_token_scalar_style(event->scalar.value);
//        switch (style) {
//          case FYSS_PLAIN:

//            break;
//          case FYSS_SINGLE_QUOTED:

//            break;
//          case FYSS_DOUBLE_QUOTED:

//            break;
//          case FYSS_LITERAL:

//            break;
//          case FYSS_FOLDED:

//            break;
//          default:
//            break;
//        }

//        text = fy_token_get_text(event->scalar.value, &text_len);
//        //        if (text && text_len > 0)
//        //          print_escaped(text, text_len);
//        break;
//      }
//      case FYET_ALIAS: {
//        break;
//      }
//    }
//    fy_parser_event_free(parser, event);
//    event = fy_parser_parse(parser);
//  }

  if (document) {
    // This shouldn't happen in a good document.
    m_documents.append(document);
    document = nullptr;
  }

  //  auto docs = text.split("---",Qt::SkipEmptyParts);
  //  if (docs.size() > 1) {
  //    for (auto doc : docs) {
  //      auto root = YAML::Load(text);
  //      auto node = parseNode(root);
  //      m_documents.append(node);
  //    }
  //  } else {
  //    auto root = YAML::Load(text);
  //    auto node = parseNode(root);
  //    m_documents.append(node);
  //  }
  return true;
}

bool
YamlBuilder::parse(const QString& text)
{
  m_text = text;
  const char* cText;
  auto s = text.toStdString();
  cText = s.c_str();

  auto result = parse(cText);
  return result;
}

const QString
YamlBuilder::filename() const
{
  return m_filename;
}

bool
YamlBuilder::loadFile(const QString& filename)
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
YamlBuilder::loadFromZip(const QString& zipFile, const QString& href)
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
YamlBuilder::root() const
{
  return m_documents;
}

void
YamlBuilder::setRoot(QList<YamlDocument*> root)
{
  m_documents = root;
}

QString
YamlBuilder::text() const
{
  return m_text;
}

//====================================================================
//=== YamlNode
//====================================================================
YamlNode::YamlNode(QObject* parent)
  : QObject(parent)
  , m_type(None)
{
}

YamlNode::Type
YamlNode::type() const
{
  return m_type;
}

void
YamlNode::setType(Type type)
{
  m_type = type;
}

int
YamlNode::getColumn() const
{
  return m_column;
}

void
YamlNode::setColumn(int Column)
{
  m_column = Column;
}

int
YamlNode::getLine() const
{
  return m_line;
}

void
YamlNode::setLine(int Line)
{
  m_line = Line;
}

int
YamlNode::getPos() const
{
  return m_pos;
}

void
YamlNode::setPos(int Pos)
{
  m_pos = Pos;
}

// void
// YamlNode::setMark(YAML::Mark mark)
//{
//   m_pos = mark.pos;
//   m_line = mark.line;
//   m_column = mark.column;
// }

//====================================================================
//=== YamlMap
//====================================================================
YamlMap::YamlMap(QObject* parent)
  : YamlNode(parent)
{
  setType(Map);
}

QString
YamlMap::inlinePrint() const
{
  QString result;
  result += Characters::OPEN_SQUARE_BRACKET;
  for (auto value : m_values) {
    result += value->inlinePrint();
    result += Characters::COMMA;
  }
  result += Characters::CLOSE_SQUARE_BRACKET;
  return result;
}

QString
YamlMap::prettyPrint() const
{
  QString result;
  for (auto value : m_values) {
    result += value->inlinePrint();
    result += Characters::COMMA;
  }
  return result;
}

YamlNode*
YamlMap::value(const QString& key)
{
  return m_values.value(key);
}

void
YamlMap::insert(const QString& key, YamlNode* value)
{
  m_values.insert(key, value);
}

void
YamlMap::insert(const QString& key, const QString& value)
{
  m_values.insert(key, new YamlScalar(value, this));
}

//====================================================================
//=== YamlSequence
//====================================================================
YamlSequence::YamlSequence(QObject* parent)
  : YamlNode(parent)
{
  setType(Sequence);
}

QString
YamlSequence::inlinePrint() const
{
  return QString();
}

QString
YamlSequence::prettyPrint() const
{
  return QString();
}

YamlNode*
YamlSequence::value(int index)
{
  return m_values.at(index);
}

void
YamlSequence::append(YamlNode* value)
{
  m_values.append(value);
}

void
YamlSequence::append(const QString& value)
{
  auto s = new YamlScalar(value, this);
  m_values.append(s);
}

//====================================================================
//=== YamlScalar
//====================================================================
YamlScalar::YamlScalar(QObject* parent)
  : YamlNode(parent)
{
  setType(Scalar);
}

YamlScalar::YamlScalar(const QString& value, QObject* parent)
  : YamlNode(parent)
{
  setValue(value);
}

YamlScalar::YamlScalar(const QString& value,
                       const QString& tag,
                       QObject* parent)
  : YamlNode(parent)
{
  setValue(value);
  setTag(tag);
}

QString
YamlScalar::inlinePrint() const
{
  return QString();
}

QString
YamlScalar::prettyPrint() const
{
  return QString();
}

QString
YamlScalar::name() const
{
  return m_name;
}

void
YamlScalar::setName(const QString& Name)
{
  m_name = Name;
}

QVariant
YamlScalar::value() const
{
  return m_value;
}

void
YamlScalar::setValue(const QString& Value)
{
  m_value = Value;
}

QString
YamlScalar::tag() const
{
  return m_tag;
}

void
YamlScalar::setTag(const QString& Tag)
{
  m_tag = Tag;
}

//====================================================================
//=== YamlInteger
//====================================================================
YamlInteger::YamlInteger(QObject* parent)
  : YamlScalar(parent)
{
}

QString
YamlInteger::inlinePrint() const
{
  return QString();
}

QString
YamlInteger::prettyPrint() const
{
  return QString();
}

void
YamlInteger::setValue(int& Value)
{
}

//====================================================================
//=== YamlReal
//====================================================================
YamlReal::YamlReal(QObject* parent)
  : YamlScalar(parent)
{
}

QString
YamlReal::inlinePrint() const
{
  return QString();
}

QString
YamlReal::prettyPrint() const
{
  return QString();
}

void
YamlReal::setValue(qreal& Value)
{
}

//====================================================================
//=== YamlComment
//====================================================================
YamlComment::YamlComment(QObject* parent)
  : YamlNode(parent)
{
  setType(Comment);
}

QString
YamlComment::inlinePrint() const
{
  return QString();
}

QString
YamlComment::prettyPrint() const
{
  return QString();
}

QString
YamlComment::value() const
{
  return m_value;
}

void
YamlComment::setValue(const QString& Value)
{
  m_value = Value;
}

//====================================================================
//=== YamlAlias
//====================================================================
YamlAlias::YamlAlias(QObject* parent)
  : YamlNode(parent)
{
}

YamlNode*
YamlAlias::alias()
{
  return m_alias;
}

void
YamlAlias::setAlias(YamlNode* node)
{
  m_alias = node;
}

QString
YamlAlias::inlinePrint() const
{
  return QString();
}

QString
YamlAlias::prettyPrint() const
{
  return QString();
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

constexpr inline YamlMark&
YamlDocument::startMark() noexcept
{
  return m_start;
}

constexpr inline YamlMark&
YamlDocument::endMark() noexcept
{
  return m_start;
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
