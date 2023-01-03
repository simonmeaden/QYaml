#pragma once

#include <QObject>
#include <QTextCursor>

#include "qyaml/yamlerrors.h"
#include "qyaml_global.h"

class YamlNode;
class YamlMap;
class YamlSequence;
class YamlMapItem;

class QYAML_SHARED_EXPORT QYamlDocument : public QObject
{
  Q_OBJECT
public:
  explicit QYamlDocument(QObject* parent);

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

  //! Returns the value of the implicit version flag.
  //!
  //! true if an optional document version ('%YAML 1.2') was not set in the
  //! document, otherwise false. If no version directive is supplied the implied
  //! version is 1.2 (at present)
  bool isImplicitVersion() const;
  //! Sets the value of the implicit version flag.node
  //!
  //! true if a document version was not explicitly set in the document,
  //! otherwise false.
  void setImplicitVersion(bool ExplicitVersion);

  QTextCursor documentStart();
  void setDocumentStart(QTextCursor mark);
  QTextCursor documentEnd();
  void setDocumentEnd(QTextCursor mark);

  //! Returns the implicit start flag.
  //!
  //! If the document started with a start document tag (---) then false,
  //! otherwise false.
  bool implicitStart() const;
  //! Sets the value of the implicit start flag.
  //!
  //! If the document started with a start document tag (---) then false,
  //! otherwise false.
  void setImplicitStart(bool ExplicitStart);

  //! Returns the implicit end flag.  bool addMapData(YamlMap* map);

  //!
  //! The document can merely end, or alternatively reach another document
  //! start, in which case this returns true as there was no document end (...).
  //! If there was a document end then this will return false.
  bool implicitEnd() const;
  //! Sets the value of the implicit end flag.
  //!
  //! The document can merely end, or alternatively reach another document
  //! start, in which case this returns true as there was no document end (...).
  //! If there was a document end then this will return false.
  void setImplicitEnd(bool ImplicitEnd);

  bool getExplicitTags() const;
  void setExplicitTags(bool ExplicitTags);

  //! Returns the yaml root node list.
  QList<YamlNode*> data() const;

  //! Returns the yaml root node item at index.
  YamlNode* data(int index);

  //! Adds a YamlNode* to the document and returns true if successful, otherwise
  //! returns false.
  //!
  //! Only scalars, maps and sequences can be added to the document. Other
  //! types are internal sub types of these types.
  bool addData(YamlNode* Data);

  //! returns the list of document errors.
  const YamlErrors& errors() const;

  //! Sets/clears an error for the document
  void setError(const YamlError& error, bool set);

  //! Sets a number of errors for the document
  void setErrors(const YamlErrors& newErrors);

  //! returns the list of document warnings.
  const YamlWarnings& warnings() const;

  //! Sets/clears a warning for the document
  void setWarning(const YamlWarning& warning, bool set);

  //! Sets a number of warnings for the document#include "qyaml/yamlnode.h"

  void setWarnings(const YamlWarnings& newWarnings);

  const QMap<QTextCursor, YamlNode*>& nodes() const;

private:
  bool m_implicitVersion;
  int m_majorVersion = 1;
  int m_minorVersion = 2;
  bool m_implicitStart = true;
  bool m_implicitEnd = true;
  bool explicitTags = false;
  QTextCursor m_start;
  QTextCursor m_end;
  // holds ROOT layer of data
  QList<YamlNode*> m_data;
  // holds a map position => node* of ALL nodes.
  QMap<QTextCursor, YamlNode*> m_nodes;

  YamlErrors m_errors;
  YamlWarnings m_warnings;

  bool addSequenceData(YamlSequence* sequence, YamlMapItem* item = nullptr);
  bool addMapData(YamlMap* map, YamlMapItem* item = nullptr);
  bool addMapItemData(YamlMapItem* item);
};
