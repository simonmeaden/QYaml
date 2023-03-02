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
const QString QYamlParser::YAML = QStringLiteral("YAML");
const QString QYamlParser::TAG = QStringLiteral("TAG");
// const QString QYamlParser::DOCSTART = QStringLiteral("^[\\s]*---");
// const QString QYamlParser::IND_DOCSTART = QStringLiteral("---");
// const QString QYamlParser::DOCEND = QStringLiteral("^...");
// const QString QYamlParser::IND_DOCEND = QStringLiteral("[\\s]*...");
// const QRegularExpression
// QYamlParser::YAML_DIRECTIVE("%YAML\\s+1[.][0-3]\\s*"); const
// QRegularExpression QYamlParser::TAG_DIRECTIVE("%TAG\\s+[!\\w\\s:.,]*");

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

void
QYamlParser::createDocIfNull(int start, SharedDocument& currentDoc)
{
  if (!currentDoc) {
    currentDoc = SharedDocument(new QYamlDocument());
    currentDoc->setStart(createCursor(start));
  }
}

// SharedComment
// QYamlParser::storeCommentIfExists(SharedDocument currentDoc,
//                                   QString comment,
//                                   int& start)
//{
//   if (!comment.isEmpty()) {
//     auto yamlcomment = SharedComment(new YamlComment(comment, this));
//     yamlcomment->setStart(createCursor(start));
//     start += comment.length();
//     yamlcomment->setEnd(createCursor(start));
//     start++; // newline
//     currentDoc->addNode(yamlcomment);
//     comment.clear();
//     return yamlcomment;
//   }
//   return nullptr;
// }

void
QYamlParser::storeNode(SharedDocument currentDoc,
                       SharedNode node,
                       int& start,
                       int length)
{
  if (node) {
    node->setStart(createCursor(start));
    start += length;
    node->setEnd(createCursor(start));
    start++;
    currentDoc->addNode(node);
  }
}

bool
QYamlParser::parse(const QString& text, int startPos, int length)
{
  m_text = text;

  c_printable(Characters::NW_ARROW_BAR);

  auto row = 0;
  auto pos = startPos;
  auto indent = 0;
  auto rowStart = 0;
  SharedNode node = nullptr;
  auto isTagStart = false;
  auto isAnchorStart = false;
  auto isLinkStart = false;
  auto isHyphen = false;
  auto isIndentComplete = false;
  auto hasYamlDirective = false;
  SharedDocument currentDoc = nullptr;
  bool directivesEnd = false;

  SharedNode sharednode = nullptr;
  SharedComment sharedcomment = nullptr;

  auto lines = text.split(Characters::NEWLINE);
  for (auto& line : lines) {
    createDocIfNull(pos, currentDoc);
    if (l_directive(line, pos, sharednode, sharedcomment)) {
      pos++; // step past NL
      // TODO error no directives after directives end
      if (sharednode) {
        if (sharednode->type() == YamlNode::YamlDirective) {
          if (hasYamlDirective) {
            sharednode->setError(YamlError::TooManyYamlDirectivesError, true);
          }
          hasYamlDirective = true;
        }
        currentDoc->addDirective(sharednode);
        sharednode = nullptr;
      }
      if (sharedcomment) {
        currentDoc->addNode(sharedcomment);
        sharedcomment = nullptr;
      }
      continue;
    }

    if (s_l_comments(line, pos, sharedcomment)) {
      if (sharedcomment) {
        currentDoc->addNode(sharedcomment);
        sharedcomment = nullptr;
      }
      pos++; // step past NL

      continue;
    }

    SharedAnchorBase property;
    auto s = text.mid(pos);
    if (c_ns_anchor_property(line, property)) {

    }

    if (c_directives_end(line, pos, sharednode)) { // ---
      pos++;                                       // step past NL
      if (sharednode) {
        currentDoc->addNode(sharednode, true);
        sharednode = nullptr;
        directivesEnd = true;
      }
      continue;
    }

    if (l_document_suffix(
          line, pos, sharednode, sharedcomment)) { //... plus optional comment
      pos++;                                       // step past NL
      if (sharednode) {
        currentDoc->addNode(sharednode, true);
        sharednode = nullptr;
        if (sharedcomment) {
          currentDoc->addNode(sharedcomment);
          sharedcomment = nullptr;
        }
        currentDoc->setEnd(createCursor(pos));
        m_documents.append(currentDoc);
        currentDoc = nullptr;
        directivesEnd = false; // directives can start again.
      }
      continue;
    }
  }

  if (currentDoc)
    m_documents.append(currentDoc);

  if (resolveAnchors()) {
    // TODO errors
  }

  emit parseComplete();

  return true;
}

// void
// QYamlParser::buildDocuments(const QString& text,
//                             QList<SharedNode> nodes,
//                             QList<SharedNode> rootNodes)
//{
//   SharedDocument doc = nullptr;
//   auto docStarted = false;

//  for (int i = 0; i < nodes.size(); i++) {
//    auto node = nodes.at(i);
//    auto type = node->type();
//    if (!doc) {
//      doc = SharedDocument(new QYamlDocument(this));
//      m_documents.append(doc);
//    }

//    if (!docStarted) {
//      if (type == YamlNode::YamlDirective) {
//        if (doc->hasDirective()) {
//          // TODO error - there must be two in this document
//          // should't happen here - new document.
//          node->setError(YamlError::TooManyYamlDirectivesError, true);
//        }
//        doc->setDirective(qSharedPointerDynamicCast<YamlYamlDirective>(node));
//        // if the document has a %YAML directive then set start to it's start.
//        doc->setStart(node->start());
//        continue;
//      }
//    } else {
//      if (type == YamlNode::YamlDirective) {
//        // this is either a new document or an error
//        // TODO handle error
//        if (doc->hasDirective()) {
//          // TODO error - there must be two in this document
//          node->setError(YamlError::TooManyYamlDirectivesError, true);
//        }
//        i--; // step back before directive.
//        if (!doc->hasEnd()) {
//          // create end position but no node.
//          doc->setEnd(createCursor(node->startPos() - 1));
//        }
//        doc = nullptr;
//        docStarted = false;
//        continue;
//      }
//    }

