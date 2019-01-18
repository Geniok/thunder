#ifndef BUILDER_H
#define BUILDER_H

#include "converter.h"
#include "resources/text.h"

#include <QMap>

class ProjectManager;

typedef QMap<QString, QString>      StringMap;

class NEXT_LIBRARY_EXPORT IBuilder : public IConverter {
    Q_OBJECT
public:
    IBuilder                        ();

    virtual bool                    buildProject        () = 0;

    virtual QString                 builderVersion      () = 0;

    virtual uint32_t                contentType         () const { return IConverter::ContentCode; }
    virtual uint32_t                type                () const { return MetaType::type<Text *>(); }
    virtual uint8_t                 convertFile         (IConverterSettings *);

    void                            copyTemplate        (const QString &src, const QString &dst, StringMap &values);

    QString                         project             () const { return m_Project; }

    QString                         artifact            () const { return m_Artifact; }

    QStringList                     rescanSources       (const QString &path) const;

signals:
    void                            buildFinished       (int exitCode);

protected:
    StringMap                       m_Values;

    QString                         m_Suffix;
    QString                         m_Project;
    QString                         m_Artifact;

};

#endif // BUILDER_H