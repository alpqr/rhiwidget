#ifndef RHIWIDGET_H
#define RHIWIDGET_H

#include <QWidget>

class QRhiWidgetPrivate;
class QRhi;
class QRhiTexture;
class QRhiCommandBuffer;

class QRhiWidget : public QWidget
{
    Q_DECLARE_PRIVATE(QRhiWidget)
public:
    QRhiWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~QRhiWidget();

    enum Api {
        OpenGL,
        Metal,
        Vulkan,
        D3D11,
        Null
    };

    void setApi(Api api);
    void setDebugLayer(bool enable);

    virtual void initialize(QRhi *rhi, QRhiTexture *outputTexture);
    virtual void render(QRhiCommandBuffer *cb);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
};

#endif
