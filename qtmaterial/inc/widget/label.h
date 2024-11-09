#pragma once
#include "qtmaterial_global.h"

#include <QLabel>

namespace widget {
class QTMATERIAL_EXPORT VectorLabel : public QLabel {
public:
    VectorLabel(QWidget *parent = nullptr);
    ~VectorLabel() override {
    }
};
} // namespace widget