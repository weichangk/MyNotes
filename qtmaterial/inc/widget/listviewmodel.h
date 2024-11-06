#pragma once
#include <QAbstractListModel>

namespace widget {
template <typename T>
class ListViewModel : public QAbstractListModel {
public:
    ListViewModel(QObject *parent) {
    }

    void changeModels(const QList<T> &datas) {
        beginResetModel();
        datas_ = datas;
        endResetModel();
    }

    void changeData(int row, const T &data) {
        beginResetModel();
        datas_[row] = data;
        endResetModel();
    }

    int rowCount(const QModelIndex &parent) const {
        return datas_.count();
    }

    QVariant data(const QModelIndex &index, int role) const {
        if (!index.isValid()) {
            return QVariant();
        }
        if (index.row() >= datas_.size()) {
            return QVariant();
        }
        if (role == Qt::UserRole) {
            return QVariant::fromValue(datas_[index.row()]);
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &index) const {
        if (!index.isValid())
            return Qt::NoItemFlags;
        return Qt::ItemIsEditable | QAbstractListModel::flags(index);
    }

protected:
    QList<T> datas_;
};
} // namespace widget