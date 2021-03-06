/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "styledwindow.h"
#include "logger.h"
#include "titlebar.h"

#include <QApplication>
#include <QEvent>
#include <QVBoxLayout>
#include <QWindowStateChangeEvent>

#ifdef _WIN32
#include <Windows.h>
#include <windowsx.h>
#endif

template <class T> StyledWidget<T>::StyledWidget(QWidget * parent) : T(parent)
{
    T::setWindowFlag(Qt::Window);
    T::setObjectName("mainWindow");
    bgLayout_ = new QVBoxLayout;
    bgLayout_->setMargin(1);
    QWidget * bg = new QWidget;
    bg->setContentsMargins(0, 0, 0, 0);
    bg->setAutoFillBackground(true);
    bgLayout_->addWidget(bg);
    layout_ = new QVBoxLayout;
    layout_->setSpacing(0);

#ifdef _WIN32
    layout_->setMargin(0);
    titleBar_ = new TitleBar(this);
    titleBar_->setTitle("Test");

    layout_->addWidget(titleBar_);

    T::setWindowFlags(T::windowFlags() | Qt::Window | Qt::FramelessWindowHint |
                      Qt::WindowSystemMenuHint);
#else
    layout_->setMargin(0);
    bgLayout_->setMargin(0);
    // setWindowFlags(Qt::FramelessWindowHint);
#endif

    bg->setLayout(layout_);
    T::setLayout(bgLayout_);
    setResizable(true);
}

template <class T> void StyledWidget<T>::setResizable(bool resizable)
{
    resizable_ = resizable;
    if (resizable)
    {
        T::setWindowFlags(T::windowFlags() | Qt::WindowMaximizeButtonHint);

#ifdef _WIN32
        HWND hwnd = reinterpret_cast<HWND>(T::winId());
        DWORD style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE,
                      style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
#endif
    }
    else
    {
        T::setWindowFlags(T::windowFlags() & ~Qt::WindowMaximizeButtonHint);

#ifdef _WIN32
        HWND hwnd = reinterpret_cast<HWND>(T::winId());
        DWORD style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
#endif
    }
}

#ifdef _WIN32
namespace
{
void fix_maximized_window(HWND window, int maxWidth, int maxHeight, RECT & rect)
{
    auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
    if (!monitor)
    {
        return;
    }

    MONITORINFO monitor_info{};
    monitor_info.cbSize = sizeof(monitor_info);
    if (!GetMonitorInfoW(monitor, &monitor_info))
    {
        return;
    }

    RECT r = monitor_info.rcWork;
    auto width = r.right - r.left;
    auto height = r.bottom - r.top;
    if (width > maxWidth || height > maxHeight)
    {
        return;
    }
    rect = r;
}
} // namespace
#endif

