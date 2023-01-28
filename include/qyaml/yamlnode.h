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
    YamlDirective,
    TagDirective,
    Start,
    End,
    Scalar,
    Map,
    MapItem,
    Sequence,
    Comment,
  };

  YamlNode(QObject* parent = nullptr);

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
  //  void setLength(int newLength);

  //! Returns the value of the errors flags.
  const YamlErrors& errors() const;

  //! Sets the value of the errors flag if set is true, otherwise
  //! removes the value of the flag.
  void setError(const YamlError& error, bool set);

  //! Sets the value of the error flags to the supplied set.
  void setErrors(const YamlErrors& newErrors);

  //! Returns true if the node has any errors.
  bool hasErrors();

  //! Returns the value of the warning flags.
  const YamlWarnings& warnings() const;

  //! Sets the value of the warning flag if set is true, otherwise
  //! removes the value of the flag.
  void setWarning(const YamlWarning& warning, bool set);

  //! Sets the value of the earnings flags to the supplied set.
  void setWarnings(const YamlWarnings& newWarnings);

  //! Returns true if the node has any warnings.
  bool hasWarnings();

  //! Returns true if non-preferred characters exist within the value.
  //!
  //! Certain characters such as inline tabs are allowed but not preferred
  //! within scalar values.
  //!
  //! This will indicate that there are non-preferred characters if and only
  //! if the type of the node is Scalar.
  bool hasDodgyChar();

  //! Returns a map of QTextCursor => YamlWarning.
  //!
  //! Certain characters such as inline tabs are allowed but not preferred
  //! within scalar values.
  //!
  //! This will indicate that there are non-preferred characters if and only
  //! if the type of the node is Scalar.
  QMap<QTextCursor, YamlWarning> dodgyChars() const;

  //! Certain characters such as inline tabs are allowed but not preferred
  //! within scalar values.
  //!
  //! This will indicate that there are non-preferred characters if and only
  //! if the type of the node is Scalar.
  void addDodgyChar(QTextCursor pos, YamlWarning warning);

  //! Certain characters such as inline tabs are allowed but not preferred
  //! within scalar values.
  //!
  //! This will indicate that there are non-preferred characters if and only
  //! if the type of the node is Scalar.
  QMap<QTextCursor, YamlWarning>::size_type removeDodgyChar(QTextCursor pos);

  Type type() const;

  FlowType flowType() const;
  void setFlowType(FlowType flowType);

  //! Returns the text as a string.
  //!
  //! The text parameter should hold the entire text of the wrapping
  //! document rather than the sub QYamlDocuments.
  //! The optional override FlowType allows the user to return a Flow/Block
  //! type irrespective of the original stored type.
  //!
  //! In most cases the returns the string as an unmodified string but in
  //! some cases, flow maps, sequences and scalars for example the actual
  //! string will depend on circumstances. For instance flow collections
  //! can only contain flow collections and flow scalars, not block collections
  //! and scalars. In these cases the override flow can be used to override
  //! the stored flow type.
  virtual QString toString(const QString& text, FlowType override = NoType);

  static const QStringList errorText(YamlErrors errors)
  {
    QStringList list;
    if (errors.testFlag(NoErrors))
      return list;
    else {
      if (errors.testFlag(InvalidMajorVersion))
        list.append(tr("The version is invalid"));
      if (errors.testFlag(BadYamlDirective))
        list.append(tr("The %YAML is invalid"));
      if (errors.testFlag(TooManyYamlDirectivesError))
        list.append(tr("The document has more than one %YAML directive"));
      if (errors.testFlag(IllegalFirstCharacter))
        list.append(tr("The first character of the scalar is invalid"));
      if (errors.testFlag(MissingMatchingQuote))
        list.append(
          tr("The scalar has a start ' or \" but no a matching closer"));
      if (errors.testFlag(EmptyFlowValue))
        list.append(tr("In flow values cannot be empty"));
      // TODO complete the entire errors flags
    }
    return list;
  }

  static const QStringList warningText(YamlWarnings warnings)
  {
    QStringList list;
    if (warnings.testFlag(NoWarnings))
      return list;
    else {
      if (warnings.testFlag(InvalidMinorVersionWarning))
        list.append(tr("The minor version is invalid"));
      if (warnings.testFlag(TabCharsDiscouraged))
        list.append(tr("The use of literal tabs (\\t) characters is discouraged."));
      // TODO complete the entire warnings flags
    }
    return list;
  }

protected:
  Type m_type = Undefined;
  FlowType m_flowType = Block;

private:
  YamlNode* m_parent;
  int m_indent = 0;
  int m_row = 0;
  int m_column = 0;
  QTextCursor m_start;
  QTextCursor m_end;
  //  int m_length;

  YamlErrors m_errors;
  YamlWarnings m_warnings;
  QMap<QTextCursor, YamlWarning> m_dodgyChars;

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
  int m_major = 1;
  int m_minor = 2;
};

class YamlTagDirective : public YamlNode
{
  Q_OBJECT
public:
  enum HandleType {
    NoType,
    Named,
    Secondary,
    Primary,
  };
  YamlTagDirective(const QString& handle, const QString& value, QObject* parent);

  bool isValid();

  QString value() const;
  void setValue(const QString& value);

  QString handle() const;
  void setHandle(const QString &id);

  HandleType handleType() const;
  void setHandleType(HandleType handleType);

private:
  HandleType m_handleType = NoType;
  QString m_handle;
  QString m_value;
};

class YamlStart : public YamlNode
{
  Q_OBJECT
public:
  YamlStart(QObject* parent);
};

class YamlEnd : public YamlNode
{
  Q_OBJECT
public:
  YamlEnd(QObject* parent);
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
    SINGLEQUOTED,
    DOUBLEQUOTED,
  };
  YamlScalar(QObject* parent = nullptr);
  YamlScalar(QString value, QObject* parent = nullptr);

  QString data() const;
  void setData(const QString& data);

  Style style() const;
  void setStyle(Style style);

  // YamlNode interface
  int length() const override;

  bool multiline() const;

  // YamlNode interface
  QString toString(const QString& text, FlowType override) override;

private:
  QString m_data;
  Style m_style;
  bool m_multiline = false;

  QString toFlowScalar(const QString& text);
  QString toBlockScalar(const QString& text);
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
  // YamlNode interface
  QString toString(const QString& text, FlowType override) override;

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

  // YamlNode interface
  QString toString(const QString& text, FlowType override) override;

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

  // YamlNode interface
  QString toString(const QString& text, FlowType override) override;

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
  int length() const override;
  QString toString(const QString& text, FlowType override) override;

private:
  QString m_data;
};
