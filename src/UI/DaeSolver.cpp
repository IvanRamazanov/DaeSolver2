/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "DaeSolver.h"

namespace dae_solver{
    DaeSolver::DaeSolver(int argc, char** argv){
        switch(argc) {
            case 3:
                if (string(argv[1]) == "-o") {
                    filesystem::path  f(argv[1]);
                    state.load(argv[2]);
                    currentFile = QString::fromStdString(f.stem().string());
                }
                break;
            case 2:
                string f_path(argv[1]);
                filesystem::path uri(f_path);
                if(filesystem::exists(uri)){
                    state.load(f_path);
                    currentFile = QString::fromStdString(uri.stem().string());
                }
        }

        // TODO implement
        //std::cerr.rdbuf(myLog.rdbuf());

        logger = make_shared<Logger>(true, true); // TODO get log flags from arguments

        init_gui();
    }

    void DaeSolver::init_gui(){
        setWindowTitle("OmniSystem Simulator");
        QWidget *main_zone = new QWidget();
        QVBoxLayout *rootLayout = new QVBoxLayout();
        resize(800, 600);
        this->setStyleSheet("mod.css");

        // add menu bar
        setMenuBar(createMenu(rootLayout));

        // main area
        rootLayout->addWidget(state.getMainSystem()->openDialogStage());

        // create control area
        QLabel* Status = new QLabel();
        QWidget *statusBar = new QWidget();
        QGridLayout *bottomBox = new QGridLayout();

        // progress
        QProgressBar *progBar = new QProgressBar();
        progBar->setMinimum(0);
        progBar->setMaximum(100);
        bottomBox->addWidget(progBar, 0, 1);
        bottomBox->addWidget(Status, 0, 0);

        // opened file
        bottomBox->addWidget(new QLabel("File: "), 0, 2);
        fileNameLabel = new QLabel(currentFile);
        bottomBox->addWidget(fileNameLabel, 0, 3);

        // control buttons
        QPushButton *startBtn = new QPushButton("Start");
        QPushButton *stopBtn = new QPushButton("Stop");
        stopBtn->setEnabled(false);
        QPushButton *errBtn = new QPushButton();
        errBtn->setFixedHeight(24);
        
        bottomBox->addWidget(startBtn, 0, 4);
        bottomBox->addWidget(stopBtn, 0, 5);
        bottomBox->addWidget(errBtn, 0, 6);

        connect(startBtn, &QPushButton::clicked, [this, startBtn, stopBtn, progBar, Status](){
            //myLog.initLogs();
            QThread *solverThread = new QThread();
            SolverWorker *eval = new SolverWorker(&state, logger);
            eval->moveToThread(solverThread);

            startBtn->setEnabled(false);
            stopBtn->setEnabled(true);

            connect(solverThread, &QThread::finished, solverThread, &QObject::deleteLater);
            connect(eval, &SolverWorker::finished, solverThread, &QThread::quit);
            connect(eval, &SolverWorker::finished, eval, &QObject::deleteLater);

            connect(eval, &SolverWorker::progressIncrement, [progBar, Status, this](const int &v){
                //if(Status.textProperty().isBound())
                //    Status.textProperty().unbind();
                progBar->setValue(v);
                if(v == 100){
                    Status->setText("Done!");
                    //Toolkit.getDefaultToolkit().beep();
                    cout << "\a";
                }else{
                    Status->setText(QString::asprintf("T={%.3f}", v));
                }
            });

            connect(stopBtn, &QPushButton::clicked, [eval](){
                eval->abort();
            });
            
            connect(eval, &SolverWorker::success, [stopBtn, startBtn](){
                stopBtn->setEnabled(false);
                startBtn->setEnabled(true);
            });
            connect(eval, &SolverWorker::cancelled, [Status, stopBtn, startBtn](){
                //Status.textProperty().unbind();
                Status->setText("Stopped by user");
                stopBtn->setEnabled(false);
                startBtn->setEnabled(true);
            });
            connect(eval, &SolverWorker::failed, [Status, stopBtn, startBtn](){
                //Status.textProperty().unbind();
                Status->setText("Fatal error");
                stopBtn->setEnabled(false);
                startBtn->setEnabled(true);
                //myLog.errorLayout();

                //Toolkit.getDefaultToolkit().beep();
                cout << "\a";
            });

            
            //solverThread.setDaemon(true);
            connect(solverThread, &QThread::started, eval, &SolverWorker::call);

            solverThread->start();
            //solverThread.wait();
            //delete eval;
        });

        //myLog.setButton(errBtn);
        
        statusBar->setLayout(bottomBox);
        rootLayout->addWidget(statusBar);

        state.getMainSystem()->linkBottomBar(bottomBox);
        
        // primaryStage.setOnCloseRequest((WindowEvent we) -> {
        //     Stage st=new Stage();
        //     BorderPane bp=new BorderPane();
        //     Scene sc=new Scene(bp);
        //     st.setScene(sc);
        //     ButtonBar bb=new ButtonBar(ButtonBar.BUTTON_ORDER_WINDOWS);
        //     Button yes=new Button("Yes");
        //     yes.setOnAction(ae->{
        //         Platform.exit();
        //     });
        //     ButtonBar.setButtonData(yes, ButtonBar.ButtonData.YES);
        //     Button no=new Button("No");
        //     no.setOnAction(ae->{
        //         st.close();
        //     });
        //     ButtonBar.setButtonData(no, ButtonBar.ButtonData.NO);
        //     bb.getButtons().addAll(no,yes);
        //     bb.setButtonOrder("+YN");
        //     bp.setBottom(bb);

        //     bp.setTop(new Label("Are you really want to exit?"));

        //     st.sizeToScene();
        //     st.show();

        //     we.consume();
        // });

        main_zone->setLayout(rootLayout);
        setCentralWidget(main_zone);
    }
     
