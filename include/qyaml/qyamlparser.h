#pragma once

#include <QObject>
#include <QTextDocument>
#include <QRegularExpression>
#include <QFile>

#include "quazip/JlCompress.h"

#include "qyaml_global.h"
#include "qyaml/yamlnode.h"

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

    bool parseLine(const QString& line);
    bool parse(const QString& text, int startPos = 0);
    bool parse(const char* text);

    //! Returns the file name loaded via loadFile(const QString&) or
    //! loadHref(const QString&, const QString&)
    const QString filename() const;
    //! Loads the file in href into the editor.
    bool loadFile(const QString& filename);
    //! Loads the file in href from the zipped file zipfile.
    bool loadFromZip(const QString& zipFile, const QString& href);

    QList<QYamlDocument*> documents() const;
    void setDocuments(QList<QYamlDocument*> root);
    void append(QYamlDocument* document);
    bool isMultiDocument();
    bool isEmpty();
    int count();
    QList<QYamlDocument*>::iterator begin();
    QList<QYamlDocument*>::const_iterator constBegin();
    QList<QYamlDocument*>::iterator end();
    QList<QYamlDocument*>::const_iterator constEnd();
    QString text() const;

    const QMap<QTextCursor, YamlNode*>& nodes() const;

signals:
    void parseComplete();

protected:
private:
    QString m_text;
    QTextDocument* m_document = nullptr;
    QMap<QTextCursor, YamlNode*> m_nodes;
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

    bool fyParse(const char* text);
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
    void addScalarToSequence(YamlSequence* sequence,
                             const QString t,
                             int& tStart,
                             int& indent);
    void parseFlowSequence(YamlSequence* sequence, int& i, const QString& text);
    void parseFlowMap(YamlMap* map, int& i, const QString& text);
    YamlComment* parseComment(int& i, const QString& text);
};

