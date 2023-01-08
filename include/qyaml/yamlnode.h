#pragma once

#include <QObject>
#include <QTextCursor>

#include "qyaml/yamlerrors.h"

class YamlNode : public QObject
{
  Q_OBJECT
public:
  enum FlowType
  {
    NoType,
    Flow,
    Block,
  };
  enum Type
  {
    Undefined,
    Scalar,
    Map,
    MapItem,
    Sequence,
    Comment,
  };

  YamlNode(QObject* parent);

  YamlNode* parent() const;
  void setParent(YamlNode* Parent);

  int indent() const;
  void setIndent(int indent);

  int row() const;
  void setRow(int row);

  int getColumn() const;
  void setColumn(int column);

  const QTextCursor& start() const;
  int startPos() const;
  void setStart(const QTextCursor& start);

  const QTextCursor& end() const;
  int endPos() const;
  virtual void setEnd(const QTextCursor& end);

  virtual int length() const;
  void setLength(int newLength);

  const YamlErrors& errors() const;
  void setError(const YamlError& error, bool set);
  void setErrors(const YamlErrors& newErrors);

  const YamlWarnings& warnings() const;
  void setWarning(const YamlWarning& warning, bool set);
  void setWarnings(const YamlWarnings& newWarnings);

  Type type() const;

  FlowType flowType() const;
  void setFlowType(FlowType flowType);

protected:
  Type m_type;
  FlowType m_flowType;

private:
  YamlNode* m_parent;
  int m_indent = 0;
  int m_row = 0;
  int m_column = 0;
  QTextCursor m_start;
  QTextCursor m_end;
  int m_length;

  YamlErrors m_errors;
  YamlWarnings m_warnings;
};

class YamlDirective : public YamlNode
{
  Q_OBJECT
public:
  YamlDirective(int major, int minor, QObject* parent);

  int major() const;
  int minor() const;
  bool isValid();

private:
  int m_major = 0;
  int m_minor = 0;
};

class YamlTag : public YamlNode
{
  Q_OBJECT
public:
  YamlTag(QObject* parent)
    : YamlNode(parent)
  {
  }

  int major() const;
  int minor() const;
  bool isValid();

private:
};

class YamlStart : public YamlNode
{
  Q_OBJECT
public:
  YamlStart(QObject* parent)
    : YamlNode(parent)
  {
  }
};

class YamlEnd : public YamlNode
{
  Q_OBJECT
public:
  YamlEnd(QObject* parent)
    : YamlNode(parent)
  {
  }
};

class YamlAnchor : public YamlNode
{
  Q_OBJECT
public:
  YamlAnchor(QObject* parent)
    : YamlNode(parent)
  {
  }
};

class YamlScalar : public YamlNode
{
  Q_OBJECT
public:
  enum Style
  {
    PLAIN,
    SINGLE_QUOTED,
    DOUBLE_QUOTED,
  };
  YamlScalar(QObject* parent = nullptr);
  YamlScalar(QString value, QObject* parent = nullptr);

  QString data() const;
  void setData(const QString& data);

  Style style() const;

  // YamlNode interface
  int length() const override;

private:
  QString m_data;
  Style m_style;
};

class YamlMapItem : public YamlNode
{
  Q_OBJECT
public:
  YamlMapItem(QObject* parent);
  YamlMapItem(const QString& key, YamlNode* data, QObject* parent);

  const QString& key() const;
  void setKey(const QString& key);
  int keyLength() const;

  YamlNode* data() const;
  void setData(YamlNode* data);

//  const QTextCursor& keyStart() const;
//  int keyStartPos();
//  void setKeyStart(const QTextCursor& keyStart);

private:
  QString m_key;
  YamlNode* m_data = nullptr;
  QTextCursor m_keyStart;
  QTextCursor m_dataStart;
};

class YamlMap : public YamlNode
{
  Q_OBJECT
public:
  YamlMap(QObject* parent = nullptr);
  YamlMap(QMap<QString, YamlMapItem*> data, QObject* parent = nullptr);

  QMap<QString, YamlMapItem*> data() const;
  void setData(QMap<QString, YamlMapItem*> data);
  bool insert(const QString& key, YamlMapItem* data);
  int remove(const QString& key);
  YamlMapItem* value(const QString& key);
  bool contains(const QString& key);

private:
  QMap<QString, YamlMapItem*> m_data;
};

class YamlSequence : public YamlNode
{
  Q_OBJECT
public:
  YamlSequence(QObject* parent = nullptr);
  YamlSequence(QVector<YamlNode*> sequence, QObject* parent = nullptr);

  QVector<YamlNode*> data() const;
  void setData(QVector<YamlNode*> data);
  bool append(YamlNode* data);
  void remove(int index);
  int indexOf(YamlNode* node);

  //  void setEnd(const QTextCursor& end) override;

private:
  QVector<YamlNode*> m_data;
};

class YamlComment : public YamlNode
{
  Q_OBJECT
public:
  YamlComment(QObject* parent = nullptr);
  YamlComment(QString value, QObject* parent = nullptr);

  void append(QChar c);
  QString data() const;
  void setData(const QString& data);

  // YamlNode interface
  int length() const;

private:
  QString m_data;
};

