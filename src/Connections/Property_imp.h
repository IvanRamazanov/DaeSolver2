#include "Property.h"

using namespace std;

namespace Connections{
    template <typename T>
    Property<T>::Property(){
        onChange = nullptr;
    };

    template <typename T>
    Property<T>::Property(T value): value(value){
        onChange = nullptr;
    };

    template <typename T>
    Property<T>::Property(T value, function<void (T, T)> onChange): value(value), onChange(onChange){
        //this->value = value;
        //this->onChange = onChange;
    };

    template <typename T>
    void Property<T>::unbind(size_t idx){
        bindings.erase(bindings.begin() + idx);
    }

    template <typename T>
    void Property<T>::setValue(T newValue){
        if (newValue != value){
            T old_val = value;
            value = newValue;
            if (onChange){
                onChange(old_val, value);
            }

            // propagate value
            for (auto& b:bindings){
                auto bp = b.lock();
                if(bp) bp->setValue(newValue);
            }
        }
    }

    template <typename T>
    const T& Property<T>::getValue() const {
        return value;
    }

    template <typename T>
    bool Property<T>::isBound(){
        return bindings.size() > 0;
    }

    template <typename T>
    size_t Property<T>::getBindingsLen(){
        return bindings.size();
    }

    template <typename T>
    void Property<T>::bind(shared_ptr<Property<T>> other){
        bool found = false;
        for (auto& b:bindings){
            if (auto bp = b.lock()){
                if (bp == other){
                    found = true;
                    break;
                }
            }
        }
        if (!found){
            // not already bound, so add
            bindings.push_back(other);
            other->setValue(value);
        }
    }

    template <typename T>
    void Property<T>::bindBidirectional(shared_ptr<Property<T>> self, shared_ptr<Property<T>> other){
        self->bind(other);
        other->bind(self);
    }

    template <typename T>
    void Property<T>::unbind(){
        bindings.clear();
    }

    template <typename T>
    void Property<T>::unbind(Property<T> *other){
        for (size_t idx=bindings.size(); idx-->0;){
            if(auto bp = bindings[idx].lock()){
                if (bp.get() == other){
                    unbind(idx);
                    break;
                }
            }
        }
    }

    template <typename T>
    void Property<T>::unbindBidirectional(shared_ptr<Property<T>> other){
        unbind(other.get());
        // unbind self from other
        for (size_t idx=other->bindings.size(); idx-->0;){
            if(auto bp = other->bindings[idx].lock()){
                if (bp.get() == this){
                    bp->unbind(this);
                    break;
                }
            }
        }
    }

    template <typename T>
    void Property<T>::setOnChange(function<void (T, T)> onChange){
        this->onChange = onChange;
    };
}
