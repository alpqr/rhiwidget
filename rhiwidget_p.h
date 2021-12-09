#ifndef RHIWIDGET_P_H
#define RHIWIDGET_P_H

#include "rhiwidget.h"

#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#include <private/qwidget_p.h>
#include <private/qwidgetrepaintmanager_p.h>
#include <private/qbackingstorerhisupport_p.h>

class QRhiWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QRhiWidget)
public:
    QRhiTexture *texture() const override { return t; }
    QPlatformBackingStoreRhiConfig rhiConfig() const override;

    void ensureRhi();

    QRhi *rhi = nullptr;
    QRhiTexture *t = nullptr;
    bool noSize = false;
    QPlatformBackingStoreRhiConfig config;
};

#endif
