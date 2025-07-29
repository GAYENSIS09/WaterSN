#ifndef LOGOWIDGET_H
#define LOGOWIDGET_H

#include <QLabel>

class LogoWidget : public QLabel {
    Q_OBJECT
public:
    explicit LogoWidget(QWidget* parent = nullptr);
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // LOGOWIDGET_H
