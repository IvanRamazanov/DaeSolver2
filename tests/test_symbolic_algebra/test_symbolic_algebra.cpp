#include <QTest>
#include <memory>
#include <string>
#include <limits>
#include "StringGraph.h"
#include "mem_api.h"

using namespace MathPack;

class TestStringGraph : public QObject {
    Q_OBJECT

private slots:
    void testBracketElimination() {
        StringGraph sg("(((V1)))");
        
        // link to vars
		WorkSpace::Variable v1("V1", 5);
		sg.linkVariableToWorkSpace("V1", &v1);
        QCOMPARE(sg.evaluate(), 5);
        QCOMPARE(sg.toString(), "V1");

        sg = StringGraph("-(((V1)))");
        sg.linkVariableToWorkSpace("V1", &v1);
        QCOMPARE(sg.evaluate(), -5);
    }

    void testBracketInvalidDetection() {
        string sg_err,
                sg_proto = "((V1)))",
                err_prefix = "Invalid brackets in function: ",
                expectedErr = err_prefix + sg_proto;
        try{
            StringGraph sg(sg_proto);
        } catch (exception & e){
            sg_err = e.what();
        }

        QCOMPARE(sg_err, expectedErr);
        
        sg_err.clear();
        sg_proto = "(((V1))";
        expectedErr = err_prefix + sg_proto;
        try{
            StringGraph sg(sg_proto);
        } catch (exception & e){
            sg_err = e.what();
        }
        QCOMPARE(sg_err, expectedErr);
    }

    void testSgSimplify() {
        StringGraph sg("-((15.000000-(Subsystem::Resistor::p1.p))/(10.000000))-(-(Subsystem::Capasitor::d.X*(0.010000)))");
        
        // link to vars
        std::string p1_name = "Subsystem::Resistor::p1.p",
					x_name = "Subsystem::Capasitor::d.X";
		auto p1 = make_shared<WorkSpace::Variable>(p1_name, 5, false, WorkSpace::Potential);
		WorkSpace::DifferentialVar dx(p1, WorkSpace::Potential);
        dx.setValue(13);
		sg.linkVariableToWorkSpace(p1_name, p1.get());
		sg.linkVariableToWorkSpace(x_name, &dx);

        // test simplify function
		sg.simplify();

        QCOMPARE(sg.evaluate(), -0.87);
    }

    void testSgSimplify2() {
        StringGraph sg("x1/(x2*1*x3/x4)");
        WorkSpace::Variable x1("x1", 6),
                            x2("x2", 5),
                            x3("x3", 3),
                            x4("x4", 4);
        sg.simplify();
        QCOMPARE(sg.getRoot()->type(), MathPack::FunctionNode);
        auto fsg = MathPack::cast_node<FuncNode>(sg.getRoot());
        QCOMPARE(fsg->getRank(), 4);

        sg.linkVariableToWorkSpace(x1.getName(), &x1);
        sg.linkVariableToWorkSpace(x2.getName(), &x2);
        sg.linkVariableToWorkSpace(x3.getName(), &x3);
        sg.linkVariableToWorkSpace(x4.getName(), &x4);
        QCOMPARE(sg.evaluate(), 1.6);
    }

    // test that division by 0 is handled correctly
    void testSgSimplify3() {
        StringGraph sg("2*5/0");
        sg.simplify();
        QCOMPARE(sg.evaluate(), std::numeric_limits<double>::infinity());
    }

    void testScientificFormat() {
        StringGraph sg("1.5e3");
        QCOMPARE(sg.evaluate(), 1500);

        sg = StringGraph("10-83E-2");
        QCOMPARE(sg.evaluate(), 9.17);
    }

    void testFunctionFormat() {
        WorkSpace::Variable t("T1", 2);

        StringGraph sg("sin(T1)");
        sg.linkVariableToWorkSpace("T1", &t);
        QCOMPARE(sg.evaluate(), 0.9092974268256817);

        sg = StringGraph("-cos(T1)");
        sg.linkVariableToWorkSpace("T1", &t);
        QCOMPARE(sg.evaluate(), 0.4161468365471424);

        sg = StringGraph(" cos(  1 + T1* 5)");
        sg.linkVariableToWorkSpace("T1", &t);
        QCOMPARE(sg.evaluate(), 0.004425697988050785);
    }

    void testFunctionArgsVerification() {
        bool failed=false;
        try{
            StringGraph sg("if(true, 10, Y123, extraArg*2)");
        }catch (exception &e){
            string errMsg = e.what();
            failed = errMsg.substr(0, 28) == "Function if expects 3 inputs";

            if (!failed)
                qDebug() << "What:" << Qt::endl << errMsg;
        }
        QVERIFY(failed);

        StringGraph sg("gr(10, extraArg*2)");
    }

