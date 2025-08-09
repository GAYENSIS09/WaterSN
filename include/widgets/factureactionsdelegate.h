#pragma once
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

class FactureActionsDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    FactureActionsDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        painter->save();
        QRect r = option.rect;
        int iconSize = 24;
        int spacing = 10;
        int y = r.top() + (r.height() - iconSize) / 2;
        int x = r.left() + spacing;
        // Définir les icônes Unicode
        QStringList icons = {"💳", "📄", "🗑️"};
        for (int i = 0; i < icons.size(); ++i) {
            QRect iconRect(x, y, iconSize, iconSize);
            painter->setFont(QFont("Arial", 18));
            painter->drawText(iconRect, Qt::AlignCenter, icons[i]);
            x += iconSize + spacing;
        }
        painter->restore();
    }

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QRect r = option.rect;
            int iconSize = 24;
            int spacing = 10;
            int y = r.top() + (r.height() - iconSize) / 2;
            int x = r.left() + spacing;
            QPoint pos = mouseEvent->pos();
            for (int i = 0; i < 3; ++i) {
                QRect iconRect(x, y, iconSize, iconSize);
                if (iconRect.contains(pos)) {
                    emit actionClicked(index.row(), i);
                    break;
                }
                x += iconSize + spacing;
            }
        }
        return false;
    }
signals:
    void actionClicked(int row, int actionIndex) const;
};
