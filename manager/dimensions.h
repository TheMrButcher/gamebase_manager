#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#include <QComboBox>

struct Dimensions
{
    static QString toString(int width, int height);
    static Dimensions fromString(QString string);
    static void createDimensionsVariants(QComboBox* comboBox);

    int width;
    int height;
};

#endif // DIMENSIONS_H
