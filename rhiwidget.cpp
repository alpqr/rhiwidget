#include "rhiwidget_p.h"

QRhiWidget::QRhiWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(*(new QRhiWidgetPrivate), parent, f)
{
    Q_D(QRhiWidget);
    if (Q_UNLIKELY(!QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::RhiBasedRendering)))
        qWarning("QRhiWidget: QRhi is not supported on this platform.");
    else
        d->setRenderToTexture();
}

QRhiWidget::~QRhiWidget()
{
    Q_D(QRhiWidget);
    // rhi resources must be destroyed here, cannot be left to the private dtor
    delete d->t;
}

void QRhiWidget::resizeEvent(QResizeEvent *e)
{
    Q_D(QRhiWidget);

    if (e->size().isEmpty()) {
        d->noSize = true;
        return;
    }
    d->noSize = false;

    d->ensureRhi();
    if (!d->rhi)
        return;

    const QSize newSize = size() * devicePixelRatio();
    if (!d->t) {
        d->t = d->rhi->newTexture(QRhiTexture::RGBA8, newSize, 1, QRhiTexture::RenderTarget);
        if (!d->t->create())
            qWarning("Failed to create backing texture for QRhiWidget");
    }
    if (d->t->pixelSize() != newSize) {
        d->t->setPixelSize(newSize);
        d->t->create();
    }

    initialize(d->rhi, d->t);

    d->sendPaintEvent(QRect(QPoint(0, 0), size()));
}

void QRhiWidget::paintEvent(QPaintEvent *)
{
    Q_D(QRhiWidget);
    if (updatesEnabled() && d->rhi && !d->noSize) {
        QRhiCommandBuffer *cb = nullptr;
        d->rhi->beginOffscreenFrame(&cb);
        render(cb);
        d->rhi->endOffscreenFrame();
    }
}

QPlatformBackingStoreRhiConfig QRhiWidgetPrivate::rhiConfig() const
{
    return config;
}

void QRhiWidgetPrivate::ensureRhi()
{
    if (rhi)
        return;

    Q_Q(QRhiWidget);
    // the QRhi and infrastructure belongs to the top-level widget, not to this widget
    QWidget *tlw = q->window();
    QWidgetPrivate *wd = get(tlw);

    if (QWidgetRepaintManager *repaintManager = wd->maybeRepaintManager())
        rhi = repaintManager->rhi();

    // a real widget would need to check this too, in case another
    // render-to-texture widget has already set things up with another API
    if (rhi && rhi->backend() != QBackingStoreRhiSupport::apiToRhiBackend(config.api())) {
        qWarning("The top-level window is already using another graphics API for composition, "
                 "'%s' is not compatible with this widget",
                 rhi->backendName());
        return;
    }

    if (!rhi)
        qWarning("No QRhi");
}

void QRhiWidget::setApi(Api api)
{
    Q_D(QRhiWidget);
    d->config.setEnabled(true);
    switch (api) {
    case OpenGL:
        d->config.setApi(QPlatformBackingStoreRhiConfig::OpenGL);
        break;
    case Metal:
        d->config.setApi(QPlatformBackingStoreRhiConfig::Metal);
        break;
    case Vulkan:
        d->config.setApi(QPlatformBackingStoreRhiConfig::Vulkan);
        break;
    case D3D11:
        d->config.setApi(QPlatformBackingStoreRhiConfig::D3D11);
        break;
    default:
        d->config.setApi(QPlatformBackingStoreRhiConfig::Null);
        break;
    }
}

void QRhiWidget::setDebugLayer(bool enable)
{
    Q_D(QRhiWidget);
    d->config.setDebugLayer(enable);
}

void QRhiWidget::initialize(QRhi *rhi, QRhiTexture *outputTexture)
{
    Q_UNUSED(rhi);
    Q_UNUSED(outputTexture);
}

void QRhiWidget::render(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
}