//    if (!docStarted) {
//      if (type == YamlNode::TagDirective) {
//        doc->addTag(qobject_cast<YamlTagDirective*>(node));
//        if (!doc->hasDirective() && !doc->hasTag()) {
//          // if the document has no %YAML directive then set start to
//          // the first tag directives start.
//          doc->setStart(node->start());
//        }
//        continue;start
//      }
//    } else {
//      if (type == YamlNode::TagDirective) {
//        // this is either a new document or an error
//        // TODO handle error
//        i--; // step back before directive.
//        if (!doc->hasEnd()) {
//          // create end position but no node.
//          doc->setEnd(createCursor(node->startPos() - 1));
//        }
//        doc = nullptr;
//        docStarted = false;
//        continue;
//      }
//    }

//    if (!docStarted) {
//      if (type == YamlNode::Start) {
//        if (doc->hasStart()) {
//          doc->setStart(doc->start(), qobject_cast<YamlStart*>(node));
//        } else {
//          doc->setStart(node->start(), qobject_cast<YamlStart*>(node));
//        }
//        docStarted = true;
//      } else {
//        if (type == YamlNode::End) {
//          // TODO error??
//        } else if (!(type == YamlNode::YamlDirective ||
//                     type == YamlNode::TagDirective)) {
//          doc->addNode(node, rootNodes.contains(node));
//        }
//      }
//      continue;
//    }

//    if (docStarted) {
//      if (type == YamlNode::End) {
//        doc->setEnd(node->end(), qobject_cast<YamlEnd*>(node));
//        doc = nullptr;
//        docStarted = false;
//      } else if (type == YamlNode::Start) {
//        i--; // step back before start.
//        if (!doc->hasEnd()) {
//          // create end position but no node.
//          doc->setEnd(createCursor(node->startPos() - 1));
//        }
//        doc = nullptr;
//        docStarted = false;
//      } else if (!(type == YamlNode::YamlDirective ||
//                   type == YamlNode::TagDirective || type == YamlNode::Start
//                   || type == YamlNode::End)) {
//        doc->addNode(node, rootNodes.contains(node));
//      }
//    }
//  }

//  if (doc && !doc->hasEnd()) {
//    doc->setEnd(createCursor(text.length()));
//  }
//}

// void
// QYamlParser::parseFlowSequence(SharedSequence sequence,
//                                int& i,
//                                const QString& text)
//{
//   QString t;

//  while (i < text.length()) {
//    auto c = text.at(i);
//    if (c == Characters::COMMA) {
//      if (t.trimmed().isEmpty()) {
//        auto scalar = parseFlowScalar(t, i);
//        sequence->append(scalar);
//        t.clear();
//      } else {
//        auto scalar = parseFlowScalar(t, i);
//        sequence->append(scalar);
//        t.clear();
//      }
//    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start map flow
//      auto subMap = QSharedPointer<YamlMap>(new YamlMap(this));
//      subMap->setStart(createCursor(i));
//      subMap->setFlowType(YamlNode::Flow);
//      parseFlowMap(subMap, ++i, text);
//      sequence->append(subMap);
//      continue;
//    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
//      // TODO error?
//      qWarning();
//    } else if (c == Characters::OPEN_SQUARE_BRACKET) {
//      // end sequence flow
//      auto subSequence = SharedSequence(new YamlSequence(this));
//      subSequence->setStart(createCursor(i));
//      subSequence->setFlowType(YamlNode::Flow);
//      parseFlowSequence(subSequence, ++i, text);
//      sequence->append(subSequence);
//    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
//      sequence->setEnd(createCursor(i));
//      if (!t.isEmpty()) { // only if missing final comma
//        auto scalar = parseFlowScalar(t, i);
//        sequence->append(scalar);
//        t.clear();
//      }
//      i++;
//      return;
//    } else {
//      if (t.isEmpty() && c == Characters::SPACE) {
//        i++;
//        continue;
//      }
//      t += c;
//    }
//    i++;
//  }
//}

// void
// QYamlParser::parseFlowMap(QSharedPointer<YamlMap> map,
//                           int& i,
//                           const QString& text)
//{
//   QString t;
//   QString key;
//   int keyStart = -1;
//   bool stringLiteral = false;
//   bool foldedLiteral = false;

//  while (i < text.length()) {
//    auto c = text.at(i);
//    if (c == Characters::NEWLINE) {
//      if (!t.isEmpty()) {
//        t += Characters::SPACE; // new lines inside strings are ignored.
//      }
//      i++;
//      continue;
//    } else if (c == Characters::COLON) {
//      if (!t.isEmpty()) {
//        key = t;
//        keyStart = i - key.length();
//        t.clear();
//      }
//    } else if (c == Characters::VERTICAL_LINE) {
//      // TODO flow string literal.
//      if (t.isEmpty()) {
//        stringLiteral = true;
//      }
//    } else if (c == Characters::GT) {
//      // TODO flow string literal.
//      if (t.isEmpty()) {
//        foldedLiteral = true;
//      }
//    } else if (c == Characters::COMMA) {
//      // TODO maps & sequences at comma
//      if (!t.isEmpty() && !key.isEmpty()) {
//        auto scalar = parseFlowScalar(t, i);
//        auto item = QSharedPointer<YamlMapItem>(new YamlMapItem(key, scalar));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(scalar->end());
//        map->insert(key, item);
//        t.clear();
//        key.clear();
//        keyStart = -1;
//      }
//    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start sub map flow
//      auto subMap = QSharedPointer<YamlMap>(new YamlMap());
//      subMap->setStart(createCursor(i));
//      subMap->setFlowType(YamlNode::Flow);
//      parseFlowMap(subMap, ++i, text);
//      if (!key.isEmpty()) {
//        auto item = QSharedPointer<YamlMapItem>(new YamlMapItem(key, subMap));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(subMap->end());
//        map->insert(key, item);
//        key.clear();
//        keyStart = -1;
//      } else {
//        // TODO error map item has no key.
//      }
//      continue;
//    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
//      map->setEnd(createCursor(i));
//      if (!t.isEmpty() && !key.isEmpty()) { // only if missing final comma
//        auto scalar = parseFlowScalar(t, i);
//        auto item = QSharedPointer<YamlMapItem>(new YamlMapItem(key, scalar));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(createCursor(i));
//        map->insert(key, item);
//        t.clear();
//        key.clear();
//        keyStart = -1;
//      }
//      i++;
//      return;
//    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
//      auto subSequence = SharedSequence(new YamlSequence());
//      subSequence->setStart(createCursor(i));
//      subSequence->setFlowType(YamlNode::Flow);
//      parseFlowSequence(subSequence, ++i, text);
//      if (!key.isEmpty()) {
//        auto item =
//          QSharedPointer<YamlMapItem>(new YamlMapItem(key, subSequence));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(subSequence->end());
//        map->insert(key, item);
//        key.clear();
//        keyStart = -1;
//      } else {
//        // TODO error map item has no key.
//      }
//      continue;
//    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
//      // TODO error
//      qWarning();
//    } else {
//      if (t.isEmpty() && c == Characters::SPACE) {
//        i++;
//        continue;
//      }
//      t += c;//void
// QYamlParser::parseFlowMap(QSharedPointer<YamlMap> map,
//                          int& i,
//                          const QString& text)
//{
//  QString t;
//  QString key;
//  int keyStart = -1;
//  bool stringLiteral = false;
//  bool foldedLiteral = false;

