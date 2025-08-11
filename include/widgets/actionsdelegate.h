#ifndef ACTIONSDELEGATE_H
#define ACTIONSDELEGATE_H

#include <QStyledItemDelegate>
#include <QModelIndex>

class MyActionsDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit MyActionsDelegate(QWidget* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
signals:
    void emitArchiver(const QModelIndex& index) ;
    void emitTransferer(const QModelIndex& index) ;
};

#endif // ACTIONSDELEGATE_H
