#ifndef HIERARCHYBROWSER_H
#define HIERARCHYBROWSER_H

#include <QWidget>
#include <QTreeView>

#include <aobject.h>

namespace Ui {
    class HierarchyBrowser;
}

const QString gMimeObject("text/object");

class TreeView : public QTreeView {
    Q_OBJECT

public:
    TreeView                (QWidget *parent) :
            QTreeView(parent) {
    }

signals:
    void                    dragStarted                 (Qt::DropActions supportedActions);

    void                    dragEnter                   (QDragEnterEvent *e);

    void                    dragLeave                   (QDragLeaveEvent *e);

    void                    dragMove                    (QDragMoveEvent *e);

    void                    drop                        (QDropEvent *e);

protected:
    void                    startDrag                   (Qt::DropActions supportedActions) {
        emit dragStarted(supportedActions);
    }

    void                    dragEnterEvent              (QDragEnterEvent *e) {
        emit dragEnter(e);
    }

    void                    dragLeaveEvent              (QDragLeaveEvent *e) {
        emit dragLeave(e);
    }

    void                    dragMoveEvent               (QDragMoveEvent *e) {
        emit dragMove(e);
    }

    void                    dropEvent                   (QDropEvent *e) {
        emit drop(e);
    }
};

class ObjectsFilter;

class HierarchyBrowser : public QWidget {
    Q_OBJECT

public:
    HierarchyBrowser        (QWidget *parent = 0);

    ~HierarchyBrowser       ();

    void                    setObject                   (AObject *object);

signals:
    void                    selected                    (AObject::ObjectList &objects);

    void                    removed                     (AObject::ObjectList &objects);

    void                    focused                     (AObject *object);

    void                    parented                    (AObject::ObjectList &objects, AObject::ObjectList &parents);

    void                    updated                     ();

public slots:
    void                    onSelected                  (AObject::ObjectList &objects);

    void                    onHierarchyUpdated          ();

    void                    onDragEnter                 (QDragEnterEvent *e);

    void                    onDragLeave                 (QDragLeaveEvent *e);

    void                    onDragMove                  (QDragMoveEvent *e);

    void                    onDrop                      (QDropEvent *e);

private slots:
    void                    onDragStarted               (Qt::DropActions supportedActions);

    void                    on_treeView_clicked         (const QModelIndex &index);

    void                    on_treeView_doubleClicked   (const QModelIndex &index);

    void                    on_lineEdit_textChanged     (const QString &arg1);

    void                    onItemDuplicate             ();
    void                    onItemRename                ();
    void                    onItemDelete                ();

private:
    void                    createAction                (const QString &name, const char *member, const QKeySequence &shortcut = 0);

private:
    Ui::HierarchyBrowser   *ui;

    QRubberBand            *m_pRect;

    ObjectsFilter          *m_pFilter;

};

#endif // HIERARCHYBROWSER_H
