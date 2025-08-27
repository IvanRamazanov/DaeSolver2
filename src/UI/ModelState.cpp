/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "ModelState.h"


/**
 *
 * @author Ivan Ramazanov
 */
using namespace std;

namespace dae_solver{
    void ModelState::save(){
        QFile file_out(QString::fromStdString(filePath));
        try{
            XmlParser::XmlElement *main = new XmlParser::XmlElement("root");
            main->attributes["version"] = "1.0";

            XmlParser::XmlElement *root = new XmlParser::XmlElement("config");

            XmlParser::XmlElement *node = new XmlParser::XmlElement("Solver");
            node->text = solver;
            root->append(node);

            node = new XmlParser::XmlElement("tEnd");
            node->text = to_string(tend);
            root->append(node);

            node = new XmlParser::XmlElement("dt");
            node->text = to_string(dt);
            root->append(node);

            node = new XmlParser::XmlElement("JacobEsim");
            node->text = to_string(jacobianEstimationType);
            root->append(node);

            node = new XmlParser::XmlElement("AbsTolerance");
            node->text = to_string(AbsTol);
            root->append(node);

            node = new XmlParser::XmlElement("RelativeTolerance");
            node->text = to_string(RelTol);
            root->append(node);

            node = new XmlParser::XmlElement("TryReduce");
            node->text = to_string(simplyfingFlag);
            root->append(node);

            node = new XmlParser::XmlElement("WindowSize");
            //node->text = std::format("{},{}", getMainSystem().getWindowWidth(), getMainSystem().getWindowHeight());
            root->append(node);

            main->append(root);

            // main subsystem
            root = mainSystem->to_xml();
            main->append(root);

            if (!file_out.open(QIODevice::WriteOnly)){
                throw ios_base::failure("Can't open file: " + filePath);
            }

            auto buf = main->toStrings(true); // TODO: 'true' only for debug!
            for (auto& line:buf)
                file_out.write(line.c_str());
            delete main;
        } catch(exception &ex) {
            cerr << ex.what();
        }
        file_out.close();
    }

    void ModelState::parseConfig(XmlParser::XmlElement *config){
        // TODO validate
        auto node = config->find("Solver");
        if (node)
            solver = node->text;

        node = config->find("tEnd");
        if (node)
            tend = stod(node->text);

        node = config->find("dt");
        if (node)
            dt = stod(node->text);

        node = config->find("JacobEsim");
        if (node)
            setJacobianEstimationType(stoi(node->text));

        node = config->find("AbsTolerance");
        if (node)
            AbsTol = stod(node->text);

        node = config->find("RelativeTolerance");
        if (node)
            RelTol = stod(node->text);

        node = config->find("TryReduce");
        if (node)
            simplyfingFlag = XmlParser::stob(node->text);

        node = config->find("WindowSize");
        if (node){
            auto data = XmlParser::split(node->text, ",");
            //getMainSystem().getStage().setWidth(stod(data[0]));
            //getMainSystem().getStage().setHeight(stod(data[1]));
        }
    }

    void ModelState::setFileName(string fileName) {
        filePath = fileName;
    }

    ModelState::ModelState(){
        mainSystem = new ElementBase::Subsystem();
    }

    string ModelState::getFilePath() {
        return filePath;
    }

    void ModelState::Save(string filePath){
        setFileName(filePath);
        save();
    }

    bool ModelState::load(string filePath){
        filesystem::path path(filePath);
        XmlParser::ElementTree *tmp_etree = nullptr;
        try
        {
            clearState();

            tmp_etree = XmlParser::ElementTree::fromPath(path);
            auto state_info = tmp_etree->getRoot();
            delete tmp_etree;

            auto cfg = state_info->find("config");
            if (cfg)
                parseConfig(cfg);

            auto main_sys = state_info->find("System");
            if (main_sys){
                mainSystem->loadState(main_sys);
                setFileName(filePath);
                
                return true;
            }
            return false;
        } 
        catch(exception &io)
        {
            cerr << io.what();
            return false;
        }
    }

    void ModelState::clearState(){
        mainSystem->clear();
    }

    ElementBase::Subsystem* ModelState::getMainSystem(){
        return mainSystem;
    }

    /**
     * @return the dt
     */
    double ModelState::getDt() {
        return dt;
    }

    void ModelState::setDt(double value){
        dt = value;
    }

    /**
     * @return the tend
     */
    double ModelState::getTend() {
        return tend;
    }

    void ModelState::setTend(double value){
        tend = value;
    }

    /**
     * @return the solver
     */
    const string& ModelState::getSolver() {
        return solver;
    }

    void ModelState::setSolver(const string& value) {
        // TODO sanitize?
        solver = value;
    }

    /**
     * @return the jacobianEstimationType
     */
    int ModelState::getJacobianEstimationType() {
        return jacobianEstimationType;
    }

    /**
     * @param jacobianEstimationType the jacobianEstimationType to set
     */
    void ModelState::setJacobianEstimationType(int jacobianEstimationType) {
        this->jacobianEstimationType = jacobianEstimationType;
    }

    double ModelState::getAbsTol() {
        return AbsTol;
    }

    void ModelState::setAbsTol(double value){
        AbsTol = value;
    }

    double ModelState::getRelTol() {
        return RelTol;
    }

    void ModelState::setRelTol(double value){
        RelTol = value;
    }

    bool ModelState::getSimplyfingFlag() {
        return simplyfingFlag;
    }

    void ModelState::setSimplyfingFlag(bool value){
        simplyfingFlag = value;
    }

}
