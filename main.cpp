#include <QApplication>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include "examplewidget.h"

int main(int argc, char **argv)
{
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

    layout->addWidget(edit);
    layout->addWidget(slider);
    layout->addWidget(rw);

    rw->setCubeTextureText(edit->text());

    QWidget w;
    w.setLayout(layout);
    w.resize(1280, 720);
    w.show();

    return app.exec();
}
