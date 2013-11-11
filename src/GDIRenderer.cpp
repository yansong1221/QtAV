/******************************************************************************
    QtAV:  Media play library based on Qt and FFmpeg
    Copyright (C) 2012-2013 Wang Bin <wbsecg1@gmail.com>

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

#include "GDIRenderer.h"
#include "private/GDIRenderer_p.h"
#include <QResizeEvent>

//http://msdn.microsoft.com/en-us/library/ms927613.aspx
//#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
namespace QtAV {

GDIRenderer::GDIRenderer(QWidget *parent, Qt::WindowFlags f):
    QWidget(parent, f),VideoRenderer(*new GDIRendererPrivate())
{
    DPTR_INIT_PRIVATE(GDIRenderer);
    d_func().widget_holder = this;
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    /* To rapidly update custom widgets that constantly paint over their entire areas with
     * opaque content, e.g., video streaming widgets, it is better to set the widget's
     * Qt::WA_OpaquePaintEvent, avoiding any unnecessary overhead associated with repainting the
     * widget's background
     */
    setAttribute(Qt::WA_OpaquePaintEvent);
    //setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    //setDCFromPainter(false); //changeEvent will be called on show()
    setAttribute(Qt::WA_PaintOnScreen, true);
}

GDIRenderer::~GDIRenderer()
{
}

QPaintEngine* GDIRenderer::paintEngine() const
{
    return 0;
}

bool GDIRenderer::receiveFrame(const VideoFrame& frame)
{
    DPTR_D(GDIRenderer);
    QMutexLocker locker(&d.img_mutex);
    Q_UNUSED(locker);
    d.video_frame = frame;

    update();
    return true;
}

bool GDIRenderer::needUpdateBackground() const
{
    DPTR_D(const GDIRenderer);
    return (d.update_background && d.out_rect != rect()) || !d.video_frame.isValid();
}

void GDIRenderer::drawBackground()
{
    DPTR_D(GDIRenderer);
    //HDC hdc = d.device_context;
    Graphics g(d.device_context);
    SolidBrush brush(Color(255, 0, 0, 0)); //argb
    g.FillRectangle(&brush, 0, 0, width(), height());
}

void GDIRenderer::drawFrame()
{
    DPTR_D(GDIRenderer);
    /* http://msdn.microsoft.com/en-us/library/windows/desktop/ms533829%28v=vs.85%29.aspx
     * Improving Performance by Avoiding Automatic Scaling
     * TODO: How about QPainter?
     */
    //steps to use BitBlt: http://bbs.csdn.net/topics/60183502
    Bitmap bitmap(d.video_frame.width(), d.video_frame.height(), d.video_frame.bytesPerLine(0)
                  , PixelFormat32bppRGB, (BYTE*)d.video_frame.bits(0));
#if USE_GRAPHICS
    if (d.graphics)
        d.graphics->DrawImage(&bitmap, d.out_rect.x(), d.out_rect.y(), d.out_rect.width(), d.out_rect.height());
#else
    if (FAILED(bitmap.GetHBITMAP(Color(), &d.off_bitmap))) {
        qWarning("Failed GetHBITMAP");
        return;
    }
    HDC hdc = d.device_context;
    HBITMAP hbmp_old = (HBITMAP)SelectObject(d.off_dc, d.off_bitmap);
    QRect roi = realROI();
    // && image.size() != size()
    //assume that the image data is already scaled to out_size(NOT renderer size!)
    /*if (!d.scale_in_renderer || (roi.size() == d.out_rect.size())) {
        BitBlt(hdc
               , d.out_rect.left(), d.out_rect.top()
               , d.out_rect.width(), d.out_rect.height()
               , d.off_dc
               , 0, 0
               , SRCCOPY);
    } else {*/
        StretchBlt(hdc
                   , d.out_rect.left(), d.out_rect.top()
                   , d.out_rect.width(), d.out_rect.height()
                   , d.off_dc
                   , roi.x(), roi.y()
                   , roi.width(), roi.height()
                   , SRCCOPY);
    //}
    SelectObject(d.off_dc, hbmp_old);
    DeleteObject(d.off_bitmap); //avoid mem leak
#endif
    //end paint
}

void GDIRenderer::paintEvent(QPaintEvent *)
{
    handlePaintEvent();
}

void GDIRenderer::resizeEvent(QResizeEvent *e)
{
    d_func().update_background = true;
    resizeRenderer(e->size());
    update();
}

void GDIRenderer::showEvent(QShowEvent *)
{
    DPTR_D(GDIRenderer);
    d.update_background = true;
    d_func().prepare();
}

} //namespace QtAV
