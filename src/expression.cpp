#include "expression.h"

QString getDataTypeName(DataType type) {
    QString name;

    switch (type) {
        case DataType::NoDataType:
            name = "Unknown";
        break;
    case DataType::StringType:
        name = "String";
    break;
    default:
        break;
    }

    return name;
}
