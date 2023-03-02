#pragma once

#include <QFile>
#include <QObject>
#include <QRegularExpression>
#include <QSharedPointer>
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
  QList<SharedDocument> documents() const;

  //! Returns a pointer to the QYamlDocument at index, or nullptr if the
  //! index is out of range.
  //!
  //! \sa QYamlParser::documents()
  SharedDocument document(int index);

  void setDocuments(QList<SharedDocument> root);
  void append(SharedDocument document);
  bool isMultiDocument();

  //! Returns the node at the cursor position if it exists otherwise
  //! returns nullptr;
  SharedNode nodeAt(QTextCursor cursor);

  bool isEmpty();
  int count();

  QList<SharedDocument>::iterator begin();
  QList<SharedDocument>::const_iterator constBegin();
  QList<SharedDocument>::iterator end();
  QList<SharedDocument>::const_iterator constEnd();

  QString text() const;

  //  const QMap<QTextCursor, SharedNode>& nodes() const;

signals:
  void parseComplete();

protected:
  bool l_directive(const QString& line,
                   int& start,
                   SharedNode& d,
                   SharedComment& c);
  bool s_l_comments(const QString& line,
                    int& start,
                    SharedComment& sharedcomment);
  bool c_directives_end(const QString& line, int& start, SharedNode& node);
  bool c_document_end(const QString& line, int& start, SharedNode& node);
  bool l_document_suffix(const QString& line,
                         int& start,
                         SharedNode& n,
                         SharedComment& c);
  bool s_b_comment(const QString& line,
                   int& start,
                   SharedComment& sharedcomment);

