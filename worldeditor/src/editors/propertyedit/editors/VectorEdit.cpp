#include "VectorEdit.h"
#include "ui_VectorEdit.h"

#include <QDoubleValidator>
#include <QLocale>

#include <float.h>

Q_DECLARE_METATYPE(Vector3)

VectorEdit::VectorEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::VectorEdit) {

    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);

    ui->x->setValidator(validator);
    ui->y->setValidator(validator);
    ui->z->setValidator(validator);

    connect(ui->x, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
    connect(ui->y, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
    connect(ui->z, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
}

VectorEdit::~VectorEdit() {
    delete ui;
}

Vector3 VectorEdit::data() const {
    return Vector3(ui->x->text().toFloat(),
                   ui->y->text().toFloat(),
                   ui->z->text().toFloat());
}

void VectorEdit::setData(const Vector3 &v) {
    ui->x->setText(QString::number(v.x, 'f', 3));
    ui->y->setText(QString::number(v.y, 'f', 3));
    ui->z->setText(QString::number(v.z, 'f', 3));
}

void VectorEdit::onValueChanged() {
    emit dataChanged(QVariant::fromValue(data()));
}
