#include <QApplication>

#include <QDialog>

#include <stdio.h>

#include "managers/undomanager/undomanager.h"

#include "editors/scenecomposer/scenecomposer.h"

#include "assetmanager.h"
#include "codemanager.h"
#include "projectmanager.h"
#include <engine.h>

#include "common.h"
#include "qlog.h"

#include <QDesktopServices>
#include <QUrl>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QFile qss(":/Style/styles/dark/style.qss");
    if(qss.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }

    ProjectManager *mgr = ProjectManager::instance();
    mgr->init(QApplication::arguments().at(1));

    IFile *file = new IFile();
    file->finit(qPrintable(QApplication::arguments().at(0)));
    file->fsearchPathAdd(qPrintable(mgr->importPath()), true);

    Engine engine(file);
    engine.init();

    SceneComposer w(&engine);
    QApplication::connect(AssetManager::instance(), SIGNAL(importFinished()), &w, SLOT(show()));

    CodeManager::instance()->init();
    AssetManager::instance()->init();
    UndoManager::instance()->init();

    return a.exec();
}
