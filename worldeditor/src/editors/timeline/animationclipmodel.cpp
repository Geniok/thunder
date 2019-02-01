#include "animationclipmodel.h"

#include <resources/animationclip.h>
#include <components/animationcontroller.h>
#include <components/actor.h>

#include <iterator>

#include <QColor>

AnimationClipModel::AnimationClipModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_pController(nullptr),
        m_isHighlighted(false),
        m_Position(0.0f),
        m_Row(0),
        m_Col(-1) {

}

void AnimationClipModel::setController(AnimationController *controller) {
    m_pController = controller;

    emit layoutAboutToBeChanged();
    emit layoutChanged();

    setRow(0);
}
#include <QDebug>
QVariant AnimationClipModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || m_pController == nullptr || m_pController->clip() == nullptr) {
        return QVariant();
    }

    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            AnimationClip *clip = m_pController->clip();
            auto it = clip->m_Tracks.begin();
            if(index.internalPointer() == &clip->m_Tracks) {
                advance(it, index.row());

                QStringList lst = QString::fromStdString(it->path).split('/');
                QString name = lst.last();
                int32_t size = lst.size();
                if(name.isEmpty()) {
                    name    = QString::fromStdString(m_pController->actor().name());
                    size    = 0;
                }

                QString spaces;
                for(int32_t i = 0; i < size; i++) {
                    spaces  += "    ";
                }
                return QString("%1%2 : %3").arg(spaces).arg(name).arg(it->property.c_str());
            } else {
                static QStringList list = {"x", "y", "z", "w"};
                advance(it, index.parent().row());
                int component = std::next(it->curves.begin(), index.row())->first;
                return QString("%1.%2").arg(it->property.c_str()).arg(list.at(component));
            }
        }
        case Qt::BackgroundColorRole: {
            if(m_isHighlighted && (index == m_HoverIndex)) {
                return QColor(229, 0, 0);
            }
        } break;
        default: break;
    }

    return QVariant();
}

QVariant AnimationClipModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("");
        }
    }
    return QVariant();
}

int AnimationClipModel::columnCount(const QModelIndex &) const {
    return 1;
}

QModelIndex AnimationClipModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if(m_pController) {
        AnimationClip *clip = m_pController->clip();
        AnimationClip::TrackList *list  = &clip->m_Tracks;
        if(!parent.isValid()) {
            return createIndex(row, column, list);
        } else {
            if(parent.internalPointer() == list) {
                void *ptr = &(std::next(list->begin(), parent.row())->curves);
                return createIndex(row, column, ptr);
            }
        }
    }
    return QModelIndex();
}

QModelIndex AnimationClipModel::parent(const QModelIndex &index) const {
    if(index.isValid() && m_pController) {
        AnimationClip *clip = m_pController->clip();
        AnimationClip::TrackList *list  = &clip->m_Tracks;
        if(index.internalPointer() != list) {
            int row = 0;
            for(auto &it : clip->m_Tracks) {
                if(index.internalPointer() == &(it.curves)) {
                    break;
                }
                row++;
            }
            return createIndex(row, 0, list);
        }

    }
    return QModelIndex();
}

int AnimationClipModel::rowCount(const QModelIndex &parent) const {
    if(m_pController && m_pController->clip()) {
        AnimationClip *clip = m_pController->clip();
        AnimationClip::TrackList *list = &clip->m_Tracks;
        if(!list->empty()) {
            if(!parent.isValid()) {
                return list->size();
            } else {
                if(parent.internalPointer() == list) {
                    return std::next(list->begin(), parent.row())->curves.size();
                }
            }
        }
    }
    return 0;
}

QVariant AnimationClipModel::trackData(int track) const {
    if(m_pController && m_pController->clip() && track >= 0) {
        AnimationClip *clip = m_pController->clip();
        if(!clip->m_Tracks.empty()) {
            auto &curves = (*std::next(clip->m_Tracks.begin(), track)).curves;

            QVariantList track;
            for(auto &it : curves) {
                QVariantList curve;
                curve.push_back(it.first);
                for(auto &k : it.second.m_Keys) {
                    curve.push_back(QVariantList({ k.m_Position, k.m_Type, k.m_Value, k.m_LeftTangent, k.m_RightTangent }));
                }
                track.push_back(curve);
            }
            return track;
        }
    }
    return QVariant();
}

