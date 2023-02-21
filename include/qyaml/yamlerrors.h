#pragma once

#include <QObject>

enum YamlError
{
  NoErrors = 0,
  InvalidMajorVersion = 0x1,
  BadYamlDirective = 0x2,
  TooManyYamlDirectivesError = 0x4,

  IllegalFirstCharacter = 0x100,

  EmptyFlowValue = 0x1000,

  MissingMatchingQuote = 0x10000,
};
Q_DECLARE_FLAGS(YamlErrors, YamlError)
Q_DECLARE_OPERATORS_FOR_FLAGS(YamlErrors)

enum YamlWarning
{
  NoWarnings = 0,
  InvalidMinorVersionWarning = 0x1,
  TabCharsDiscouraged = 0x2,
  PossibleCommentInScalar = 0x4,
  ReservedDirectiveWarning = 0x8,
  InvalidSpaceWarning = 0x10,
  IllegalCommentPosition = 0x20,
};
Q_DECLARE_FLAGS(YamlWarnings, YamlWarning)
Q_DECLARE_OPERATORS_FOR_FLAGS(YamlWarnings)
