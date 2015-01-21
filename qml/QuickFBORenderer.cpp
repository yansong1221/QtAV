/******************************************************************************
    QtAV:  Media play library based on Qt and FFmpeg
    Copyright (C) 2015 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#include "QmlAV/QuickFBORenderer.h"
#include "QmlAV/QmlAVPlayer.h"
#include "QtAV/AVPlayer.h"
#include "QtAV/FactoryDefine.h"
#include "QtAV/OpenGLVideo.h"
#include "QtAV/private/VideoRenderer_p.h"
#include "QtAV/private/mkid.h"
#include "QtAV/private/prepost.h"
#include <QtGui/QOpenGLFramebufferObject>
// check glv.ctx and set it in Renderer::render()

namespace QtAV {
VideoRendererId VideoRendererId_QuickFBO = mkid::id32base36_4<'Q','F','B','O'>::value;
FACTORY_REGISTER_ID_AUTO(VideoRenderer, QuickFBO, "QuickFBO");

class FBORenderer : public QQuickFramebufferObject::Renderer
{
public:
    FBORenderer(QuickFBORenderer* item) : m_item(item) {}
    QOpenGLFramebufferObject* createFramebufferObject(const QSize &size) {
        m_item->fboSizeChanged(size);
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }
    void render() {
        Q_ASSERT(m_item);
        m_item->renderToFbo();
    }
    void synchronize(QQuickFramebufferObject *item) {
        m_item = static_cast<QuickFBORenderer*>(item);
    }
private:
    QuickFBORenderer *m_item;
};

class QuickFBORendererPrivate : public VideoRendererPrivate
{
public:
    QuickFBORendererPrivate():
        VideoRendererPrivate()
      , opengl(true)
      , frame_changed(false)
      , fill_mode(QuickFBORenderer::PreserveAspectFit)
      , node(0)
      , source(0)
      , glctx(0)
    {}
    void setupAspectRatio() {
        matrix.setToIdentity();
        matrix.scale((GLfloat)out_rect.width()/(GLfloat)renderer_width, (GLfloat)out_rect.height()/(GLfloat)renderer_height, 1);
        if (orientation)
            matrix.rotate(orientation, 0, 0, 1); // Z axis
    }
    bool opengl;
    bool frame_changed;
    QuickFBORenderer::FillMode fill_mode;
    QSGNode *node;
    QObject *source;
    QOpenGLContext *glctx;
    QMatrix4x4 matrix;
    OpenGLVideo glv;
};

QuickFBORenderer::QuickFBORenderer(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
    , VideoRenderer(*new QuickFBORendererPrivate())
{
}

VideoRendererId QuickFBORenderer::id() const
{
    return VideoRendererId_QuickFBO;
}

QQuickFramebufferObject::Renderer* QuickFBORenderer::createRenderer() const
{
    return new FBORenderer((QuickFBORenderer*)this);
}

bool QuickFBORenderer::isSupported(VideoFormat::PixelFormat pixfmt) const
{
    if (!isOpenGL())
        return VideoFormat::isRGB(pixfmt);
    return pixfmt != VideoFormat::Format_YUYV && pixfmt != VideoFormat::Format_UYVY;
}

void QuickFBORenderer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry); //geometry will be updated
    DPTR_D(QuickFBORenderer);
    resizeRenderer(newGeometry.size().toSize());
    if (d.fill_mode == PreserveAspectCrop) {
        QSizeF scaled = d.out_rect.size();
        scaled.scale(boundingRect().size(), Qt::KeepAspectRatioByExpanding);
        d.out_rect = QRect(QPoint(), scaled.toSize());
        d.out_rect.moveCenter(boundingRect().center().toPoint());
    }
}

bool QuickFBORenderer::receiveFrame(const VideoFrame &frame)
{
    DPTR_D(QuickFBORenderer);
    d.video_frame = frame;
    d.frame_changed = true;
    d.glv.setCurrentFrame(frame);
//    update();  // why update slow? because of calling in a different thread?
    //QMetaObject::invokeMethod(this, "update"); // slower than directly postEvent
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    return true;
}

QObject* QuickFBORenderer::source() const
{
    return d_func().source;
}

void QuickFBORenderer::setSource(QObject *source)
{
    DPTR_D(QuickFBORenderer);
    if (d.source == source)
        return;
    d.source = source;
    ((QmlAVPlayer*)source)->player()->addVideoRenderer(this);
    emit sourceChanged();
}

QuickFBORenderer::FillMode QuickFBORenderer::fillMode() const
{
    return d_func().fill_mode;
}

void QuickFBORenderer::setFillMode(FillMode mode)
{
    DPTR_D(QuickFBORenderer);
    if (d.fill_mode == mode)
        return;
    d_func().fill_mode = mode;
    if (d.fill_mode == Stretch) {
        setOutAspectRatioMode(RendererAspectRatio);
    } else {//compute out_rect fits video aspect ratio then compute again if crop
        setOutAspectRatioMode(VideoAspectRatio);
    }
    //m_geometryDirty = true;
    //update();
    emit fillModeChanged(mode);
}

bool QuickFBORenderer::isOpenGL() const
{
    return d_func().opengl;
}

void QuickFBORenderer::setOpenGL(bool o)
{
    DPTR_D(QuickFBORenderer);
    if (d.opengl == o)
        return;
    d.opengl = o;
    emit openGLChanged();
    if (o)
        setPreferredPixelFormat(VideoFormat::Format_YUV420P);
    else
        setPreferredPixelFormat(VideoFormat::Format_RGB32);
}

void QuickFBORenderer::fboSizeChanged(const QSize &size)
{
    resizeRenderer(size);
    d_func().glv.setProjectionMatrixToRect(QRectF(0, 0, size.width(), size.height()));
}

void QuickFBORenderer::renderToFbo()
{
    handlePaintEvent();
}

bool QuickFBORenderer::needUpdateBackground() const
{
    DPTR_D(const QuickFBORenderer);
    return d.out_rect != boundingRect().toRect();
}

bool QuickFBORenderer::needDrawFrame() const
{
    return true; //always call updatePaintNode, node must be set
}

void QuickFBORenderer::drawFrame()
{
    DPTR_D(QuickFBORenderer);
    if (d.glctx != QOpenGLContext::currentContext()) {
        d.glctx = QOpenGLContext::currentContext();
        d.glv.setOpenGLContext(d.glctx);
    }
    //d.glv.setCurrentFrame(d.video_frame);
    d.glv.render(d.out_rect, normalizedROI(), d.matrix);
    d.frame_changed = false;
}


bool QuickFBORenderer::event(QEvent *e)
{
    if (e->type() != QEvent::User)
        return QQuickItem::event(e);
    update();
    return true;
}

bool QuickFBORenderer::onSetRegionOfInterest(const QRectF &roi)
{
    Q_UNUSED(roi);
    emit regionOfInterestChanged();
    return true;
}

bool QuickFBORenderer::onSetOrientation(int value)
{
    Q_UNUSED(value);
    emit orientationChanged();
    d_func().setupAspectRatio();
    return true;
}

} //namespace QtAV
