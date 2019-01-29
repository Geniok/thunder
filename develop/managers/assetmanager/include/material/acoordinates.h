#ifndef ACOORDINATES_H
#define ACOORDINATES_H

#include "../shaderbuilder.h"

#define UV   "UV"

class ProjectionCoord : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Coordinates")

public:
    Q_INVOKABLE ProjectionCoord() { }

    virtual AbstractSchemeModel::Node  *createNode  (ShaderBuilder *model, const QString &path) {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
        out->name   = "";
        out->out    = true;
        out->pos    = 0;
        out->type   = QMetaType::QVector2D;
        result->list.push_back(out);

        return result;
    }

    uint32_t build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        if(m_Position == -1) {
            size    = QMetaType::QVector2D;
            value  += QString("\tvec2 local%1 = (0.5 * ( _vertex.xyz / _vertex.w ) + 0.5).xy;\n").arg(depth);
        }
        return ShaderFunction::build(value, link, depth, size);
    }
};

class TexCoord : public ProjectionCoord {
    Q_OBJECT
    Q_CLASSINFO("Group", "Coordinates")

    Q_PROPERTY(uint32_t Index READ index WRITE setIndex DESIGNABLE true USER true)

public:
    Q_INVOKABLE TexCoord() :
            m_Index(0) {
    }

    uint32_t build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        if(m_Position == -1) {
            size    = QMetaType::QVector2D;
            value  += QString("\tvec2 local%1 = _uv%2;\n").arg(depth).arg(m_Index);
        }
        return ShaderFunction::build(value, link, depth, size);
    }

    uint16_t index() const {
        return m_Index;
    }

    void setIndex(uint32_t index) {
        m_Index = index;
    }
protected:
    uint32_t    m_Index;
};



class CoordPanner : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Coordinates")

    Q_PROPERTY(double X READ valueX WRITE setValueX DESIGNABLE true USER true)
    Q_PROPERTY(double Y READ valueY WRITE setValueY DESIGNABLE true USER true)

public:
    Q_INVOKABLE CoordPanner() { m_Speed = Vector2(); }

    virtual AbstractSchemeModel::Node  *createNode  (ShaderBuilder *model, const QString &path) {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = "";
            out->out    = true;
            out->pos    = 0;
            out->type   = QMetaType::QVector2D;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = UV;
            out->out    = false;
            out->pos    = 0;
            out->type   = QMetaType::QVector2D;
            result->list.push_back(out);
        }

        return result;
    }

    uint32_t build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        if(m_Position == -1) {
            const AbstractSchemeModel::Link *l = m_pModel->findLink(m_pNode, UV);
            if(l) {
                ShaderFunction *node   = static_cast<ShaderFunction *>(l->sender->ptr);
                if(node) {
                    uint8_t type;
                    uint32_t index  = node->build(value, *l, depth, type);
                    size    = type;
                    QString sub = convert("local" + QString::number(index), type, size);
                    depth++;
                    value  += QString("\tvec2 local%1 = %2").arg(depth).arg(sub);
                    value  += QString(" + vec2(%1, %2) * p.time;\n").arg(m_Speed.x).arg(m_Speed.y);
                }
            }
        }
        return ShaderFunction::build(value, link, depth, size);
    }

    double      valueX          () const {
        return m_Speed.x;
    }

    void        setValueX       (const double value) {
        m_Speed.x   = value;
        emit updated();
    }

    double      valueY          () const {
        return m_Speed.y;
    }

    void        setValueY       (const double value) {
        m_Speed.y   = value;
        emit updated();
    }

protected:
    Vector2   m_Speed;

};

#endif // ACOORDINATES_H