    QMenuBar* DaeSolver::createMenu(QVBoxLayout *rootLayout){
        QMenuBar *menuBar = new QMenuBar();
        QMenu *fileMenu = new QMenu("File");

        QAction *newFile = new QAction("New");
        connect(newFile, &QAction::triggered, [&](){
            if(isSaveNeeded){

            }else{
                state.clearState();
                currentFile = "Untitled.rim";
            }
        });

        QAction *menuOpenFile = new QAction("Open");
        menuOpenFile->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
        connect(menuOpenFile, &QAction::triggered, [this, rootLayout](){
            QFileDialog filechoose;
            filechoose.setNameFilter("RIM (*.rim)");
            filechoose.setWindowTitle("Choose a file");
            filechoose.setFileMode(QFileDialog::AnyFile);
            filechoose.setViewMode(QFileDialog::Detail);
            if(filechoose.exec()){
                auto fileName = filechoose.selectedFiles()[0];
                bool success = state.load(fileName.toStdString());
                if (success)
                    currentFile = fileName;

                // new system would be created in any case, so replace
                rootLayout->insertWidget(0, state.getMainSystem()->openDialogStage());
            }
        });

        QAction *menuSaveFile = new QAction("Save");
        menuSaveFile->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
        connect(menuSaveFile, &QAction::triggered, [&](){
            if(state.getFilePath().empty()){
                QString fileName = QFileDialog::getSaveFileName(nullptr, // this?
                    tr("Save as..."), "", tr("RIM (*.rim)"));
                if(!fileName.isNull()){
                    state.Save(fileName.toStdString());
                    currentFile = fileName;
                }
            }else{
                state.save();
            }
        });

        QAction *menuSaveAsFile = new QAction("Save as...");
        menuSaveAsFile->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
        connect(menuSaveAsFile, &QAction::triggered, [&](){
            QString fileName = QFileDialog::getSaveFileName(nullptr, // this?
                tr("Save as..."), "", tr("RIM (*.rim)"));
            if(!fileName.isNull()){
                state.Save(fileName.toStdString());
                currentFile = fileName;
            }
        });

        QAction *menuExit = new QAction("Exit");
        connect(menuExit, &QAction::triggered, [&](){
            QApplication::quit();
        });

        fileMenu->addActions({newFile,menuOpenFile,menuSaveFile,menuSaveAsFile,menuExit});
        menuBar->addMenu(fileMenu);


        // Simulation Menu
        QMenu *evalMenu = new QMenu("Simulation");

        QAction *config = new QAction("Solver configuration");
        connect(config, &QAction::triggered, [&](){
            auto window = showConfigurator();
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
        });
        evalMenu->addAction(config);

        menuBar->addMenu(evalMenu);

        // Catalogue Menu
        QMenu *catMenu = new QMenu("Element catalogue");
        QAction *item = new QAction("Open catalogue");
        item->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
        connect(item, &QAction::triggered, [&](){
            QSplitter *st = createElementCatalog();
            st->setAttribute(Qt::WA_DeleteOnClose);
            st->show();
            st->setSizes({st->width() / 2, st->width() / 2});
            st->raise();
        });
        catMenu->addAction(item);

        menuBar->addMenu(catMenu);

        return menuBar;
    }

