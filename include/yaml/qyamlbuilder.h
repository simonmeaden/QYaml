#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QPoint>
#include <QVariant>

#include <libfyaml.h>

#include "qyaml_global.h"

class YamlDocument;

class QYAML_SHARED_EXPORT QYamlBuilder : public QObject
{
  Q_OBJECT
public:
  enum YamlError
  {
    NoErrors = 0,
    FatalParserCreationError = 0x1,
    FatalLoadStringError = 0x2,
  };
  Q_DECLARE_FLAGS(YamlErrors, YamlError)
  Q_FLAG(YamlErrors)

  enum YamlWarning
  {
    NoWarnings = 0,
  };
  Q_DECLARE_FLAGS(YamlWarnings, YamlWarning)
  Q_FLAG(YamlWarnings)

  explicit QYamlBuilder(QObject* parent = nullptr);

  QString inlinePrint() const;

  QString prettyPrint() const;

  bool parse(const QString& text);
  bool parse(const char* text);

  //! Returns the file name loaded via loadFile(const QString&) or
  //! loadHref(const QString&, const QString&)
  const QString filename() const;
  //! Loads the file in href into the editor.
  bool loadFile(const QString& filename);
  //! Loads the file in href from the zipped file zipfile.
  bool loadFromZip(const QString& zipFile, const QString& href);

  QList<YamlDocument*> root() const;
  void setRoot(QList<YamlDocument*> root);

  QString text() const;

protected:

private:
  QString m_text;
  QList<YamlDocument*> m_documents;
  QString m_filename;
  QString m_zipFile;
  YamlErrors m_errors = NoErrors;
  YamlWarnings m_warnings = NoWarnings;
  //  YamlNode* parseNode(YAML::Node node);
  //  YamlNode* parseMap(YAML::Node node);
  //  YamlNode* parseScalar(YAML::Node node);
  //  YamlNode* parseSequence(YAML::Node node);
  //  YamlNode* parseComment(YAML::Node node);
  YamlDocument *parseDocumentStart(struct fy_event* event);
};

//#include "SMLibraries/qyamlcpp/qyamlcpp.h"
struct composer_data
{
  struct fy_parser* parser;
  struct fy_document* document;
  struct fy_emitter* emitter;
  bool null_output;
  bool document_ready;
  bool verbose;
  bool single_document;
};

struct YamlMark
{
  int pos = 0;
  int line = 0;
  int column = 0;
};

class YamlDocument : public QObject
{
public:
  explicit YamlDocument(QObject* parent);

  //! Returns the major version value.
  //!
  //! default value 1.
  int majorVersion() const;
  //! Sets the major version value.
  //!
  //! default value is 2 unless specified by document.
  void setMajorVersion(int Major);

  //! Returns the minor version value.
  int minorVersion() const;
  //! Sets the minor version value.
  void setMinorVersion(int Minor);

  //! Returns the value of the explicit version flag.
  //!
  //! true if an optional document version ('%YAML 1.2') was set in the
  //! document, otherwise false. At present the valid versions are 1.0,
  //! 1.1 and 1.2.
  bool explicitVersion() const;
  //! Sets the value of the explicit version flag.
  //!
  //! true if a document version was set in the document,
  //! otherwise false.
  void setExplicitVersion(bool ExplicitVersion);

  constexpr inline YamlMark& startMark() noexcept;
  constexpr inline YamlMark& endMark() noexcept;

  //! Returns the implicit start flag.
  //!
  //! If the document started with a start document tag (---) then true,
  //! otherwise false.
  bool implicitStart() const;
  //! Sets the value of the implicit start flag.
  //!
  //! If the document started with a start document tag (---) then true,
  //! otherwise false.
  void setImplicitStart(bool ExplicitStart);

  //! Returns the implicit end flag.
  //!
  //! If the document started with a end document tag (...), or another
  //! start document tag (---) then true, otherwise false. A '...' tag
  //! implies that this is the last document in the file and is optional,
  //! the file can merely end.
  bool implicitEnd() const;
  //! Sets the value of the implicit end flag.
  //!
  //! If the document started with a end document tag (...), or another
  //! start document tag (---) then true, otherwise false.
  void setImplicitEnd(bool ImplicitEnd);


  bool getExplicitTags() const;
  void setExplicitTags(bool ExplicitTags);

private:
  bool m_explicitVersion;
  int m_majorVersion = 1;
  int m_minorVersion = 2;
  YamlMark m_start;
  YamlMark m_end;
  bool m_implicitStart = true;
  bool m_implicitEnd = true;
  bool explicitTags = false;
};

class YamlNode : public QObject
{
public:
  enum Type
  {
    None,
    Map,
    Sequence,
    Scalar,
    Comment,
  };

  explicit YamlNode(QObject* parent);

  virtual QString inlinePrint() const = 0;

  virtual QString prettyPrint() const = 0;

  Type type() const;

  //  void setMark(YAML::Mark mark);

  int getPos() const;
  void setPos(int Pos);

  int getLine() const;
  void setLine(int Line);

  int getColumn() const;
  void setColumn(int Column);

protected:
  void setType(Type type);

private:
  Type m_type;
  int m_pos = -1;
  int m_line = -1;
  int m_column = -1;
};

class YamlScalar : public YamlNode
{
public:
  YamlScalar(QObject* parent);
  YamlScalar(const QString& value, QObject* parent);
  YamlScalar(const QString& value, const QString& tag, QObject* parent);

  QString inlinePrint() const override;

  QString prettyPrint() const override;

  QString name() const;
  void setName(const QString& Name);

  QVariant value() const;
  void setValue(const QString& Value);

  QString tag() const;
  void setTag(const QString& Tag);

private:
  QString m_name;
  QVariant m_value;
  QString m_tag;
};

class YamlInteger : public YamlScalar
{
public:
  YamlInteger(QObject* parent);

  QString inlinePrint() const override;

  QString prettyPrint() const override;

  void setValue(int& Value);
};

class YamlReal : public YamlScalar
{
public:
  YamlReal(QObject* parent);

  QString inlinePrint() const override;

  QString prettyPrint() const override;

  void setValue(qreal& Value);
};

class YamlComment : public YamlNode
{
public:
  YamlComment(QObject* parent);

  QString inlinePrint() const override;

  QString prettyPrint() const override;

  QString value() const;
  void setValue(const QString& Value);

private:
  QString m_value;
};

class YamlMap : public YamlNode
{
  Q_OBJECT
public:
  explicit YamlMap(QObject* parent = nullptr);

  QString inlinePrint() const override;

  QString prettyPrint() const override;

  YamlNode* value(const QString& key);
  void insert(const QString& key, YamlNode* value);
  void insert(const QString& key, const QString& value);

private:
  QMultiMap<QString, YamlNode*> m_values;
};

class YamlSequence : public YamlNode
{
  Q_OBJECT
public:
  explicit YamlSequence(QObject* parent = nullptr);

  QString inlinePrint() const override;

  QString prettyPrint() const override;

  YamlNode* value(int index);

  void append(YamlNode* value);

  void append(const QString& value);

private:
  QList<YamlNode*> m_values;
};

class YamlAlias : public YamlNode
{
  Q_OBJECT
public:
  YamlAlias(QObject* parent = nullptr);

  YamlNode* alias();
  void setAlias(YamlNode* node);

  QString inlinePrint() const override;

  QString prettyPrint() const override;

private:
  YamlNode* m_alias = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QYamlBuilder::YamlErrors)
Q_DECLARE_OPERATORS_FOR_FLAGS(QYamlBuilder::YamlWarnings)
