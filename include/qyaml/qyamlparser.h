#pragma once

#include <QFile>
#include <QObject>
#include <QRegularExpression>
#include <QTextDocument>

#include "quazip/JlCompress.h"

#include "qyaml/yamlnode.h"
#include "qyaml_global.h"

class QYamlDocument;
class YamlNode;
class YamlAnchor;

class QYAML_SHARED_EXPORT QYamlParser : public QObject
{
  Q_OBJECT
public:
  explicit QYamlParser(QTextDocument* doc, QObject* parent = nullptr);

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
  bool isEmpty();
  int count();
  QYamlDocument* currentDoc() const;

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
  QString m_text;
  QTextDocument* m_document = nullptr;
  QYamlDocument* m_currentDoc = nullptr;
  //    QMap<QTextCursor, YamlNode*> m_nodes;
  QMap<QString, YamlAnchor*> m_anchors;
  QList<QYamlDocument*> m_documents;
  QString m_filename;
  QString m_zipFile;
  YamlErrors m_errors = NoErrors;
  YamlWarnings m_warnings = NoWarnings;
  QTextCursor m_start;
  QTextCursor m_end;

  static constexpr int VERSION_MAJOR = 1;
  static constexpr int VERSION_MINOR = 2;
  static constexpr int VERSION_PATCH = 2;
  static const QString VERSION_STRING;
  static const QString VERSION_PATCH_STRING;

  static const QRegularExpression YAML_DIRECTIVE;
  static const QRegularExpression YAML_DOCSTART;
  static const QRegularExpression YAML_DOCEND;

  QYamlDocument* parseDocumentStart(struct fy_event* event);
  bool resolveAnchors();
  QTextCursor createCursor(int position);
  void parseYamlDirective(QString s,
                          int& tStart,
                          int i,
                          QChar c,
                          bool hasYamlDirective,
                          int row,
                          int& indent);
  void parseFlowSequence(YamlSequence* sequence, int& i, const QString& text);
  void parseFlowMap(YamlMap* map, int& i, const QString& text);
  YamlComment* parseComment(int& i, const QString& text);
  YamlScalar* parseScalar(const QString& t, int i);
};
