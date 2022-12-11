#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QPoint>
#include <QVariant>

#include <libfyaml.h>

#include "qyaml_global.h"

class YamlDocument;
class YamlNode;
class YamlMap;
class YamlSequence;
class YamlScalar;
class YamlComment;

struct YamlMark
{
  int pos = 0;
  int line = 0;
  int column = 0;

  inline void clear() {
    pos=0;
    line=0;
    column=0;
  }
};

class Markable  {
public:
  virtual YamlMark startMark();
  virtual void setStartMark(YamlMark mark);
  virtual YamlMark endMark();
  virtual void setEndMark(YamlMark mark);

protected:
  YamlMark m_start;
  YamlMark m_end;

};

class KeyMarkable {
public:
  virtual YamlMark startMark(const QString& key);
  virtual void setStartMark(const QString& key, YamlMark mark);
  virtual YamlMark endMark(const QString& key);
  virtual void setEndMark(const QString& key, YamlMark mark);

protected:
  QMap<QString, QPair<YamlMark, YamlMark>> m_marks;

};

class QYAML_SHARED_EXPORT QYamlBuilder : public QObject, public Markable
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

  QList<YamlDocument*> documents() const;
  void setDocuments(QList<YamlDocument*> root);
  void append(YamlDocument* document);
  bool isMultiDocument();
  bool isEmpty();
  int count();
  QList<YamlDocument*>::iterator begin();
  QList<YamlDocument*>::const_iterator constBegin();
  QList<YamlDocument*>::iterator end();
  QList<YamlDocument*>::const_iterator constEnd();

  QString text() const;

  YamlMark startMark() override;
  void setStartMark(YamlMark mark) override;
  YamlMark endMark() override;
  void setEndMark(YamlMark mark) override;

protected:
private:
  QString m_text;
  QList<YamlDocument*> m_documents;
  QString m_filename;
  QString m_zipFile;
  YamlErrors m_errors = NoErrors;
  YamlWarnings m_warnings = NoWarnings;
  YamlDocument* parseDocumentStart(struct fy_event* event);
  YamlSequence* parseSequence(fy_event* event, fy_parser* parser, YamlSequence *sequence);
  YamlMap* parseMap(fy_event* event, struct fy_parser* parser, YamlMap *map);
  YamlComment* parseComment(fy_token* token);
  YamlMark m_start;
  YamlMark m_end;
  void assignStartMark(fy_token *token, Markable* markable);
  void assignEndMark(fy_token *token, Markable* markable);
  void assignStartMark(fy_token *token, KeyMarkable *markable, QString key);
  void assignEndMark(fy_token *token, KeyMarkable *markable, QString key);
  void assignStartMark(fy_token *token, YamlMark &yamlmark);
  void assignEndMark(fy_token *token, YamlMark &yamlmark);
};

class YamlNode : public QObject
{
public:
  YamlNode(QObject* parent);

  YamlNode* parent() const;
  void setParent(YamlNode* Parent);

private:
  YamlNode* m_parent;
};

class YamlMap : public YamlNode, public Markable, public KeyMarkable
{
public:
  YamlMap(QObject* parent = nullptr);
  YamlMap(QMap<QString, YamlNode*> data, QObject* parent = nullptr);

  QMap<QString, YamlNode*> data() const;
  void setData(QMap<QString, YamlNode*> data);
  bool insert(const QString& key, YamlNode* data);
  bool insert(const QString& key, YamlNode* data, YamlMark start, YamlMark end);
  int remove(const QString& key);
  YamlNode* value(const QString& key);
  bool contains(const QString& key);

private:
  QMap<QString, YamlNode*> m_data;
};

class YamlSequence : public YamlNode, public Markable
{
public:
  YamlSequence(QObject* parent = nullptr);
  YamlSequence(QVector<YamlNode*> sequence, QObject* parent = nullptr);

  QVector<YamlNode*> data() const;
  void setData(QVector<YamlNode*> data);
  bool append(YamlNode* data);
  void remove(int index);
  int indexOf(YamlNode* node);  

private:
  QVector<YamlNode*> m_data;
};

class YamlScalar : public YamlNode, public Markable
{
public:
  YamlScalar(QObject* parent = nullptr);
  YamlScalar(QString value, QObject* parent = nullptr);

  QString data() const;
  void setData(const QString& data);

private:
  QString m_data;
};

class YamlComment : public YamlNode, public Markable
{
public:
  YamlComment(QObject* parent = nullptr);
  YamlComment(QString value, QObject* parent = nullptr);

  QString data() const;
  void setData(const QString& data);

private:
  QString m_data;
};

class YamlDocument : public QObject, public Markable
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

  YamlMark startMark() override;
  void setStartMark(YamlMark mark) override;
  YamlMark endMark() override;
  void setEndMark(YamlMark mark) override;

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

  QList<YamlNode*> data() const;
  YamlNode data(int index) const;
  void addData(YamlNode* Data);

private:
  bool m_explicitVersion;
  int m_majorVersion = 1;
  int m_minorVersion = 2;
  bool m_implicitStart = true;
  bool m_implicitEnd = true;
  bool explicitTags = false;
  QList<YamlNode*> m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QYamlBuilder::YamlErrors)
Q_DECLARE_OPERATORS_FOR_FLAGS(QYamlBuilder::YamlWarnings)
