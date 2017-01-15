#include "dimensions.h"

QString Dimensions::toString(int width, int height)
{
    return QString::number(width) + "x" + QString::number(height);
}

Dimensions Dimensions::fromString(QString string)
{
    static const Dimensions DEFAULT_VALUE{ 1024, 768 };
    QStringList parts = string.split('x', QString::SkipEmptyParts);
    if (parts.size() != 2)
        return DEFAULT_VALUE;
    int width = parts[0].toInt();
    int height = parts[1].toInt();
    if (width <= 640 || height <= 480)
        return DEFAULT_VALUE;
    return Dimensions{ width, height };
}

void Dimensions::createDimensionsVariants(QComboBox* comboBox)
{
    comboBox->addItems(
        QStringList()
                << toString(800, 600)
                << toString(1024, 600)
                << toString(1024, 768)
                << toString(1136, 640)
                << toString(1280, 720)
                << toString(1280, 1024)
                << toString(1360, 768)
                << toString(1600, 900)
                << toString(1920, 1080));
}
