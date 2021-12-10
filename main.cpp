#include <QApplication>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include "examplewidget.h"

int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);

    QVBoxLayout *layout = new QVBoxLayout;

    QLineEdit *edit = new QLineEdit(QLatin1String("Text on cube"));
    QSlider *slider = new QSlider(Qt::Horizontal);
    ExampleRhiWidget *rw = new ExampleRhiWidget;

    QObject::connect(edit, &QLineEdit::textChanged, edit, [edit, rw] {
        rw->setCubeTextureText(edit->text());
    });

    slider->setMinimum(0);
    slider->setMaximum(360);
    QObject::connect(slider, &QSlider::valueChanged, slider, [slider, rw] {
        rw->setCubeRotation(slider->value());
    });

    QPushButton *btn = new QPushButton(QLatin1String("Grab to image"));
    QObject::connect(btn, &QPushButton::clicked, btn, [rw] {
        QImage image = rw->grab();
        qDebug() << image;
        if (!image.isNull()) {
            QFileDialog fd(rw->parentWidget());
            fd.setAcceptMode(QFileDialog::AcceptSave);
            fd.setDefaultSuffix("png");
            fd.selectFile("test.png");
            if (fd.exec() == QDialog::Accepted)
                image.save(fd.selectedFiles().first());
        }
    });

    layout->addWidget(edit);
    layout->addWidget(slider);
    layout->addWidget(btn);
    layout->addWidget(rw);

    rw->setCubeTextureText(edit->text());

    QWidget w;
    w.setLayout(layout);
    w.resize(1280, 720);
    w.show();

    return app.exec();
}
