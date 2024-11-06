#pragma once

namespace core {
template <class T>
class Singlton {
protected:
    Singlton() {
    }
    ~Singlton() {
    }

public:
    static T *getInstance() {
        if (m_ins == 0) {
            m_ins = new T();
        }
        return m_ins;
    }
    static void release() {
        if (m_ins != 0) {
            delete m_ins;
            m_ins = 0;
        }
    }

private:
    static T *m_ins;
};
template <class T>
T *Singlton<T>::m_ins = 0;
} // namespace core