    QSplitter* DaeSolver::createElementCatalog(){
        QSplitter *ret = new QSplitter();
        ret->setParent(this);
        ret->setWindowFlag(Qt::Window, true);
        ret->setWindowTitle("Element catalogue");
        ret->resize(600, 400);

        // tree
        
        QWidget *elems = new QWidget();
        FlowLayout *layout = new FlowLayout(5, 5, 5);
        elems->setLayout(layout);
        elems->setStyleSheet("background: white;");

        ret->addWidget(new ElementBase::ElementCatalogue(elems));
        ret->addWidget(elems);

        return ret;
    }

    QWidget* DaeSolver::showConfigurator(){
        QWidget *subWind = new QWidget(this);
        subWind->setWindowFlags(Qt::Window);
        subWind->setWindowTitle("Solver confuguration");
        subWind->resize(400, 300);
        QLineEdit *delta=new QLineEdit(QString("%1").arg(state.getDt()));
        QLineEdit *endTime=new QLineEdit(QString("%1").arg(state.getTend()));
        QLineEdit *abst=new QLineEdit(QString("%1").arg(state.getAbsTol()));
        QLineEdit *relt=new QLineEdit(QString("%1").arg(state.getRelTol()));
        QComboBox *solver=new QComboBox();
        solver->setInsertPolicy(QComboBox::NoInsert);
        solver->addItems({"Euler",
                        "Adams4",
                        "RungeKuttaFehlberg",
                        "RK4",
                        "Rosenbrok",
                        "BDF1",
                        "BDF2",
                        "TRBDF",
                        "TR"});
        solver->setCurrentText(QString::fromStdString(state.getSolver())); // TODO: verify it exists in the list!
        QComboBox *jacobEstType = new QComboBox();
        jacobEstType->setInsertPolicy(QComboBox::NoInsert);
        jacobEstType->addItems({"Full symbolic",
                                "Symbolic inverse Jacobian",
                                "Symbolic only Jacobian"});
        jacobEstType->setCurrentIndex(state.getJacobianEstimationType());
        QGridLayout *top = new QGridLayout();
        top->setContentsMargins(0, 5, 0, 5);
        top->addWidget(new QLabel("Fixed step size:"), 0, 0);
        top->addWidget(delta, 0, 1);
        top->addWidget(new QLabel("Simulation time:"), 1, 0);
        top->addWidget(endTime, 1, 1);
        top->addWidget(new QLabel("Absolute tolerance:"), 2, 0);
        top->addWidget(abst, 2, 1);
        top->addWidget(new QLabel("Relative tolerance:"), 3, 0);
        top->addWidget(relt, 3, 1);
        top->addWidget(new QLabel("Solver type:"), 4, 0);
        top->addWidget(solver, 4, 1);
        top->addWidget(new QLabel("Jacobian type:"), 5, 0);
        top->addWidget(jacobEstType, 5, 1);
        top->addWidget(new QLabel("Try to reduce system size"), 6, 0);
        QCheckBox *cb = new QCheckBox();
        cb->setChecked(state.getSimplyfingFlag());
        connect(cb, &QCheckBox::checkStateChanged, 
            [this](Qt::CheckState chState){
                this->state.setSimplyfingFlag(chState == Qt::Checked);
            }
        );
        top->addWidget(cb, 6, 1);

        QVBoxLayout *root = new QVBoxLayout();
        root->addLayout(top);
        QHBoxLayout *bot = new QHBoxLayout(); // QButtonGroup?
        QPushButton *okBtn = new QPushButton("Ok");
        connect(okBtn, &QPushButton::clicked, [this, delta, endTime, solver, jacobEstType, abst, relt, subWind](){
            double tmpVal;
            bool s2d_ok;

            tmpVal = delta->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setDt(tmpVal);
            }

            tmpVal = endTime->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setTend(tmpVal);
            }

            state.setSolver(solver->currentText().toStdString());
            state.setJacobianEstimationType(jacobEstType->currentIndex());

            tmpVal = abst->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setAbsTol(tmpVal);
            }