//  while (i < text.length()) {
//    auto c = text.at(i);
//    if (c == Characters::NEWLINE) {
//      if (!t.isEmpty()) {
//        t += Characters::SPACE; // new lines inside strings are ignored.
//      }
//      i++;
//      continue;
//    } else if (c == Characters::COLON) {
//      if (!t.isEmpty()) {
//        key = t;
//        keyStart = i - key.length();
//        t.clear();
//      }
//    } else if (c == Characters::VERTICAL_LINE) {
//      // TODO flow string literal.
//      if (t.isEmpty()) {
//        stringLiteral = true;
//      }
//    } else if (c == Characters::GT) {
//      // TODO flow string literal.
//      if (t.isEmpty()) {
//        foldedLiteral = true;
//      }
//    } else if (c == Characters::COMMA) {
//      // TODO maps & sequences at comma
//      if (!t.isEmpty() && !key.isEmpty()) {
//        auto scalar = parseFlowScalar(t, i);
//        auto item = QSharedPointer<YamlMapItem>(new YamlMapItem(key, scalar));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(scalar->end());
//        map->insert(key, item);
//        t.clear();
//        key.clear();
//        keyStart = -1;
//      }
//    } else if (c == Characters::OPEN_CURLY_BRACKET) { // start sub map flow
//      auto subMap = QSharedPointer<YamlMap>(new YamlMap());
//      subMap->setStart(createCursor(i));
//      subMap->setFlowType(YamlNode::Flow);
//      parseFlowMap(subMap, ++i, text);
//      if (!key.isEmpty()) {
//        auto item = QSharedPointer<YamlMapItem>(new YamlMapItem(key, subMap));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(subMap->end());
//        map->insert(key, item);
//        key.clear();
//        keyStart = -1;
//      } else {
//        // TODO error map item has no key.
//      }
//      continue;
//    } else if (c == Characters::CLOSE_CURLY_BRACKET) { // end map flow
//      map->setEnd(createCursor(i));
//      if (!t.isEmpty() && !key.isEmpty()) { // only if missing final comma
//        auto scalar = parseFlowScalar(t, i);
//        auto item = QSharedPointer<YamlMapItem>(new YamlMapItem(key, scalar));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(createCursor(i));
//        map->insert(key, item);
//        t.clear();
//        key.clear();
//        keyStart = -1;
//      }
//      i++;
//      return;
//    } else if (c == Characters::OPEN_SQUARE_BRACKET) { // start sequence flow
//      auto subSequence = SharedSequence(new YamlSequence());
//      subSequence->setStart(createCursor(i));
//      subSequence->setFlowType(YamlNode::Flow);
//      parseFlowSequence(subSequence, ++i, text);
//      if (!key.isEmpty()) {
//        auto item =
//          QSharedPointer<YamlMapItem>(new YamlMapItem(key, subSequence));
//        item->setStart(createCursor(keyStart));
//        item->setEnd(subSequence->end());
//        map->insert(key, item);
//        key.clear();
//        keyStart = -1;
//      } else {
//        // TODO error map item has no key.
//      }
//      continue;
//    } else if (c == Characters::CLOSE_SQUARE_BRACKET) { // end sequence flow
//      // TODO error
//      qWarning();
//    } else {
//      if (t.isEmpty() && c == Characters::SPACE) {
//        i++;
//        continue;
//      }
//      t += c;
//    }
//    i++;
//  }
//}

// QSharedPointer<YamlComment>
// QYamlParser::parseComment(int& i, const QString& text)
//{
//   auto comment = QSharedPointer<YamlComment>(new YamlComment());
//   comment->setStart(createCursor(i));
//   // todo set indent
//   ++i;
//   while (i < text.length()) {
//     auto c = text.at(i);
//     if (c == '\n') {
//       comment->append(c);
//       comment->setEnd(createCursor(i));
//       return comment;
//     }
//     comment->append(c);
//     i++;
//   }
//   return nullptr;
// }

// QSharedPointer<YamlScalar>
// QYamlParser::parseFlowScalar(const QString& text, int i)
//{
//   auto indent = 0, nl = 0;
//   int textlength = text.length();
//   auto t = text;
//   while (t.startsWith(Characters::NEWLINE)) {
//     nl++;
//     t = t.mid(1);
//   }
//   while (t.at(0).isSpace()) {
//     indent++;
//     t = t.mid(1);
//   }
//   //  t = StringUtil::lTrim(t, indent);
//   auto trimmed = t.trimmed();

//  auto firstChar = trimmed.at(0);  // TODO to at when working.
//  auto secondChar = trimmed.at(1); // TODO to at when working.
//  auto start = i - textlength + indent + nl;
//  auto scalar = QSharedPointer<YamlScalar>(new YamlScalar(trimmed));
//  scalar->setStart(createCursor(start));
//  scalar->setEnd(createCursor(i));

