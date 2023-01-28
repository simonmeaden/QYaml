#pragma once

#include <QObject>
#include <QTextCursor>

#include "qyaml/yamlerrors.h"
#include "qyaml/yamlnode.h"
#include "qyaml_global.h"

class YamlNode;
class YamlMap;
class YamlSequence;
class YamlMapItem;
class YamlDirective;
class YamlTagDirective;

class QYAML_SHARED_EXPORT QYamlDocument : public QObject
{
  Q_OBJECT
public:
  explicit QYamlDocument(QObject* parent);

  //! Returns the major version value.
  //!
  //! default value 1.
  int majorVersion() const;

  //  //! Sets the major version value.
  //  //!
  //  //! default value is 2 unless specified by document.
  //  void setMajorVersion(int Major);

  //! Returns the minor version value.
  int minorVersion() const;

  //  //! Sets the minor version value.
  //  void setMinorVersion(int Minor);

  //! Returns the value of the implicit version flag.
  //!
  //! true if an optional document version ('%YAML 1.2') was not set in the
  //! document, otherwise false. If no version directive is supplied the implied
  //! version is 1.2 (at present)
  bool isImplicitVersion() const;

  //! Returns the QTextCursor start position of the version directive
  QTextCursor versionStart() const;

  //! Returns the int start position of the version directive
  int versionStartPos() const;

  //  //! Returns the start QTextCursor position of the version directive
  //  void setVersionStart(const QTextCursor &versionStart);

  //! Returns the length of the version directive
  int versionLength() const;

  //  //! Sets the length of the version directive
  //  void setVersionLength(int versionLength);

  //! Sets the value of the implicit version flag.node
  //!
  //! true if a document version was not explicitly set in the document,
  //! otherwise false.
  void setImplicitVersion(bool implicitVersion);

  //! Returns the QTextCursor for the start of the document text.
  QTextCursor start();

  //! Returns the int position for the start of the document text.
  int startPos();

  //! Sets the QTextCursor for the start of the document text.
  void setStart(QTextCursor position, YamlStart* start = nullptr);

  bool hasStart();

  //! Returns the QTextCursor for the end of the document text.
  QTextCursor end();

  //! Returns the int position for the end of the document text.
  int endPos();

  //! Sets the QTextCursor for the end of the document text.
  void setEnd(QTextCursor mark, YamlEnd* end = nullptr);

  bool hasEnd();

  //! Returns the text length for this document.
  int textLength();

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

  //! Returns the ordered list of yaml nodes.
  //!
  //! To return the map of QTextCursor->Node then use the nodeMap() method.
  QList<YamlNode*> nodes() const;

  //! Returns the map of QTextCursor->Node.
  //!
  //! To return the ordered node list use the nodes() method.
  const QMap<QTextCursor, YamlNode*>& nodeMap() const;

  //! Returns the yaml root node item at index.
  YamlNode* node(int index);

  //! Returns the yaml root node item at the cursor.
  YamlNode* node(QTextCursor cursor);

  //! Adds a YamlNode* to the document and returns true if successful, otherwise
  //! returns false.
  //!
  //! Only scalars, maps and sequences can be added to the document. Other
  //! types are internal sub types of these types.
  bool addNode(YamlNode* Data, bool root = false);

  QMap<QTextCursor, YamlTagDirective*> tags() const;
  void setTags(const QMap<QTextCursor, YamlTagDirective*>& tags);
  void addTag(YamlTagDirective* tag);
  bool hasTag();
  void removeTag(QTextCursor position);

  YamlDirective* getDirective() const;
  bool hasDirective();
  void setDirective(YamlDirective* directive);


  //! returns the list of document errors.
  const YamlErrors& errors() const;

  //! Sets/clears an error for the document
  void setError(const YamlError &error, bool set);

  //! Sets a number of errors for the document
  void setErrors(const YamlErrors& newErrors);

  //! returns the list of document warnings.
  const YamlWarnings& warnings() const;

  //! Sets/clears a warning for the document
  void setWarning(const YamlWarning& warning, bool set);

  //! Sets a number of warnings for the document#include "qyaml/yamlnode.h"
  void setWarnings(const YamlWarnings& newWarnings);

private:
  YamlDirective* m_directive = nullptr;
  //  QTextCursor
  //      m_versionStart;  //!< the start position of '%YAML 1.n' YAML directive
  //  int m_versionLength; //!< the length of the directive string
  bool m_implicitVersion =
    true; //!< true if the directive string is NOT in document.
          //  int m_majorVersion = 1; //!< the major version number, default 1
          //  int m_minorVersion = 2; //!< the minor version number, default 2

  bool m_implicitStart = true; //!< true if '---' is NOT in document
  bool m_implicitEnd = true;   //!< true if '...' is not in document

  bool explicitTags = false;

  QTextCursor m_start;
  QTextCursor m_end;
  // holds ordered ROOT list of data
  QList<YamlNode*> m_root;
  // holds ordered list of all nodes in document
  QList<YamlNode*> m_data;
  // holds a map position => node* of ALL nodes.
  QMap<QTextCursor, YamlNode*> m_nodes;
  QMap<QTextCursor, YamlTagDirective*> m_tags;

  YamlErrors m_errors;
  YamlWarnings m_warnings;

  bool addSequenceData(YamlSequence* sequence, YamlMapItem* item = nullptr);
  bool addMapData(YamlMap* map, YamlMapItem* item = nullptr);
  bool addMapItemData(YamlMapItem* item);
};
