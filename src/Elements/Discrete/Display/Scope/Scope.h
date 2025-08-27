/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMS_SCOPE_H
#define ELEMS_SCOPE_H

#include <limits>
#include "../../../../ElementBase/Element.h"
#include "../../../../UI/Painter.h"

namespace Elements{

/**
 *
 * @author Ivan
 */
class Scope : public ElementBase::Element {
    private:
        double maxVal,
                minVal;
        vector<vector<double>> data;
        vector<double> time;
        int cnt,
            topCnt;
        ElementBase::Parameter *dr;
        ElementBase::Pin *input;

    public:
        Scope();

        void discreteInit() override;
        
        void discreteStep(double t) override;

        QWidget* openDialogStage() override;

        void initView() override;

        friend class ScopeView;
    };
}

#endif