//  auto splits = trimmed.split(Characters::NEWLINE, Qt::KeepEmptyParts);
//  auto count = 0;
//  QMap<int, YamlWarning> warnings;
//  for (auto i = 0; i < splits.size(); i++) {
//    auto s = splits.at(i);
//    if (s.isEmpty()) {
//      warnings.insert(count, YamlWarning::NoWarnings);
//      count++;
//    }
//    if (i <= splits.size() - 1) {
//      if (s.contains(Characters::HASH)) {
//        warnings.insert(count, YamlWarning::PossibleCommentInScalar);
//      } else {
//        warnings.insert(count, YamlWarning::NoWarnings);
//      }
//      count += s.length();
//    }
//  }
//  for (auto [c, t] : asKeyValueRange(warnings)) {
//    if (t == YamlWarning::NoWarnings) {
//      continue;
//    } else if (t == YamlWarning::PossibleCommentInScalar) {
//      if (t != warnings.last()) {
//        int l = trimmed.indexOf(Characters::HASH, c);
//        scalar->addDodgyChar(createCursor(c + l),
//                             YamlWarning::PossibleCommentInScalar);
//      }
//    }
//  }

//  auto p = 0;
//  while ((p = t.indexOf(Characters::TAB, p)) != -1) {
//    scalar->addDodgyChar(createCursor(i - textlength + indent + nl + p),
//                         YamlWarning::TabCharsDiscouraged);
//    p++;
//  }

//  if ((firstChar == Characters::COLON || firstChar == Characters::HASH) &&
//      secondChar.isSpace()) {
//    scalar->setError(YamlError::IllegalFirstCharacter, true);
//  } else if (firstChar == Characters::AMPERSAND ||
//             firstChar == Characters::EXCLAMATIONMARK ||
//             firstChar == Characters::ASTERISK ||
//             firstChar == Characters::VERTICAL_LINE ||
//             firstChar == Characters::GT ||
//             firstChar == Characters::COMMERCIAL_AT ||
//             firstChar == Characters::BACKTICK) {
//    scalar->setError(YamlError::IllegalFirstCharacter, true);
//  } else if (firstChar == Characters::OPEN_CURLY_BRACKET) {
//    // TODO distinguish from start map??
//    qWarning();
//  } else if (firstChar == Characters::OPEN_SQUARE_BRACKET) {
//    // TODO distinguish from start sequence??
//    qWarning();
//  } else if (firstChar == Characters::CLOSE_CURLY_BRACKET) {
//    // TODO distinguish from start map??
//    qWarning();
//  } else if (firstChar == Characters::CLOSE_SQUARE_BRACKET) {
//    // TODO distinguish from start sequence??
//    qWarning();
//  } else if (firstChar == Characters::COMMA) {
//    // TODO distinguish from extra comma??
//    qWarning();
//  } else if (firstChar == Characters::DOUBLEQUOTE) {
//    if (trimmed.endsWith(Characters::DOUBLEQUOTE)) {
//      scalar->setStyle(YamlScalar::DOUBLEQUOTED);
//    } else {
//      scalar->setError(YamlError::IllegalFirstCharacter, true);
//      scalar->setError(YamlError::MissingMatchingQuote, true);
//    }
//  } else if (firstChar == Characters::SINGLEQUOTE) {
//    if (trimmed.endsWith(Characters::SINGLEQUOTE)) {
//      scalar->setStyle(YamlScalar::SINGLEQUOTED);
//    } else {
//      scalar->setError(YamlError::IllegalFirstCharacter, true);
//      scalar->setError(YamlError::MissingMatchingQuote, true);
//    }
//  }
//  return scalar;
//}

//    }
//    i++;
//  }
//}

// QSharedPointer<YamlComment>
// QYamlParser::parseComment(int& i, const QString& text)
//{
//   auto comment = QSharedPointer<YamlComment>(new YamlComment());
//   comment->setStart(createCursor(i));
//   // todo set indent
//   ++i;
//   while (i < text.length()) {
//     auto c = text.at(i);
//     if (c == '\n') {
//       comment->append(c);
//       comment->setEnd(createCursor(i));
//       return comment;
//     }
//     comment->append(c);
//     i++;
//   }
//   return nullptr;
// }

// QSharedPointer<YamlScalar>
// QYamlParser::parseFlowScalar(const QString& text, int i)
//{
//   auto indent = 0, nl = 0;
//   int textlength = text.length();
//   auto t = text;
//   while (t.startsWith(Characters::NEWLINE)) {
//     nl++;
//     t = t.mid(1);
//   }
//   while (t.at(0).isSpace()) {
//     indent++;
//     t = t.mid(1);
//   }
//   //  t = StringUtil::lTrim(t, indent);
//   auto trimmed = t.trimmed();

//  auto firstChar = trimmed.at(0);  // TODO to at when working.
//  auto secondChar = trimmed.at(1); // TODO to at when working.
//  auto start = i - textlength + indent + nl;
//  auto scalar = QSharedPointer<YamlScalar>(new YamlScalar(trimmed));
//  scalar->setStart(createCursor(start));
//  scalar->setEnd(createCursor(i));

//  auto splits = trimmed.split(Characters::NEWLINE, Qt::KeepEmptyParts);
//  auto count = 0;
//  QMap<int, YamlWarning> warnings;
//  for (auto i = 0; i < splits.size(); i++) {
//    auto s = splits.at(i);
//    if (s.isEmpty()) {
//      warnings.insert(count, YamlWarning::NoWarnings);
//      count++;
//    }
//    if (i <= splits.size() - 1) {
//      if (s.contains(Characters::HASH)) {
//        warnings.insert(count, YamlWarning::PossibleCommentInScalar);
//      } else {
//        warnings.insert(count, YamlWarning::NoWarnings);
//      }
//      count += s.length();
//    }
//  }
//  for (auto [c, t] : asKeyValueRange(warnings)) {
//    if (t == YamlWarning::NoWarnings) {
//      continue;
//    } else if (t == YamlWarning::PossibleCommentInScalar) {
//      if (t != warnings.last()) {
//        int l = trimmed.indexOf(Characters::HASH, c);
//        scalar->addDodgyChar(createCursor(c + l),
//                             YamlWarning::PossibleCommentInScalar);
//      }
//    }
//  }

