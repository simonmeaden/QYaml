#pragma once

#include "SMLibraries/widgets/lnplaintextedit.h"

#include "qyaml_global.h"

class QYamlHighlighter;
class YamlBuilder;

class QYAML_SHARED_EXPORT QYamlEdit : public LNPlainTextEdit
{
  Q_OBJECT
public:
  QYamlEdit(QWidget *parent);


  //! Returns the file name loaded via loadFile(const QString&) or
  //! loadHref(const QString&, const QString&)
  const QString filename() const;
  //! Loads the file in href into the editor.
  void loadFile(const QString& filename);
  //! Loads the file in href from the zipped file zipfile.
  void loadFromZip(const QString& zipFile, const QString& href);
  //! Loads plain text into the editor
  void setText(const QString& text);

private:
  YamlBuilder* m_parser;
  QYamlHighlighter* m_highlighter;
  QString m_filename;
  QString m_zipFile;

  void textHasChanged(int position, int charsRemoved, int charsAdded);

};

