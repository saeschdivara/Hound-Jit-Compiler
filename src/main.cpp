#include <QCoreApplication>

#include "virtualmachine.h"
#include "parser.h"
#include "compiler.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    VirtualMachine machine;

    Parser parser(":/examples/ex_01.hound");
    QList< QSharedPointer<Expression> > expressions = parser.parse();

    VmCompiler comp;

    return 0;
}
