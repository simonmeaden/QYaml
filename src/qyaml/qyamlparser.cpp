#include "qyaml/qyamlparser.h"
#include "qyaml/qyamldocument.h"
#include "qyaml/yamlnode.h"
#include "utilities/ContainerUtil.h"

#include <JlCompress.h>

//====================================================================
//=== QYamlParser
//====================================================================
const QString QYamlParser::VERSION_STRING = QStringLiteral("1.2");
const QString QYamlParser::VERSION_PATCH_STRING = QStringLiteral("1.2.2");
// const QString QYamlParser::DOCSTART = QStringLiteral("^[\\s]*---");
// const QString QYamlParser::IND_DOCSTART = QStringLiteral("---");
// const QString QYamlParser::DOCEND = QStringLiteral("^...");
// const QString QYamlParser::IND_DOCEND = QStringLiteral("[\\s]*...");
const QRegularExpression QYamlParser::YAML_DIRECTIVE("%YAML\\s+1[.][0-3]\\s*");
const QRegularExpression QYamlParser::TAG_DIRECTIVE("%TAG\\s+[!\\w\\s:.,]*");

QYamlParser::QYamlParser(QTextDocument* doc, QObject* parent)
  : QObject{ parent }
  , m_document(doc)
{
}

QYamlParser::QYamlParser(QYamlSettings* settings,
                         QTextDocument* doc,
                         QObject* parent)
  : QObject{ parent }
  , m_document(doc)
  , m_settings(settings)
{
}

QString
QYamlParser::inlinePrint() const
{
  // TODO
  return QString();
}

QString
QYamlParser::prettyPrint() const
{
  // TODO
  return QString();
}

bool
QYamlParser::getNextChar(QChar& c, const QString& text, int& i)
{
  i++;
  if (i < text.length()) {
    c = text.at(i);
    if (!c.isNull())
      return true;
  }
  return false;
}

int
QYamlParser::getInitialSpaces(const QString& text,
                              int initialIndent,
                              int& i,
                              QChar& c)
{
  int indent = initialIndent;
  while (getNextChar(c, text, i)) {
    if (c.isSpace())
      indent++;
    else
      return indent;
  }
  return -1;
}

