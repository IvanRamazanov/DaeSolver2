#ifndef CONN_PROP_H
#define CONN_PROP_H

#include <vector>
#include <memory>
#include <algorithm>
#include <functional>

using namespace std;

namespace Connections{
    template <typename T> class Property{
        private:
            vector<weak_ptr<Property<T>>> bindings;
            T value;
            function<void (T, T)> onChange;

            

        public:
            Property();

            Property(T value);

            Property(T value, function<void (T, T)> onChange);

            void setValue(T newValue);
            const T& getValue() const;

            bool isBound();
            size_t getBindingsLen();

            void bind(shared_ptr<Property<T>> other);
            static void bindBidirectional(shared_ptr<Property<T>> self, shared_ptr<Property<T>> other);

            void unbind();
            void unbind(size_t idx);
            void unbind(Property<T> *other);
            void unbindBidirectional(shared_ptr<Property<T>> other);

            void setOnChange(function<void (T, T)> onChange);
    };
}

#include "Property_imp.h"

#endif
