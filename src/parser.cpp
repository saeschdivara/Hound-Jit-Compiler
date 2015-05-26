#include "parser.h"
#include "operators.h"


Parser::Parser(const QString fileName, QObject *parent) : QObject(parent),
    m_fileName(fileName)
{
}

bool isReturnCharacter(QChar c) {
    return c == '\r' || c == '\n';
}

bool isVariableConform(QChar c, const QString & identifier) {
    if ( identifier.isEmpty() )
        return c.isLetter() || c == '_';
    else
        return c.isLetterOrNumber() || c == '_';
}

bool isNumberConform(QChar c, const QString & identifier) {
    if ( identifier.isEmpty() )
        return c.isNumber() || c == '.';
    else
        return c.isNumber() || ( !identifier.contains(".") && c == '.' );
}

bool consumeSpace(QTextStream & stream, ParsingData * data, bool updateIndetention = false) {

    bool hasSpace = data->lastChar.isSpace();

    if ( hasSpace && updateIndetention ) {
        data->previousIndent = data->currentIndent;
        data->currentIndent = 0;
    }

    while ( data->lastChar.isSpace() ) {

        if ( updateIndetention ) {
            data->currentIndent++;
        }

        stream >> data->lastChar;
    }

    return hasSpace;
}

bool hasNewBlock(ParsingData * data) {
    return data->currentIndent > data->previousIndent;
}

void consumeIndetention(QTextStream & stream, ParsingData * data) {

    while ( isReturnCharacter(data->lastChar) ) {
        consumeSpace(stream, data, true);
    }
}

bool isInValidExpr(QSharedPointer<Expression> expr) {
    return expr.isNull() || expr->isUnknown();
}

QSharedPointer<Expression> getEmptyExpr() {
    return QSharedPointer<Expression>::create();
}

QSharedPointer<Expression> parseCommentExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<Expression> expr = QSharedPointer<CommentExpression>::create();

    while ( !isReturnCharacter(data->lastChar) && !stream.atEnd() ) {
        stream >> data->lastChar;
    }

    return expr;
}

QSharedPointer<Expression> parsePackageExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<Expression> expr = QSharedPointer<PackageExpression>::create();

    if ( !consumeSpace(stream, data) ) {
        qDebug() << "No space between package and path";
    }

    data->identifier.clear();

    do {
        data->identifier += data->lastChar;
        stream >> data->lastChar;
    }
    while ( data->lastChar.isLetterOrNumber() || data->lastChar == '.' );

    expr.dynamicCast<PackageExpression>()->setPath(data->identifier);

    return expr;
}

QSharedPointer<Expression> parseImportExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<Expression> expr = QSharedPointer<ImportExpression>::create();

    if ( !consumeSpace(stream, data) ) {
        qDebug() << "No space between import and path";
    }

    data->identifier.clear();

    do {
        data->identifier += data->lastChar;
        stream >> data->lastChar;
    }
    while ( data->lastChar.isLetterOrNumber() || data->lastChar == '.' );

    expr.dynamicCast<ImportExpression>()->setPath(data->identifier);

    return expr;
}

QSharedPointer<Expression> parseVariableExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<VariableExpression> expr = QSharedPointer<VariableExpression>::create();

    data->identifier.clear();

    // Read identifier
    while ( isVariableConform( data->lastChar, data->identifier ) ) {
        data->identifier += data->lastChar;
        stream >> data->lastChar;
    }

    if ( data->identifier.isEmpty() )
        return QSharedPointer<Expression>::create();

    expr->setName(data->identifier);

    return expr;
}

QSharedPointer<Expression> parseStringExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<RawDataExpression> expr = QSharedPointer<RawDataExpression>::create();

    QString stringData;

    stream >> data->lastChar;

    while ( data->lastChar != '"' ) {
        stringData += data->lastChar;
        stream >> data->lastChar;
    }

    // Consume next after "
    stream >> data->lastChar;

    expr->setDataType(DataType::StringType);
    expr->setData(stringData);

    return expr;
}