bool
QYamlParser::parse(const QString& text, int startPos, int length)
{
  m_text = text;

  c_printable(Characters::NW_ARROW_BAR);

  auto row = 0;
  auto start = startPos;
  QString t;
  auto tStart = -1;
  auto indent = 0;
  auto rowStart = 0;
  auto pos = 0;
  YamlNode* node = nullptr;
  auto isTagStart = false;
  auto isAnchorStart = false;
  auto isLinkStart = false;
  auto isHyphen = false;
  auto isIndentComplete = false;
  //  QTextCursor cursor;
  auto hasYamlDirective = false;
  QList<YamlNode*> nodes;
  QList<YamlNode*> rootNodes;
  QString key;
  int keyStart = -1;

  for (auto i = start; i < text.size(); i++) {
    auto c = text.at(i);
    if (c == Characters::NEWLINE) {
      // get leading spaces.
      if (indent > 0) {
        // TODO scalar??
      }
      indent = getInitialSpaces(text, 0, i, c);
      i--; // reposition index.
      t.clear();
      continue;
    } else if (c.isSpace()) {
      if (!isIndentComplete) {
        indent++;
        continue;
      }
      t += c;
      continue;
    } else if (c_directive(c)) {
      int start = i;
      auto tag = lookahead(i, text);
      if (tag.startsWith("%YAML ")) {
        auto s = tag.mid(5).trimmed();
        auto splits = s.split(Characters::STOP, Qt::SkipEmptyParts);
        if (splits.size() == 2) {
          bool majorOk, minorOk;
          auto major = splits[0].toInt(&majorOk);
          auto minor = splits[1].toInt(&minorOk);
          auto directive = new YamlDirective(major, minor, this);
          if (!majorOk) {
            // for future expansion ?
            if (major < MIN_VERSION_MAJOR || major > MAX_VERSION_MAJOR) {
              directive->setError(YamlError::InvalidMajorVersion, true);
            }
          }
          if (!minorOk) {
            if (minor < MIN_VERSION_MINOR || minor > MAX_VERSION_MAJOR) {
              directive->setError(YamlError::InvalidMinorVersion, true);
            }
          }
          directive->setStart(createCursor(start));
          directive->setEnd(createCursor(i));
          nodes.append(directive);
          rootNodes.append(directive);
        }
      } else if (tag.startsWith("%TAG ")) {
        auto directive = ns_tag_directive(tag, start);
        //        auto directive = new YamlTagDirective(t, this);
        //        QRegularExpression re("[!][^!]*[!]");
        //        auto match = re.match(s);
        //        if (match.hasMatch()) {
        //          auto tagid = match.captured(0);
        //          auto len = match.capturedLength(0);
        //          directive->setId(tagid.mid(1, len - 2));
        //          s = s.mid(len).trimmed();
        //          directive->setValue(s);
        //        }
        //        directive->setStart(createCursor(start));
        //        directive->setEnd(createCursor(start + t.length()));
        nodes.append(directive);
        rootNodes.append(directive);
      }
    } else if (c == Characters::MINUS) {
      auto start = i;

      t += c;
      if (!getNextChar(c, text, i)) {
        // TODO NOT A TAG DIRECTIVE carry on with something
        // else.
        continue;
      } else if (c == Characters::MINUS) {
        t += c;
        if (!getNextChar(c, text, i)) {
          // TODO NOT A TAG DIRECTIVE carry on with something
          // else.
          continue;
        } else if (c == Characters::MINUS) {
          t += c;
          if (indent == 0) {
            auto startNode = new YamlStart(this);
            startNode->setStart(createCursor(start));
            startNode->setEnd(createCursor(start + t.length()));
            //          if (!doc->hasDirective() || !doc->hasTag()) {
            //            doc->setStart(startNode->start());
            //          }
            nodes.append(startNode);
            rootNodes.append(startNode);
          } else {
            // TODO this will be inside a scalar string
            continue;
          }
          t.clear();
        }
      }
    } else if (c == Characters::POINT) {
      auto start = i;
      t += c;
      if (!getNextChar(c, text, i)) {
        // TODO NOT A TAG DIRECTIVE carry on with something
        // else.
        continue;
      } else if (c == Characters::POINT) {
        t += c;
        if (!getNextChar(c, text, i)) {
          // TODO NOT A TAG DIRECTIVE carry on with something
          // else.
          continue;
        } else if (c == Characters::POINT) {
          t += c;
          if (indent == 0) {
            auto endNode = new YamlEnd(this);
            endNode->setStart(createCursor(start));
            endNode->setEnd(createCursor(start + t.length()));
            //          if (!doc->hasDirective() || !doc->hasTag()) {
            //            doc->setEnd(endNode->end());
            //          }
            nodes.append(endNode);
            rootNodes.append(endNode);
          } else {
            // TODO this will be inside a scalar string
            continue;
          }
          t.clear();
        }
      }
    } else if (c == Characters::HASH) {
    } else if (c == Characters::VERTICAL_LINE) { // comments
      qWarning();
      continue;
    } else if (c == Characters::COLON) { // start map item
      if (!t.isEmpty()) {
        key = t;
        keyStart = i - key.length();
        t.clear();
      }
      qWarning();
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
      auto map = new YamlMap(this);
      map->setStart(createCursor(i));
      map->setFlowType(YamlNode::Flow);
      parseFlowMap(map, ++i, text);
      //      doc->addData(map);
      nodes.append(map);
      rootNodes.append(map);
    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
      // should happen in parseFlowMap
      // TODO error ?
      qWarning();
    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
      auto sequence = new YamlSequence(this);
      sequence->setStart(createCursor(i));
      sequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(sequence, ++i, text);
      //      doc->addData(sequence);
      nodes.append(sequence);
      rootNodes.append(sequence);
    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
      // should happen in parseFlowSequence.
      // TODO error ?
      qWarning();
    } else if (c == '-') {
      if (!isHyphen) {
        isHyphen = true;
        tStart = i;
      }
      t += c;
      continue;
    } else if (c == Characters::STOP) {
      t += c;
      //      } else if (c == Characters::PERCENT) {
      //        isTagStart = true;
      //        tStart = i;
      //        t += c;
    } else if (c == Characters::AMPERSAND) {
      isAnchorStart = true;
    } else if (c == Characters::ASTERISK) {
      isLinkStart = true;
    } else {
      t += c;
      if (!isIndentComplete) {
        isIndentComplete = true;
      }
      continue;
    }
  }

  // build the documents.
  buildDocuments(text, nodes, rootNodes);

  if (resolveAnchors()) {
    // TODO errors
  }

  emit parseComplete();

  return true;
}

