#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

/// LANGUAGE CONECEPTS
///
/// Features:
/// - Namespacing (more like Java than C++)
/// - kwargs at compile time
/// - import (like in java (only once) and like python from .. import ... / import ...)
/// - classes
/// - templating (eventuell)
/// - Easy way to print out
/// - Formating string like in Python ("%s" % var1)
/// - Enum Types
/// - in operator for arrays and strings
/// - utf8 or unicode standard string
/// - function and method overloading
/// - indetention like python
/// - can be functional and oop
/// - Comment: #
/// - pass = do nothing (like Python)
///

class VirtualMachine : public QObject
{
    Q_OBJECT
public:
    explicit VirtualMachine(QObject *parent = 0);

Q_SIGNALS:

public Q_SLOTS:
};

#endif // VIRTUALMACHINE_H
