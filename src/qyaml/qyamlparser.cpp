#include "qyaml/qyamlparser.h"
#include "qyaml/qyamldocument.h"
// #include "utilities/ContainerUtil.h"
#include "utilities/characters.h"

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
    } else if (c == Characters::PERCENT) {
      int start = i;
      t = c;
      if (!getNextChar(c, text, i)) {
        i = start + 1;
        // TODO NOT A YAML DIRECTIVE carry on with something else.
        continue;
      } else if (c == 'Y') {
        t += c;
        if (!getNextChar(c, text, i)) {
          i = start + 1;
          // TODO NOT A YAML DIRECTIVE carry on with something else.
          continue;
        } else if (c == 'A') {
          t += c;
          if (!getNextChar(c, text, i)) {
            i = start + 1;
            // TODO NOT A YAML DIRECTIVE carry on with something else.
            continue;
          } else if (c == 'M') {
            t += c;
            if (!getNextChar(c, text, i)) {
              i = start + 1;
              // TODO NOT A YAML DIRECTIVE carry on with something else.
              continue;
            } else if (c == 'L') {
              t += c;
              if (!getNextChar(c, text, i)) {
                i = start + 1;
                // TODO NOT A YAML DIRECTIVE carry on with something else.
                continue;
              } else {
                while (c.isSpace()) {
                  t += c;
                  if (!getNextChar(c, text, i)) {
                    // TODO NOT A YAML DIRECTIVE carry on with something
                    // else.
                    continue;
                  }
                }

                if (c.isDigit()) {
                  auto major = c.digitValue();
                  if (major == VERSION_MAJOR) {
                    t += c;
                    if (!getNextChar(c, text, i)) {
                      i = start + 1;
                      // TODO NOT A YAML DIRECTIVE carry on with something
                      // else.
                      continue;
                    } else if (c == Characters::STOP) {
                      t += c;
                      if (!getNextChar(c, text, i)) {
                        i = start + 1;
                        // TODO NOT A YAML DIRECTIVE carry on with
                        // something else.
                        continue;
                      } else if (c.isDigit()) {
                        auto minor = c.digitValue();
                        if (minor >= 0 && minor <= VERSION_MINOR) {
                          t += c;
                          auto directive =
                            new YamlDirective(major, minor, this);
                          directive->setStart(createCursor(start));
                          directive->setEnd(createCursor(start + t.length()));
                          //                          doc->setDirective(directive);
                          //                          // reposition the document
                          //                          start to the YAML
                          //                          // directive.
                          //                          if (!doc->hasDirective())
                          //                          {
                          //                            doc->setStart(directive->start());
                          //                          }
                          nodes.append(directive);
                          t.clear();
                          continue;
                        } else {
                          // TODO error version number;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      } else if (c == 'T') {
        t += c;
        if (!getNextChar(c, text, i)) {
          i = start + 1;
          // TODO NOT A YAML DIRECTIVE carry on with something else.
          continue;
        } else if (c == 'A') {
          t += c;
          if (!getNextChar(c, text, i)) {
            i = start + 1;
            // TODO NOT A YAML DIRECTIVE carry on with something else.
            continue;
          } else if (c == 'G') {
            t += c;
            if (!getNextChar(c, text, i)) {
              // TODO NOT A YAML DIRECTIVE carry on with something
              // else.
              continue;
            } else {
              while (c != Characters::NEWLINE) {
                t += c;
                if (!getNextChar(c, text, i)) {
                  // TODO NOT A TAG DIRECTIVE carry on with something
                  // else.
                  continue;
                }
              }
              auto directive = new YamlTagDirective(t, this);
              directive->setStart(createCursor(start));
              directive->setEnd(createCursor(start + t.length()));
              //              doc->addTag(directive->start(), directive);
              //              // reposition doc start if no YAML directive has
              //              been set
              //              // and if no tag has been already set.
              //              if (!doc->hasDirective() && !doc->hasTag()) {
              //                doc->setStart(directive->start());
              //              }
              nodes.append(directive);
              t.clear();
              continue;
            }
          }
        }
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
  buildDocuments(text, nodes);

  if (resolveAnchors()) {
    // TODO errors
  }

  emit parseComplete();

  return true;
}

void
QYamlParser::buildDocuments(const QString& text, QList<YamlNode*> nodes)
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
          doc->addData(node);
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
        doc->addData(node);
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
    if (c == Characters::NEWLINE) {
      qWarning();
    } else if (c == Characters::COMMA) {
      if (!t.isEmpty()) {
        auto scalar = parseScalar(t, i);
        sequence->append(scalar);
        t.clear();
      }
    } else if (c == Characters::MINUS) {
      i++;
      if (i < text.size()) {
        c = text.at(i);
        if (c.isSpace()) {
          // must be a BLOCK SEQUENCE if followed by a space
        }
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
        auto scalar = parseScalar(t, i);
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
        auto scalar = parseScalar(t, i);
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
        auto scalar = parseScalar(t, i);
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
QYamlParser::parseScalar(const QString& t, int i)
{
  auto trimmed = t.trimmed();
  auto start = i - trimmed.length();
  auto scalar = new YamlScalar(trimmed, this);
  scalar->setStart(createCursor(start));
  scalar->setEnd(createCursor(i));
  auto firstChar = trimmed.first(1);
  if (firstChar == Characters::AMPERSAND ||
      firstChar == Characters::EXCLAMATIONMARK ||
      firstChar == Characters::ASTERISK ||
      firstChar == Characters::VERTICAL_LINE || firstChar == Characters::GT ||
      firstChar == Characters::COMMERCIAL_AT ||
      firstChar == Characters::BACKTICK || firstChar == Characters::HASH) {
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
    }
  } else if (firstChar == Characters::SINGLEQUOTE) {
    if (trimmed.endsWith(Characters::SINGLEQUOTE)) {
      scalar->setStyle(YamlScalar::SINGLEQUOTED);
    } else {
      scalar->setError(YamlError::IllegalFirstCharacter, true);
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

// QYamlDocument *QYamlParser::currentDoc() const { return m_currentDoc; }

// const QMap<QTextCursor, YamlNode*>&
// QYamlParser::nodes() const
//{
//   return m_nodes;
// }

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