void
QYamlParser::buildDocuments(const QString& text,
                            QList<YamlNode*> nodes,
                            QList<YamlNode*> rootNodes)
{
  QYamlDocument* doc = nullptr;
  auto docStarted = false;

  for (int i = 0; i < nodes.size(); i++) {
    auto node = nodes.at(i);
    auto type = node->type();
    if (!doc) {
      doc = new QYamlDocument(this);
      m_documents.append(doc);
    }

    if (!docStarted) {
      if (type == YamlNode::YamlDirective) {
        doc->setDirective(qobject_cast<YamlDirective*>(node));
        // if the document has a %YAML directive then set start to it's start.
        doc->setStart(node->start());
        continue;
      }
    } else {
      if (type == YamlNode::YamlDirective) {
        // this is either a new document or an error
        // TODO handle error
        i--; // step back before directive.
        if (!doc->hasEnd()) {
          // create end position but no node.
          doc->setEnd(createCursor(node->startPos() - 1));
        }
        doc = nullptr;
        docStarted = false;
        continue;
      }
    }

    if (!docStarted) {
      if (type == YamlNode::TagDirective) {
        doc->addTag(qobject_cast<YamlTagDirective*>(node));
        if (!doc->hasDirective() && !doc->hasTag()) {
          // if the document has no %YAML directive then set start to
          // the first tag directives start.
          doc->setStart(node->start());
        }
        continue;
      }
    } else {
      if (type == YamlNode::TagDirective) {
        // this is either a new document or an error
        // TODO handle error
        i--; // step back before directive.
        if (!doc->hasEnd()) {
          // create end position but no node.
          doc->setEnd(createCursor(node->startPos() - 1));
        }
        doc = nullptr;
        docStarted = false;
        continue;
      }
    }

    if (!docStarted) {
      if (type == YamlNode::Start) {
        if (doc->hasStart()) {
          doc->setStart(doc->start(), qobject_cast<YamlStart*>(node));
        } else {
          doc->setStart(node->start(), qobject_cast<YamlStart*>(node));
        }
        docStarted = true;
      } else {
        if (type == YamlNode::End) {
          // TODO error??
        } else if (!(type == YamlNode::YamlDirective ||
                     type == YamlNode::TagDirective)) {
          doc->addNode(node, rootNodes.contains(node));
        }
      }
      continue;
    }

    if (docStarted) {
      if (type == YamlNode::End) {
        doc->setEnd(node->end(), qobject_cast<YamlEnd*>(node));
        doc = nullptr;
        docStarted = false;
      } else if (type == YamlNode::Start) {
        i--; // step back before start.
        if (!doc->hasEnd()) {
          // create end position but no node.
          doc->setEnd(createCursor(node->startPos() - 1));
        }
        doc = nullptr;
        docStarted = false;
      } else if (!(type == YamlNode::YamlDirective ||
                   type == YamlNode::TagDirective || type == YamlNode::Start ||
                   type == YamlNode::End)) {
        doc->addNode(node, rootNodes.contains(node));
      }
    }
  }

  if (doc && !doc->hasEnd()) {
    doc->setEnd(createCursor(text.length()));
  }
}

void
QYamlParser::parseFlowSequence(YamlSequence* sequence,
                               int& i,
                               const QString& text)
{
  QString t;

  while (i < text.length()) {
    auto c = text.at(i);
    if (c == Characters::COMMA) {
      if (t.trimmed().isEmpty()) {
        auto scalar = parseFlowScalar(t, i);
        sequence->append(scalar);
        t.clear();
      } else {
        auto scalar = parseFlowScalar(t, i);
        sequence->append(scalar);
        t.clear();
      }
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
      auto subMap = new YamlMap(this);
      subMap->setStart(createCursor(i));
      subMap->setFlowType(YamlNode::Flow);
      parseFlowMap(subMap, ++i, text);
      sequence->append(subMap);
      continue;
    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
      // TODO error?
      qWarning();
    } else if (c == Characters::OPEN_SQUARE_BRACKET) {
      // end sequence flow
      auto subSequence = new YamlSequence(this);
      subSequence->setStart(createCursor(i));
      subSequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(subSequence, ++i, text);
      sequence->append(subSequence);
    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
      sequence->setEnd(createCursor(i));
      if (!t.isEmpty()) { // only if missing final comma
        auto scalar = parseFlowScalar(t, i);
        sequence->append(scalar);
        t.clear();
      }
      i++;
      return;
    } else {
      if (t.isEmpty() && c == Characters::SPACE) {
        i++;
        continue;
      }
      t += c;
    }
    i++;
  }
}

