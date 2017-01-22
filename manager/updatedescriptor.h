#ifndef UPDATEDESCRIPTOR_H
#define UPDATEDESCRIPTOR_H

#include "version.h"

class UpdateDescriptor
{
public:
    UpdateDescriptor() {}
    UpdateDescriptor(QString url, QString fileName);

    static bool isUpdate(QString name);

    QString url;
    QString fileName;
    Version from;
    Version to;
};

inline bool operator==(const UpdateDescriptor& desc1, const UpdateDescriptor& desc2)
{
    return desc1.from == desc2.from && desc1.to == desc2.to;
}

inline bool operator<(const UpdateDescriptor& desc1, const UpdateDescriptor& desc2)
{
    if (desc1.from != desc2.from)
        return desc1.from < desc2.from;
    return desc1.to < desc2.to;
}

#endif // UPDATEDESCRIPTOR_H
