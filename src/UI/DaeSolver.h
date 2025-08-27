/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#ifndef UI_DAE_SOLVER_H
#define UI_DAE_SOLVER_H

#include <QApplication>
#include <QMainWindow>
#include <QSplitter>
#include <QThread>
#include <QString>
#include <QPushButton>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QObject>
#include <QCheckBox>
#include <QProgressBar>
#include <QProperty>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <format>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include "../MathPack/Parser.h"
#include "../MathPackODE/SolverWorker.h"
#include "ModelState.h"
#include "../ElementBase/ElementCatalogue.h"
#include "FlowLayout.h"
#include "Logger.h"

namespace dae_solver{
    class DaeSolver : public QMainWindow{
        Q_OBJECT

        QLabel *fileNameLabel = nullptr;

        public:
            QProperty<int> simProgress; // 0-100
            //Logger myLog;

            DaeSolver(int argc=0, char** argv=nullptr);

            void init_gui();

        private:
            ModelState state;
            bool isSaveNeeded = false;
            QString currentFile = "untitled";
            shared_ptr<Logger> logger;
        
            QMenuBar* createMenu(QVBoxLayout *rootLayout);

            QSplitter* createElementCatalog();

            QWidget* showConfigurator();

            /**
             * Output text in pop-up window (i.e. exceptions)
             */
            static void layoutString(QString & input);
    };
}

#endif
