#include "qyaml/qyamlparser.h"
#include "qyaml/qyamldocument.h"
#include "utilities/ContainerUtil.h"
#include "utilities/characters.h"

#include <JlCompress.h>

//====================================================================
//=== QYamlParser
//====================================================================
const QString QYamlParser::VERSION_STRING = QStringLiteral("1.2");
const QString QYamlParser::VERSION_PATCH_STRING = QStringLiteral("1.2.2");
const QString QYamlParser::DOCSTART = QStringLiteral("---");
const QString QYamlParser::DOCEND = QStringLiteral("...");
const QRegularExpression QYamlParser::YAML_DIRECTIVE("%YAML\\s+1[.][0-3]\\s*");
const QRegularExpression QYamlParser::TAG_DIRECTIVE("%TAG\\s+[!\\w\\s:.,]*");

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
  //  QTextCursor cursor;
  auto hasYamlDirective = false;

  splitMultiDocument(text);

  for (auto doc : m_documents) {

    auto docStart = doc->startPos();
    auto docText = text.mid(docStart, doc->textLength());

    for (auto i = start; i < docText.size(); i++) {
      auto c = docText.at(i);
      if (c == Characters::NEWLINE) {
        // TODO add parseScalar()
        auto s = t.trimmed();
        row++;
        indent = 0;
        //        rowStart = i;
        isIndentComplete = false;
        //        hasYamlDirective = true;
        t.clear();
      } else if (c.isSpace()) {
        if (!isIndentComplete) {
          indent++;
          continue;
        }
        t += c;
        continue;
      } else if (c == Characters::HASH) { // comments
        auto comment = parseComment(i, docText, docStart);
        if (comment) {
          doc->addData(comment);
        }
        continue;
      } else if (c == Characters::COLON) { // start map flow
        qWarning();
      } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
        auto map = new YamlMap(this);
        map->setStart(createCursor(i + docStart));
        map->setFlowType(YamlNode::Flow);
        parseFlowMap(map, ++i, docText, docStart);
        doc->addData(map);
      } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
        // should happen in parseFlowMap
        // TODO error ?
        qWarning();
      } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
        auto sequence = new YamlSequence(this);
        sequence->setStart(createCursor(i));
        sequence->setFlowType(YamlNode::Flow);
        parseFlowSequence(sequence, ++i, docText, docStart);
        doc->addData(sequence);
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
          //        doc->addData(node);
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
  }

  qWarning();

  if (resolveAnchors()) {
    // TODO errors
  }

  emit parseComplete();

  return true;
}

QYamlDocument *QYamlParser::splitMultiDocument(const QString &text) {
  QYamlDocument *document = nullptr;
  auto pos = 0;

  QMap<int, Directive *> directives;
  for (auto match : YAML_DIRECTIVE.globalMatch(text)) {
    auto d = new Directive();
    auto s = match.captured(0).trimmed();
    d->start = match.capturedStart(0);
    d->length = s.length();
    if (s.startsWith("%YAML")) {
      d->type = Directive::YAML;
      s = s.mid(5).trimmed();
      auto split = s.split(".", Qt::SkipEmptyParts);
      if (split.size() == 2) {
        s = split.at(0);
        auto c = s.at(0);
        d->major = c.digitValue();
        s = split.at(1);
        c = s.at(0);
        d->minor = c.digitValue();
        directives.insert(d->start, d);
      }
    } else {
      // TODO bad version
    }
  }

  for (auto match : TAG_DIRECTIVE.globalMatch(text)) {
    auto d = new Directive();
    auto s = match.captured(0);
    d->start = match.capturedStart(0);
    d->length = match.capturedLength(0);
    if (s.startsWith("%TAG")) {
      d->type = Directive::TAG;
      s = s.mid(4).trimmed();
      d->text = s;
      directives.insert(d->start, d);
    }
  }

  if (text.contains(DOCSTART)) {
    while ((pos = text.indexOf(DOCSTART, pos)) != -1) {
      document = new QYamlDocument(this);
      auto start = new YamlStart(this);
      start->setStart(createCursor(pos));
      start->setEnd(createCursor(pos + 3));
      document->setStart(createCursor(pos), start);
      pos++;
      auto size = directives.size();
      setTagDirectives(document, directives, pos);
      append(document);
    }

    // set ends for explicit ends
    QList<int> ends;
    pos = 0;
    while ((pos = text.indexOf(DOCEND, pos)) != -1) {
      ends.append(pos); // end of '...'
      pos++;
    }
    if (!ends.isEmpty()) {
      for (auto endPos : ends) {
        for (int i = 0; i < m_documents.size() - 1; i++) {
          auto doc = m_documents.at(i);
          auto nextDoc = m_documents.at(i + 1);
          if (endPos > doc->start().position() &&
              endPos < nextDoc->start().position()) {
            auto cursor = createCursor(endPos);
            auto end = new YamlEnd(this);
            end->setStart(cursor);
            end->setEnd(createCursor(endPos + 3));
            doc->setEnd(cursor, end);
          }
        }
      }
    }
    // set end positions for implicit ends.
    for (auto i = 0; i < m_documents.size() - 1; i++) {
      auto first = m_documents.at(i);
      auto second = m_documents.at(i + 1);
      if (first->end().isNull()) {
        first->setEnd(createCursor(second->startPos() - 1));
      }
    }
    auto last = m_documents.last();
    if (last->end().isNull()) {
      last->setEnd(createCursor(text.length() - 1));
    }

    document = m_documents.at(0);
  } else {
    // no start tag '---'
    document = new QYamlDocument(this);
    document->setStart(createCursor(0));
    document->setEnd(createCursor(text.length()));
    setTagDirectives(document, directives, pos);
    append(document);
  }

  // just in case I missed one.
  qDeleteAll(directives);

  return document;
}

