#pragma once

#include <QObject>
#include <QTextCursor>

#include "qyaml/yamlerrors.h"

class YamlNode; // forward declare so QSharedPointer works;
//! \typedef typedef SharedNode SharedNode
//! typedef for a shared pointer to YamlNode.
typedef QSharedPointer<YamlNode> SharedNode;
class YamlNode : public QObject
{
  Q_OBJECT
public:
  enum FlowType
  {
    NoFlowType,
    Flow,
    Block,
  };
  enum Type
  {
    Undefined,
    Directive,
    YamlDirective,
    TagDirective,
    ReservedDirective,
    Start,
    End,
    Scalar,
    Map,
    MapItem,
    Sequence,
    Comment,
    Anchor,
  };
  enum TagHandleType
  {
    NoTagType,
    Named,
    Secondary,
    Primary,
    Shorthand,
    NonSpecific,
  };

  YamlNode(QObject* parent = nullptr);

  SharedNode parent() const;
  void setParent(SharedNode Parent);

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
  virtual QString toString(const QString& text, FlowType override = NoFlowType);

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
      //      if (errors.testFlag(EmptyFlowValue))
      //        list.append(tr("In flow values cannot be empty"));
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
      if (warnings.testFlag(ReservedDirectiveWarning))
        list.append(tr("Illegal tag directive\n"
                       "Reserved for futute YAML use."));
      if (warnings.testFlag(InvalidSpaceWarning))
        list.append(tr("The tag name should follow immediately\n"
                       "after the % with no spaces."));
      if (warnings.testFlag(TabCharsDiscouraged))
        list.append(
          tr("The use of literal tabs (\\t) characters is discouraged."));
      if (warnings.testFlag(IllegalCommentPosition))
        list.append(tr("Comments are technically illegal at this position."));
      // TODO complete the entire warnings flags
    }
    return list;
  }

protected:
  Type m_type = Undefined;
  FlowType m_flowType = Block;

private:
  SharedNode m_parent;
  int m_indent = 0;
  int m_row = 0;
  int m_column = 0;
  QTextCursor m_start;
  QTextCursor m_end;

  YamlErrors m_errors;
  YamlWarnings m_warnings;
  QMap<QTextCursor, YamlWarning> m_dodgyChars;
};

class YamlDirective : public YamlNode
{
  Q_OBJECT
public:
  YamlDirective(QObject* parent = nullptr);

  QTextCursor nameStart() const;
  void setNameStart(const QTextCursor& nameStart);

  QString name() const;
  void setName(const QString& name);

private:
  QString m_name;
  QTextCursor m_nameStart;
};

//! \typedef typedef QSharedPointer<YamlDirective> SharedDirective
//! typedef for a shared pointer to YamlDirective.
typedef QSharedPointer<YamlDirective> SharedDirective;

class YamlReservedDirective : public YamlDirective
{
  Q_OBJECT
public:
  YamlReservedDirective(QObject* parent = nullptr);

  void addParameter(QTextCursor cursor, const QString& param);
  QString parameter(QTextCursor cursor);
  QMap<QTextCursor, QString> parameters();

private:
  QMap<QTextCursor, QString> m_parameters;
};
//! \typedef typedef QSharedPointer<YamlReservedDirective>
//! SharedReservedDirective typedef for a shared pointer to
//! YamlReservedDirective.
typedef QSharedPointer<YamlReservedDirective> SharedReservedDirective;

class YamlYamlDirective : public YamlDirective
{
  Q_OBJECT
public:
  YamlYamlDirective(QObject* parent = nullptr);
  YamlYamlDirective(int major, int minor, QObject* parent = nullptr);

  int major() const;
  void setMajor(int major);

  int minor() const;
  void setMinor(int minor);

  bool isValid();

  QTextCursor versionStart() const;
  int versionStartPos() const;
  void setVersionStart(const QTextCursor& versionStart);

private:
  int m_major = 1;
  int m_minor = 2;
  QTextCursor m_versionStart;
};
//! \typedef typedef SharedYamlDirective SharedYamlDirective
//! typedef for a shared pointer to YamlYamlDirective.
typedef QSharedPointer<YamlYamlDirective> SharedYamlDirective;

class YamlTagDirective : public YamlDirective
{
  Q_OBJECT
public:
  YamlTagDirective(QObject* parent = nullptr);
  YamlTagDirective(const QString& handle,
                   const QString& value,
                   QObject* parent = nullptr);

  bool isValid();

  QString value() const;
  void setValue(const QString& value);
  void setValueStart(QTextCursor cursor);
  QTextCursor valueStart();
  int valueStartPos();

  QString handle() const;
  void setHandle(const QString& id);
  void setHandleStart(QTextCursor cursor);
  QTextCursor handleStart();
  int handleStartPos();

  TagHandleType handleType() const;
  void setHandleType(TagHandleType handleType);

private:
  TagHandleType m_handleType = NoTagType;
  QTextCursor m_handleStart;
  QString m_handle;
  QTextCursor m_valueStart;
  QString m_value;
};
//! \typedef typedef SharedTagDirective SharedTagDirective
//! typedef for a shared pointer to YamlTagDirective.
typedef QSharedPointer<YamlTagDirective> SharedTagDirective;

