#include "ElementCatalogue.h"


namespace ElementBase{

    /**
     * find or create (if doesn'r exist) a tree view item
     */
    QTreeWidgetItem* findTreeItem(QList<QTreeWidgetItem*> & tree_items, QStringList const& entry, size_t depth_offset=0){
        QList<QTreeWidgetItem*> sub_list = tree_items;
        QTreeWidgetItem* parent_item = nullptr;
        QTreeWidgetItem* ret = nullptr;
        // create every folder in file path
        for (size_t depth=0; depth<entry.size()-depth_offset; depth++){
            // check if item already exists
            QTreeWidgetItem *sub_result = nullptr;
            for (auto& ti:sub_list){
                if (ti->text(0) == entry[depth]){
                    // folder (item) already exists
                    sub_result = ti;
                    break;
                }
            }
            if (!sub_result){
                // was not found; create new Item
                QTreeWidgetItem *twi = new QTreeWidgetItem({entry[depth]});
                sub_list.append(twi);
                sub_result = twi;

                // append parent
                if (depth == 0){
                    // at root level, simply append the list
                    tree_items.append(sub_result);
                }else{
                    // has parent; 
                    parent_item->addChild(sub_result);
                }
            }
            // cycle deeper
            parent_item = sub_result;
            sub_list.clear();
            for (auto j=sub_result->childCount(); j-->0;){
                sub_list.append(sub_result->child(j));
            }
            // main ref
            ret = sub_result;
        }
        return ret;
    }

    void appendTreeItemData(QTreeWidgetItem *item, QString const& data){
        QStringList eList = item->data(0, Qt::UserRole).toStringList();
        eList.append(data);
        item->setData(0, Qt::UserRole, eList);
    }

    ElementCatalogue::ElementCatalogue(QWidget *elemLayout, QWidget *parent): QTreeWidget(parent), elemLayout(elemLayout){
        QDir rootPath(":/src/data/Elements");
        QDirIterator it(rootPath.path(), {"*.xml"}, QDir::Files, QDirIterator::Subdirectories);
        QList<QStringList> file_list;
        while (it.hasNext()){
            file_list << rootPath.relativeFilePath(it.next()).split("/");
        }

        QList<QTreeWidgetItem*> tree_items;
        QTreeWidgetItem* first_selection = nullptr;
        // construct the tree for each file
        for (auto& entry : file_list) {
            QTreeWidgetItem* existing_item = findTreeItem(tree_items, entry, 1);
            
            // cycled through the path; fill with data
            appendTreeItemData(existing_item, rootPath.filePath(entry.join("/")));

            // cosmetic
            if (first_selection == nullptr){
                first_selection = existing_item;
            }
        }

        // Custom phys (Note: will be removed when moved to metaclasses)
        auto tvItem = findTreeItem(tree_items, {"Physics", "Electric", "Measurements"});
        appendTreeItemData(tvItem, "Ampermeter");
        appendTreeItemData(tvItem, "Voltmeter");
        appendTreeItemData(tvItem, "ThreePhMultimeter");

        tvItem = findTreeItem(tree_items, {"Physics", "Rotational", "Measurements"});
        appendTreeItemData(tvItem, "SpeedSensor");
        appendTreeItemData(tvItem, "TorqueSensor");

        tvItem = findTreeItem(tree_items, {"Physics", "Electric", "ThreePhase", "Measurements"});
        appendTreeItemData(tvItem, "Multimeter");

        // Default Math Elems
        QTreeWidgetItem *math_root = new QTreeWidgetItem({"Discrete"});
        // basics
        QTreeWidgetItem *cat = new QTreeWidgetItem({"Basic"});
        math_root->addChild(cat);
        QStringList eList = {"Gain", "Product", "Saturation", "Sum"};
        cat->setData(0, Qt::UserRole, eList);

        // continous
        cat = new QTreeWidgetItem({"Continuous"});
        math_root->addChild(cat);
        eList = {"Delay", "Integrator", "TransferFunction"};
        cat->setData(0, Qt::UserRole, eList);

        // control
        cat = new QTreeWidgetItem({"Control"});
        math_root->addChild(cat);
        eList = {"PIDController", "ABCtoAB0", "ABCtoDQ", "Decompose", "RMSvalue", "SwitchPath"};
        cat->setData(0, Qt::UserRole, eList);

        // display
        QTreeWidgetItem *display_cat = new QTreeWidgetItem({"Display"});
        math_root->addChild(display_cat);
        eList = {"Scope", "XYGraph"};
        display_cat->setData(0, Qt::UserRole, eList);

        // env
        QTreeWidgetItem *env_cat = new QTreeWidgetItem({"Environment"});
        math_root->addChild(env_cat);
        eList = {"Inport", "Outport", "Mux", "Demux", SYS_ID};
        env_cat->setData(0, Qt::UserRole, eList);

        // sources
        QTreeWidgetItem *src_cat = new QTreeWidgetItem({"Sources"});
        math_root->addChild(src_cat);
        eList = {"Constant", "Step", "Sinus", "Ramp", "SimulationTime", "SqeezeWave"};
        src_cat->setData(0, Qt::UserRole, eList);

        tree_items.append(math_root);


        // init Tree View
        this->addTopLevelItems(tree_items);
        setHeaderHidden(true);

        // on selection
        connect(this, &QTreeWidget::itemSelectionChanged, [this](){
            auto selection = selectedItems();
            if (selection.size() && selection[0]->childCount() == 0){
                // clear view
                //QList<QWidget*> elems = this->elemLayout->findChildren<QWidget*>(Qt::FindDirectChildrenOnly);
                QLayoutItem *item;
                while ((item = this->elemLayout->layout()->takeAt(0)) != nullptr) {
                    delete item->widget();
                    delete item; 
                }
                for (size_t i=currentLib.size(); i-->0;){
                    delete currentLib[i];
                }
                currentLib.clear();

                // refill
                auto lib = selection[0]->data(0, Qt::UserRole).toStringList();
                for (auto& elem:lib){
                    Element *e;
                    try{
                        e = makeElement(elem.toStdString());
                    }catch (exception &e){
                        cerr << e.what() << endl;
                        continue;
                    }
                    currentLib.append(e);
                    // visuals (convert to QWidget)
                    this->elemLayout->layout()->addWidget(e->getLibView());
                }
            }
        });
        
        // open first element
        setCurrentItem(first_selection);
    }
}
