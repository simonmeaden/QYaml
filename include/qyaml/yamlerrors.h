#pragma once

#include <QObject>

enum YamlError {
  NoErrors = 0,
  InvalidVersionError,
  BadYamlDirective,
  TooManyYamlDirectivesError,
};
Q_DECLARE_FLAGS(YamlErrors, YamlError)
Q_DECLARE_OPERATORS_FOR_FLAGS(YamlErrors)

enum YamlWarning {
  NoWarnings = 0,
  InvalidMinorVersionWarning,
};
Q_DECLARE_FLAGS(YamlWarnings, YamlWarning)
Q_DECLARE_OPERATORS_FOR_FLAGS(YamlWarnings)