void
QYamlParser::parseFlowMap(YamlMap* map, int& i, const QString& text)
{
  QString t;
  QString key;
  int keyStart = -1;
  bool stringLiteral = false;
  bool foldedLiteral = false;

  while (i < text.length()) {
    auto c = text.at(i);
    if (c == Characters::NEWLINE) {
      if (!t.isEmpty()) {
        t += Characters::SPACE; // new lines inside strings are ignored.
      }
      i++;
      continue;
    } else if (c == Characters::COLON) {
      if (!t.isEmpty()) {
        key = t;
        keyStart = i - key.length();
        t.clear();
      }
    } else if (c == Characters::VERTICAL_LINE) {
      // TODO flow string literal.
      if (t.isEmpty()) {
        stringLiteral = true;
      }
    } else if (c == Characters::GT) {
      // TODO flow string literal.
      if (t.isEmpty()) {
        foldedLiteral = true;
      }
    } else if (c == Characters::COMMA) {
      // TODO maps & sequences at comma
      if (!t.isEmpty() && !key.isEmpty()) {
        auto scalar = parseFlowScalar(t, i);
        auto item = new YamlMapItem(key, scalar, this);
        item->setStart(createCursor(keyStart));
        item->setEnd(scalar->end());
        map->insert(key, item);
        t.clear();
        key.clear();
        keyStart = -1;
      }
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start sub map flow
      auto subMap = new YamlMap(this);
      subMap->setStart(createCursor(i));
      subMap->setFlowType(YamlNode::Flow);
      parseFlowMap(subMap, ++i, text);
      if (!key.isEmpty()) {
        auto item = new YamlMapItem(key, subMap, this);
        item->setStart(createCursor(keyStart));
        item->setEnd(subMap->end());
        map->insert(key, item);
        key.clear();
        keyStart = -1;
      } else {
        // TODO error map item has no key.
      }
      continue;
    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
      map->setEnd(createCursor(i));
      if (!t.isEmpty() && !key.isEmpty()) { // only if missing final comma
        auto scalar = parseFlowScalar(t, i);
        auto item = new YamlMapItem(key, scalar, this);
        item->setStart(createCursor(keyStart));
        item->setEnd(createCursor(i));
        map->insert(key, item);
        t.clear();
        key.clear();
        keyStart = -1;
      }
      i++;
      return;
    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
      auto subSequence = new YamlSequence(this);
      subSequence->setStart(createCursor(i));
      subSequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(subSequence, ++i, text);
      if (!key.isEmpty()) {
        auto item = new YamlMapItem(key, subSequence, this);
        item->setStart(createCursor(keyStart));
        item->setEnd(subSequence->end());
        map->insert(key, item);
        key.clear();
        keyStart = -1;
      } else {
        // TODO error map item has no key.
      }
      continue;
    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
      // TODO error
      qWarning();
    } else {
      if (t.isEmpty() && c == Characters::SPACE) {
        i++;
        continue;
      }
      t += c;
    }
    i++;
  }
}

YamlComment*
QYamlParser::parseComment(int& i, const QString& text)
{
  auto comment = new YamlComment(this);
  comment->setStart(createCursor(i));
  // todo set indent
  ++i;
  while (i < text.length()) {
    auto c = text.at(i);
    if (c == '\n') {
      comment->append(c);
      comment->setEnd(createCursor(i));
      return comment;
    }
    comment->append(c);
    i++;
  }
  return nullptr;
}