QSharedPointer<Expression> parseNumberExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<RawDataExpression> expr = QSharedPointer<RawDataExpression>::create();

    data->identifier.clear();

    data->identifier += data->lastChar;
    stream >> data->lastChar;

    while ( isNumberConform(data->lastChar, data->identifier) ) {
        data->identifier += data->lastChar;
        stream >> data->lastChar;
    }

    if ( data->identifier.contains(".") ) {
        expr->setDataType(DataType::Float);
        expr->setData(data->identifier.toFloat());
    }
    else {
        expr->setDataType(DataType::Int32);
        expr->setData(data->identifier.toInt());
    }

    return expr;
}

QSharedPointer<Expression> parseParameterExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<Expression> expr = QSharedPointer<Expression>::create();

    // Comment
    if ( data->lastChar == '#' ) {
        expr = parseCommentExpr(stream, data);
    }

    // Identifier String
    else if ( data->lastChar.isLetter() ) {

        data->identifier.clear();

        // Read identifier
        while ( isVariableConform( data->lastChar, data->identifier ) ) {
            data->identifier += data->lastChar;
            stream >> data->lastChar;
        }

        // Check if identifier is a keyword
        if (data->keywords.contains(data->identifier)) {
            ExpressionType type = data->keywords.value(data->identifier);
            switch (type)
            {
            case ExpressionType::Package:
                qDebug() << "Package declaration must be a top level expression";
                break;

            case ExpressionType::Import:
                qDebug() << "Import can only be a top level expression";
                break;

            case ExpressionType::FunctionExpressionType:
                expr = parseFunctionExpr(stream, data);
                break;

                default:
                break;
            }
        }
        else if ( data->lastChar == '(' ) {
            expr = parseFunctionInvokationExpr(stream, data);
        }
        else {
            expr = QSharedPointer<VariableExpression>::create();
            expr.dynamicCast<VariableExpression>()->setName(data->identifier);
        }
    }

    // String expression
    else if ( data->lastChar == '"' ) {
        expr = parseStringExpr(stream, data);
    }

    // Numbers
    else if ( isNumberConform(data->lastChar, data->identifier) ) {
        expr = parseNumberExpr(stream, data);
    }

    return expr;
}


QSharedPointer<Expression> parseFunctionInvokationExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<FunctionInvokationExpression> expr = QSharedPointer<FunctionInvokationExpression>::create();

    QString functionName = data->identifier;

    qDebug() << "Calling function: " << functionName;

    expr->setFunctionName(functionName);

    stream >> data->lastChar;

    consumeIndetention(stream, data);

    QSharedPointer<Expression> variableExpr = parseParameterExpr(stream, data);
    while ( !variableExpr.isNull() && !variableExpr->isUnknown() ) {
        expr->addParameter(variableExpr);
        qDebug() << "Parameter: " << variableExpr->toString();
        variableExpr = parseParameterExpr(stream, data);
        consumeSpace(stream, data);
    }

    if ( data->lastChar != ')' && data->lastUnknownChar != ')' ) {
        qDebug() << "Call didn't end with )";
        return getEmptyExpr();
    }

    // Consume char after )
    stream >> data->lastChar;

    return expr;
}

bool isPossibleOperator(ParsingData * data) {

    if ( data->identifier.isEmpty() ) {
        //
        for( const QString & key : data->operators.keys() ) {
            if ( key.startsWith(data->lastChar) ) {
                return true;
            }
        }
    }
    else {
        QString searchString = data->identifier + data->lastChar;
        //
        for( const QString & key : data->operators.keys() ) {
            if ( key.startsWith(searchString) ) {
                return true;
            }
        }
    }


    return false;
}


