/*
 * The MIT License
 *
 * Copyright 2017 Иван.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef UI_LOGGER_H
#define UI_LOGGER_H

#include <string>
#include <format>
#include <vector>
#include <memory>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QDateTime>
#include <stdexcept>
#include "../Connections/Property.h"

namespace dae_solver{

    class Logger {
        private:
            QWidget *messageWindow;
            QLabel *text;
            size_t iterator;
            QPushButton *errBtn;
            shared_ptr<Connections::Property<bool>> showBth;
            //PrintStream sysErr;
            size_t vectorSpacing = 1; // width of elements when printing vector/matrices
            QFile logFile, errFile;
            bool writeLogs, // to file
                writeErrors; // to file
            string buffer; // message storage (stores old errors)


        public:
            Logger(bool logMsgs=false, bool logErrors=false);
            
            void printErr(string const& msg);

            void printMsg(string const& msg);

            void printMatrixHeader(vector<string> const& arr);

            /**
             * Print matrix row with header spacing applied
             */
            void printMatRow(vector<int> const& row);

            void errorLayout();

            void initLogs();

        private:
            void showWindow();

            void showWindow(string const& message);
        
    };
}

#endif
