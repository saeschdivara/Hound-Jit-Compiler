#ifndef PARSER_H
#define PARSER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include "expression.h"

///////////////////////////////////////////


class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(const QString fileName, QObject *parent = 0);
    QList< QSharedPointer<Expression> > parse();

Q_SIGNALS:

public Q_SLOTS:

private:
    QString m_fileName;
};


QSharedPointer<Expression> parseFunctionExpr(QTextStream & stream, ParsingData * data);
QSharedPointer<Expression> parseFunctionInvokationExpr(QTextStream & stream, ParsingData * data);
QSharedPointer<Expression> parseBlockExpr(QTextStream & stream, ParsingData * data);
QSharedPointer<Expression> parseCodeBlockExpr(QTextStream & stream, ParsingData * data);

#endif // PARSER_H