LanguageOperator parseOperator(QTextStream & stream, ParsingData * data) {
    LanguageOperator myOperator;
    data->identifier.clear();

    // stream >> data->lastChar;

    while ( isPossibleOperator(data) ) {
        data->identifier += data->lastChar;
        stream >> data->lastChar;
    }

    myOperator = data->operators.value(data->identifier, LanguageOperator::UnknownOperator);

    qDebug() << "Operator: " << myOperator;

    return myOperator;
}

QSharedPointer<Expression> parsePossibleComparsionExpr(QTextStream & stream, ParsingData * data) {
    // - Variables
    // - Integers
    // - Floats
    // - Strings

    data->identifier.clear();

    if ( isVariableConform( data->lastChar, data->identifier ) ) {
        return parseVariableExpr(stream, data);
    }
    else if ( data->lastChar == '"' ) {
        return parseStringExpr(stream, data);
    }
    else if ( isNumberConform(data->lastChar, data->identifier)  ) {
        return parseNumberExpr(stream, data);
    }

    return getEmptyExpr();
}


QSharedPointer<BinaryExpression> parseBinaryExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<BinaryExpression> expr = QSharedPointer<BinaryExpression>::create();

    data->identifier.clear();
    //
    stream >> data->lastChar;

    QSharedPointer<Expression> left = parsePossibleComparsionExpr(stream, data);

    if ( isInValidExpr(left) ) {
        qDebug() << "Invalid left expression";
        return getEmptyExpr().dynamicCast<BinaryExpression>();
    }

    qDebug() << "left expression: " << left->toString();

    consumeSpace(stream, data);

    LanguageOperator myOperator = parseOperator(stream, data);

    if ( myOperator == LanguageOperator::UnknownOperator ) {
        qDebug() << "This operator '" << data->identifier << "' is unknown";
        return getEmptyExpr().dynamicCast<BinaryExpression>();
    }

    consumeSpace(stream, data);

    QSharedPointer<Expression> right = parsePossibleComparsionExpr(stream, data);

    if ( isInValidExpr(right) ) {
        qDebug() << "Invalid right expression";
        return getEmptyExpr().dynamicCast<BinaryExpression>();
    }

    qDebug() << "right expression: " << right->toString();

    expr->setLeftExpression(left);
    expr->setOperator(myOperator);
    expr->setRightExpression(right);

    return expr;
}

QSharedPointer<Expression> parseIfExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<IfExpression> expr = QSharedPointer<IfExpression>::create();

    QSharedPointer<BinaryExpression> condition = parseBinaryExpr(stream, data);
    if ( condition.isNull() || condition->isUnknown() ) {
        qDebug() << "Could not parse binary condition expression";
        return getEmptyExpr();
    }

    consumeSpace(stream, data);

    data->identifier.clear();

    // Read identifier
    while ( isVariableConform( data->lastChar, data->identifier ) ) {
        data->identifier += data->lastChar;
        stream >> data->lastChar;
    }

    if ( data->identifier.toLower() != "then" ) {
        qDebug() << "Expected then after binary expression";
        return getEmptyExpr();
    }

    consumeIndetention(stream, data);

    if ( !hasNewBlock(data) ) {
        qDebug() << "After then comes a new block!";
        return getEmptyExpr();
    }

    QSharedPointer<Expression> block = parseCodeBlockExpr(stream, data);

    if ( isInValidExpr(block) ) {
        qDebug() << "Invalid block parsed";
        return getEmptyExpr();
    }

    qDebug() << "Block: " << block->toString();

    consumeIndetention(stream, data);

    return expr;
}