//  auto p = 0;
//  while ((p = t.indexOf(Characters::TAB, p)) != -1) {
//    scalar->addDodgyChar(createCursor(i - textlength + indent + nl + p),
//                         YamlWarning::TabCharsDiscouraged);
//    p++;
//  }

//  if ((firstChar == Characters::COLON || firstChar == Characters::HASH) &&
//      secondChar.isSpace()) {
//    scalar->setError(YamlError::IllegalFirstCharacter, true);
//  } else if (firstChar == Characters::AMPERSAND ||
//             firstChar == Characters::EXCLAMATIONMARK ||
//             firstChar == Characters::ASTERISK ||
//             firstChar == Characters::VERTICAL_LINE ||
//             firstChar == Characters::GT ||
//             firstChar == Characters::COMMERCIAL_AT ||
//             firstChar == Characters::BACKTICK) {
//    scalar->setError(YamlError::IllegalFirstCharacter, true);
//  } else if (firstChar == Characters::OPEN_CURLY_BRACKET) {
//    // TODO distinguish from start map??
//    qWarning();
//  } else if (firstChar == Characters::OPEN_SQUARE_BRACKET) {
//    // TODO distinguish from start sequence??
//    qWarning();
//  } else if (firstChar == Characters::CLOSE_CURLY_BRACKET) {
//    // TODO distinguish from start map??
//    qWarning();
//  } else if (firstChar == Characters::CLOSE_SQUARE_BRACKET) {
//    // TODO distinguish from start sequence??
//    qWarning();
//  } else if (firstChar == Characters::COMMA) {
//    // TODO distinguish from extra comma??
//    qWarning();
//  } else if (firstChar == Characters::DOUBLEQUOTE) {
//    if (trimmed.endsWith(Characters::DOUBLEQUOTE)) {
//      scalar->setStyle(YamlScalar::DOUBLEQUOTED);
//    } else {
//      scalar->setError(YamlError::IllegalFirstCharacter, true);
//      scalar->setError(YamlError::MissingMatchingQuote, true);
//    }
//  } else if (firstChar == Characters::SINGLEQUOTE) {
//    if (trimmed.endsWith(Characters::SINGLEQUOTE)) {
//      scalar->setStyle(YamlScalar::SINGLEQUOTED);
//    } else {
//      scalar->setError(YamlError::IllegalFirstCharacter, true);
//      scalar->setError(YamlError::MissingMatchingQuote, true);
//    }
//  }
//  return scalar;
//}

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

QList<SharedDocument>
QYamlParser::documents() const
{
  return m_documents;
}

SharedDocument
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
QYamlParser::setDocuments(QList<SharedDocument> root)
{
  m_documents = root;
}

void
QYamlParser::append(SharedDocument document)
{
  m_documents.append(document);
}

bool
QYamlParser::isMultiDocument()
{
  return (m_documents.size() > 1);
}

SharedNode
QYamlParser::nodeAt(QTextCursor cursor)
{
  for (auto doc : m_documents) {
    for (auto node : doc->nodes()) {
      SharedNode n;
      if ((n = nodeOrRecurse(cursor, node))) {
        return n;
      }
    }
  }
  return nullptr;
}

SharedNode
QYamlParser::nodeInSequence(QTextCursor cursor, SharedSequence seq)
{
  if (!seq->data().isEmpty()) {
    for (auto node : seq->data()) {
      SharedNode n;
      if ((n = nodeOrRecurse(cursor, node))) {
        return n;
      }
    }
  }

  return seq;
}

SharedNode
QYamlParser::nodeInMap(QTextCursor cursor, QSharedPointer<YamlMap> map)
{
  if (!map->data().isEmpty()) {
    for (auto node : map->data()) {
      SharedNode n;
      if ((n = nodeOrRecurse(cursor, node))) {
        return n;
      }
      break;
    }
  }
  return map;
}

