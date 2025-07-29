#include <QApplication>
#include "widgets/authdialog.h"
#include "widgets/resetpassworddialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AuthDialog authDialog;
    QObject::connect(&authDialog, &AuthDialog::forgotPasswordClicked, [&]() {
        ResetPasswordDialog resetDialog;
        resetDialog.exec();
    });

    if (authDialog.exec() == QDialog::Accepted) {
        // Ici, lancer la fenêtre principale si authentification OK
        // ...
    }

    return 0;
}
