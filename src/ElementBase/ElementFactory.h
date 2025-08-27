#ifndef EBASE_ELEM_FACTORY_H
#define EBASE_ELEM_FACTORY_H

#include "Element.h"
#include "../MathPack/Parser.h"


namespace ElementBase{
    Element* makeElement(std::string const& id) noexcept;

    Element* makeElement(XmlParser::XmlElement *elemInfo) noexcept;
}

#endif
