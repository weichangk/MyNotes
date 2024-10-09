/*
 * @Author: weick
 * @Date: 2023-12-05 22:48:39
 * @Last Modified by: weick
 * @Last Modified time: 2023-12-10 00:30:03
 */

#pragma once

template <class T>
class ASinglton {
protected:
    ASinglton() {
    }
    ~ASinglton() {
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
T *ASinglton<T>::m_ins = 0;
