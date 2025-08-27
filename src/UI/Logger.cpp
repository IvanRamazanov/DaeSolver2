#include "Logger.h"

namespace dae_solver{

    Logger::Logger(bool logMsgs, bool logErrors):
        logFile(QFile("logFile.txt")),
        errFile(QFile("errorLog.txt")),
        writeLogs(logMsgs),
        writeErrors(logErrors)
    {
        // TODO intercept cerr?
        //sysErr=System.err;

        // UI
        messageWindow=new QWidget();
        messageWindow->setWindowTitle("Error!");
        QVBoxLayout *root = new QVBoxLayout();
        text = new QLabel();

        root->addWidget(text);
        messageWindow->resize(400, 400);
        messageWindow->setLayout(root);

        showBth = make_shared<Connections::Property<bool>>(false);
        errBtn = new QPushButton();
        QPixmap icon = QPixmap(":src/data/UI/errIcon.png").scaled(24,24);
        errBtn->setIcon(QIcon(icon));
        QWidget::connect(errBtn, &QPushButton::clicked, [this](){
            showWindow();
        });
        showBth->setOnChange([this](bool oldV, bool newV){
            (void)oldV; // unused

            errBtn->setVisible(newV);
        });
    }
    
    void Logger::printErr(string const& msg){
        showBth->setValue(true);
        iterator++;
        // TODO into console
        //sysErr.print((char)b);
        if (writeErrors && errFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            errFile.write(msg.data(), msg.length());

            errFile.close();
        }
    }

    void Logger::printMsg(string const& msg){
        if (writeLogs && logFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            logFile.write(msg.data(), msg.length());

            logFile.close();
        }
    }

    void Logger::printMatrixHeader(vector<string> const& arr){
        if (writeLogs && logFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            for (auto& v:arr){
                if (v.length() > vectorSpacing)
                    vectorSpacing = v.length();
            }

            // extra whitespace for opening bracket '['
            logFile.write(" ");
            for (auto&v : arr){
                string formatted = std::format("{:>{}s} ", v, vectorSpacing);
                logFile.write(formatted.data(), formatted.length());
            }
            logFile.write("\n");

            logFile.close();
        }
    }

    /**
     * Print matrix row with header spacing applied
     */
    void Logger::printMatRow(vector<int> const& row){
        if(writeLogs && logFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            logFile.write("[");
            for (size_t i=0; i<row.size()-1; i++){
                string formatted = std::format("{:{}d},", row[i], vectorSpacing);
                logFile.write(formatted.data(), formatted.length());
            }
            if (!row.empty()){
                string formatted = std::format("{:{}d}", row[row.size()-1], vectorSpacing);
                logFile.write(formatted.data(), formatted.length());
            }
            logFile.write("]");

            logFile.close();
        }
    }

    void Logger::errorLayout(){
        showWindow();

        if (writeErrors && errFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            errFile.write("\n\n");
            errFile.write(QDateTime::currentDateTime().toString().toUtf8());
            errFile.write("\n");

            errFile.close();
        }
    }

    void Logger::initLogs(){
        showBth->setValue(false);
        buffer.clear();

        // clear log files
        if (writeLogs){
            // try open log file
            if(logFile.open(QIODevice::WriteOnly))
                // just silently disable logging
                logFile.close(); 
        }
        if (writeErrors){
            // try open log file
            if(errFile.open(QIODevice::WriteOnly))
                errFile.close();
        }
    }

    void Logger::showWindow(){
        text->setText(QString::fromStdString(buffer));
        messageWindow->show();
        //messageWindow.sizeToScene();
    }

    void Logger::showWindow(string const& message){
        buffer += "\n"+message;
        text->setText(QString::fromStdString(buffer));
        messageWindow->show();
        //messageWindow.sizeToScene();
    }
        
}
