#ifndef RHIWIDGET_H
#define RHIWIDGET_H

#include <QWidget>
#include <QtGui/private/qrhi_p.h>

class QRhiWidgetPrivate;

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
    void setTextureFormat(QRhiTexture::Format format);

    virtual void initialize(QRhi *rhi, QRhiTexture *outputTexture);
    virtual void render(QRhiCommandBuffer *cb);

    QImage grab();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
};

#endif
