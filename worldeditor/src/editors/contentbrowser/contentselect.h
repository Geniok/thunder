#ifndef CONTENTSELECT_H
#define CONTENTSELECT_H

#include <QWidget>

#include <stdint.h>

class QSortFilterProxyModel;
class ContentBrowser;

namespace Ui {
    class ContentSelect;
}

class ContentSelect : public QWidget {
    Q_OBJECT

public:
    explicit ContentSelect      (QWidget *parent = 0);
    ~ContentSelect              ();

    QString                     data                            () const;

    void                        setType                         (const uint8_t type);
    void                        setData                         (const QString &guid);

signals:
    void                        assetChanged                    (const QString &source);

private slots:
    void                        onAssetSelected                 (const QString &source);

private:
    Ui::ContentSelect          *ui;

    ContentBrowser             *m_pBrowser;
};

#endif // CONTENTSELECT_H
