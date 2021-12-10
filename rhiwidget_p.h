#ifndef RHIWIDGET_P_H
#define RHIWIDGET_P_H

#include "rhiwidget.h"

#include <private/qwidget_p.h>
#include <private/qbackingstorerhisupport_p.h>

class QRhiWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QRhiWidget)
public:
    QRhiTexture *texture() const override { return textureInvalid ? nullptr : t; }
    QPlatformBackingStoreRhiConfig rhiConfig() const override;

    void ensureRhi();
    void ensureTexture();

    QRhi *rhi = nullptr;
    QRhiTexture *t = nullptr;
    bool noSize = false;
    QPlatformBackingStoreRhiConfig config;
    QRhiTexture::Format format = QRhiTexture::RGBA8;
    QSize explicitSize;
    QBackingStoreRhiSupport::RhiRenderResources offscreenRhiResources;
    bool textureInvalid = false;
};

#endif