void QYamlParser::setTagDirectives(QYamlDocument *document,
                                   QMap<int, Directive *> &directives,
                                   int pos) {
  QList<int> deletes;
  for (auto [key, d] : asKeyValueRange(directives)) {
    if (d) {
      if (d->start < pos) {
        if (d->type == Directive::YAML) {
          auto yaml = new YamlDirective(d->major, d->minor, this);
          yaml->setStart(createCursor(d->start));
          yaml->setEnd(createCursor(d->start + d->length));
          document->setDirective(yaml);
          // if YAML directive then reset start to start of directive.
          document->setStart(createCursor(d->start));
          deletes.append(key);
        } else if (d->type == Directive::TAG) {
          auto tag = new YamlTagDirective(d->text, this);
          tag->setStart(createCursor(d->start));
          tag->setEnd(createCursor(d->start + d->length));
          document->addTag(tag->start(), tag);
          deletes.append(key);
        }
      }
    }
  }
  while (!deletes.isEmpty()) {
    auto key = deletes.takeFirst();
    auto d = directives.take(key);
    delete d;
  }
}

void QYamlParser::parseFlowSequence(YamlSequence *sequence, int &i,
                               const QString &text, int docStart) {
  QString t;

  while (i < text.length()) {
    auto c = text.at(i);
    if (c == Characters::NEWLINE) {
      qWarning();
    } else if (c == Characters::COMMA) {
      if (!t.isEmpty()) {
        auto scalar = parseScalar(t, i, docStart);
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
      subMap->setStart(createCursor(i + docStart));
      parseFlowMap(subMap, ++i, text, docStart);
      sequence->append(subMap);
      continue;
    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
      // TODO error?
      qWarning();
    } else if (c == Characters::OPEN_SQUARE_BRACKET) {
      // end sequence flow
      auto subSequence = new YamlSequence(this);
      subSequence->setStart(createCursor(i + docStart));
      subSequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(subSequence, ++i, text, docStart);
      sequence->append(subSequence);
    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
      sequence->setEnd(createCursor(i + docStart));
      if (!t.isEmpty()) { // only if missing final comma
        auto scalar = parseScalar(t, i, docStart);
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

void QYamlParser::parseFlowMap(YamlMap *map, int &i, const QString &text,
                               int docStart) {
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
        auto scalar = parseScalar(t, i, docStart);
        auto item = new YamlMapItem(key, scalar, this);
        item->setStart(createCursor(keyStart + docStart));
        item->setEnd(scalar->end());
        map->insert(key, item);
        t.clear();
        key.clear();
        keyStart = -1;
      }
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start sub map flow
      auto subMap = new YamlMap(this);
      subMap->setStart(createCursor(i + docStart));
      subMap->setFlowType(YamlNode::Flow);
      parseFlowMap(subMap, ++i, text, docStart);
      if (!key.isEmpty()) {
        auto item = new YamlMapItem(key, subMap, this);
        item->setStart(createCursor(keyStart + docStart));
        item->setEnd(subMap->end());
        map->insert(key, item);
        key.clear();
        keyStart = -1;
      } else {
        // TODO error map item has no key.
      }
      continue;
    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
      map->setEnd(createCursor(i + docStart));
      if (!t.isEmpty() && !key.isEmpty()) { // only if missing final comma
        auto scalar = parseScalar(t, i, docStart);
        auto item = new YamlMapItem(key, scalar, this);
        item->setStart(createCursor(keyStart + docStart));
        item->setEnd(createCursor(i + docStart));
        map->insert(key, item);
        t.clear();
        key.clear();
        keyStart = -1;
      }
      i++;
      return;
    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
      auto subSequence = new YamlSequence(this);
      subSequence->setStart(createCursor(i + docStart));
      subSequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(subSequence, ++i, text, docStart);
      if (!key.isEmpty()) {
        auto item = new YamlMapItem(key, subSequence, this);
        item->setStart(createCursor(keyStart + docStart));
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

YamlComment *QYamlParser::parseComment(int &i, const QString &text,
                                       int docStart) {
  auto comment = new YamlComment(this);
  comment->setStart(createCursor(i + docStart));
  // todo set indent
  ++i;
  while (i < text.length()) {
    auto c = text.at(i);
    if (c == '\n') {
      comment->append(c);
      comment->setEnd(createCursor(i + docStart));
      return comment;
    }
    comment->append(c);
    i++;
  }
  return nullptr;
}

YamlScalar *QYamlParser::parseScalar(const QString &t, int i, int docStart) {
  auto start = i - t.length();
  auto scalar = new YamlScalar(t.trimmed(), this);
  scalar->setStart(createCursor(start + docStart));
  scalar->setEnd(createCursor(i + docStart));
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

//QYamlDocument *QYamlParser::currentDoc() const { return m_currentDoc; }

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