YamlScalar*
QYamlParser::parseFlowScalar(const QString& text, int i)
{
  auto indent = 0, nl = 0;
  int textlength = text.length();
  auto t = text;
  while (t.startsWith(Characters::NEWLINE)) {
    nl++;
    t = t.mid(1);
  }
  while (t.at(0).isSpace()) {
    indent++;
    t = t.mid(1);
  }
  //  t = StringUtil::lTrim(t, indent);
  auto trimmed = t.trimmed();

  auto firstChar = trimmed.at(0);  // TODO to at when working.
  auto secondChar = trimmed.at(1); // TODO to at when working.
  auto start = i - textlength + indent + nl;
  auto scalar = new YamlScalar(trimmed, this);
  scalar->setStart(createCursor(start));
  scalar->setEnd(createCursor(i));

  auto splits = trimmed.split(Characters::NEWLINE, Qt::KeepEmptyParts);
  auto count = 0;
  QMap<int, YamlWarning> warnings;
  for (auto i = 0; i < splits.size(); i++) {
    auto s = splits.at(i);
    if (s.isEmpty()) {
      warnings.insert(count, YamlWarning::NoWarnings);
      count++;
    }
    if (i <= splits.size() - 1) {
      if (s.contains(Characters::HASH)) {
        warnings.insert(count, YamlWarning::PossibleCommentInScalar);
      } else {
        warnings.insert(count, YamlWarning::NoWarnings);
      }
      count += s.length();
    }
  }
  for (auto [c, t] : asKeyValueRange(warnings)) {
    if (t == YamlWarning::NoWarnings) {
      continue;
    } else if (t == YamlWarning::PossibleCommentInScalar) {
      if (t != warnings.last()) {
        int l = trimmed.indexOf(Characters::HASH, c);
        scalar->addDodgyChar(createCursor(c + l),
                             YamlWarning::PossibleCommentInScalar);
      }
    }
  }

  auto p = 0;
  while ((p = t.indexOf(Characters::TAB, p)) != -1) {
    scalar->addDodgyChar(createCursor(i - textlength + indent + nl + p),
                         YamlWarning::TabCharsDiscouraged);
    p++;
  }

  if ((firstChar == Characters::COLON || firstChar == Characters::HASH) &&
      secondChar.isSpace()) {
    scalar->setError(YamlError::IllegalFirstCharacter, true);
  } else if (firstChar == Characters::AMPERSAND ||
             firstChar == Characters::EXCLAMATIONMARK ||
             firstChar == Characters::ASTERISK ||
             firstChar == Characters::VERTICAL_LINE ||
             firstChar == Characters::GT ||
             firstChar == Characters::COMMERCIAL_AT ||
             firstChar == Characters::BACKTICK) {
    scalar->setError(YamlError::IllegalFirstCharacter, true);
  } else if (firstChar == Characters::OPEN_CURLY_BRACKET) {
    // TODO distinguish from start map??
    qWarning();
  } else if (firstChar == Characters::OPEN_SQUARE_BRACKET) {
    // TODO distinguish from start sequence??
    qWarning();
  } else if (firstChar == Characters::CLOSE_CURLY_BRACKET) {
    // TODO distinguish from start map??
    qWarning();
  } else if (firstChar == Characters::CLOSE_SQUARE_BRACKET) {
    // TODO distinguish from start sequence??
    qWarning();
  } else if (firstChar == Characters::COMMA) {
    // TODO distinguish from extra comma??
    qWarning();
  } else if (firstChar == Characters::DOUBLEQUOTE) {
    if (trimmed.endsWith(Characters::DOUBLEQUOTE)) {
      scalar->setStyle(YamlScalar::DOUBLEQUOTED);
    } else {
      scalar->setError(YamlError::IllegalFirstCharacter, true);
      scalar->setError(YamlError::MissingMatchingQuote, true);
    }
  } else if (firstChar == Characters::SINGLEQUOTE) {
    if (trimmed.endsWith(Characters::SINGLEQUOTE)) {
      scalar->setStyle(YamlScalar::SINGLEQUOTED);
    } else {
      scalar->setError(YamlError::IllegalFirstCharacter, true);
      scalar->setError(YamlError::MissingMatchingQuote, true);
    }
  }
  return scalar;
}

bool
QYamlParser::resolveAnchors()
{
  // TODO resolve all the anchors
  return true;
}

const QString
QYamlParser::filename() const
{
  return m_filename;
}

bool
QYamlParser::loadFile(const QString& filename)
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
QYamlParser::loadFromZip(const QString& zipFile, const QString& href)
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

QList<QYamlDocument*>
QYamlParser::documents() const
{
  return m_documents;
}

QYamlDocument*
QYamlParser::document(int index)
{
  if (index >= 0 && index < count()) {
    return m_documents.at(index);
  }
  return nullptr;
}

QString
QYamlParser::text() const
{
  return m_text;
}

void
QYamlParser::setDocuments(QList<QYamlDocument*> root)
{
  m_documents = root;
}

void
QYamlParser::append(QYamlDocument* document)
{
  m_documents.append(document);
}

bool
QYamlParser::isMultiDocument()
{
  return (m_documents.size() > 1);
}

YamlNode*
QYamlParser::nodeAt(QTextCursor cursor)
{
  for (auto doc : m_documents) {
    for (auto node : doc->nodes()) {
      YamlNode* n;
      if ((n = nodeOrRecurse(cursor, node))) {
        return n;
      }
    }
  }
  return nullptr;
}

YamlNode*
QYamlParser::nodeInSequence(QTextCursor cursor, YamlSequence* seq)
{
  if (!seq->data().isEmpty()) {
    for (auto node : seq->data()) {
      YamlNode* n;
      if ((n = nodeOrRecurse(cursor, node))) {
        return n;
      }
    }
  }

  return seq;
}

YamlNode*
QYamlParser::nodeInMap(QTextCursor cursor, YamlMap* map)
{
  if (!map->data().isEmpty()) {
    for (auto node : map->data()) {
      YamlNode* n;
      if ((n = nodeOrRecurse(cursor, node))) {
        return n;
      }
      break;
    }
  }
  return map;
}

