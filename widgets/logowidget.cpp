#include "widgets/logowidget.h"
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>

LogoWidget::LogoWidget(QWidget* parent)
    : QLabel(parent)
{
    setFixedSize(140, 140); // Taille augmentée pour meilleure visibilité
    setAlignment(Qt::AlignCenter);
    setScaledContents(true);
    QStringList logoPaths = {
        QDir::current().filePath("logo_watersn.png"),
        QCoreApplication::applicationDirPath() + "/logo_watersn.png",
        QDir::current().absoluteFilePath("logo_watersn.png"),
        QCoreApplication::applicationDirPath() + "/../logo_watersn.png"
    };
    QPixmap logoPixmap;
    QString foundPath;
    for (const QString& path : logoPaths) {
        if (QFile::exists(path)) {
            logoPixmap.load(path);
            foundPath = path;
            break;
        }
    }
    qDebug() << "[LogoWidget] Chemins testés pour le logo:" << logoPaths;
    if (!logoPixmap.isNull()) {
        qDebug() << "[LogoWidget] Logo chargé depuis:" << foundPath;
        setPixmap(logoPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        qWarning("[LogoWidget] Aucun logo trouvé. Chemins testés: %s", qPrintable(logoPaths.join(", ")));
        setText("[Logo]");
    }

    // Positionnement dynamique en haut à droite du parent via event filter
    if (parentWidget()) {
        int margin = 24;
        auto parent = parentWidget();
        parent->installEventFilter(this);
        // Position initiale
        move(parent->width() - width() - margin, margin);
        raise();
        show();
    }
}

// Event filter pour suivre le resize du parent
bool LogoWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == parentWidget() && event->type() == QEvent::Resize) {
        int margin = 24;
        QWidget* parent = parentWidget();
        move(parent->width() - width() - margin, margin);
        return false;
    }
    return QLabel::eventFilter(obj, event);
}
