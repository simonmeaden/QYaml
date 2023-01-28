#pragma once

#include <QObject>

enum YamlError
{
  NoErrors = 0,
  InvalidMajorVersion = 0x1,
  InvalidMinorVersion = 0x2,
  BadYamlDirective = 0x4,
  TooManyYamlDirectivesError = 0x8,

  IllegalFirstCharacter = 0x10,

  EmptyFlowValue = 0x100,

  MissingMatchingQuote = 0x1000,
};
Q_DECLARE_FLAGS(YamlErrors, YamlError)
Q_DECLARE_OPERATORS_FOR_FLAGS(YamlErrors)

enum YamlWarning
{
  NoWarnings = 0,
  InvalidMinorVersionWarning = 0x1,
  TabCharsDiscouraged = 0x2,
  PossibleCommentInScalar = 0x4,
};
Q_DECLARE_FLAGS(YamlWarnings, YamlWarning)
Q_DECLARE_OPERATORS_FOR_FLAGS(YamlWarnings)