            tmpVal=relt->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setRelTol(tmpVal);
            }

            subWind->close();
        });
        QPushButton *applyBtn = new QPushButton("Apply");
        connect(applyBtn, &QPushButton::clicked, [this, delta, endTime, solver, jacobEstType, abst, relt](){
            double tmpVal;
            bool s2d_ok;

            tmpVal = delta->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setDt(tmpVal);
            }

            tmpVal = endTime->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setTend(tmpVal);
            }

            state.setSolver(solver->currentText().toStdString());
            state.setJacobianEstimationType(jacobEstType->currentIndex());

            tmpVal = abst->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setAbsTol(tmpVal);
            }

            tmpVal=relt->text().toDouble(&s2d_ok);
            if(s2d_ok){
                state.setRelTol(tmpVal);
            }
        });
        QPushButton *cancelBtn = new QPushButton("Cancel");
        connect(cancelBtn, &QPushButton::clicked, [subWind](){
            subWind->close();
        });
        bot->setAlignment(Qt::AlignBottom | Qt::AlignRight);
        bot->addWidget(okBtn);
        bot->addWidget(applyBtn);
        bot->addWidget(cancelBtn);
        root->addLayout(bot);
        subWind->setLayout(root);

        return subWind;

        // TODO ?
        // root.setOnKeyReleased(ke->{
        //     if(ke.getCode()==KeyCode.ENTER){
        //         try {
        //             DoubleStringConverter conv=new DoubleStringConverter();
        //             dt.set(conv.fromString(delta.getText()));
        //             t_end.set(conv.fromString(endTime.getText()));
        //             solverType.set((String)solver.getValue());
        //             subWind.close();
        //         } catch (Exception e) {
        //             layoutString(e.getCause().toString());
        //         }
        //     }
        // });
    }
    

    /**
     * Output text in pop-up window (i.e. exceptions)
     */
    void DaeSolver::layoutString(QString & input){
        QWidget stag(NULL, Qt::Window);
        QHBoxLayout* root = new QHBoxLayout();
        QLabel* lbl = new QLabel(input);
        root->addWidget(lbl);
        stag.setLayout(root);
        stag.resize(QSize(300, 250));
        stag.show();
    }

    /*
    static void DaeSolver::initCustomDomains(){
        try{
//            URL s=getClass().getResource("/Elements");

            File f=new File("");
            //ZipInputStream zis=new ZipInputStream(new FileInputStream(f.getAbsolutePath()+"\\RaschetKz.jar"));
            String path="C:/Users/Ivan/Documents/Java Projects/RaschetKz/out/artifacts/RaschetKz_jar/"+"RaschetKz.jar";
            JarFile zis=new JarFile(path);
            Enumeration entries=zis.entries();

            for(; entries.hasMoreElements();) {
                JarEntry je=(JarEntry) entries.nextElement();
                String entryName=je.getName();
                if (entryName.startsWith("ElementBase")&&!je.isDirectory() && entryName.endsWith(".cfg")
                        && !entryName.contains("$")) {
                    URL url = new URL("jar:file:"+path+"!/"+entryName);
                    InputStream is=url.openStream();
                    //default
                    String stg=Parser.getBlock(is,"<Default>");
                    List<String> userDomains=Parser.getBlockList(stg);
                    for(String domData:userDomains){
                        //Connector s=new Connector(domData); //TODO
                        //compoundConnectors.add(s);
                    }

                    //compound
                    stg=Parser.getBlock(is,"<Domains>");
                    userDomains=Parser.getBlockList(stg);
                    for(String domData:userDomains){
                        //CompoundConnector s=new CompoundConnector(domData);
                        //compoundConnectors.add(s);
                    }
                    break;
                }
            }

            //parse result file
        }catch(Exception ex){
            ex.printStackTrace(System.err);

            Path p=new File("src/ElementBase/Domains.cfg").toPath();

            //parse


        }
    }
            */

}
