#include "expression.h"

QString getDataTypeName(DataType type) {
    QString name;

    switch (type)
    {
    case DataType::NoDataType:
        name = "Unknown";
        break;
    case DataType::StringType:
        name = "String";
        break;
    case DataType::Int32:
        name = "Int32";
        break;
    case DataType::Float:
        name = "Float";
        break;
    default:
        break;
    }

    return name;
}
