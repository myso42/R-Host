/* ****************************************************************************
*
* Copyright (c) Microsoft Corporation. All rights reserved.
*
*
* This file is part of Microsoft R Host.
*
* Microsoft R Host is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Microsoft R Host is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Microsoft R Host.  If not, see <http://www.gnu.org/licenses/>.
*
* ***************************************************************************/

#include "stdafx.h"
#include "RPlotHost.h"

#define WM_ACTIVATE_PLOT (WM_USER + 100)

using namespace rplots;

HWND RPlotHost::m_hwndToolWindow = NULL;
HHOOK RPlotHost::m_hOldHook = NULL;
bool RPlotHost::m_fProcessing = false;
std::unique_ptr<RPlotHost> RPlotHost::m_pInstance = NULL;

RPlotHost::RPlotHost(HWND wndToolWindow) {
    m_hwndToolWindow = wndToolWindow;
    m_hOldHook = ::SetWindowsHookEx(WH_CBT, CBTProc, NULL, ::GetCurrentThreadId());
}

void RPlotHost::Init(HWND handle) {
    if (!m_pInstance && handle != NULL) {
        m_pInstance = std::unique_ptr<RPlotHost>(new RPlotHost(handle));
    }
}

void RPlotHost::Terminate() {
    if (m_hOldHook != NULL) {
        ::UnhookWindowsHookEx(m_hOldHook);
        m_hOldHook = NULL;

    }
    m_pInstance = NULL;
}

LRESULT CALLBACK RPlotHost::CBTProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    ) {
    if (nCode == HCBT_ACTIVATE) {
        if (!m_fProcessing) {
            m_fProcessing = true;
            HWND hwndPlot = (HWND)wParam;
            WCHAR buf[100];
            ::RealGetWindowClass(hwndPlot, buf, _countof(buf));
            if (wcscmp(buf, L"GraphApp") == 0) {
                if (m_hwndToolWindow != GetParent(hwndPlot)) {
                    HMENU hMenu = ::GetMenu(hwndPlot);

                    ::SetWindowLong(hwndPlot, GWL_STYLE, WS_CHILD);
                    ::SetWindowLong(hwndPlot, GWL_EXSTYLE, 0);
                    ::SetWindowText(hwndPlot, NULL);

                    ::SetParent(hwndPlot, m_hwndToolWindow);

                    // Resize tool window to make sure it fits plot window
                    ::ShowWindow(hwndPlot, SW_HIDE);
                    ::SetWindowPos(hwndPlot, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
                    ::PostMessage(m_hwndToolWindow, WM_ACTIVATE_PLOT, (WPARAM)hMenu, 0);
                }
            }
            m_fProcessing = false;
        }
    }

    return ::CallNextHookEx(m_hOldHook, nCode, wParam, lParam);
};
