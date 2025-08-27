/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#ifndef EBASE_ELEM_CATALOGUE_H
#define EBASE_ELEM_CATALOGUE_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFile>
#include <QDirIterator>
#include <iostream>
#include "Element.h"

//#include <QDebug>


namespace ElementBase{
    /**
     *
     * @author Ivan
     */
    class ElementCatalogue : public QTreeWidget {
        QWidget *elemLayout;
        QList<Element*> currentLib; // to take memory ownership

        public:
            ElementCatalogue(QWidget *elemLayout, QWidget *parent=nullptr);

            virtual ~ElementCatalogue() noexcept = default;
    };
}

#endif
