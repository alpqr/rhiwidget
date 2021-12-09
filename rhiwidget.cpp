#include "rhiwidget_p.h"

/*!
  \class QRhiWidget
  \inmodule QtWidgets
  \since 6.x

  \brief The QRhiWidget class is a widget for rendering 3D graphics via an
  accelerated grapics API, such as Vulkan, Metal, or Direct 3D.

  QRhiWidget provides functionality for displaying 3D content rendered through
  the QRhi APIs within a QWidget-based application.

  QRhiWidget is expected to be subclassed. Subclasses should reimplement the
  virtual functions initialize() and render(). In addition, they should
  configure the widget by calling setApi().
 */

/*!
    Constructs a widget which is a child of \a parent, with widget flags set to \a f.
 */
QRhiWidget::QRhiWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(*(new QRhiWidgetPrivate), parent, f)
{
    Q_D(QRhiWidget);
    if (Q_UNLIKELY(!QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::RhiBasedRendering)))
        qWarning("QRhiWidget: QRhi is not supported on this platform.");
    else
        d->setRenderToTexture();
}

/*!
    Destructor.
 */
QRhiWidget::~QRhiWidget()
{
    Q_D(QRhiWidget);
    // rhi resources must be destroyed here, cannot be left to the private dtor
    delete d->t;
}

/*!
    Handles resize events that are passed in the \a e event parameter. Calls
    the virtual function initialize().

    \note Avoid overriding this function in derived classes. If that is not
    feasible, make sure that QRhiWidget's implementation is invoked too.
    Otherwise the underlying texture object and related resources will not get
    resized properly and will lead to incorrect rendering.
 */
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

/*!
    Handles paint events.

    Calling QWidget::update() will lead to sending a paint event \a e, and thus
    invoking this function. (NB this is asynchronous and will happen at some
    point after returning from update()). This function will then, after some
    preparation, call the virtual render() to update the contents of the
    QRhiWidget's associated texture. The widget's top-level window will then
    composite the texture with the rest of the window.
 */
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

/*!
    Sets the graphics API and QRhi backend to use to \a api.

    This function must be called early enough, before the widget is added to a
    widget hierarchy and displayed on screen. For example, aim to call the
    function for the subclass constructor.
 */
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

/*!
    Requests the debug or validation layer of the underlying graphics API
    when \a enable is true.

    Applicable for Vulkan and Direct 3D.
 */
void QRhiWidget::setDebugLayer(bool enable)
{
    Q_D(QRhiWidget);
    d->config.setDebugLayer(enable);
}

/*!
    Called when the widget is initialized and every time its size changes.

    The implementation should be prepared that both \a rhi and \a outputTexture
    can change between invocations of this function, although this is not
    guaranteed to happen in practice. For example, when the widget size changes,
    it is likely that this function is called with the same \a rhi and \a
    outputTexture as before, but \a outputTexture may have been rebuilt,
    meaning its \l{QRhiTexture::pixelSize()}{size} and the underlying native
    texture resource may be different than in the last invocation.

    Implementations will typically create or rebuild a QRhiTextureRenderTarget
    in order to allow the subsequent render() call to render into the texture.
    When a depth buffer is necessary create a QRhiRenderBuffer as well. The
    size if this must follow the size of \a outputTexture. A compact and
    efficient way for this is the following:

    \code
    m_rhi = rhi;
    m_output = outputTexture;
    if (!m_ds) {
        // no depth-stencil buffer yet, create one
        m_ds = m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, m_output->pixelSize());
        m_ds->create();
    } else if (m_ds->pixelSize() != m_output->pixelSize()) {
        // the size has changed, update the size and rebuild
        m_ds->setPixelSize(m_output->pixelSize());
        m_ds->create();
    }
    if (!m_rt) {
        m_rt = m_rhi->newTextureRenderTarget({ { m_output }, m_ds });
        m_rp = m_rt->newCompatibleRenderPassDescriptor();
        m_rt->setRenderPassDescriptor(m_rp);
        m_rt->create();
    }
    \endcode

    The created resources are expected to be released in the destructor
    implementation of the subclass. \a rhi and \a outputTexture are not owned
    by, and are guaranteed to outlive the QRhiWidget.

    \sa render()
 */
void QRhiWidget::initialize(QRhi *rhi, QRhiTexture *outputTexture)
{
    Q_UNUSED(rhi);
    Q_UNUSED(outputTexture);
}

/*!
    Called when the widget contents (i.e. the contents of the texture) need
    updating.

    There is always at least one call to initialize() before this function is
    called.

    To request updates, call QWidget::update(). Calling update() from within
    render() will lead to updating continuously, throttled by vsync.

    \a cb is the QRhiCommandBuffer for the current frame of the Qt Quick
    scenegraph. The function is called with a frame being recorded, but without
    an active render pass.

    \sa initialize()
 */
void QRhiWidget::render(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
}