YamlNode*
QYamlParser::nodeOrRecurse(QTextCursor cursor, YamlNode* node)
{
  if (cursor >= node->start() && cursor < node->end()) {
    switch (node->type()) {
      case YamlNode::Undefined:
        // should never happen.
        break;
      case YamlNode::YamlDirective:
      case YamlNode::TagDirective:
      case YamlNode::Start:
      case YamlNode::End:
      case YamlNode::MapItem:
      case YamlNode::Comment:
        return node;
      case YamlNode::Scalar:
        return node;
      case YamlNode::Sequence:
        return nodeInSequence(cursor, qobject_cast<YamlSequence*>(node));
      case YamlNode::Map:
        return nodeInMap(cursor, qobject_cast<YamlMap*>(node));
    }
  }
  return nullptr;
}

QString
QYamlParser::lookahead(int& index, const QString& text, QChar endof)
{
  auto c = text.at(index);
  QString result;
  while (c != endof) {
    index++;
    result += c;
    c = text.at(index);
  }
  return result;
}

bool
QYamlParser::isEmpty()
{
  return m_documents.empty();
}

int
QYamlParser::count()
{
  return m_documents.size();
}

bool
QYamlParser::c_indicator(QChar c)
{
  return (c_sequence_entry(c) || c_mapping_key(c) || c_mapping_value(c) ||
          c_collect_entry(c) || c_sequence_start(c) || c_sequence_end(c) ||
          c_mapping_start(c) || c_mapping_end(c) | c_comment(c) ||
          c_anchor(c) || c_alias(c) || c_tag(c) || c_literal(c) ||
          c_folded(c) || c_single_quote(c) || c_double_quote(c) ||
          c_directive(c) || c_reserved(c));
}

bool
QYamlParser::c_flow_indicator(QChar c)
{
  return (c_collect_entry(c) || c_sequence_start(c) || c_sequence_end(c) ||
          c_mapping_start(c) || c_mapping_end(c));
}

bool
QYamlParser::c_byte_order_mark(QChar c)
{
  if (c == Characters::BYTEORDERMARK)
    return true;
  return false;
}

bool
QYamlParser::c_sequence_entry(QChar c)
{
  return (c == Characters::HYPHEN);
}

bool
QYamlParser::c_mapping_key(QChar c)
{
  return (c == Characters::QUESTIONMARK);
}

bool
QYamlParser::c_mapping_value(QChar c)
{
  return (c == Characters::COLON);
}

bool
QYamlParser::c_collect_entry(QChar c)
{
  return (c == Characters::COMMA);
}

bool
QYamlParser::c_sequence_start(QChar c)
{
  return (c == Characters::OPEN_SQUARE_BRACKET);
}

bool
QYamlParser::c_sequence_end(QChar c)
{
  return (c == Characters::CLOSE_SQUARE_BRACKET);
}

bool
QYamlParser::c_mapping_start(QChar c)
{
  return (c == Characters::OPEN_CURLY_BRACKET);
}

bool
QYamlParser::c_mapping_end(QChar c)
{
  return (c == Characters::CLOSE_CURLY_BRACKET);
}

bool
QYamlParser::c_comment(QChar c)
{
  return (c == Characters::HASH);
}

bool
QYamlParser::c_anchor(QChar c)
{
  return (c == Characters::AMPERSAND);
}

bool
QYamlParser::c_alias(QChar c)
{
  return (c == Characters::ASTERISK);
}

bool
QYamlParser::c_tag(QChar c)
{
  return (c == Characters::EXCLAMATIONMARK);
}

bool
QYamlParser::c_literal(QChar c)
{
  return (c == Characters::VERTICAL_LINE);
}

bool
QYamlParser::c_folded(QChar c)
{
  return (c == Characters::GT);
}

bool
QYamlParser::c_single_quote(QChar c)
{
  return (c == Characters::SINGLEQUOTE);
}

bool
QYamlParser::c_double_quote(QChar c)
{
  return (c == Characters::DOUBLEQUOTE);
}

bool
QYamlParser::c_directive(QChar c)
{
  return (c == Characters::PERCENT);
}

bool
QYamlParser::c_reserved(QChar c)
{
  return (c == Characters::COMMERCIAL_AT || c == Characters::BACKTICK);
}

bool
QYamlParser::b_line_feed(QChar c)
{
  return (c == Characters::LF);
}

bool
QYamlParser::b_carriage_return(QChar c)
{
  return (c == Characters::CR);
}

bool
QYamlParser::b_char(QChar c, int version)
{
  if (version == 11)
    if (b_line_feed(c) || b_carriage_return(c) || c == Characters::NEXTLINE ||
        c == Characters::LINESEPERATOR || c == Characters::PARASEPERATOR)
      return true;
  if (version == 12)
    return (b_line_feed(c) || b_carriage_return(c));
  return false;
}