QSharedPointer<Expression> parseBlockExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<Expression> expr = getEmptyExpr();

    if ( data->lastChar == '#' ) {
        expr = parseCommentExpr(stream, data);
    }

    // Identifier String
    else if ( data->lastChar.isLetter() ) {

        data->identifier.clear();

        // Read identifier
        while ( isVariableConform( data->lastChar, data->identifier ) ) {
            data->identifier += data->lastChar;
            stream >> data->lastChar;
        }

        // Check if identifier is a keyword
        if (data->keywords.contains(data->identifier)) {
            ExpressionType type = data->keywords.value(data->identifier);
            switch (type)
            {
            case ExpressionType::Package:
                qDebug() << "Package declaration must be a top level expression";
                break;

            case ExpressionType::Import:
                qDebug() << "Import can only be a top level expression";
                break;

            case ExpressionType::FunctionExpressionType:
                expr = parseFunctionExpr(stream, data);
                break;

            case ExpressionType::If:
                expr = parseIfExpr(stream, data);
                break;

            default:
                break;
            }
        }
        else if ( data->lastChar == '(' ) {
            expr = parseFunctionInvokationExpr(stream, data);
        }
        else {
            expr = QSharedPointer<VariableExpression>::create();
            expr.dynamicCast<VariableExpression>()->setName(data->identifier);
        }
    }
    else if ( isNumberConform(data->lastChar, data->identifier) ) {
        expr = parseNumberExpr(stream, data);
    }

    return expr;
}

QSharedPointer<Expression> parseCodeBlockExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<CodeBlockExpression> expr = QSharedPointer<CodeBlockExpression>::create();

    QSharedPointer<Expression> codeExpr = parseBlockExpr(stream, data);

    if ( isInValidExpr(codeExpr) ) {
        qDebug() << "Invalid code block";
        return getEmptyExpr();
    }

    while ( !isInValidExpr(codeExpr) ) {
        expr->addExpression(codeExpr);
        qDebug() << "Block: " << codeExpr->toString();
        codeExpr = parseBlockExpr(stream, data);
    }

    return expr;
}

QSharedPointer<Expression> parseFunctionExpr(QTextStream & stream, ParsingData * data) {
    QSharedPointer<FunctionExpression> expr = QSharedPointer<FunctionExpression>::create();

    if ( !consumeSpace(stream, data) ) {
        qDebug() << "No space between keyword and rest";
    }

    data->identifier.clear();

    while ( isVariableConform( data->lastChar, data->identifier ) ) {
        data->identifier += data->lastChar;
        stream >> data->lastChar;
    }

    if ( data->identifier.isEmpty() ) {
        expr->setAnonymous(true);
    }
    else {
        expr->setName(data->identifier);
    }

    // Take space away if there is
    consumeSpace(stream, data);

    if ( data->lastChar != '(' && data->lastUnknownChar != '(' ) {
        qDebug() << "( is missing";
        return QSharedPointer<Expression>::create();
    }

    stream >> data->lastChar;
    consumeSpace(stream, data);

    // Parameter parsing
    // TODO:

    Q_FOREVER {
        // Empty string for new parameter
        data->identifier.clear();

        while ( isVariableConform( data->lastChar, data->identifier ) ) {
            data->identifier += data->lastChar;
            stream >> data->lastChar;
        }

        if ( data->identifier.size() > 0 ) {
            QSharedPointer<VariableExpression> param = QSharedPointer<VariableExpression>::create();
            param->setName(data->identifier);
            expr->addParameter(param);
        }

        if ( stream.atEnd() || data->lastChar == ')' ) {
            break;
        }
    }

    if ( data->lastChar != ')' && data->lastUnknownChar != ')' ) {
        qDebug() << ") is missing";
        return QSharedPointer<Expression>::create();
    }

    stream >> data->lastChar;
    consumeSpace(stream, data);

    if ( data->lastChar != '-' ) {
        qDebug() << "-> is missing";
        return QSharedPointer<Expression>::create();
    }

    stream >> data->lastChar;
    consumeSpace(stream, data);

    if ( data->lastChar != '>' ){
        qDebug() << "-> is missing";
        return getEmptyExpr();
    }

    // Check for new function block
    stream >> data->lastChar;
    consumeIndetention(stream, data);

    if ( !hasNewBlock(data) ) {
        qDebug() << "Function expects a function body with enough indetention";
        return getEmptyExpr();
    }

    QSharedPointer<Expression> codeBlock = parseCodeBlockExpr(stream, data);
    if ( codeBlock.isNull() ) {
        qDebug() << "Could not parse function block";
        return getEmptyExpr();
    }

    expr->setCode(codeBlock);

    return expr;
}

