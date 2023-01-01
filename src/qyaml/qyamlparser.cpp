#include "qyaml/qyamlparser.h"
#include "utilities/characters.h"

//====================================================================
//=== QYamlParser
//====================================================================
const QRegularExpression QYamlParser::YAML_DIRECTIVE("%YAML 1.[0-3]");
const QRegularExpression QYamlParser::YAML_DOCSTART("[-]{3}");
const QRegularExpression QYamlParser::YAML_DOCEND("[.]{3}");
const QString QYamlParser::VERSION_STRING = "1.2";
const QString QYamlParser::VERSION_PATCH_STRING = "1.2.2";

QYamlParser::QYamlParser(QTextDocument* doc, QObject* parent)
  : QObject{ parent }
  , m_document(doc)
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
QYamlParser::parseLine(const QString& line)
{
  // TODO
  return false;
}

void
QYamlParser::addScalarToSequence(YamlSequence* sequence,
                                 const QString t,
                                 int& tStart,
                                 int& indent)
{
  auto node = new YamlScalar(t, this);
  sequence->append(node);
  auto cursor = createCursor(tStart);
  node->setStart(cursor);
  cursor = createCursor(tStart + t.length());
  node->setEnd(cursor);
  node->setIndent(indent);
  m_nodes.insert(cursor, node);
  tStart = 0;
  indent = 0;
}

void
QYamlParser::parseYamlDirective(QString s,
                                int& tStart,
                                int i,
                                QChar c,
                                bool hasYamlDirective,
                                int row,
                                int& indent)
{
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
      auto node = new YamlDirective(major, minor, this);
      auto cursor = createCursor(tStart);
      node->setStart(cursor);
      cursor = createCursor(i);
      node->setEnd(cursor);
      node->setRow(row);
      node->setIndent(indent);
      if (major == VERSION_MAJOR) {
        node->setError(InvalidVersionError, false);
        if (minor >= 0 && minor <= VERSION_MINOR) {
          node->setWarning(InvalidMinorVersionWarning, false);
        } else {
          node->setWarning(InvalidMinorVersionWarning, true);
        }
      } else {
        node->setError(InvalidVersionError, true);
      }
      if (hasYamlDirective) {
        node->setError(TooManyYamlDirectivesError, true);
      } else {
        node->setError(TooManyYamlDirectivesError, false);
      }
      m_nodes.insert(cursor, node);
      tStart = 0;
      indent = 0;
    } else {
      // TODO bad version
    }
  }
}

