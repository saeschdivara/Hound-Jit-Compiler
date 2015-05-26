#ifndef COMPILER_H
#define COMPILER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include "expression.h"

class VmCompiler : public QObject
{
    Q_OBJECT
public:
    explicit VmCompiler(QObject *parent = 0);
    void compile(QList<QSharedPointer<Expression> > expressions);
};

#endif // COMPILER_H
