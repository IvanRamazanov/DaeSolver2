#include "../StringGraph.h"

namespace MathPack{
    shared_ptr<StringGraph> StringGraph::mul(shared_ptr<StringGraph> gr, double value){
        shared_ptr<StringGraph> out = make_shared<StringGraph>(*gr);
        out->multiply(value);
        out->simplify();
        return out;
    }

    shared_ptr<StringGraph> StringGraph::mul(shared_ptr<StringGraph> gr1, shared_ptr<StringGraph> gr2, int gain){
        vector<node_container<Node>> inps;
        inps.push_back(gr1->root->copy());
        inps.push_back(gr2->root->copy());
        vector<int> gains;
        gains.push_back(1);
        gains.push_back(gain);
        auto out = make_shared<StringGraph>(make_node<FuncNode>("*", inps, gains));
        out->simplify();
        return out;
    }

    shared_ptr<StringGraph> StringGraph::sum(vector<shared_ptr<StringGraph>> const& gr, vector<int> gains){
        vector<node_container<Node>> inps;

        // prepare gains
        if (gains.empty()){
            for(auto& sg:gr){
                gains.push_back(1);
            }
        }
        
        // prepare inputs
        for(auto& sg:gr){
            inps.push_back(sg->root->copy());
        }

        auto out = make_shared<StringGraph>(make_node<FuncNode>("+", inps, gains));
        out->simplify();
        return out;
    }

    shared_ptr<StringGraph> StringGraph::sum(shared_ptr<StringGraph> gr1, shared_ptr<StringGraph> gr2, int gain){
        if(gain!=1 && gain!=-1){
            throw invalid_argument("Gain: " + to_string(gain) + ". Gain must be equal +/-1");
        }

        vector<node_container<Node>> inps;
        vector<int> gains;

        inps.push_back(gr1->root->copy());
        gains.push_back(1);

        inps.push_back(gr2->root->copy());
        gains.push_back(gain);

        auto out = make_shared<StringGraph>(make_node<FuncNode>("+", inps, gains));
        out->simplify();
        return out;
    }

    /**
     * Devides value/gr
     * @param value
     * @param gr
     * @return out=value/gr
     */
    shared_ptr<StringGraph> StringGraph::divide(int value, shared_ptr<StringGraph> gr){
        vector<node_container<Node>> inps;
        vector<int> gains;

        inps.push_back(make_node<Const>(value));
        gains.push_back(1);
        inps.push_back(gr->root->copy());
        gains.push_back(-1);

        auto out = make_shared<StringGraph>(make_node<FuncNode>("*", inps, gains));
        out->simplify();
        return out;
    }

}