    void testPowOperator() {
        StringGraph sg("pow(pow(2, 3), 2)");
        QCOMPARE(sg.evaluate(), 64);

        sg = StringGraph("pow(2, pow(3,2))");
        QCOMPARE(sg.evaluate(), 512);
    }

    void testCanGet() {
        StringGraph sg("X1+1");
        QCOMPARE(sg.canGet("X1"), true);
        QCOMPARE(sg.canGet("X2"), false);
        QCOMPARE(sg.canGet("X"), false);
        QCOMPARE(sg.canGet("1"), false);

        sg = StringGraph("X2+sin(7*X1+X3)");
        QEXPECT_FAIL("", "At the moment, only 'simple' functions are supported", Continue);
        QCOMPARE(sg.canGet("X1"), true);
        QCOMPARE(sg.canGet("X2"), true);
        QEXPECT_FAIL("", "At the moment, only 'simple' functions are supported", Continue);
        QCOMPARE(sg.canGet("X3"), true);
        QCOMPARE(sg.canGet("sin"), false);
    }

    void testGetVarBasic() {
        StringGraph sg("X1-112");
        WorkSpace::Variable var("X1", 2);
        sg.linkVariableToWorkSpace("X1", &var);

        sg.getVariable("X1");

        QCOMPARE(sg.evaluate(), 112);
    }

    void testGetVarWithLeft() {
        StringGraph sg("8/X1");
        auto left = make_shared<StringGraph>("exp(2.7)");

        sg.getVariable("X1", left);

        QCOMPARE(sg.evaluate(), 0.537644101917998);
    }

    // not yet supported?
    void testGetVarFromFunction() {
        StringGraph sg("sin(X1)");
        WorkSpace::Variable var("X1", 2);
        sg.linkVariableToWorkSpace("X1", &var);
        auto lhs = make_shared<StringGraph>(0.5);

        sg.getVariable("X1", lhs);

        QEXPECT_FAIL("", "At the moment, only 'simple' functions are supported", Continue); // see FuncNode::invertNode
        QCOMPARE(sg.evaluate(), 0.5235987755982989);

        // test two argument function
        sg = StringGraph("pow(X1, 2)");
        var.setValue(5);
        sg.linkVariableToWorkSpace("X1", &var);
        lhs = make_shared<StringGraph>(0.5);

        sg.getVariable("X1", lhs);

        QEXPECT_FAIL("", "At the moment, only 'simple' functions are supported", Continue); // see FuncNode::invertNode
        QCOMPARE(sg.evaluate(), 0.5235987755982989);
    }

    void testGetVarComplex() {
        StringGraph sg ("(-(Inductance::d.p1.f*(0.030000))-(Ampermeter::p2.p))/(10.000000)");
        auto lhs = make_shared<StringGraph>("Inductance::p1.f");
        
        sg.getVariable("Ampermeter::p2.p", lhs);

        sg.simplify();
        QCOMPARE(sg.toString(), "-(Inductance::d.p1.f*0.030000)-(Inductance::p1.f*10.000000)");
    }

    void testReplaceVar() {
        StringGraph sg ("(-(Inductance::d.p1.f*0.030000)-Ampermeter::p2.p)/10.000000");
        auto rep = make_shared<StringGraph>("-(Inductance::d.p1.f*0.030000)-((Inductance::p1.f)*10.000000)");
        
        sg.replaceVariable("Ampermeter::p2.p", rep);

        sg.simplify();
        QCOMPARE(sg.toString(), "Inductance::p1.f");
    }
    

    void testDifferentials() {
        StringGraph sg("1*X1");
        auto sgd = sg.getDiffer("X1");

        QCOMPARE(sgd->evaluate(), 1);
    }

    void testMemoryLeaks() {
        StringGraph *sg;
        size_t memSize0,
                memSizeFinal;

        memSize0 = getCurrentMemSize();
        if (memSize0 == 0)
            QFAIL("Can't get process info");

        for(int i=0; i<10000; i++){
            sg = new StringGraph("X1-X2-(X1+X2*5-X3*exp(6))");
            sg->simplify();
            sg->replaceVariable("X2", make_shared<StringGraph>("Y1*logn(Y2)"));
            delete sg;
        }

        memSizeFinal = getCurrentMemSize();
        if (memSizeFinal == 0) {
            QFAIL("Can't get process info");
        }

        if (memSizeFinal > memSize0){
            // consider tolerance
            memSize0 = memSizeFinal - memSize0;

            if (memSize0 > 13000){
                qDebug() << "Memory overhead:" << memSize0;
                QFAIL("Memory leaks!");
            }
        }
    }
};

QTEST_MAIN(TestStringGraph)
#include "test_symbolic_algebra.moc"
