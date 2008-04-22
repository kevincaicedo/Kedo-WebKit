/*
    Copyright (C) 2007 Trolltech ASA
    Copyright (C) 2007 Staikos Computing Services Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#ifndef QWEBFRAME_H
#define QWEBFRAME_H

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtGui/qicon.h>
#if QT_VERSION >= 0x040400
#include <QtNetwork/qnetworkaccessmanager.h>
#endif
#include "qwebkitglobal.h"

QT_BEGIN_NAMESPACE
class QRect;
class QPoint;
class QPainter;
class QPixmap;
class QMouseEvent;
class QWheelEvent;
class QNetworkRequest;
class QRegion;
class QPrinter;
QT_END_NAMESPACE

class QWebNetworkRequest;
class QWebFramePrivate;
class QWebPage;

namespace WebCore {
    class WidgetPrivate;
    class FrameLoaderClientQt;
    class ChromeClientQt;
}
class QWebFrameData;

class QWEBKIT_EXPORT QWebFrame : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal textSizeMultiplier READ textSizeMultiplier WRITE setTextSizeMultiplier)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QIcon icon READ icon)
private:
    QWebFrame(QWebPage *parent, QWebFrameData *frameData);
    QWebFrame(QWebFrame *parent, QWebFrameData *frameData);
    ~QWebFrame();

public:
    QWebPage *page() const;

    void load(const QUrl &url);
#if QT_VERSION < 0x040400
    void load(const QWebNetworkRequest &request);
#else
    void load(const QNetworkRequest &request,
              QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation,
              const QByteArray &body = QByteArray());
#endif
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void setHtml(const QByteArray &html, const QUrl &baseUrl = QUrl());
    void setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());

    void addToJavaScriptWindowObject(const QString &name, QObject *object);
    QString toHtml() const;
    QString toPlainText() const;
    QString renderTreeDump() const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QIcon icon() const;

    QString frameName() const;

    QWebFrame *parentFrame() const;
    QList<QWebFrame*> childFrames() const;

    Qt::ScrollBarPolicy scrollBarPolicy(Qt::Orientation orientation) const;
    void setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy);

    void setScrollBarValue(Qt::Orientation orientation, int value);
    int scrollBarValue(Qt::Orientation orientation) const;
    int scrollBarMinimum(Qt::Orientation orientation) const;
    int scrollBarMaximum(Qt::Orientation orientation) const;

    void render(QPainter *painter, const QRegion &clip);
    void render(QPainter *painter);

    void setTextSizeMultiplier(qreal factor);
    qreal textSizeMultiplier() const;

    QPoint pos() const;
    QRect geometry() const;

    virtual bool event(QEvent *);

public Q_SLOTS:
    QVariant evaluateJavaScript(const QString& scriptSource);
    void print(QPrinter *printer) const;

Q_SIGNALS:
    void javaScriptWindowObjectCleared();
    void loadDone(bool ok);
    void provisionalLoad();
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);

    void loadStarted();
    void loadFinished();

    void initialLayoutCompleted();

    void iconChanged();

private:
    friend class QWebPage;
    friend class QWebPagePrivate;
    friend class QWebFramePrivate;
    friend class WebCore::WidgetPrivate;
    friend class WebCore::FrameLoaderClientQt;
    friend class WebCore::ChromeClientQt;
    QWebFramePrivate *d;
};

#endif
