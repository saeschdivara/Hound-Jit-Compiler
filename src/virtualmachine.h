#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

class VirtualMachine : public QObject
{
    Q_OBJECT
public:
    explicit VirtualMachine(QObject *parent = 0);

Q_SIGNALS:

public Q_SLOTS:
};

#endif // VIRTUALMACHINE_H
