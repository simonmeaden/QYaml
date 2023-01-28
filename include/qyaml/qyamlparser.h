#pragma once

#include <QFile>
#include <QObject>
#include <QRegularExpression>
#include <QTextDocument>

#include <config/baseconfig.h>

#include "qyaml/qyamldocument.h"
#include "qyaml/yamlnode.h"
#include "qyaml_global.h"
#include "utilities/characters.h"

class QYamlDocument;
class YamlNode;
class YamlAnchor;

class QYAML_SHARED_EXPORT QYamlSettings : public BaseConfig
{
public:
  QYamlSettings(QObject* parent)
    : BaseConfig(parent)
  {
  }

  // BaseConfig interface
  bool save(const QString& filename);
  bool load(const QString& filename);

  int indentStep() const;
  void setIndentStep(int indentStep);

private:
  int m_indentStep = 2;
};

class QYAML_SHARED_EXPORT QYamlParser : public QObject
{
  Q_OBJECT

  struct Directive
  {
    enum Type
    {
      NOTYPE,
      YAML,
      TAG,
    };

    int start = -1;
    int length = -1; // TODO bad version

    int major = 1;
    int minor = 2;
    QString text;
    Type type = NOTYPE;
  };

public:
  explicit QYamlParser(QTextDocument* doc, QObject* parent = nullptr);
  explicit QYamlParser(QYamlSettings* settings,
                       QTextDocument* doc,
                       QObject* parent = nullptr);

  QString inlinePrint() const;

  QString prettyPrint() const;

  //! Returns the file name loaded via loadFile(const QString&) or
  //! loadFromZip(const QString&, const QString&)
  const QString filename() const;

  //! Loads the file in href into the editor.
  bool loadFile(const QString& filename);

  //! Loads the file in href from the zipped file zipfile.
  bool loadFromZip(const QString& zipFile, const QString& href);

  //! Parses the text string from startPos for length characters.
  //!
  //! By default parse(const QString&) parses the entire text string
  //! from the beginning.
  bool parse(const QString& text, int startPos = 0, int length = -1);

  //! Returns the list of QyamlDocument's.
  //!
  //! Use isMultiDocument() to detect if there is more than one document,
  //! count() to find the number of documents.
  //!
  //! \sa QYamlParser::document(int)
  //! \sa QYamlParser::isEmpty()
  QList<QYamlDocument*> documents() const;

  //! Returns a pointer to the QYamlDocument at index, or nullptr if the
  //! index is out of range.
  //!
  //! \sa QYamlParser::documents()
  QYamlDocument* document(int index);

  void setDocuments(QList<QYamlDocument*> root);
  void append(QYamlDocument* document);
  bool isMultiDocument();

  //! Returns the node at the cursor position if it exists otherwise
  //! returns nullptr;
  YamlNode* nodeAt(QTextCursor cursor);

  bool isEmpty();
  int count();

  QList<QYamlDocument*>::iterator begin();
  QList<QYamlDocument*>::const_iterator constBegin();
  QList<QYamlDocument*>::iterator end();
  QList<QYamlDocument*>::const_iterator constEnd();

  QString text() const;

  //  const QMap<QTextCursor, YamlNode*>& nodes() const;

signals:
  void parseComplete();

protected:
private:
  QYamlSettings* m_settings = nullptr;
  QString m_text;
  QTextDocument* m_document = nullptr;
  //  QYamlDocument *m_currentDoc = nullptr;
  //    QMap<QTextCursor, YamlNode*> m_nodes;
  QMap<QString, YamlAnchor*> m_anchors;
  QList<QYamlDocument*> m_documents;
  QString m_filename;
  QString m_zipFile;
  YamlErrors m_errors = NoErrors;
  YamlWarnings m_warnings = NoWarnings;
  QTextCursor m_start;
  QTextCursor m_end;
  int m_currentVersion = 12;

  static constexpr int MAX_VERSION = 12;
  static constexpr int MIN_VERSION_MINOR = 1;
  static constexpr int MAX_VERSION_MINOR = 2;
  static constexpr int MIN_VERSION_MAJOR = 1;
  static constexpr int MAX_VERSION_MAJOR = 1; // for future expansion ??
  static constexpr int VERSION_MAJOR = 1;
  static constexpr int VERSION_MINOR = 2;
  static constexpr int VERSION_PATCH = 2;
  static const QString VERSION_STRING;
  static const QString VERSION_PATCH_STRING;

  //  static const QString DOCSTART;
  //  static const QString IND_DOCSTART;
  //  static const QString DOCEND;
  //  static const QString IND_DOCEND;
  static const QRegularExpression YAML_DIRECTIVE;
  static const QRegularExpression TAG_DIRECTIVE;

  QYamlDocument* parseDocumentStart(struct fy_event* event);
  bool resolveAnchors();
  QTextCursor createCursor(int position);
  void parseFlowSequence(YamlSequence* sequence, int& i, const QString& text);
  void parseFlowMap(YamlMap* map, int& i, const QString& text);
  YamlComment* parseComment(int& i, const QString& text);
  YamlScalar* parseFlowScalar(const QString& text, int i);
  bool getNextChar(QChar& c, const QString& text, int& i);
  int getInitialSpaces(const QString& docText,
                       int initialIndent,
                       int& i,
                       QChar& c);
  void buildDocuments(const QString& text,
                      QList<YamlNode*> nodes,
                      QList<YamlNode*> rootNodes);
  YamlNode* nodeInSequence(QTextCursor cursor, YamlSequence* seq);
  YamlNode* nodeInMap(QTextCursor cursor, YamlMap* map);
  YamlNode* nodeOrRecurse(QTextCursor cursor, YamlNode* node);

