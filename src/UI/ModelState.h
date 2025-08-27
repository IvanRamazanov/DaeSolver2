/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef MAIN_MODELSTATE_H
#define MAIN_MODELSTATE_H

#include <string>
#include <format>
#include <filesystem>
#include "../MathPack/Parser.h"
#include "../ElementBase/Subsystem/Subsystem.h"
#include "../Connections/Wire.h"
#include "../ElementBase/Element.h"


/**
 *
 * @author Ivan Ramazanov
 */
using namespace std;

namespace dae_solver{

    class ModelState{
        private:
            ElementBase::Subsystem *mainSystem;
            string solver = "Adams4";
            double dt = 1e-3,
                    tend = 10,
                    AbsTol = 1e-5,
                    RelTol = 1e-3;
            int jacobianEstimationType = 2;
            string filePath;
            bool simplyfingFlag = false;

        public:
            void save();

            void parseConfig(XmlParser::XmlElement *config);

        public:
            void setFileName(string fileName);

            ModelState();

            string getFilePath();

            void Save(string filePath);

            /**
             * @return true - if load was successfull; false - otherwise
             */
            bool load(string filePath);

            void clearState();

            ElementBase::Subsystem* getMainSystem();

            /**
             * @return the dt
             */
            double getDt();
            void setDt(double value);

            /**
             * @return the tend
             */
            double getTend();
            void setTend(double value);

            double getAbsTol();
            void setAbsTol(double value);

            double getRelTol();
            void setRelTol(double value);

            bool getSimplyfingFlag();
            void setSimplyfingFlag(bool value);

            /**
             * @return the solver
             */
            const string& getSolver();
            void setSolver(string const& value);

            /**
             * @return the jacobianEstimationType
             */
            int getJacobianEstimationType();

            /**
             * @param jacobianEstimationType the jacobianEstimationType to set
             */
            void setJacobianEstimationType(int jacobianEstimationType);
    };
}

#endif
