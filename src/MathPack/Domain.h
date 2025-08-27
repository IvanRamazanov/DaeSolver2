#ifndef DOMAINS_H
#define DOMAINS_H

#include <vector>
#include <string>
#include <stdexcept>
#include <QColor>
#include <QPolygon>
#include <QPainter>
#include <QRectF>
#include <QFile>
#include <QSvgRenderer>
#include "Parser.h"
#include "WorkSpace.h"

using namespace std;

namespace Domains{
    enum ConnDirection {Uni, Input, Output};

    struct Connector{
        vector<string> varNames;
        vector<WorkSpace::WsDataType> varTypes;
    };

    class Domain {
        private:
            QSvgRenderer *pinShape, // "Circle" or semicolon separated Polygon (SVG?)
                         *markerShape;
            inline static vector<Domain*> domains;

            void __init__(XmlParser::XmlElement *data);

        public:
            const bool directional; // true - domain is discrete; false - domain is 'physical'
            const string typeName; // unique name
            const string typeId; // type name for DAE compiler (could have repeats)
            const string typeTitle; // Human readable name; cosmetic
            const int varSize=1; // default size (width) of a variable in this domain (for now, only for Electric 3-phase)
            const QColor wireColour;

            /**
             * Load Domain from xml
             */
            Domain(XmlParser::XmlElement *data);
            ~Domain();

            /**
             * for default domains only
             */
            static unique_ptr<Domain> _fromId(string const& domId);

            Connector getConnector(string const& pinName);

            QSvgRenderer* getPinRenderer();
            QSvgRenderer* getMrkRenderer();

            static Domain* getDomain(string const& name);
            static const vector<Domain*>& getDomains();
    };

    ConnDirection s2dir(string const& str);
    string dir2s(ConnDirection const& dir);

    extern unique_ptr<Domain> ELECTRIC;
    extern unique_ptr<Domain> MECH_ROTATIONAL;
    extern unique_ptr<Domain> ELECTRIC_3PH;
    extern unique_ptr<Domain> MATH;
}

#endif
