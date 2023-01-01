#include "qyaml/qyamldocument.h"
#include "qyaml/yamlnode.h"

//====================================================================
//=== YamlDocument
//====================================================================
YamlDocument::YamlDocument(QObject* parent)
    : QObject(parent)
{
}

int
YamlDocument::majorVersion() const
{
    return m_majorVersion;
}

void
YamlDocument::setMajorVersion(int Major)
{
    m_majorVersion = Major;
}

int
YamlDocument::minorVersion() const
{
    return m_minorVersion;
}

void
YamlDocument::setMinorVersion(int Minor)
{
    m_minorVersion = Minor;
}

bool
YamlDocument::implicitStart() const
{
    return m_implicitStart;
}

void
YamlDocument::setImplicitStart(bool ExplicitStart)
{
    m_implicitStart = ExplicitStart;
}

bool
YamlDocument::isImplicitVersion() const
{
    return m_implicitVersion;
}

void
YamlDocument::setImplicitVersion(bool ExplicitVersion)
{
    m_implicitVersion = ExplicitVersion;
}

QTextCursor
YamlDocument::documentStart()
{
    return m_start;
}

void
YamlDocument::setDocumentStart(QTextCursor mark)
{
    m_start = mark;
}

QTextCursor
YamlDocument::documentEnd()
{
    return m_end;
}

void
YamlDocument::setDocumentEnd(QTextCursor mark)
{
    m_end = mark;
}

bool
YamlDocument::implicitEnd() const
{
    return m_implicitEnd;
}

void
YamlDocument::setImplicitEnd(bool ImplicitEnd)
{
    m_implicitEnd = ImplicitEnd;
}

bool
YamlDocument::getExplicitTags() const
{
    return explicitTags;
}

void
YamlDocument::setExplicitTags(bool ExplicitTags)
{
    explicitTags = ExplicitTags;
}

QList<YamlNode*>
YamlDocument::data() const
{
    return m_data;
}

YamlNode
YamlDocument::data(int index) const
{
    return m_data.at(index);
}

bool
YamlDocument::addData(YamlNode* data)
{
    m_data.append(data);
    return true;
}
