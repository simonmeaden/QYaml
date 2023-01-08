#include "qyaml/qyamlparser.h"
#include "qyaml/qyamldocument.h"
#include "utilities/characters.h"

//====================================================================
//=== QYamlParser
//====================================================================
const QString QYamlParser::VERSION_STRING = QStringLiteral("1.2");
const QString QYamlParser::VERSION_PATCH_STRING = QStringLiteral("1.2.2");
const QString QYamlParser::DOCSTART = QStringLiteral("---");
const QString QYamlParser::DOCEND = QStringLiteral("...");

QYamlParser::QYamlParser(QTextDocument *doc, QObject *parent)
    : QObject{parent}, m_document(doc) {}

QString QYamlParser::inlinePrint() const {
  // TODO
  return QString();
}

QString QYamlParser::prettyPrint() const {
  // TODO
  return QString();
}

void QYamlParser::parseYamlDirective(QString s, int &tStart, int i, QChar c,
                                     bool hasYamlDirective, int row,
                                     int &indent) {
  if (s.startsWith("%YAML")) {
    s = s.mid(5).trimmed();
    auto split = s.split(".", Qt::SkipEmptyParts);
    if (split.size() == 2) {
      s = split.at(0);
      c = s.at(0);
      int major = c.digitValue();
      s = split.at(1);
      c = s.at(0);
      int minor = c.digitValue();
      if (m_currentDoc && m_currentDoc->majorVersion() > 0) {
        m_currentDoc->setMajorVersion(major);
        m_currentDoc->setMinorVersion(minor);
        if (major == VERSION_MAJOR) {
          m_currentDoc->setError(InvalidVersionError, false);
          if (minor >= 0 && minor <= VERSION_MINOR) {
            m_currentDoc->setWarning(InvalidMinorVersionWarning, false);
          } else {
            m_currentDoc->setWarning(InvalidMinorVersionWarning, true);
          }
        } else {
          m_currentDoc->setError(InvalidVersionError, true);
        }
      } else {
        m_currentDoc->setError(TooManyYamlDirectivesError, true);
      }
      tStart = 0;
      indent = 0;
    } else {
      // TODO bad version
    }
  }
}

