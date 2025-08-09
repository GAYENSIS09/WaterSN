#include "widgets/actionsdelegate.h"
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>

MyActionsDelegate::MyActionsDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void MyActionsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionButton btnArchiver, btnTransferer;
    int btnSize = 28, spacing = 8;
    QRect r = option.rect;
    int y = r.top() + (r.height()-btnSize)/2;
    btnArchiver.rect = QRect(r.left() + spacing, y, btnSize, btnSize);
    btnArchiver.icon = QIcon(":/icons/material/archive.svg");
    btnArchiver.iconSize = QSize(20,20);
    btnArchiver.state = QStyle::State_Enabled;
    btnTransferer.rect = QRect(r.left() + btnSize + 2*spacing, y, btnSize, btnSize);
    btnTransferer.icon = QIcon(":/icons/material/transfer.svg");
    btnTransferer.iconSize = QSize(20,20);
    btnTransferer.state = QStyle::State_Enabled;
    QApplication::style()->drawControl(QStyle::CE_PushButton, &btnArchiver, painter);
    QApplication::style()->drawControl(QStyle::CE_PushButton, &btnTransferer, painter);
}

bool MyActionsDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        int btnSize = 28, spacing = 8;
        QRect r = option.rect;
        int y = r.top() + (r.height()-btnSize)/2;
        QPoint pos = mouseEvent->pos();
        QRect archiverRect(r.left() + spacing, y, btnSize, btnSize);
        QRect transfererRect(r.left() + btnSize + 2*spacing, y, btnSize, btnSize);
        if (archiverRect.contains(pos)) {
            emit emitArchiver(index);
            return true;
        } else if (transfererRect.contains(pos)) {
            emit emitTransferer(index);
            return true;
        }
    }
    return false;
}
