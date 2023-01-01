#ifndef QYAMLDOCUMENT_H
#define QYAMLDOCUMENT_H

#include <QObject>
#include <QTextCursor>

#include "qyaml_global.h"

class YamlNode;

class QYAML_SHARED_EXPORT YamlDocument : public QObject
{
    Q_OBJECT
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

    //! Returns the value of the implicit version flag.
    //!
    //! true if an optional document version ('%YAML 1.2') was not set in the
    //! document, otherwise false. If no version directive is supplied the implied
    //! version is 1.2 (at present)
    bool isImplicitVersion() const;
    //! Sets the value of the implicit version flag.
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

    //! Returns the implicit end flag.
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

    QList<YamlNode*> data() const;
    YamlNode data(int index) const;
    bool addData(YamlNode* Data);

private:
    bool m_implicitVersion;
    int m_majorVersion = 1;
    int m_minorVersion = 2;
    bool m_implicitStart = true;
    bool m_implicitEnd = true;
    bool explicitTags = false;
    QTextCursor m_start;
    QTextCursor m_end;
    QList<YamlNode*> m_data;
    QMap<QTextCursor, YamlNode*> m_nodes;
};


#endif // QYAMLDOCUMENT_H
