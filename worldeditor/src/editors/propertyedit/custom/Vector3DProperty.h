#ifndef VECTOR3DPROPERTY_H
#define VECTOR3DPROPERTY_H

#include "Property.h"

#include <amath.h>
Q_DECLARE_METATYPE(Vector3)

class Vector3DProperty : public Property {
    Q_OBJECT

public:
    Vector3DProperty                    (const QString& name = QString(), QObject* propertyObject = 0, QObject* parent = 0);

    QVariant            value           (int role = Qt::UserRole) const;

    QWidget            *createEditor    (QWidget *parent, const QStyleOptionViewItem &);
    bool                setEditorData   (QWidget *editor, const QVariant &data);
    QVariant            editorData      (QWidget *editor);

    QSize               sizeHint        (const QSize& size) const;

protected slots:
    void                onDataChanged   (const QVariant &data);

};

#endif // VECTOR3DPROPERTY_H
