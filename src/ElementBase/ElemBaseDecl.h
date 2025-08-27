#ifndef EBASE_DECLARATIONS_H
#define EBASE_DECLARATIONS_H

#include <QString>

namespace ElementBase{
    class Element;
    class Pin;
    class Subsystem;

    enum ParamType {Matrix=0, Vector, Scalar, ComboBox};

    constexpr auto SYS_ID = "System";
    constexpr auto ELEM_TAG = "Elem";
    constexpr auto ELEM_LAYOUT_TAG = "Layout";
    constexpr auto ELEM_PARAMS_TAG = "Parameters";
    constexpr auto ELEM_INITS_TAG = "InitParameters";
    constexpr auto PIN_TAG = "pin";
    constexpr auto PARAM_TAG = "parm";
    constexpr auto CONN_TAG = "Connections";
    constexpr auto ATTR_ID = "id";
    constexpr auto ATTR_ELEM_VIEW = "view";
    constexpr auto ATTR_ELEM_NAME = "name";
    constexpr auto ATTR_ELEM_DESCR = "description";
    constexpr auto ATTR_PARAM_NAME = "name";
    constexpr auto ATTR_PARAM_TITLE = "title";
    constexpr auto ATTR_PARAM_TYPE = "type";
    constexpr auto ATTR_INIT_PRIOR = "priority";
    constexpr auto ELEM_LAYOUT_FORMAT = "{},{},{}";
    extern const QString CUSTOM_FORMAT; // defined in Element.cpp
    constexpr auto ELEM_PATH_DELIM = "::";
    constexpr auto PIN_CACHE_VAR_SUFFIX = "_old";
}

namespace Connections{
    class WireCluster;
    class LineMarker;
    class ConnectionLine;
}

#endif
