#ifndef QNIKONCAMERA_GLOBAL_H
#define QNIKONCAMERA_GLOBAL_H

#include <QtCore/qglobal.h>


#ifdef QNIKONCAMERA_LIB
# define QNIKONCAMERA_EXPORT Q_DECL_EXPORT
#else
# define QNIKONCAMERA_EXPORT Q_DECL_IMPORT
#endif




#endif // QNIKONCAMERA_GLOBAL_H