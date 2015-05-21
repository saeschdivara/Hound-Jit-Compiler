#ifndef EXPRESSION
#define EXPRESSION

#include <QtCore/QChar>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QTextStream>
#include <QtCore/QSharedPointer>

/////////////////////////////////////////////////////

enum ExpressionType {
    UnknownExpression,

    // Comment types
    Comment,

    // Information
    Variable,

    Package,
    Import,

    FunctionInvokation,

    RawData,

    // Blocks
    FunctionExpressionType,
    CodeBlock,

    //
    If,
    ElIf,
    Else,

};

enum DataType {
    NoDataType,

    // Integers
    Int8,
    Int16,
    Int32,
    Int64,

    // Floats

    // Extra types
    StringType,
};

struct ParsingData {
    // Char data
    QChar lastChar;
    QChar lastUnknownChar;

    QString identifier;

    // Reserved keywords
    QHash<QString, uint> operators;
    QHash<QString, ExpressionType> keywords;

    // Indetention
    uint indentStep : 4;
    uint previousIndent;
    uint currentIndent;

    // Stream
    QTextStream * stream;
};


QString getDataTypeName(DataType type);



class Expression
{
public:
    virtual ~Expression() {}

    virtual ExpressionType type() const { return ExpressionType::UnknownExpression; }
    virtual QString toString() const { return "Unknown"; }

    bool is(ExpressionType checkType) { return type() == checkType; }
    bool isUnknown() { return is(ExpressionType::UnknownExpression); }
    bool isComment() { return is(ExpressionType::Comment); }
    bool isPackage() { return is(ExpressionType::Package); }
    bool isImport()  { return is(ExpressionType::Import); }
    bool isFunction()  { return is(ExpressionType::FunctionExpressionType); }
    bool isFunctionInvokation()  { return is(ExpressionType::FunctionInvokation); }
    bool isRawValue()  { return is(ExpressionType::RawData); }
    bool isCodeBlock()  { return is(ExpressionType::CodeBlock); }
};


class CommentExpression : public Expression
{
public:
    virtual ~CommentExpression() {}

    virtual ExpressionType type() const { return ExpressionType::Comment; }
    virtual QString toString() const { return "Comment"; }
};


class VariableExpression : public Expression
{
    QString m_name;
    DataType m_type;
public:
    // Name
    QString name() const { return m_name; }
    void setName(const QString & name) { m_name = name; }

    // Type
    void setDataType(DataType type) { m_type = type; }
    DataType dataType() const { return m_type; }

    virtual ~VariableExpression() {}

    virtual ExpressionType type() const { return ExpressionType::Variable; }
    virtual QString toString() const {
        return QString("Variable ") + name() + QString(" : ") + getDataTypeName( dataType() );
    }
};


class PackageExpression : public Expression
{
    QString m_path;
public:
    // Path
    QString path() const { return m_path; }
    void setPath(const QString & path) { m_path = path; }

    virtual ~PackageExpression() {}

    virtual ExpressionType type() const { return ExpressionType::Package; }
    virtual QString toString() const { return "Package: " + path(); }
};


class ImportExpression : public Expression
{
    QString m_path;
public:
    // Path
    QString path() const { return m_path; }
    void setPath(const QString & path) { m_path = path; }

    virtual ~ImportExpression() {}

    virtual ExpressionType type() const { return ExpressionType::Import; }
    virtual QString toString() const { return "Import: " + path(); }
};


class FunctionExpression : public Expression
{
    QString m_name;
    bool m_isAnonymous = false;
    QSharedPointer<Expression> m_codeBlock;
    QList< QSharedPointer<Expression> > m_parameters;
public:
    // Name
    QString name() const { return m_name; }
    void setName(const QString & name) { m_name = name; }

    // Parameters
    void addParameter(QSharedPointer<Expression> expr) {
        m_parameters.append(expr);
    }

    QList< QSharedPointer<Expression> > parameters() const { return m_parameters; }

    // Is anonymous
    bool isAnonymous() const { return m_isAnonymous; }
    void setAnonymous(bool ano) { m_isAnonymous = ano; }

    // Code
    QSharedPointer<Expression> code() const { return m_codeBlock; }
    void setCode(QSharedPointer<Expression> code) { m_codeBlock = code; }

    virtual ~FunctionExpression() {}

    virtual ExpressionType type() const { return ExpressionType::FunctionExpressionType; }
    virtual QString toString() const {
        if ( isAnonymous() )
            return "Anonymous Function";
        else {
            if ( m_parameters.size() > 0 ) {
                QString parameters;
                for( QSharedPointer<Expression> param : m_parameters ) {
                    parameters +=  param->toString() + ", ";
                }

                return "Function: " + name() + " ( " + parameters + " )";
            }
            else {
                return "Function: " + name();
            }
        }
    }
};

class FunctionInvokationExpression : public Expression
{
    QString m_functionName;
    QList< QSharedPointer<Expression> > m_parameters;
public:
    // Name
    QString functionName() const { return m_functionName; }
    void setFunctionName(const QString & name) { m_functionName = name; }

    void addParameter(QSharedPointer<Expression> expr) {
        m_parameters.append(expr);
    }

    QList< QSharedPointer<Expression> > parameters() const { return m_parameters; }

    virtual ~FunctionInvokationExpression() {}

    virtual ExpressionType type() const { return ExpressionType::FunctionInvokation; }
    virtual QString toString() const { return "Function invokation"; }
};


class CodeBlockExpression : public Expression
{
    QList< QSharedPointer<Expression> > m_expressions;
public:

    void addExpression(QSharedPointer<Expression> expr) {
        m_expressions.append(expr);
    }

    QList< QSharedPointer<Expression> > expressions() const {
        return m_expressions;
    }

    virtual ~CodeBlockExpression() {}

    virtual ExpressionType type() const { return ExpressionType::CodeBlock; }
    virtual QString toString() const { return "Codeblock"; }
};


class RawDataExpression : public Expression
{
    DataType m_type;
    QVariant m_variant;
public:
    // Data type
    DataType dataType() const { return m_type; }
    void setDataType(DataType type) { m_type = type; }

    bool hasStringType() const { return dataType() == DataType::StringType; }

    // Data
    QVariant data() const { return m_variant; }
    void setData(QVariant data) { m_variant = data; }

    virtual ~RawDataExpression() {}

    virtual ExpressionType type() const { return ExpressionType::RawData; }
    virtual QString toString() const {
        return "Raw Data: " + data().toString() + QString(" (") + getDataTypeName( dataType() ) + QString(")");
    }
};


#endif // EXPRESSION