class YamlStart : public YamlNode
{
  Q_OBJECT
public:
  YamlStart(QObject* parent = nullptr);
};
//! \typedef typedef SharedStart SharedStart
//! typedef for a shared pointer to YamlStart.
typedef QSharedPointer<YamlStart> SharedStart;

class YamlEnd : public YamlNode
{
  Q_OBJECT
public:
  YamlEnd(QObject* parent = nullptr);
};
//! \typedef typedef QSharedPointer<YamlEnd> SharedEnd
//! typedef for a shared pointer to YamlEnd.
typedef QSharedPointer<YamlEnd> SharedEnd;

class YamlAnchorBase : public YamlNode
{
  Q_OBJECT
public:
  YamlAnchorBase(QObject* parent = nullptr);

  QString name() const;
  void setName(const QString& name);

  QTextCursor nameStart() const;
  void setNameStart(const QTextCursor& nameStart);

private:
  QString m_name;
  QTextCursor m_nameStart;
};
//! \typedef typedef QSharedPointer<YamlAnchor> SharedAnchor
//! typedef for a shared pointer to YamlAnchor.
typedef QSharedPointer<YamlAnchorBase> SharedAnchorBase;

class YamlAnchor : public YamlAnchorBase
{
  Q_OBJECT
public:
  YamlAnchor(QObject* parent = nullptr);

};
//! \typedef typedef QSharedPointer<YamlAnchor> SharedAnchor
//! typedef for a shared pointer to YamlAnchor.
typedef QSharedPointer<YamlAnchor> SharedAnchor;

class YamlAlias : public YamlAnchorBase
{
  Q_OBJECT
public:
  YamlAlias(QObject* parent = nullptr);

private:
  SharedAnchor m_anchor;

};
//! \typedef typedef QSharedPointer<YamlAnchor> SharedAnchor
//! typedef for a shared pointer to YamlAnchor.
typedef QSharedPointer<YamlAlias> SharedAlias;

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
//! \typedef typedef QSharedPointer<YamlScalar> SharedScalar
//! typedef for a shared pointer to YamlScalar.
typedef QSharedPointer<YamlScalar> SharedScalar;

class YamlMapItem : public YamlNode
{
  Q_OBJECT
public:
  YamlMapItem(QObject* parent = nullptr);
  YamlMapItem(const QString& key, SharedNode data, QObject* parent = nullptr);

  const QString& key() const;
  void setKey(const QString& key);
  int keyLength() const;

  SharedNode data() const;
  void setData(SharedNode data);

  //  const QTextCursor& keyStart() const;
  //  int keyStartPos();
  //  void setKeyStart(const QTextCursor& keyStart);
  // YamlNode interface
  QString toString(const QString& text, FlowType override) override;

private:
  QString m_key;
  SharedNode m_data = nullptr;
  QTextCursor m_keyStart;
  QTextCursor m_dataStart;
};
//! \typedef typedef QSharedPointer<YamlMapItem> SharedMapItem
//! typedef for a shared pointer to YamlMapItem.
typedef QSharedPointer<YamlMapItem> SharedMapItem;

class YamlMap : public YamlNode
{
  Q_OBJECT
public:
  YamlMap(QObject* parent = nullptr);
  YamlMap(QMap<QString, QSharedPointer<YamlMapItem>> data,
          QObject* parent = nullptr);

  QMap<QString, QSharedPointer<YamlMapItem>> data() const;
  void setData(QMap<QString, QSharedPointer<YamlMapItem>> data);
  bool insert(const QString& key, QSharedPointer<YamlMapItem> data);
  int remove(const QString& key);
  QSharedPointer<YamlMapItem> value(const QString& key);
  bool contains(const QString& key);

  // YamlNode interface
  QString toString(const QString& text, FlowType override) override;

private:
  QMap<QString, QSharedPointer<YamlMapItem>> m_data;
};
//! \typedef typedef QSharedPointer<YamlMap> SharedMap
//! typedef for a shared pointer to YamlMap.
typedef QSharedPointer<YamlMap> SharedMap;

class YamlSequence : public YamlNode
{
  Q_OBJECT
public:
  YamlSequence(QObject* parent = nullptr);
  YamlSequence(QVector<SharedNode> sequence, QObject* parent = nullptr);

  QVector<SharedNode> data() const;
  void setData(QVector<SharedNode> data);
  bool append(SharedNode data);
  void remove(int index);
  int indexOf(SharedNode node);

  //  void setEnd(const QTextCursor& end) override;

  // YamlNode interface
  QString toString(const QString& text, FlowType override) override;

private:
  QVector<SharedNode> m_data;
};
//! \typedef typedef QSharedPointer<YamlSequence> SharedSequence
//! typedef for a shared pointer to YamlSequence.
typedef QSharedPointer<YamlSequence> SharedSequence;

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
//! \typedef typedef QSharedPointer<YamlComment> SharedComment
//! typedef for a shared pointer to YamlComment.
typedef QSharedPointer<YamlComment> SharedComment;
