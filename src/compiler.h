#ifndef COMPILER_H
#define COMPILER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

class VmCompiler : public QObject
{
    Q_OBJECT
public:
    explicit VmCompiler(QObject *parent = 0);
    void compile();

Q_SIGNALS:

public Q_SLOTS:
};

#endif // COMPILER_H