private:
  QYamlSettings* m_settings = nullptr;
  QString m_text;
  QTextDocument* m_document = nullptr;
  QMap<QString, SharedAnchor> m_anchors;
  QList<SharedDocument> m_documents;
  QString m_filename;
  QString m_zipFile;
  YamlErrors m_errors = NoErrors;
  YamlWarnings m_warnings = NoWarnings;
  QTextCursor m_start;
  QTextCursor m_end;
  int m_currentVersion = 12;

  static constexpr int MAX_VERSION = 12;
  static constexpr int MIN_VERSION_MAJOR = 1;
  static constexpr int MAX_VERSION_MAJOR = 1; // for future expansion ??
  static constexpr int MIN_VERSION_MINOR = 0;
  static constexpr int MAX_VERSION_MINOR = 2;
  static constexpr int VERSION_MAJOR = 1;
  static constexpr int VERSION_MINOR = 2;
  static constexpr int VERSION_PATCH = 2;
  static const QString VERSION_STRING;
  static const QString VERSION_PATCH_STRING;
  static const QString YAML;
  static const QString TAG;

  //  static const QString DOCSTART;
  //  static const QString IND_DOCSTART;
  //  static const QString DOCEND;
  //  static const QString IND_DOCEND;
  //  static const QRegularExpression YAML_DIRECTIVE;
  //  static const QRegularExpression TAG_DIRECTIVE;

  SharedDocument parseDocumentStart(struct fy_event* event);
  bool resolveAnchors();
  QTextCursor createCursor(int position);
  //  void parseFlowSequence(SharedSequence sequence,
  //                         int& i,
  //                         const QString& text);
  //  void parseFlowMap(SharedMap map, int& i, const QString& text);
  //  QSharedPointer<YamlComment> parseComment(int& i, const QString& text);
  //  QSharedPointer<YamlScalar> parseFlowScalar(const QString& text, int i);
  bool getNextChar(QChar& c, const QString& text, int& i);
  int getInitialSpaces(const QString& docText,
                       int initialIndent,
                       int& i,
                       QChar& c);
  //  void buildDocuments(const QString& text,
  //                      QList<SharedNode> nodes,
  //                      QList<SharedNode> rootNodes);
  SharedNode nodeInSequence(QTextCursor cursor, SharedSequence seq);
  SharedNode nodeInMap(QTextCursor cursor, SharedMap map);
  SharedNode nodeOrRecurse(QTextCursor cursor, SharedNode node);

  QString lookahead(int& index,
                    const QString& text,
                    QChar endof = Characters::NEWLINE);
  //  bool hasYamlDirective()
  //  {
  //    // TODO
  //    //    for (auto node : m_nodes) {
  //    //    }
  //    return false;
  //  }

  bool l_explicit_document() { return false; }
  bool l_bare_document() { return false; }
  bool e_node() { return false; }
  bool l_directive_document() { return false; }

  bool ns_reserved_directive(const QString& line,
                             int& start,
                             SharedNode& sharednode,
                             SharedComment& sharedcomment);

  bool ns_char_plus(const QString& s, QString& value)
  {
    QString result;
    for (auto c : s) {
      if (!ns_char(c)) {
        if (!result.isEmpty()) {
          value = result;
          return true;
        }
        break;
      }
      result += c;
    }
    return false;
  }

  bool ns_directive_parameter(const QString& s, QString& param);

  bool ns_directive_name(const QString& s, QString& name);

  bool ns_tag_directive(const QString& line,
                        int& start,
                        SharedNode& sharednode,
                        SharedComment& sharedcomment);

  bool ns_yaml_directive(const QString& line,
                         int& start,
                         SharedNode& sharednode,
                         SharedComment& sharedcomment);

  //! Returns true if a forbidden construct is passed
  bool c_forbidden(const QString& line, SharedNode& node)
  {
    //    auto indent = initial_whitespace(line);
    //    // should start at beginning of line.
    //    if (indent > 0)
    //      return true;

    //    if (c_directives_end(line)) {
    //      node.reset(new YamlStart());
    //      return false;
    //    } else if (c_document_end(line)) {
    //      node.reset(new YamlEnd());
    //      return false;
    //    }
    return true;
  }
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
  //! non content or end of text
  bool b_comment(const QString& line);
  bool s_b_comment(const QString& s, QString& comment);
  bool l_comment(const QString& s, int& start, SharedComment& sharedcomment);
  bool b_as_line_feed(QChar c) { return (b_break(c)); }
  bool c_nb_comment_text(const QString& line, QString& comment);
  bool c_anchor(QChar c);
  bool c_alias(QChar c);
  bool c_tag(QChar c);
  bool c_literal(QChar c);
  bool c_folded(QChar c);
  bool c_single_quote(QChar c);
  bool c_double_quote(QChar c);
  bool c_directive(QChar c);
  bool c_reserved(QChar c);
  bool c_escape(QChar c);
  bool c_ns_esc_char(QChar c);
  bool c_ns_esc_char(const QString& line);
  bool c_printable(QChar c);
  bool c_ns_properties(const QString s,
                       int& start,
                       QString& result,
                       YamlNode::TagHandleType& type);

  bool c_ns_tag_property(const QString& text,
                         SharedAnchorBase& base,
                         YamlNode::TagHandleType& type);

  bool c_verbatim_tag(const QString& line, QString& uri);

  //! Returns true if the string s contains valid ns shorthand tag. The tag
  //! value is set to the correct tag name if the tag is a named type. The
  //! type attribute is set to either YamlNode::Named, YamlNode::Secondary or
  //! YamlNode::Primary to specify the tag type.
  bool c_ns_shorthand_tag(const QString& line,
                          QString& tag,
                          YamlNode::TagHandleType& type);
  bool c_tag_handle(const QString& line,
                    QString& tag,
                    YamlNode::TagHandleType& type);
  bool c_primary_tag_handle(const QString& line);
  bool c_secondary_tag_handle(const QString& line);
  bool c_named_tag_handle(const QString& line, QString& tag);

  //! Returns true if c is a line feed character
  bool b_line_feed(QChar c);

  //! Returns true if c is a carriage return character
  bool b_carriage_return(QChar c);

  //! Returns true if a break character, acarriage return or a line feed
  bool b_char(QChar c);

  bool b_break(QChar c1, QChar c2);
  bool b_break(QChar c);
  bool b_as_line_feed(QChar& c);
  bool b_as_line_feed(QChar& c1, QChar& c2);

  //! Returns true if c is a a non content character, a carriage return or
  //! linefeed.
  bool b_non_content(QChar& c);

  //! Returns true for an empty scalar, otherwise false.
  bool e_scalar(const QString& s) { return s.isEmpty(); }

  bool c_ns_alias_node(const QString& text, QString& name)
  {
    if (text.isEmpty())
      return false;
    auto i = 0;
    auto c = text.at(i++);
    if (c_alias(c)) {
      if (ns_anchor_name(text, name)) {
        return true;
      }
      c = text.at(i++);
    }

    return false;
  }

  bool ns_anchor_name(const QString& text, QString& name)
  {
    auto i = 0;
    auto c = text.at(i++);
    while (ns_anchor_char(c)) {
      name += c;
      c = text.at(i++);
    }
    if (!name.isEmpty())
      return true;
    return false;
  }

  //  bool c_ns_anchor_property(const QString& text, int& start, QString&
  //  property)
  //  {
  //    if (text.isEmpty())
  //      return false;
  //    auto s = text.mid(start);
  //    auto c = s.at(0);
  //    if (!c_anchor(c))
  //      return false;
  //    if (ns_anchor_name(s, property)) {
  //      return true;
  //    }
  //    return false;
  //  }

  bool c_ns_anchor_property(const QString& text,
                            SharedAnchorBase& base);

  bool nb_double_char(QChar c)
  {
    if (c_ns_esc_char(c) ||
        (nb_json(c) && !(c_escape(c) || c_double_quote(c)))) {
      return true;
    }
    return false;
  }

  bool ns_double_char(QChar c) { return (nb_double_char(c) && !s_white(c)); }

  bool c_double_quoted(const QString& line, QString& name)
  {
    if (line.isEmpty())
      return false;
    auto s = line;
    auto c = s.at(0);
    s = s.mid(1);
    if (!c_double_quote(c))
      return false;
    // TODO
    return true;
  }

  bool c_flow_sequence(const QString& text, int& start)
  {
    if (text.isEmpty())
      return false;
    auto s = text;
    auto c = s.at(0);
    if (!c_sequence_start(c))
      return false;
    while (!s.isEmpty()) {
      s = s.mid(1);
      c = s.at(0);
      if (c_sequence_end(c)) {
        // TODO
        break;
      }
      SharedComment comment = nullptr;
      int len;
      if (s_separate_lines(s, comment, len)) {
      }
    }
    return false;
  }

  bool ns_flow_sequence_entries(const QString& text)
  {
    if (text.isEmpty())
      return false;
    auto s = text;
    auto c = s.at(0);
    s = s.mid(1);
    if (c_mapping_key(c)) {

    } else if (ns_flow_pair_entry(text)) {
    }
    return false;
  }

  bool ns_flow_pair_entry(const QString& text)
  {
    if (ns_flow_pair_yaml_key_entry(text) ||
        c_ns_flow_map_empty_key_entry(text) ||
        c_ns_flow_pair_json_key_entry(text)) {
      return true;
    }
    return false;
  }

  bool ns_flow_pair_yaml_key_entry(const QString& text) { return false; }

  bool c_ns_flow_map_empty_key_entry(const QString& text) { return false; }

  bool c_ns_flow_pair_json_key_entry(const QString& text)
  {
    //    if (c_flow_json_node(c)) {
    //    }
    return false;
  }

  bool c_flow_json_node(const QString& text)
  {
    if (c_ns_properties(text)) {
      return true;
    }
    return false;
  }

  bool c_ns_properties(const QString& text)
  {
    if (text.isEmpty())
      return false;

    SharedAnchorBase property=nullptr,anchor=nullptr,tag=nullptr;
    YamlNode::TagHandleType type;
    SharedComment comment = nullptr;
    auto len = 0;
    if (c_ns_tag_property(text, property, type)) {
      if (s_separate_lines(text, comment, len)) {
        if (c_ns_anchor_property(text,  property)) {

          return true;
        }
      }
    } else if (c_ns_anchor_property(text, anchor)) {
      if (s_separate_lines(text, comment, len)) {
        if (c_ns_tag_property(text, tag, type)) {

          return true;
        }
      }
    }
    return false;
  }

  bool ns_flow_pair() {return false;}

  bool ns_flow_node() {return false;}

  bool ns_flow_sequence_entry() {return false;}

  bool s_separate_lines(const QString& text, SharedComment comment, int& length)
  {
    auto s = text;
    auto p = 0;
    if (s_l_comments(s, p, comment)) {
      if (comment) {
        s = s.mid(comment->length());
      }
    }
    if (s_flow_line_prefix(s, length)) {
    }
    return false;
  }

  bool s_separate_in_line(const QString& text, int& length)
  {
    if (text.isEmpty())
      return false;
    auto s = text;
    auto len = 0;
    while (!s.isEmpty()) {
      auto c = s.at(0);
      if (s_white(c)) {
        len++;
        continue;
      } else if (c == Characters::NEWLINE) {
        break;
      }
      s = s.mid(1);
    }
    if (len > 0) {
      length = len;
      return true;
    }
    return false;
  }

  bool s_flow_line_prefix(const QString& text, int& length)
  {
    auto indent = initial_whitespace(text);
    auto len = 0;
    if (s_separate_in_line(text, len)) {
      length = indent + len;
    } else {
      length = indent;
    }
    if (length > 0)
      return true;
    return false;
  }

  bool s_separation_spaces(const QString& text,
                           int& length,
                           int& indent,
                           QList<SharedComment> comments)
  {
    if (text.isEmpty()) {
      length = 0;
      return false;
    }

    auto len = 0;
    auto s = text;
    auto newline = false;
    QChar c;
    while (!s.isEmpty()) {
      c = s.at(0);
      if (s_white(c)) {
        len++;
        s = s.mid(1);
        if (newline) {
          indent++;
        }
        continue;
      } else if (c == Characters::NEWLINE) {
        len++;
        s = s.mid(1);
        indent = 0;
        newline = true;
        continue;
      } else if (c == Characters::HASH) {
        auto comm = s;
        SharedComment sharedcomment = nullptr;
        auto p = 0;
        if (s_l_comments(comm, p, sharedcomment)) {
          comments.append(sharedcomment);
          len += sharedcomment->length();
          s = s.mid(sharedcomment->length());
        }
      } else {
        if (len > 0) {
          length += len;
          return true;
        } else {
          return false;
        }
      }
      s = s.mid(1);
    }
    return false;
  }

  bool s_space(QChar c);
  bool s_tab(QChar c);
  bool s_white(QChar c);
  int s_indent(const QString& line);
  bool s_indent_less_than(int value, const QString& line);
  bool s_indent_less_or_equal(int value, const QString& line);
  bool l_empty(const QString& line, int indent)
  {
    // TODO s_line_prefix
    if (/*s_line_prefix(line) || */ s_indent_less_than(indent, line)) {
      return true;
    }
    return false;
  }

  // TODO s-line-prefix
  // TODO s-block-line-prefix
  // TODO s-flow-line-prefix
  // TODO s-separate-in-line
  // TODO l-empty
  // TODO b-l-trimmed
  // TODO b-l-folded
  // TODO s-flow-folded
  bool b_as_space(QChar c);
  //! Returns length of whitespace characters at start of text.
  int initial_whitespace(const QString& s);

  bool ns_char(QChar c);
  bool ns_dec_digit(QChar c);
  bool ns_hex_digit(QChar c);
  bool ns_ascii_char(QChar c);
  bool ns_word_char(QChar c);
  bool ns_uri_char(const QString& line);
  bool ns_tag_char(QChar c);
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
  bool ns_esc_8_bit(const QString& line);
  bool ns_esc_16_bit(const QString& line);
  bool ns_esc_32_bit(const QString& line);
  bool ns_tag_prefix(QChar c) { return false; }
  bool ns_anchor_char(QChar c);

  bool nb_json(QChar c);

  void createDocIfNull(int start, SharedDocument& currentDoc);
  void storeNode(SharedDocument currentDoc,
                 SharedNode node,
                 int& start,
                 int length);
  void bypassTextAndUpdatePos(QString& s, int& pos, QString result);
  void bypassWhitespaceAndUpdatePos(QString& s, int& pos);
};
