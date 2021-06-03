#ifndef Q3D_GLOBAL_H
#define Q3D_GLOBAL_H


#include <QtCore/qglobal.h>

#if defined(Q3D_GCL_LIBRARY)
#  define Q3DGCLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define Q3DGCLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // Q3D_GLOBAL_H