bool
QYamlParser::nb_char(QChar c, int version)
{
  return (c_printable(c) && !(b_char(c, version) || c_byte_order_mark(c)));
}

bool
QYamlParser::s_space(QChar c)
{
  return (c == Characters::SPACE);
}

bool
QYamlParser::s_tab(QChar c)
{
  return (c == Characters::TAB);
}

bool
QYamlParser::s_white(QChar c)
{
  return (s_space(c) || s_tab(c));
}

bool
QYamlParser::ns_char(QChar c)
{
  return (nb_char(c, MAX_VERSION) && !s_white(c));
}

bool
QYamlParser::b_break(QChar c1, QChar c2)
{
  return (b_carriage_return(c1) && b_line_feed(c2));
}

bool
QYamlParser::b_break(QChar c)
{
  return (b_carriage_return(c) || b_line_feed(c));
}

bool
QYamlParser::b_as_line_feed(QChar& c)
{
  if (b_break(c)) {
    c = Characters::LF;
    return true;
  }
  return false;
}

bool
QYamlParser::b_as_line_feed(QChar& c1, QChar& c2)
{
  if (b_break(c1, c2)) {
    c1 = Characters::LF;
    return true;
  }
  return false;
}

bool
QYamlParser::b_non_content(QChar& c)
{
  return b_break(c);
}

bool
QYamlParser::c_printable(QChar c)
{
  if (c.isPrint())
    return true;

  auto lower = c.cell();
  auto higher = c.row();

  if (c == Characters::HORIZONTALTABSYMBOL)
    return true;
  else if (nb_char(c, MAX_VERSION))
    return true;
  else if (lower >= 0x20 && lower <= 0x7E)
    return true;
  else if (lower == 0x85)
    return true;
  else if ((lower >= 0xA0 && lower <= 0xFF) && higher == 0)
    return true;
  else if (higher >= 0x00 && higher <= 0xD7)
    return true;
  return false;
}

bool
QYamlParser::ns_dec_digit(QChar c)
{
  // YAML ns-dec-digit
  return (c.isDigit());
}

bool
QYamlParser::ns_hex_digit(QChar c)
{
  // YAML ns-hex-digit
  auto lower = c.cell();
  auto higher = c.row();
  return (higher == 0 && ((lower >= 0x41 && lower <= 0x46) ||
                          (lower >= 0x61 && lower <= 0x66)));
}

bool
QYamlParser::ns_ascii_char(QChar c)
{
  auto lower = c.cell();
  auto higher = c.row();
  return (higher == 0 && ((lower >= 0x41 && lower <= 0x5A) ||
                          (lower >= 0x61 && lower <= 0x7A)));
}

bool
QYamlParser::ns_word_char(QChar c)
{
  return (ns_dec_digit(c) || ns_ascii_char(c) || c == Characters::HYPHEN);
}

bool
QYamlParser::ns_uri_char(const QString& s)
{
  // TODO
  return false;
}

bool
QYamlParser::ns_tag_char(QChar c)
{
  // TODO
  //  return ns_uri_char()
  return false;
}

bool
QYamlParser::c_escape(QChar c)
{
  return (c == Characters::FORWARDSLASH);
}

bool
QYamlParser::c_ns_esc_char(QChar c)
{
  return (c == Characters::FORWARDSLASH);
}