void AnimationClipModel::setTrackData(int track, const QVariant &data) {
    if(m_pController && m_pController->clip() && track >= 0 && data.isValid()) {
        AnimationClip *clip = m_pController->clip();
        if(!clip->m_Tracks.empty()) {
            auto &curves = (*std::next(clip->m_Tracks.begin(), track)).curves;

            curves.clear();

            foreach(QVariant t, data.toList()) {
                QVariantList track = t.toList();

                int32_t component = track[0].toInt();
                AnimationCurve curve;

                for(int32_t i = 1; i < track.size(); i++) {
                    QVariantList k = track[i].toList();

                    AnimationCurve::KeyFrame key;
                    key.m_Position = k[0].toUInt();
                    key.m_Type = AnimationCurve::KeyFrame::Type(k[1].toUInt());
                    key.m_Value = k[2].toFloat();
                    key.m_LeftTangent = k[3].toFloat();
                    key.m_RightTangent = k[4].toFloat();

                    curve.m_Keys.push_back(key);
                }
                curves[component] = curve;
            }

            updateController();
        }
    }
}

int AnimationClipModel::maxPosition(int track) {
    if(m_pController && m_pController->clip() && track >= 0) {
        AnimationClip *clip = m_pController->clip();
        if(!clip->m_Tracks.empty()) {
            auto &curves = (*std::next(clip->m_Tracks.begin(), track)).curves;

            int32_t result = 0;
            for(auto &it : curves) {
                if(!it.second.m_Keys.empty())
                result = MAX(it.second.m_Keys.back().m_Position, result);
            }
            return result;
        }
    }
    return 0;
}

float AnimationClipModel::position() const {
    return m_Position;
}

void AnimationClipModel::setPosition(float value) {
    m_Position = value;

    if(m_pController) {
        m_pController->setPosition(1000 * m_Position);
    }

    emit positionChanged();
}

int AnimationClipModel::row() const {
    return m_Row;
}

void AnimationClipModel::setRow(int value) {
    m_Row = value;
    emit rowChanged();
}

int AnimationClipModel::col() const {
    return m_Col;
}

void AnimationClipModel::setCol(int value) {
    m_Col = value;
}

void AnimationClipModel::selectItem(const QModelIndex &index) {
    if(index.parent().isValid()) { // Sub component
        QModelIndex p = parent(index);
        if(p.isValid()) {
            setCol(index.row());
            setRow(p.row());
        }
    } else {
        setCol(-1);
        setRow(index.row());
    }
}

AnimationCurve::KeyFrame *AnimationClipModel::key(int row, int col, int index) {
    if(row >= 0) {
        AnimationClip *clip = m_pController->clip();
        if(!clip->m_Tracks.empty()) {
            auto &curves = (*std::next(clip->m_Tracks.begin(), row)).curves;

            return &curves[col].m_Keys[index];
        }
    }
    return nullptr;
}

void AnimationClipModel::onAddKey(int row, int col, int pos) {
    if(row >= 0) {
        AnimationClip *clip = m_pController->clip();
        if(!clip->m_Tracks.empty()) {
            auto &curves = (*std::next(clip->m_Tracks.begin(), row)).curves;

            AnimationCurve::KeyFrame key;
            key.m_Position = uint32_t(pos);

            auto &curve = curves[col];
            key.m_Value  = curve.value(key.m_Position);
            key.m_LeftTangent  = key.m_Value;
            key.m_RightTangent  = key.m_Value;

            curve.m_Keys.push_back(key);
            std::sort(curve.m_Keys.begin(), curve.m_Keys.end(), AnimationClip::compare);

            updateController();
        }
    }
}

void AnimationClipModel::onRemoveKey(int row, int col, int index) {
    if(row >= 0 && index >= 0) {
        AnimationClip *clip = m_pController->clip();
        if(!clip->m_Tracks.empty()) {
            auto &curves = (*std::next(clip->m_Tracks.begin(), row)).curves;
            auto &curve = curves[col];

            curve.m_Keys.erase(std::next(curve.m_Keys.begin(), index));

            updateController();
        }
    }
}

void AnimationClipModel::updateController() {
    m_pController->setClip(m_pController->clip());
    m_pController->setPosition(1000 * m_Position);

    emit changed();

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}