bool
QYamlParser::parse(const QString& text, int startPos)
{
  m_text = text;

  auto row = 0;
  auto start = startPos;
  QString t;
  auto tStart = -1;
  auto indent = 0;
  auto rowStart = 0;
  YamlNode* node = nullptr;
  auto isTagStart = false;
  auto isAnchorStart = false;
  auto isLinkStart = false;
  auto isHyphen = false;
  auto isIndentComplete = false;
  QTextCursor cursor;
  auto hasYamlDirective = false;

  for (auto i = start; i < text.size(); i++) {
    auto c = text.at(i);
    if (c == Characters::NEWLINE) {
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
        m_nodes.insert(comment->start(), comment);
      }
      continue;
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
      auto map = new YamlMap(this);
      map->setStart(createCursor(i));
      map->setFlowType(YamlNode::Flow);
      parseFlowMap(map, ++i, text);
      m_nodes.insert(map->start(), map);
    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
      t += c;
    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
      auto sequence = new YamlSequence(this);
      sequence->setStart(createCursor(i));
      sequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(sequence, ++i, text);
      m_nodes.insert(sequence->start(), sequence);
    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
      t += c;
    } else if (c == '-') {
      if (!isHyphen) {
        isHyphen = true;
        tStart = i;
      }
      t += c;
      if (t == "---") {
        //        isDocStart = true;
        node = new YamlStart(this);
        cursor = createCursor(tStart);
        node->setStart(cursor);
        cursor = createCursor(i);
        node->setEnd(cursor);
        node->setRow(row);
        node->setIndent(indent);
        m_nodes.insert(cursor, node);
        tStart = 0;
        indent = 0;
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

  //  QRegularExpressionMatch n;
  //  auto p = 0, l = 0;
  //  QString t;
  //  QTextCursor cursor;

  //  QList<QYamlDocument*> documents;
  //  YamlDocument* doc = nullptr;
  //  QList<YamlNode*> nodes;

  //  auto it = YAML_DIRECTIVE.globalMatch(text);
  //  while (it.hasNext()) {
  //    n = it.next();
  //    p = n.capturedStart();
  //    l = n.capturedLength();
  //    t = n.captured().mid(5).trimmed();
  //    auto splits = t.split(".", Qt::SkipEmptyParts);
  //    if (splits.size() == 2) {
  //      int major = splits.at(0).toInt();
  //      int minor = splits.at(1).toInt();
  //      auto node = new YamlDirective(major, minor, this);
  //      cursor = createCursor(p);
  //      node->setStart(cursor);
  //      node->setLength(l);
  //      nodes.append(node);
  //    }
  //  }

  //  bool noStarts = true;
  //  it = YAML_DOCSTART.globalMatch(text);
  //  while (it.hasNext()) {
  //    noStarts = false;
  //    n = it.next();
  //    p = n.capturedStart();
  //    l = n.capturedLength();
  //    auto node = new YamlStart(this);
  //    cursor = createCursor(p);
  //    node->setStart(cursor);
  //    node->setLength(l);
  //    nodes.append(node);
  //  }

  //  if (!noStarts) {
  //    it = YAML_DOCEND.globalMatch(text);
  //    while (it.hasNext()) {
  //      n = it.next();
  //      p = n.capturedStart();
  //      l = n.capturedLength();
  //      auto node = new YamlEnd(this);
  //      cursor = createCursor(p);
  //      node->setStart(cursor);
  //      node->setLength(l);
  //      nodes.append(node);
  //    }
  //  }

  //  if (nodes.size() == 0) { // no directives/start/end
  //    doc = new YamlDocument(this);
  //    doc->setMajorVersion(VERSION_MAJOR);
  //    doc->setMinorVersion(VERSION_MINOR);
  //    doc->setImplicitVersion(true);
  //    doc->setImplicitStart(true);
  //    doc->setImplicitEnd(true);
  //    documents.append(doc);
  //  } else {
  //    bool directive = false, start = false, end = false;
  //    for (int i = 0; i < nodes.size(); i++) {
  //      auto value = nodes.at(i);
  //      auto d = qobject_cast<YamlDirective*>(value);
  //      if (d) {
  //        directive = true;
  //      }
  //    }
  //  }

  return true;
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
        auto scalar = new YamlScalar(t, this);
        sequence->append(scalar);
        m_nodes.insert(scalar->start(), scalar);
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
    } else if (c == Characters::HASH) { // comment
      auto comment = parseComment(i, text);
      if (comment) {
        m_nodes.insert(comment->start(), comment);
      }
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
      auto subMap = new YamlMap(this);
      subMap->setStart(createCursor(i));
      parseFlowMap(subMap, ++i, text);
      sequence->append(subMap);
      m_nodes.insert(subMap->start(), subMap);
      continue;
    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // end sequence flow
      auto subSequence = new YamlSequence(this);
      subSequence->setStart(createCursor(i));
      subSequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(subSequence, ++i, text);
      sequence->append(subSequence);
      m_nodes.insert(subSequence->start(), subSequence);
      return;
    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
      if (!t.isEmpty()) { // only if missing final comma
        auto scalar = new YamlScalar(t, this);
        sequence->append(scalar);
        m_nodes.insert(scalar->start(), scalar);
        t.clear();
      }
      sequence->setEnd(createCursor(i + 1));
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
  while (i < text.length()) {
    auto c = text.at(i);
    if (c == Characters::NEWLINE) {
      qWarning();
    } else if (c == Characters::COLON) {
      if (!t.isEmpty()) {
        key = t;
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
    } else if (c == Characters::HASH) { // comment
      // TODO comments ALWAYS finish scalars.
      auto comment = parseComment(i, text);
      if (comment) {
        m_nodes.insert(comment->start(), comment);
      }
    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
      auto subMap = new YamlMap(this);
      subMap->setStart(createCursor(i));
      parseFlowMap(subMap, ++i, text);
      if (!key.isEmpty()) {
        map->insert(key, subMap);
        m_nodes.insert(subMap->start(), subMap);
        key.clear();
      } else {
        // TODO error map item has no key.
      }
      continue;
    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // end sequence flow
      auto subSequence = new YamlSequence(this);
      subSequence->setStart(createCursor(i));
      subSequence->setFlowType(YamlNode::Flow);
      parseFlowSequence(subSequence, ++i, text);
      if (!key.isEmpty()) {
        map->insert(key, subSequence);
        m_nodes.insert(subSequence->start(), subSequence);
        key.clear();
      } else {
        // TODO error map item has no key.
      }
      continue;
    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
      if (!t.isEmpty()) { // only if missing final comma
        auto scalar = new YamlScalar(t, this);
        if (!key.isEmpty()) {
          map->insert(key, scalar);
          m_nodes.insert(scalar->start(), scalar);
          key.clear();
        } else {
          // TODO error map item has no key.
        }
        t.clear();
      }
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

bool
QYamlParser::resolveAnchors()
{
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

QString
QYamlParser::text() const
{
  return m_text;
}

const QMap<QTextCursor, YamlNode*>&
QYamlParser::nodes() const
{
  return m_nodes;
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