  QString lookahead(int& index,
                    const QString& text,
                    QChar endof = Characters::NEWLINE);

  bool c_indicator(QChar c);
  bool c_flow_indicator(QChar c);
  bool b_char(QChar c, int version = 12);
  bool nb_char(QChar c, int version = 12);
  bool c_byte_order_mark(QChar c);
  bool c_sequence_entry(QChar c);
  bool c_mapping_key(QChar c);
  bool c_mapping_value(QChar c);
  bool c_collect_entry(QChar c);
  bool c_sequence_start(QChar c);
  bool c_sequence_end(QChar c);
  bool c_mapping_start(QChar c);
  bool c_mapping_end(QChar c);
  bool c_comment(QChar c);
  bool c_anchor(QChar c);
  bool c_alias(QChar c);
  bool c_tag(QChar c);
  bool c_literal(QChar c);
  bool c_folded(QChar c);
  bool c_single_quote(QChar c);
  bool c_double_quote(QChar c);
  bool c_directive(QChar c);
  bool c_reserved(QChar c);
  bool b_line_feed(QChar c);
  bool b_carriage_return(QChar c);
  bool b_char(QChar c);
  //  bool nb_char(QChar c);
  bool s_space(QChar c);
  bool s_tab(QChar c);
  bool s_white(QChar c);
  bool ns_char(QChar c);
  bool b_break(QChar c1, QChar c2);
  bool b_break(QChar c);
  bool b_as_line_feed(QChar& c);
  bool b_as_line_feed(QChar& c1, QChar& c2);
  bool b_non_content(QChar& c);
  bool c_printable(QChar c);
  bool ns_dec_digit(QChar c);
  bool ns_hex_digit(QChar c);
  bool ns_ascii_char(QChar c);
  bool ns_word_char(QChar c);
  bool ns_uri_char(const QString& s);
  bool ns_tag_char(QChar c);
  bool c_escape(QChar c);
  bool c_ns_esc_char(QChar c);
  bool ns_esc_null(QChar& c);
  bool ns_esc_bell(QChar& c);
  bool ns_esc_backspace(QChar& c);
  bool ns_esc_horizontal_tab(QChar& c);
  bool ns_esc_linefeed(QChar& c);
  bool ns_esc_vertical_tab(QChar& c);
  bool ns_esc_form_feed(QChar& c);
  bool ns_esc_carriage_return(QChar& c);
  bool ns_esc_escape(QChar& c);
  bool ns_esc_space(QChar& c);
  bool ns_esc_double_quote(QChar& c);
  bool ns_esc_slash(QChar& c);
  bool ns_esc_next_line(QChar& c);
  bool ns_esc_backslash(QChar& c);
  bool ns_esc_nb_space(QChar& c);
  bool ns_esc_line_seperator(QChar& c);
  bool ns_esc_paragraph_seperator(QChar& c);
  bool ns_esc_8_bit(const QString& s);
  bool ns_esc_16_bit(const QString& s);
  bool ns_esc_32_bit(const QString& s);
  bool c_ns_esc_char(const QString& s);
  bool ns_tag_prefix(QChar c) { return false; }
  YamlTagDirective* ns_tag_directive(const QString& s, int start);
  int c_tag_handle(const QString& text, YamlTagDirective::HandleType& type)
  {
    auto len = 0;
    auto s = text;
    if (!c_tag(s.at(0)))
      return -1;
    s = s.mid(1);
    auto c = s.at(0);
    while (!c_tag(c)) {
      if (nb_char(c)) {
        if (len == 0)
          type = YamlTagDirective::Primary;
        // TODO possible error if len > 0
        return len;
      } else if (ns_word_char(c)) {
        len++;
        s = s.mid(1);
        c = s.at(0);
      }
    }
    if (len == 0)
      type = YamlTagDirective::Secondary;
    else
      type = YamlTagDirective::Named;
    return len;
  }
  int c_named_tag_handle(const QString& text)
  {
    auto len = 0;
    auto s = text;
    if (!c_tag(s.at(0)))
      return -1;
    s = s.mid(1);
    auto c = s.at(0);
    while (!c_tag(c)) {
      if (ns_word_char(c)) {
        len++;
        s = s.mid(1);
        c = s.at(0);
      }
    }
    return len;
  }
  int c_secondary_tag_handle(const QString& text)
  {
    auto len = 0;

    return len;
  }

  bool ns_local_tag_prefix(QChar c) { return false; }
  bool ns_global_tag_prefix(const QString& s)
  { // TODO
    return false;
  }
  int s_separate_in_line(const QString& text)
  {
    auto len = 0;
    auto s = text;
    while (s.at(0).isSpace()) {
      len++;
      s = s.mid(1);
    }
    return len;
  }

  bool start_of_line(QChar c)
  {
    // TODO
    return false;
  }

  bool nb_json(QChar c);

  bool isNSPlainFirst(QChar c)
  { // TODO
    return false;
  }
  bool isNSPlainSafe(QChar c1, QChar c2)
  { // TODO
    return false;
  }
};
