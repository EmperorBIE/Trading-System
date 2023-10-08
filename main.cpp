#include"Orderbook.h"
#include<windows.h>
#include<string>

// 窗口控件的ID
#define IDC_ASK_BUTTON 101
#define IDC_BID_BUTTON 102
#define IDC_CANCEL_BUTTON 103

HWND g_hAskPrice, g_hAskQuantity, g_hBidPrice, g_hBidQuantity, g_hId, g_hResult;

OrderBook orderBook;
size_t timestamp = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // 创建控件
        CreateWindowW(L"STATIC", L"ID:", WS_CHILD | WS_VISIBLE, 10, 10, 40, 20, hwnd, NULL, NULL, NULL);
        g_hId = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 60, 10, 100, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Ask Price:", WS_CHILD | WS_VISIBLE, 10, 40, 100, 20, hwnd, NULL, NULL, NULL);
        g_hAskPrice = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 40, 100, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Ask Quantity:", WS_CHILD | WS_VISIBLE, 10, 70, 100, 20, hwnd, NULL, NULL, NULL);
        g_hAskQuantity = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 70, 100, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Bid Price:", WS_CHILD | WS_VISIBLE, 10, 100, 100, 20, hwnd, NULL, NULL, NULL);
        g_hBidPrice = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 100, 100, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Bid Quantity:", WS_CHILD | WS_VISIBLE, 10, 130, 100, 20, hwnd, NULL, NULL, NULL);
        g_hBidQuantity = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 130, 100, 20, hwnd, NULL, NULL, NULL);

        // 创建按钮
        CreateWindowW(L"BUTTON", L"Ask", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 160, 70, 30, hwnd, (HMENU)IDC_ASK_BUTTON, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Bid", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 160, 70, 30, hwnd, (HMENU)IDC_BID_BUTTON, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 160, 70, 30, hwnd, (HMENU)IDC_CANCEL_BUTTON, NULL, NULL);

        // 结果显示框
        g_hResult = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 10, 200, 250, 30, hwnd, NULL, NULL, NULL);
    }
    break;

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_ASK_BUTTON: {
            // 获取输入的值
            timestamp++;
            wchar_t idStr[10];
            wsprintf(idStr, L"%d", timestamp);
            SetWindowText(g_hId, idStr);

            wchar_t id[10], price[10], quantity[10];
            GetWindowText(g_hId, id, 10);
            GetWindowText(g_hAskPrice, price, 10);
            GetWindowText(g_hAskQuantity, quantity, 10);

            orderBook.ask(wcstol(id, nullptr, 10), wcstol(price, nullptr, 10), wcstol(quantity, nullptr, 10));
            orderBook.print();
        }
        break;

        case IDC_BID_BUTTON: {
            // 获取输入的值
            timestamp++;
            wchar_t idStr[10];
            wsprintf(idStr, L"%d", timestamp);
            SetWindowText(g_hId, idStr);

            wchar_t id[10], price[10], quantity[10];
            GetWindowText(g_hId, id, 10);
            GetWindowText(g_hBidPrice, price, 10);
            GetWindowText(g_hBidQuantity, quantity, 10);

            orderBook.bid(wcstol(id, nullptr, 10), wcstol(price, nullptr, 10), wcstol(quantity, nullptr, 10));
            orderBook.print();
        }
        break;

        case IDC_CANCEL_BUTTON: {
            // 获取输入的值
            wchar_t id[10];
            GetWindowText(g_hId, id, 10);

            orderBook.cancel(wcstol(id, nullptr, 10));
            orderBook.print();
        }
        break;
        }
    }
    break;

    case WM_CLOSE: {
        DestroyWindow(hwnd);
    }
    break;

    case WM_DESTROY: {
        PostQuitMessage(0);
    }
    break;

    default: {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 创建主窗口
    WNDCLASSW wc = { 0 };
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInstance;
    wc.lpszClassName = L"OrderTradeApp";
    wc.lpfnWndProc = WndProc;

    RegisterClassW(&wc);
    CreateWindowW(L"OrderTradeApp", L"Order Trade App", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 300, 300, NULL, NULL, NULL, NULL);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void test() {
    OrderBook orderBook;
    orderBook.ask(1, 12, 1);
    orderBook.print();
    orderBook.ask(2, 12, 2);
    orderBook.print();
    orderBook.ask(3, 12, 3);
    orderBook.print();
    orderBook.ask(4, 12, 4);
    orderBook.print();
    orderBook.ask(5, 12, 5);
    orderBook.print();
    orderBook.ask(6, 12, 6);
    orderBook.print();
    orderBook.ask(7, 12, 7);
    orderBook.print();
    orderBook.ask(8, 12, 8);
    orderBook.print();
    orderBook.print();
    orderBook.cancel(3);
    orderBook.print();
    orderBook.cancel(4);
    orderBook.print();
    orderBook.cancel(5);
    orderBook.print();
    orderBook.cancel(6);
    orderBook.print();
    orderBook.cancel(7);
    orderBook.print();
    orderBook.ask(9, 12, 8);
    orderBook.print();
    orderBook.ask(10, 12, 8);
    orderBook.bid(11, 7, 10);
    orderBook.print();
    orderBook.bid(12, 9, 2);
    orderBook.print();
    orderBook.bid(13, 8, 3);
    orderBook.print();
    orderBook.bid(14, 8, 4);
    orderBook.print();
    orderBook.bid(15, 7, 5);
    orderBook.print();
    orderBook.bid(16, 6, 6);
    orderBook.print();
    orderBook.ask(17, 11, 5);
    orderBook.print();
    orderBook.ask(18, 13, 5);
    orderBook.print();
    orderBook.ask(19, 8, 20);
    orderBook.print();
}

int main(void) {
    WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);
}