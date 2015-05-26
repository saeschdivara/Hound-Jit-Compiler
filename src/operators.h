#ifndef OPERATORS
#define OPERATORS

enum LanguageOperator {
    UnknownOperator = 0,

    // Array / list / string
    InOperator,

    // Logic
    NotOperator,
    AndOperator,
    OrOperator,
    XorOperator,

    // Compare
    GreaterOperator,
    LessOperator,

    // Math
    PlusOperator,
    MinusOperator,
    MultiplyOperator,
    DivideOperator,
    PowerOfOperator,
};

#endif // OPERATORS

