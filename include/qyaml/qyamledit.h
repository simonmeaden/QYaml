#pragma once

#include "lnplaintext/lnplaintextedit.h"

#include "qyaml_global.h"

class QYamlHighlighter;
class QYamlParser;
class HoverWidget;
class YamlNode;

class QYAML_SHARED_EXPORT QYamlEdit : public QLNPlainTextEdit
{
  Q_OBJECT
public:
  QYamlEdit(QWidget* parent);

  //! Returns the file name loaded via loadFile(const QString&) or
  //! loadHref(const QString&, const QString&)
  const QString filename() const;
  //! Loads the file in href into the editor.
  void loadFile(const QString& filename);
  //! Loads the file in href from the zipped file zipfile.
  void loadFromZip(const QString& zipFile, const QString& href);
  //! Loads plain text into the editor
  void setText(const QString& text);

  //! Returns the time that a hover window is displayed in mS, default 4000mS.
  int hoverTime() const;
  //! Sets the time that a hover window is displayed in mS, default 4000mS.
  void setHoverTime(int hoverTime);

  void createHoverWidget(QPoint pos, QString text, QString title);

protected:
  //  //! \reimplements{QLNPlainTextEdit::mousePressEvent(QMouseEvent*)
  //  void mousePressEvent(QMouseEvent* event) override;
  //  //! \reimplements{QLNPlainTextEdit::mouseMoveEvent(QMouseEvent*)
  //  void mouseMoveEvent(QMouseEvent* event) override;
  //  //! \reimplements{QLNPlainTextEdit::mouseReleaseEvent(QMouseEvent*)
  //  void mouseReleaseEvent(QMouseEvent* event) override;

  void hoverEnter(QHoverEvent* event) override;
  void hoverLeave(QHoverEvent* event) override;
  void hoverMove(QHoverEvent* event) override;

private:
  QYamlParser* m_parser;
  QYamlHighlighter* m_highlighter;
  QString m_filename;
  QString m_zipFile;
  HoverWidget* m_hoverWidget = nullptr;
  YamlNode* m_hoverNode = nullptr;
  int m_hoverTime = HOVERTIME;

  void killHoverWidget();
  void textHasChanged(int position, int charsRemoved, int charsAdded);
  bool isInText(const QPoint& pos);

  static const int HOVERTIME = 4000;
};
