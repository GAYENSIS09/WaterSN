#ifndef EXPORTER_H
#define EXPORTER_H

#include <QString>

class Exporter {
public:
    static bool exportToPDF(const QString& filePath);
    static bool exportToExcel(const QString& filePath);
};

#endif // EXPORTER_H