QSharedPointer<Expression> parseTopLevelExpr(QTextStream & stream, ParsingData * data) {

    QSharedPointer<Expression> expr;

    consumeIndetention(stream, data);

    // Check if it is a 1 line comment
    if ( data->lastChar == '#' ) {
        expr = parseCommentExpr(stream, data);
    }

    // Identifier String
    else if ( data->lastChar.isLetter() ) {

        data->identifier.clear();

        // Read identifier
        while ( isVariableConform( data->lastChar, data->identifier ) ) {
            data->identifier += data->lastChar;
            stream >> data->lastChar;
        }

        // Check if identifier is a keyword
        if (data->keywords.contains(data->identifier)) {
            ExpressionType type = data->keywords.value(data->identifier);
            switch (type)
            {
            case ExpressionType::Package:
                expr = parsePackageExpr(stream, data);
                break;

            case ExpressionType::Import:
                expr = parseImportExpr(stream, data);
                break;

            case ExpressionType::FunctionExpressionType:
                expr = parseFunctionExpr(stream, data);
                break;

            case ExpressionType::If:
                expr = parseIfExpr(stream, data);
                break;

            default:
                break;
            }
        }
        else if ( data->lastChar == '(' ) {
            expr = parseFunctionInvokationExpr(stream, data);
        }
        else {
            expr = QSharedPointer<VariableExpression>::create();
            expr.dynamicCast<VariableExpression>()->setName(data->identifier);
        }
    }

    // If the expression is null
    if ( expr.isNull() ) {
        expr = QSharedPointer<Expression>::create();

        qDebug() << data->lastChar;

        data->lastUnknownChar = data->lastChar;
        stream >> data->lastChar;
    }

    return expr;
}

QList< QSharedPointer<Expression> > Parser::parse()
{
    QFile file(m_fileName);

    if (!file.open(QIODevice::ReadOnly))
        return QList< QSharedPointer<Expression> >();

    QTextStream stream(&file);
    QList< QSharedPointer<Expression> > expressions;

    ParsingData data;

    // Init data
    data.currentIndent = 0;

    // Set operators
    data.operators["in"] = LanguageOperator::InOperator;
    data.operators["not"] = LanguageOperator::NotOperator;
    data.operators["and"] = LanguageOperator::AndOperator;
    data.operators["or"] = LanguageOperator::OrOperator;
    data.operators["xor"] = LanguageOperator::XorOperator;
    data.operators["<"] = LanguageOperator::LessOperator;
    data.operators[">"] = LanguageOperator::GreaterOperator;
    data.operators["+"] = LanguageOperator::PlusOperator;
    data.operators["-"] = LanguageOperator::MinusOperator;
    data.operators["*"] = LanguageOperator::MultiplyOperator;
    data.operators["/"] = LanguageOperator::DivideOperator;
    data.operators["**"] = LanguageOperator::PowerOfOperator;

    // Set keywords
    data.keywords["package"] = ExpressionType::Package;
    data.keywords["import"] = ExpressionType::Import;
    data.keywords["fn"] = ExpressionType::FunctionExpressionType;
    data.keywords["if"] = ExpressionType::If;
    data.keywords["elif"] = ExpressionType::ElIf;
    data.keywords["else"] = ExpressionType::Else;

    // Start reading
    stream >> data.lastChar;

    while (!stream.atEnd()) {
        QSharedPointer<Expression> fileExpr = parseTopLevelExpr(stream, &data);
        qDebug() << "Expression: " << fileExpr->toString();

        expressions.append(fileExpr);
    }

    qDebug() << "";

    file.close();

    return expressions;
}

