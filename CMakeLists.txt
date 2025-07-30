# Copier le fichier .env dans le dossier de build à chaque génération
configure_file(${CMAKE_SOURCE_DIR}/.env ${CMAKE_BINARY_DIR}/.env COPYONLY)
# Copier le logo dans le dossier de build à chaque compilation
configure_file(${CMAKE_SOURCE_DIR}/logo_watersn.png ${CMAKE_BINARY_DIR}/logo_watersn.png COPYONLY)
# Copie automatique des QSS dans le dossier de build
configure_file(widgets/auth/authdialog.qss widgets/auth/authdialog.qss COPYONLY)
configure_file(widgets/auth/resetpassworddialog.qss widgets/auth/resetpassworddialog.qss COPYONLY)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/utils)

cmake_minimum_required(VERSION 3.16)

project(WaterSN VERSION 0.1 LANGUAGES CXX)

# Forcer le type de build à Debug par défaut si non défini
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Sql Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Sql Network)
set(PROJECT_SOURCES
    main.cpp
    resources.qrc
    watersn.cpp
    watersn.h
    watersn.ui
    # Contrôleur
    controller/controller.cpp
    include/controller/controller.h
    # Modèle
    model/model.cpp
    include/model/model.h
    # Widgets
    widgets/dashboard/dashboardwidget.cpp
    widgets/dashboard/dashboardchartwidget.cpp
    include/widgets/dashboardwidget.h
    include/widgets/dashboardchartwidget.h
    widgets/facture/facturewidget.cpp
    include/widgets/facturewidget.h
    widgets/facture/factureformdialog.cpp
    include/widgets/factureformdialog.h
    widgets/client/clientwidget.cpp
    include/widgets/clientwidget.h
    widgets/client/clientformdialog.cpp
    include/widgets/clientformdialog.h
    widgets/compteur/compteurwidget.cpp
    include/widgets/compteurwidget.h
    widgets/compteur/compteurformdialog.cpp
    include/widgets/compteurformdialog.h
    widgets/abonnement/abonnementwidget.cpp
    include/widgets/abonnementwidget.h
    widgets/historique/historiquewidget.cpp
    include/widgets/historiquewidget.h
    widgets/auth/authdialog.cpp
    include/widgets/resetpassworddialog.h
    widgets/auth/resetpassworddialog.cpp
    include/widgets/authdialog.h
    widgets/auth/rolemanagerwidget.cpp
    include/widgets/rolemanagerwidget.h
    widgets/mapwidget.cpp
    widgets/logowidget.cpp
    include/widgets/mapwidget.h
    include/widgets/logowidget.h
    # Utils
    utils/exporter.cpp
    utils/exporter.h
    utils/alerte.cpp
    utils/alerte.h
    utils/filtre.cpp
    utils/filtre.h
    utils/auth.cpp
    utils/auth.h
    utils/mail.cpp
    utils/mail.h
    utils/defaultusermanager.cpp
    utils/bcryptutils.cpp
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WaterSN
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    target_link_libraries(WaterSN PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network)
    target_include_directories(WaterSN PRIVATE
        $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Charts,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
    )
else()
    if(ANDROID)
        add_library(WaterSN SHARED
            ${PROJECT_SOURCES}
        )
        target_link_libraries(WaterSN PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts)
target_precompile_headers(WaterSN PRIVATE include/pch.h)
    else()
        add_executable(WaterSN
            ${PROJECT_SOURCES}
        )
        target_link_libraries(WaterSN PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts)
    endif()
endif()

 # déjà fait plus haut

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WaterSN)
endif()
set_target_properties(WaterSN PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WaterSN
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WaterSN)
endif()