bool QYamlParser::parse(const QString &text, int startPos, int length) {
  m_text = text;

  auto row = 0;
  auto start = startPos;
  QString t;
  auto tStart = -1;
  auto indent = 0;
  auto rowStart = 0;
  auto pos = 0;
  YamlNode *node = nullptr;
  auto isTagStart = false;
  auto isAnchorStart = false;
  auto isLinkStart = false;
  auto isHyphen = false;
  auto isIndentComplete = false;
  QTextCursor cursor;
  auto hasYamlDirective = false;

  if (text.contains(DOCSTART)) {
    while ((pos = text.indexOf(DOCSTART, pos)) != -1) {
      auto document = new QYamlDocument(this);
      document->setDocumentStart(createCursor(pos++));
      append(document);
    }
    QList<int> ends;
    pos = 0;
    while ((pos = text.indexOf(DOCEND, pos)) != -1) {
      ends.append(pos++);
    }
    if (!ends.isEmpty()) {
      for (auto end : ends) {
        for (int i = 0; i < m_documents.size() - 1; i++) {
          auto doc = m_documents.at(i);
          auto nextDoc = m_documents.at(i + 1);
          if (end > doc->documentStart().position() &&
              end < nextDoc->documentStart().position()) {
            doc->setDocumentEnd(createCursor(end));
          }
        }
      }
    }
    m_currentDoc = m_documents.at(0);
  } else {
    m_currentDoc = new QYamlDocument(this);
    m_currentDoc->setDocumentStart(createCursor(0));
    m_currentDoc->setDocumentEnd(createCursor(text.length()));
    append(m_currentDoc);
  }

  for (auto i = start; i < text.size(); i++) {
    auto c = text.at(i);
    if (c == Characters::NEWLINE) {
      // TODO add parseScalar()
      auto s = t.trimmed();
      parseYamlDirective(s, tStart, i, c, hasYamlDirective, row, indent);
      row++;
      indent = 0;
      rowStart = i;
      isIndentComplete = false;
      hasYamlDirective = true;
      t.clear();
    } else if (c.isSpace()) { // TODO maybe c.isSpace()
      if (!isIndentComplete) {
        indent++;
        continue;
      }
      t += c;
      continue;
    } else if (c == Characters::HASH) { // comments
      auto comment = parseComment(i, text);
      if (comment) {
        m_currentDoc->addData(comment);
      }
      continue;
    } else if (c == Characters::COLON) { // start map flow
      qWarning();
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
      auto map = new YamlMap(this);
      map->setStart(createCursor(i));
      map->setFlowType(YamlNode::Flow);
      parseFlowMap(map, ++i, text);
      m_currentDoc->addData(map);
    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
      // should happen in parseFlowMap
      // TODO error ?
      qWarning();
    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
      auto sequence = new YamlSequence(this);
      sequence->setStart(createCursor(i));
      sequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(sequence, ++i, text);
      m_currentDoc->addData(sequence);
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
      if (t == "---") {
        // TODO new document
        //        node = new YamlStart(this);
        //        cursor = createCursor(tStart);
        //        node->setStart(cursor);
        //        cursor = createCursor(i);
        //        node->setEnd(cursor);
        //        node->setRow(row);
        //        node->setIndent(indent);
        //        m_currentDoc->addData(node);
        //        tStart = 0;
        //        indent = 0;
      }
      continue;

    } else if (c == Characters::STOP) {
      t += c;
    } else if (c == Characters::PERCENT) {
      isTagStart = true;
      tStart = i;
      t += c;
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

  qWarning();

  if (resolveAnchors()) {
    // TODO errors
  }

  emit parseComplete();

  return true;
}

void QYamlParser::parseFlowSequence(YamlSequence *sequence, int &i,
                                    const QString &text) {
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
    } else if (c == Characters::HASH) {
      // comment
      auto comment = parseComment(i, text);
      if (comment) {
        m_currentDoc->addData(comment);
      }
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
      auto subMap = new YamlMap(this);
      subMap->setStart(createCursor(i));
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

void QYamlParser::parseFlowMap(YamlMap *map, int &i, const QString &text) {
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
    } else if (c == Characters::VERTICALLINE) {
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
    } else if (c == Characters::HASH) { // comment
      // TODO comments ALWAYS finish scalars.
      auto comment = parseComment(i, text);
      if (comment) {
        m_currentDoc->addData(comment);
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

YamlComment *QYamlParser::parseComment(int &i, const QString &text) {
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

YamlScalar *QYamlParser::parseScalar(const QString &t, int i) {
  auto start = i - t.length();
  auto scalar = new YamlScalar(t, this);
  scalar->setStart(createCursor(start));
  scalar->setEnd(createCursor(i));
  return scalar;
}

bool QYamlParser::resolveAnchors() { return true; }

const QString QYamlParser::filename() const { return m_filename; }

bool QYamlParser::loadFile(const QString &filename) {
  m_filename = filename;
  QFile file(m_filename);
  if (file.open(QIODevice::ReadOnly)) {
    QString text = file.readAll();
    return parse(text);
  }
  return false;
}

bool QYamlParser::loadFromZip(const QString &zipFile, const QString &href) {
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

QList<QYamlDocument *> QYamlParser::documents() const { return m_documents; }

QYamlDocument *QYamlParser::document(int index) {
  if (index >= 0 && index < count()) {
    return m_documents.at(index);
  }
  return nullptr;
}

QString QYamlParser::text() const { return m_text; }

QYamlDocument *QYamlParser::currentDoc() const { return m_currentDoc; }

// const QMap<QTextCursor, YamlNode*>&
// QYamlParser::nodes() const
//{
//   return m_nodes;
// }

void QYamlParser::setDocuments(QList<QYamlDocument *> root) {
  m_documents = root;
}

void QYamlParser::append(QYamlDocument *document) {
  m_documents.append(document);
}

bool QYamlParser::isMultiDocument() { return (m_documents.size() > 1); }

bool QYamlParser::isEmpty() { return m_documents.empty(); }

int QYamlParser::count() { return m_documents.size(); }

QList<QYamlDocument *>::iterator QYamlParser::begin() {
  return m_documents.begin();
}

QList<QYamlDocument *>::const_iterator QYamlParser::constBegin() {
  return m_documents.constBegin();
}

QList<QYamlDocument *>::iterator QYamlParser::end() {
  return m_documents.end();
}

QList<QYamlDocument *>::const_iterator QYamlParser::constEnd() {
  return m_documents.constEnd();
}

QTextCursor QYamlParser::createCursor(int position) {
  auto cursor = QTextCursor(m_document);
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
  return cursor;
}