SharedNode
QYamlParser::nodeOrRecurse(QTextCursor cursor, SharedNode node)
{
  if (cursor >= node->start() && cursor < node->end()) {
    switch (node->type()) {
      case YamlNode::Undefined:
        // should never happen.
        break;
      case YamlNode::YamlDirective:
      case YamlNode::TagDirective:
      case YamlNode::Directive:
      case YamlNode::ReservedDirective:
      case YamlNode::Start:
      case YamlNode::End:
      case YamlNode::MapItem:
      case YamlNode::Comment:
      case YamlNode::Anchor:
        return node;
      case YamlNode::Scalar:
        return node;
      case YamlNode::Sequence:
        return nodeInSequence(cursor,
                              qSharedPointerDynamicCast<YamlSequence>(node));
      case YamlNode::Map:
        return nodeInMap(cursor, qSharedPointerDynamicCast<YamlMap>(node));
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
QYamlParser::l_directive(const QString& line,
                         int& start,
                         SharedNode& d,
                         SharedComment& c)
{
  if (line.isEmpty())
    return false;
  auto s = line;
  if (c_directive(s.at(0))) {
    if (ns_yaml_directive(s, start, d, c)) {
      return true;
    }
    if (ns_tag_directive(s, start, d, c)) {
      return true;
    }
    if (ns_reserved_directive(s, start, d, c)) {
      return true;
    }
  }
  return false;
}

void
QYamlParser::bypassTextAndUpdatePos(QString& s, int& pos, QString result)
{
  auto len = result.length();
  pos += len;
  s = s.mid(len);
}

void
QYamlParser::bypassWhitespaceAndUpdatePos(QString& s, int& pos)
{
  auto len = initial_whitespace(s);
  pos += len;
  s = s.mid(len);
}

bool
QYamlParser::ns_reserved_directive(const QString& line,
                                   int& start,
                                   SharedNode& sharednode,
                                   SharedComment& sharedcomment)
{
  if (line.isEmpty())
    return false;
  auto len = 0;
  auto pos = start + 1;
  auto s = line.mid(1);
  QString result;
  bool invalidSpace = false;
  len = initial_whitespace(s);
  if (len > 0)
    invalidSpace = true;
  pos += len;
  s = s.mid(len);

  sharednode.reset(new YamlReservedDirective());
  auto directive = qSharedPointerDynamicCast<YamlReservedDirective>(sharednode);
  directive->setStart(createCursor(start));
  if (invalidSpace)
    directive->setWarning(InvalidSpaceWarning, true);

  if (ns_directive_name(s, result)) {
    directive->setNameStart(createCursor(pos));
    directive->setName(result);

    bypassTextAndUpdatePos(s, pos, result);
    bypassWhitespaceAndUpdatePos(s, pos);

    // Only YAML and TAG directives are allowed under YAML 1.1/1.2
    // Other directives are reserved for future expansion
    directive->setWarning(YamlWarning::ReservedDirectiveWarning, true);
    while (ns_directive_parameter(s, result)) {
      if (result == Characters::HASH) {
        break;
      }
      directive->addParameter(createCursor(pos), result);

      bypassTextAndUpdatePos(s, pos, result);
      sharednode->setEnd(createCursor(pos));
      bypassWhitespaceAndUpdatePos(s, pos);
    }
  }

  auto p = 0;
  if (!s.isEmpty()) {
    bypassWhitespaceAndUpdatePos(s, pos);
    s_l_comments(s, p, sharedcomment);
    if (sharedcomment) {
      sharedcomment->setStart(createCursor(pos));
      sharedcomment->setEnd(createCursor(pos + p));
      start = sharedcomment->endPos();
      return true;
    }
  }

  start = sharednode->endPos();
  return false;
}

bool
QYamlParser::ns_directive_parameter(const QString& s, QString& param)
{
  return ns_char_plus(s, param);
}

bool
QYamlParser::ns_directive_name(const QString& s, QString& name)
{
  return ns_char_plus(s, name);
}

bool
QYamlParser::ns_tag_directive(const QString& line,
                              int& start,
                              SharedNode& sharednode,
                              SharedComment& sharedcomment)
{
  if (line.isEmpty())
    return false;
  auto s = line.mid(1);
  auto len = 0;
  auto pos = start + 1;
  bool invalidSpace = false;
  len = initial_whitespace(s);
  if (len > 0) {
    invalidSpace = true;
    pos += len;
    s = s.mid(len);
  }

  if (!s.startsWith(TAG))
    return false;

  YamlTagDirective::TagHandleType type = YamlTagDirective::NoTagType;
  sharednode.reset(new YamlTagDirective());
  auto directive = qSharedPointerDynamicCast<YamlTagDirective>(sharednode);
  if (invalidSpace)
    directive->setWarning(InvalidSpaceWarning, true);
  directive->setStart(createCursor(start));
  directive->setName(TAG);
  directive->setNameStart(createCursor(pos));
  bypassTextAndUpdatePos(s, pos, TAG);

  bypassWhitespaceAndUpdatePos(s, pos);

  QString result;
  c_tag_handle(s, result, type);

  if (type == YamlTagDirective::Named) {
    directive->setHandleStart(createCursor(pos + 1));
    len = result.length() + 2;
    pos += len;
    s = s.mid(len);
    directive->setHandle(result); // only Named has a handle
  } else if (type == YamlTagDirective::Secondary) {
    s = s.mid(2);
    pos += 2;
  } else if (type == YamlTagDirective::Primary) {
    s = s.mid(1);
    pos += 1;
  }
  directive->setHandleType(type);
  bypassWhitespaceAndUpdatePos(s, pos);

  if (!s.isEmpty()) {
    directive->setValueStart(createCursor(pos));
    ns_directive_parameter(s, result);
    directive->setValue(result);
    bypassTextAndUpdatePos(s, pos, result);
    directive->setEnd(createCursor(pos));
  } else {
    // TODO error  no value.
  }

  if (!s.isEmpty()) {
    bypassWhitespaceAndUpdatePos(s, pos);
    auto p = 0;
    s_l_comments(s, p, sharedcomment);
    if (sharedcomment) {
      sharedcomment->setStart(createCursor(pos));
      sharedcomment->setEnd(createCursor(pos + p));
      start = sharedcomment->endPos();
      return true;
    }
  }

  start = directive->endPos();
  return true;
}

bool
QYamlParser::ns_yaml_directive(const QString& line,
                               int& start,
                               SharedNode& sharednode,
                               SharedComment& sharedcomment)
{
  if (line.isEmpty())
    return false;
  auto s = line.mid(1);
  auto len = 0;
  auto digit = 0;
  auto pos = start + 1;
  bool invalidSpace = false;
  len = initial_whitespace(s);
  if (len > 0) {
    invalidSpace = true;
    pos += len;
    s = s.mid(len);
  }

  if (!s.startsWith(YAML))
    return false;

  sharednode.reset(new YamlYamlDirective());
  auto directive = qSharedPointerDynamicCast<YamlYamlDirective>(sharednode);
  directive->setStart(createCursor(start));
  directive->setName(YAML);
  directive->setNameStart(createCursor(pos));
  if (invalidSpace)
    directive->setWarning(InvalidSpaceWarning, true);
  pos += 4;
  s = s.mid(4);

  bypassWhitespaceAndUpdatePos(s, pos);
  directive->setVersionStart(createCursor(pos));

  auto ch = s.at(0);
  pos++;
  if (ns_dec_digit(ch)) {
    digit = ch.digitValue();
    directive->setMajor(digit);
    // for future expansion ?
    if (digit < MIN_VERSION_MAJOR || digit > MAX_VERSION_MAJOR) {
      directive->setError(YamlError::InvalidMajorVersion, true);
    }
  } else {
    directive->setError(YamlError::BadYamlDirective, true);
  }
  s = s.mid(1);

  ch = s.at(0);
  pos++;
  if (ch != Characters::POINT) {
    directive->setError(YamlError::BadYamlDirective, true);
  }
  s = s.mid(1);

  ch = s.at(0);
  pos++;
  s = s.mid(1);
  if (ns_dec_digit(ch)) {
    digit = ch.digitValue();
    directive->setMinor(digit);
    if (digit < MIN_VERSION_MINOR || digit > MAX_VERSION_MINOR) {
      directive->setWarning(YamlWarning::InvalidMinorVersionWarning, true);
    }
  } else {
    directive->setError(YamlError::BadYamlDirective, true);
  }
  directive->setEnd(createCursor(pos));

  if (!s.isEmpty()) {
    bypassWhitespaceAndUpdatePos(s, pos);
    auto p = 0;
    s_l_comments(s, p, sharedcomment);
    if (sharedcomment) {
      sharedcomment->setStart(createCursor(pos));
      sharedcomment->setIndent(pos);
      sharedcomment->setEnd(createCursor(pos + p));
      start = sharedcomment->endPos();
      return true;
    }
  }

  start = directive->endPos();
  return true;
}

bool
QYamlParser::c_directives_end(const QString& line, int& start, SharedNode& node)
{
  if (line == "---") {
    node = SharedStart(new YamlStart);
    node->setStart(createCursor(start));
    start += 3;
    node->setEnd(createCursor(start));
    start++;
    return true;
  }
  return false;
}

bool
QYamlParser::c_document_end(const QString& line, int& start, SharedNode& node)
{
  if (line.startsWith("...")) {
    node = SharedEnd(new YamlEnd);
    node->setStart(createCursor(start));
    start += 3;
    node->setEnd(createCursor(start));
    start++;
    return true;
  }
  return false;
}

bool
QYamlParser::l_document_suffix(const QString& line,
                               int& start,
                               SharedNode& n,
                               SharedComment& c)
{
  if (line.length() < 3)
    return false;
  if (!c_document_end(line, start, n))
    return false;
  auto pos = 0;
  auto s = line.mid(3);
  QString comment;
  s_l_comments(s, start, c);
  return true;
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
QYamlParser::b_comment(const QString& line)
{
  if (line.isEmpty())
    return false;
  auto c = line.at(0);
  if (b_non_content(c))
    return true;
  return false;
}

bool
QYamlParser::s_b_comment(const QString& line,
                         int& start,
                         SharedComment& sharedcomment)
{
  QString comment;
  if (s_b_comment(line.mid(start), comment)) {
    sharedcomment = SharedComment(new YamlComment(comment));
    start = comment.length();
    return true;
  }
  return false;
}

bool
QYamlParser::s_b_comment(const QString& s, QString& comment)
{
  if (s.isEmpty())
    return false;
  auto len = initial_whitespace(s);
  auto text = s.mid(len);
  if (c_nb_comment_text(text, comment)) {
    return true;
  }
  return b_comment(text);
}

bool
QYamlParser::l_comment(const QString& s,
                       int& start,
                       SharedComment& sharedcomment)
{
  auto text = s.mid(start);
  auto len = initial_whitespace(text);
  if (len == 0)
    return false;
  text = text.mid(len);
  QString comment;
  if (c_nb_comment_text(text, comment)) {
    sharedcomment = SharedComment(new YamlComment(comment));
    start = comment.length();
    return true;
  }
  return b_comment(text);
}

bool
QYamlParser::s_l_comments(const QString& text,
                          int& start,
                          SharedComment& sharedcomment)
{
  auto s = text;
  auto indent = 0;
  bypassWhitespaceAndUpdatePos(s, indent);

  if (s.isEmpty())
    return false;
  // first line of possible multiline
  auto p = 0;
  if (s_b_comment(s, p, sharedcomment) || indent == 0 ||
      l_comment(s, p, sharedcomment)) {
  }
  // TODO only one line is supplied so this doen't do anything
  //    while (l_comment(line, start, comment)) {
  //      comment += Characters::NEWLINE;
  //      comment += comment;
  //    }
  if (!sharedcomment) {
    return false;
  }
  sharedcomment->setStart(createCursor(start + indent));
  sharedcomment->setEnd(createCursor(start + indent + p));
  sharedcomment->setIndent(indent);
  start = start + indent + p;
  return true;
}

bool
QYamlParser::c_nb_comment_text(const QString& line, QString& comment)
{
  if (line.isEmpty())
    return false;
  auto indent = s_indent(line);
  auto text = line;
  auto c = line.at(indent);
  text = text.mid(indent);
  if (c_comment(c)) {
    comment = c;
    text = text.mid(1);
    c = text.at(0);
    while (nb_char(c)) {
      comment += c;
      text = text.mid(1);
      if (text.isEmpty())
        return true;
      c = text.at(0);
    }
  }
  return false;
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
QYamlParser::c_ns_properties(const QString s,
                             int& start,
                             QString& result,
                             YamlNode::TagHandleType& type)
{
  SharedAnchorBase base = nullptr;
  if (c_ns_tag_property(s, base, type)) {
    return true;
  } else if (c_ns_anchor_property(s, base)) {
    return true;
  }
  return false;
}

bool
QYamlParser::c_ns_tag_property(const QString& text,
                               SharedAnchorBase& base,
                               YamlNode::TagHandleType& type)
{
  if (text.isEmpty())
    return false;
  auto value = text;
  auto c = value.at(0);
  QString tag;

  if (c_verbatim_tag(text, tag)) {
    base.reset(new YamlAlias());

    return true;
  } else if (c_ns_shorthand_tag(text, tag, type)) {
    type = YamlNode::Shorthand;
    return true;
  } else if (value.at(0) == Characters::EXCLAMATIONMARK) {
    type = YamlNode::NonSpecific;
    return true;
  }
  return false;
}

bool
QYamlParser::c_verbatim_tag(const QString& line, QString& uri)
{
  if (line.isEmpty())
    return false;
  auto value = line;
  QString result;
  auto c = value.at(0);
  if (c == Characters::LT) { // !
    value = value.mid(1);
    c = value.at(0);
    if (c_folded(c)) { // <
      value = value.mid(1);
      c = value.at(0);
      while (ns_uri_char(c)) {
        result += c;
      }
      value = value.mid(1);
      c = value.at(0);
      if (c == Characters::GT) {
        uri = result;
        return true;
      }
    }
  }
  return false;
}

bool
QYamlParser::c_ns_shorthand_tag(const QString& line,
                                QString& tag,
                                YamlNode::TagHandleType& type)
{
  if (c_tag_handle(line, tag, type)) {
    return true;
  }
  return false;
}

bool
QYamlParser::c_tag_handle(const QString& line,
                          QString& tag,
                          YamlNode::TagHandleType& type)
{
  if (line.isEmpty())
    return false;
  if (c_named_tag_handle(line, tag)) {
    type = YamlNode::Named;
  } else if (c_secondary_tag_handle(line)) {
    type = YamlNode::Secondary;
  } else if (c_primary_tag_handle(line)) {
    type = YamlNode::Primary;
  }
  return false;
}

bool
QYamlParser::c_primary_tag_handle(const QString& line)
{
  if (line.isEmpty())
    return false;
  auto value = line;
  QString result;
  auto c = value.at(0);
  if (c_tag(c)) {
    return true;
  }
  return false;
}

bool
QYamlParser::c_secondary_tag_handle(const QString& line)
{
  if (line.isEmpty())
    return false;
  auto value = line;
  QString result;
  auto c = value.at(0);
  if (c_tag(c)) {
    value = value.mid(1);
    c = value.at(0);
    if (c_tag(c)) {
      return true;
    }
  }
  return false;
}

bool
QYamlParser::c_named_tag_handle(const QString& line, QString& tag)
{
  if (line.isEmpty())
    return false;
  auto value = line;
  QString result;
  auto c = value.at(0);
  if (c_tag(c)) {
    value = value.mid(1);
    c = value.at(0);
    while (ns_word_char(c)) {
      result += c;
      value = value.mid(1);
      c = value.at(0);
    }
    if (c_tag(c)) {
      tag = result;
      return true;
    }
  }
  return false;
}

bool
QYamlParser::c_ns_anchor_property(const QString& text,
                                 SharedAnchorBase& base)
{
  if (text.isEmpty())
    return false;
  auto value = text;
  auto c = value.at(0);
  int pos; // TODO Move up
  if (!c_anchor(c))
    return false;
  value = value.mid(1);
  QString name;
  if (ns_anchor_name(value, name)) {
    base.reset(new YamlAnchor());
    base->setName(name);
    base->setNameStart(createCursor(pos));
    return true;
  }
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
QYamlParser::ns_uri_char(const QString& line)
{
  if (line.isEmpty())
    return false;
  auto value = line;
  auto c = value.at(0);
  if (ns_word_char(c) || c == Characters::HASH || c == Characters::SEMICOLON ||
      c == Characters::FORWARDSLASH || c == Characters::QUESTIONMARK ||
      c == Characters::COLON || c == Characters::COMMERCIAL_AT ||
      c == Characters::AMPERSAND || c == Characters::EQUALS ||
      c == Characters::PLUS || c == Characters::DOLLAR ||
      c == Characters::COMMA || c == Characters::LOWLINE ||
      c == Characters::STOP || c == Characters::EXCLAMATIONMARK ||
      c == Characters::TILDE || c == Characters::ASTERISK ||
      c == Characters::SINGLEQUOTE || c == Characters::OPEN_ROUND_BRACKET ||
      c == Characters::CLOSE_ROUND_BRACKET ||
      c == Characters::OPEN_SQUARE_BRACKET ||
      c == Characters::CLOSE_SQUARE_BRACKET) {
    return true;
  } else if (c == Characters::PERCENT) {
    value = value.mid(1);
    c = value.at(0);
    if (ns_hex_digit(c)) {
      value = value.mid(1);
      c = value.at(0);
      if (ns_hex_digit(c)) {
        return true;
      }
    }
  }
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
QYamlParser::ns_esc_8_bit(const QString& line)
{
  if (line.length() == 4 && c_ns_esc_char(line.at(0)) && line.at(1) == 'x' &&
      ns_hex_digit(line.at(2)) && ns_hex_digit(line.at(3)))
    return true;
  return false;
}

bool
QYamlParser::ns_esc_16_bit(const QString& line)
{
  if (line.length() == 6 && c_ns_esc_char(line.at(0)) && line.at(1) == 'u' &&
      ns_hex_digit(line.at(2)) && ns_hex_digit(line.at(3)) &&
      ns_hex_digit(line.at(4)) && ns_hex_digit(line.at(5)))
    return true;
  return false;
}

bool
QYamlParser::ns_esc_32_bit(const QString& line)
{
  // TODO 32 bit utf
  return false;
}

bool
QYamlParser::c_ns_esc_char(const QString& line)
{
  auto good = false;
  auto l = line.length();
  if (l > 0) {
    if (c_ns_esc_char(line.at(0)))
      good = true;
    if (good) {
      good = false;
      if (l == 2) {
        auto c = line.at(1);
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

int
QYamlParser::s_indent(const QString& line)
{
  if (line.isEmpty())
    return 0;
  auto indent = 0;
  auto c = line.at(indent);
  while (s_space(c)) {
    indent++;
    c = line.at(indent);
  }
  return indent;
}

bool
QYamlParser::s_indent_less_than(int value, const QString& s)
{
  int indent = s_indent(s);
  if (indent < value)
    return true;
  return false;
}

bool
QYamlParser::s_indent_less_or_equal(int value, const QString& s)
{
  int indent = s_indent(s);
  if (indent <= value)
    return true;
  return false;
}

bool
QYamlParser::b_as_space(QChar c)
{
  return (b_break(c));
}

int
QYamlParser::initial_whitespace(const QString& s)
{
  //  for (auto c : s) {
  //    if (!s_white(c))
  //      return false;
  //  }
  auto len = 0;
  for (auto c : s) {
    if (!s_white(c))
      break;
    len++;
  }
  return len;
}

bool
QYamlParser::ns_anchor_char(QChar c)
{
  return (ns_char(c) && !c_flow_indicator(c));
}

bool
QYamlParser::nb_json(QChar c)
{
  auto lower = c.cell();
  auto higher = c.row();
  return (c == Characters::TAB ||
          (higher == 0 && lower >= 0x20 && lower <= 0xFF) || higher >= 0x1);
}

QList<SharedDocument>::iterator
QYamlParser::begin()
{
  return m_documents.begin();
}

QList<SharedDocument>::const_iterator
QYamlParser::constBegin()
{
  return m_documents.constBegin();
}

QList<SharedDocument>::iterator
QYamlParser::end()
{
  return m_documents.end();
}

QList<SharedDocument>::const_iterator
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