bool
QYamlParser::ns_esc_null(QChar& c)
{
  if (c == Characters::NULLCHAR) {
    c = Characters::NULLCHAR;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_bell(QChar& c)
{
  if (c == 'a') {
    c = Characters::BELL;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_backspace(QChar& c)
{
  if (c == 'b') {
    c = Characters::BACKSPACE;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_horizontal_tab(QChar& c)
{
  if (c == 't') {
    c = Characters::TAB;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_linefeed(QChar& c)
{
  if (c == 'n') {
    c = Characters::LF;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_vertical_tab(QChar& c)
{
  if (c == 'v') {
    c = Characters::VTAB;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_form_feed(QChar& c)
{
  if (c == 'f') {
    c = Characters::FF;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_carriage_return(QChar& c)
{
  if (c == 'r') {
    c = Characters::CR;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_escape(QChar& c)
{
  if (c == 'e') {
    c = Characters::ESCAPE;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_space(QChar& c)
{
  if (c == 'r') {
    c = Characters::SPACE;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_double_quote(QChar& c)
{
  if (c == '"') {
    c = Characters::DOUBLEQUOTE;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_slash(QChar& c)
{
  if (c == Characters::FORWARDSLASH) {
    c = Characters::FORWARDSLASH;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_next_line(QChar& c)
{
  if (c == 'N') {
    c = Characters::NEXTLINE;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_backslash(QChar& c)
{
  if (c == Characters::BACKSLASH) {
    c = Characters::BACKSLASH;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_nb_space(QChar& c)
{
  if (c == Characters::LOWLINE) {
    c = Characters::NBSPACE;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_line_seperator(QChar& c)
{
  if (c == 'L') {
    c = Characters::LINESEPERATOR;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_paragraph_seperator(QChar& c)
{
  if (c == 'P') {
    c = Characters::PARASEPERATOR;
    return true;
  }
  return false;
}

bool
QYamlParser::ns_esc_8_bit(const QString& s)
{
  if (s.length() == 4 && c_ns_esc_char(s.at(0)) && s.at(1) == 'x' &&
      ns_hex_digit(s.at(2)) && ns_hex_digit(s.at(3)))
    return true;
  return false;
}

bool
QYamlParser::ns_esc_16_bit(const QString& s)
{
  if (s.length() == 6 && c_ns_esc_char(s.at(0)) && s.at(1) == 'u' &&
      ns_hex_digit(s.at(2)) && ns_hex_digit(s.at(3)) && ns_hex_digit(s.at(4)) &&
      ns_hex_digit(s.at(5)))
    return true;
  return false;
}

bool
QYamlParser::ns_esc_32_bit(const QString& s)
{
  // TODO 32 bit utf
  return false;
}

bool
QYamlParser::c_ns_esc_char(const QString& s)
{
  auto good = false;
  auto l = s.length();
  if (l > 0) {
    if (c_ns_esc_char(s.at(0)))
      good = true;
    if (good) {
      good = false;
      if (l == 2) {
        auto c = s.at(1);
        if (ns_esc_null(c) || ns_esc_bell(c) || ns_esc_backspace(c) ||
            ns_esc_horizontal_tab(c) || ns_esc_linefeed(c) ||
            ns_esc_vertical_tab(c) || ns_esc_form_feed(c) ||
            ns_esc_carriage_return(c) || ns_esc_escape(c) || ns_esc_space(c) ||
            ns_esc_double_quote(c) || ns_esc_slash(c) || ns_esc_next_line(c) ||
            ns_esc_nb_space(c) || ns_esc_line_seperator(c) ||
            ns_esc_paragraph_seperator(c) || ns_esc_8_bit(c) ||
            ns_esc_16_bit(c) || ns_esc_32_bit(c)) {
        }
      }
    }
  }
  return false;
}

YamlTagDirective*
QYamlParser::ns_tag_directive(const QString& s, int start)
{
  YamlTagDirective* directive = nullptr;
  auto tag = s;
  auto handlePos = start;
  if (!c_directive(tag.at(0)))
    return directive;
  handlePos++;
  tag = tag.mid(1);
  if (!tag.startsWith("TAG"))
    return directive;
  tag = tag.mid(3);
  handlePos += 3;
  auto len = s_separate_in_line(tag);
  handlePos += len;
  tag = tag.mid(len);

  YamlTagDirective::HandleType type = YamlTagDirective::NoType;
  len = c_tag_handle(tag, type);

  auto handle = tag.mid(1, len);
  tag = tag.mid(len + 2).trimmed();
  auto valuePos = handlePos + len + 2;
  len = s_separate_in_line(tag);
  tag = tag.mid(len);
  valuePos += len;
  directive = new YamlTagDirective(handle, tag, this);
  directive->setHandleType(type);
  return directive;
}

bool
QYamlParser::nb_json(QChar c)
{
  auto lower = c.cell();
  auto higher = c.row();
  return (c == Characters::TAB ||
          (higher == 0 && lower >= 0x20 && lower <= 0xFF) || higher >= 0x1);
}

QList<QYamlDocument*>::iterator
QYamlParser::begin()
{
  return m_documents.begin();
}

QList<QYamlDocument*>::const_iterator
QYamlParser::constBegin()
{
  return m_documents.constBegin();
}

QList<QYamlDocument*>::iterator
QYamlParser::end()
{
  return m_documents.end();
}

QList<QYamlDocument*>::const_iterator
QYamlParser::constEnd()
{
  return m_documents.constEnd();
}

QTextCursor
QYamlParser::createCursor(int position)
{
  auto cursor = QTextCursor(m_document);
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
  return cursor;
}

//====================================================================
//=== QYamlSettings
//====================================================================
bool
QYamlSettings::save(const QString& filename)
{
  return true;
}

bool
QYamlSettings::load(const QString& filename)
{
  return true;
}

int
QYamlSettings::indentStep() const
{
  return m_indentStep;
}

void
QYamlSettings::setIndentStep(int indentStep)
{
  m_indentStep = indentStep;
}