#ifdef _WIN32
template <class T>
bool StyledWidget<T>::nativeEvent(const QByteArray & eventType, void * message,
                                  long * result)
{
    Q_UNUSED(eventType);
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG * msg = *reinterpret_cast<MSG **>(message);
#else
    MSG * msg = reinterpret_cast<MSG *>(message);
#endif

    switch (msg->message)
    {
    case WM_NCCALCSIZE:
    {
        if (msg->wParam == TRUE)
        {
            auto & params = *reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);

            if (IsZoomed(msg->hwnd))
            {
                // Maximized
                fix_maximized_window(msg->hwnd, T::maximumWidth(),
                                     T::maximumHeight(), params.rgrc[0]);
            }
        }
        *result = 0;
        return true;
    }
    case WM_NCHITTEST:
    {
        static const QMargins margins(5, 5, 5, 5);
        RECT winrect;
        GetWindowRect(reinterpret_cast<HWND>(T::winId()), &winrect);

        QPoint topLeft(winrect.left, winrect.top);
        QPoint bottomRight(winrect.right, winrect.bottom);
        QRect rect = QRect(topLeft, bottomRight);

        QRect leftBorder(rect.topLeft(),
                         rect.bottomLeft() + QPoint(margins.left(), 0));
        QRect topBorder(rect.topLeft(),
                        rect.topRight() + QPoint(0, margins.top()));
        QRect rightBorder(rect.topRight() - QPoint(margins.right(), 0),
                          rect.bottomRight());
        QRect bottomBorder(rect.bottomLeft() - QPoint(0, margins.bottom()),
                           rect.bottomRight());

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        bool resizeWidth = T::minimumWidth() != T::maximumWidth();
        bool resizeHeight = T::minimumHeight() != T::maximumHeight();

        *result = 0;
        if (resizeWidth)
        {
            if (resizeHeight)
            {
                // bottom left corner
                if (leftBorder.contains(x, y) && bottomBorder.contains(x, y))
                {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                // bottom right corner
                if (bottomBorder.contains(x, y) && rightBorder.contains(x, y))
                {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                // top left corner
                if (leftBorder.contains(x, y) && topBorder.contains(x, y))
                {
                    *result = HTTOPLEFT;
                    return true;
                }
                // top right corner
                if (rightBorder.contains(x, y) && topBorder.contains(x, y))
                {
                    *result = HTTOPRIGHT;
                    return true;
                }
            }
            if (leftBorder.contains(x, y))
            {
                // Left border
                *result = HTLEFT;
                return true;
            }
            if (rightBorder.contains(x, y))
            {
                // Right border
                *result = HTRIGHT;
                return true;
            }
        }
        if (resizeHeight)
        {
            if (bottomBorder.contains(x, y))
            {
                // Bottom border
                *result = HTBOTTOM;
                return true;
            }
            if (topBorder.contains(x, y))
            {
                // Top border
                *result = HTTOP;
                return true;
            }
        }

        if (QApplication::widgetAt(QCursor::pos()) == titleBar_->title_)
        {
            *result = HTCAPTION;
            return true;
        }
        // Otherwise, let QWidget handle it.
        break;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO * mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);

        mmi->ptMinTrackSize.x = T::minimumWidth();
        mmi->ptMinTrackSize.y = T::minimumHeight();
        mmi->ptMaxTrackSize.x = T::maximumWidth();
        mmi->ptMaxTrackSize.y = T::maximumHeight();

        *result = 1;
        return true;
        break;
    }

    case WM_SIZE:
    {
        RECT winrect;
        GetClientRect(msg->hwnd, &winrect);

        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(msg->hwnd, &wp);
        if (wp.showCmd == SW_MAXIMIZE)
        {

            // SetWindowPos(reinterpret_cast<HWND>(winId()), nullptr, 0, 0, 0,
            // 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
        }
        break;
    }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

template <class T> void StyledWidget<T>::changeEvent(QEvent * e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent * ev =
            static_cast<QWindowStateChangeEvent *>(e);
        if ((T::windowState() && Qt::WindowMaximized) && !resizable_)
        {
            T::setWindowState(Qt::WindowNoState);
        }
        bool maximized = !(ev->oldState() & Qt::WindowMaximized) &&
                         (T::windowState() & Qt::WindowMaximized);
        titleBar_->setMaximized(maximized);
        bgLayout_->setMargin(maximized ? 0 : 1);
    }

    QWidget::changeEvent(e);
}
#endif

StyledWindow::StyledWindow(QWidget * parent) : StyledWidget<QWidget>(parent) {}

StyledDialog::StyledDialog(QWidget * parent) : StyledWidget<QDialog>(parent)
{
    // titleBar_->setMaximizable(false);
    // titleBar_->setMinimizable(false);
    // setResizable(false);
}

template <class T>
IntermediateWidget<T>::IntermediateWidget(QWidget * parent)
    : parent_(new StyledWidget<QWidget>(parent))
{
    parent_->mainLayout()->addWidget(this);
    T::installEventFilter(parent_);
    parent_->show();
}

template <class T> void IntermediateWidget<T>::showEvent(QShowEvent * event)
{
    Logger::debug("show event");
}

template <class T> IntermediateWidget<T>::~IntermediateWidget()
{
    // delete parent_;
}

template <class T>
bool StyledWidget<T>::eventFilter(QObject * object, QEvent * event)
{
    Logger::debug(std::to_string(static_cast<int>(event->type())));
    if (event->type() == QEvent::Show)
    {
        T::show();
    }
    return false;
}

StyledMainWindow::StyledMainWindow(QWidget * parent)
    : StyledWidget<QMainWindow>(parent)
{
}
