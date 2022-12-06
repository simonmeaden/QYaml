#pragma once

#include <QtCore/qglobal.h>

#if defined(QYAML_LIBRARY)
  #define QYAML_SHARED_EXPORT Q_DECL_EXPORT
#else
  #define QYAML_SHARED_EXPORT Q_DECL_IMPORT
#endif

