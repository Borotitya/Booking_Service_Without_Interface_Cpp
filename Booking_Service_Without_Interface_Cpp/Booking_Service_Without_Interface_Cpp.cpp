#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <tuple> // 

#define IDC_BOOK_BUTTON 1001
#define IDC_CATEGORY_COMBOBOX 1002
#define IDC_FROM_DATE_PICKER 1003
#define IDC_TO_DATE_PICKER 1004
#define IDC_SHOW_TABLE_BUTTON 1005
#define IDC_SHOW_PRICES_BUTTON 1006

HINSTANCE g_hInst;

class DateSelection {
private:
    HWND hwnd_date_time_picker_;

public:
    DateSelection(HWND hwnd_parent, int x, int y) {
        hwnd_date_time_picker_ = CreateWindowEx(0, DATETIMEPICK_CLASS, NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT,
            x, y, 200, 30, hwnd_parent, NULL, NULL, NULL);
    }

    SYSTEMTIME get_system_time() {
        SYSTEMTIME st;
        SendMessage(hwnd_date_time_picker_, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
        return st;
    }

    std::wstring select_date() {
        SYSTEMTIME st = get_system_time();
        std::wstringstream ss;
        ss << std::setw(2) << std::setfill(L'0') << st.wDay << L"/"
            << std::setw(2) << std::setfill(L'0') << st.wMonth << L"/"
            << st.wYear;
        return ss.str();
    }
};

class BookingManager {
private:
    std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring>> bookings_;
    HWND hwnd_;
    HWND g_h_destination_edit;
    HWND g_h_total_cost_label;
    HWND g_h_category_combo_box;
    DateSelection* from_date_selection_;
    DateSelection* to_date_selection_;
    double total_cost_ = 0.0;

    double calculate_cost(const std::wstring& category, int days) {
        if (category == L"Отель") {
            return 30000.0 * days;
        }
        else if (category == L"Авиабилет") {
            return 20000.0 * days;
        }
        else if (category == L"Тур") {
            return 5000.0 * days;
        }
        else if (category == L"Ресторан") {
            return 1800.0 * days;
        }
        else if (category == L"Автомобиль") {
            return 2000.0 * days;
        }
        return 0.0;
    }

public:
    BookingManager(HWND hwnd) : hwnd_(hwnd), from_date_selection_(nullptr), to_date_selection_(nullptr) {}

    ~BookingManager() {
        delete from_date_selection_;
        delete to_date_selection_;
    }

    void init_ui() {
        CreateWindowEx(0, L"STATIC", L"Введите город для отдыха:", WS_CHILD | WS_VISIBLE,
            10, 10, 200, 20, hwnd_, NULL, g_hInst, NULL);

        g_h_destination_edit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE,
            10, 30, 280, 30, hwnd_, NULL, g_hInst, NULL);

        CreateWindowEx(0, L"STATIC", L"Выбор даты от:", WS_CHILD | WS_VISIBLE,
            10, 70, 200, 20, hwnd_, NULL, g_hInst, NULL);

        from_date_selection_ = new DateSelection(hwnd_, 10, 90);

        CreateWindowEx(0, L"STATIC", L"Выбор даты до:", WS_CHILD | WS_VISIBLE,
            320, 70, 200, 20, hwnd_, NULL, g_hInst, NULL);

        to_date_selection_ = new DateSelection(hwnd_, 320, 90);

        g_h_category_combo_box = CreateWindowEx(0, L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            10, 140, 200, 30, hwnd_, (HMENU)IDC_CATEGORY_COMBOBOX, g_hInst, NULL);
        SendMessage(g_h_category_combo_box, CB_ADDSTRING, 0, (LPARAM)L"Отель");
        SendMessage(g_h_category_combo_box, CB_ADDSTRING, 0, (LPARAM)L"Авиабилет");
        SendMessage(g_h_category_combo_box, CB_ADDSTRING, 0, (LPARAM)L"Тур");
        SendMessage(g_h_category_combo_box, CB_ADDSTRING, 0, (LPARAM)L"Ресторан");
        SendMessage(g_h_category_combo_box, CB_ADDSTRING, 0, (LPARAM)L"Автомобиль");

        CreateWindowA("BUTTON", "Забронировать", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 180, 120, 30, hwnd_, (HMENU)IDC_BOOK_BUTTON, g_hInst, NULL);

        CreateWindowA("BUTTON", "Показать таблицу", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            140, 180, 140, 30, hwnd_, (HMENU)IDC_SHOW_TABLE_BUTTON, g_hInst, NULL);

        CreateWindowA("BUTTON", "Показать цены", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            290, 180, 120, 30, hwnd_, (HMENU)IDC_SHOW_PRICES_BUTTON, g_hInst, NULL);

        CreateWindowExW(0, L"STATIC", L"Общая стоимость: 0.00 руб.", WS_CHILD | WS_VISIBLE,
            10, 220, 200, 30, hwnd_, NULL, g_hInst, NULL);
        g_h_total_cost_label = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
            210, 220, 200, 30, hwnd_, NULL, g_hInst, NULL);
    }

    int calculate_days(SYSTEMTIME from, SYSTEMTIME to) {
        std::tm tm_from = { 0, from.wMinute, from.wHour, from.wDay, from.wMonth - 1, from.wYear - 1900 };
        std::tm tm_to = { 0, to.wMinute, to.wHour, to.wDay, to.wMonth - 1, to.wYear - 1900 };
        auto time_from = std::mktime(&tm_from);
        auto time_to = std::mktime(&tm_to);
        int difference = std::difftime(time_to, time_from) / (60 * 60 * 24);
        return (difference > 0) ? difference : 1;
    }

    void book() {
        wchar_t destination[256];
        SendMessage(g_h_destination_edit, WM_GETTEXT, sizeof(destination), reinterpret_cast<LPARAM>(destination));
        std::wstring selected_destination = std::wstring(destination);

        int index = SendMessage(g_h_category_combo_box, CB_GETCURSEL, 0, 0);
        if (index == CB_ERR) {
            MessageBoxW(NULL, L"Пожалуйста, выберите категорию.", L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }

        std::wstring category = get_category_from_index(index);
        std::wstring from_date = from_date_selection_->select_date();
        std::wstring to_date = to_date_selection_->select_date();

        SYSTEMTIME from_system_time = from_date_selection_->get_system_time();
        SYSTEMTIME to_system_time = to_date_selection_->get_system_time();
        int days = calculate_days(from_system_time, to_system_time);

        double cost = calculate_cost(category, days);

        total_cost_ += cost;
        bookings_.push_back(std::make_tuple(category, selected_destination, from_date, to_date));

        std::wstringstream total_cost_ss;
        total_cost_ss << std::fixed << std::setprecision(2) << total_cost_;
        SetWindowTextW(g_h_total_cost_label, (L"Общая стоимость: " + total_cost_ss.str() + L" руб.").c_str());

        MessageBoxW(NULL, (category + L" забронирован для направления: " + selected_destination + L". С: " + from_date + L". По: " + to_date).c_str(),
            L"Бронирование", MB_OK | MB_ICONINFORMATION);
    }

    std::wstring get_category_from_index(int index) {
        switch (index) {
        case 0: return L"Отель";
        case 1: return L"Авиабилет";
        case 2: return L"Тур";
        case 3: return L"Ресторан";
        case 4: return L"Автомобиль";
        default: return L"";
        }
    }

    void show_table() {
        HWND hwnd_table = CreateWindowEx(WS_EX_APPWINDOW, L"BookingsTable", L"Таблица бронирований", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 500, 300, hwnd_, NULL, g_hInst, NULL);

        ShowWindow(hwnd_table, SW_SHOW);
        UpdateWindow(hwnd_table);

        std::wstringstream ss;
        for (const auto& booking : bookings_) {
            ss << std::get<0>(booking) << L": " << std::get<1>(booking) << L" с " << std::get<2>(booking) << L" по " << std::get<3>(booking) << L"\n";
        }

        HWND h_edit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", ss.str().c_str(),
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, 460, 240, hwnd_table, NULL, g_hInst, NULL);
    }

    void show_prices() {
        HWND hwnd_prices = CreateWindowEx(WS_EX_APPWINDOW, L"PricesList", L"Список цен", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, hwnd_, NULL, g_hInst, NULL);

        ShowWindow(hwnd_prices, SW_SHOW);
        UpdateWindow(hwnd_prices);

        std::wstringstream ss;
        ss << L"Отель: 30000 руб. в день\n";
        ss << L"Авиабилет: 20000 руб. в день\n";
        ss << L"Тур: 5000 руб. в день\n";
        ss << L"Ресторан: 1800 руб. в день\n";
        ss << L"Автомобиль: 2000 руб. в день\n";

        HWND h_edit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", ss.str().c_str(),
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, 260, 140, hwnd_prices, NULL, g_hInst, NULL);
    }
};

LRESULT CALLBACK window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
    static BookingManager* booking_manager_ptr = nullptr;

    switch (u_msg) {
    case WM_CREATE:
        booking_manager_ptr = new BookingManager(hwnd);
        booking_manager_ptr->init_ui();
        return 0;
    case WM_DESTROY:
        delete booking_manager_ptr;
        PostQuitMessage(0);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(w_param)) {
        case IDC_BOOK_BUTTON:
            booking_manager_ptr->book();
            break;
        case IDC_SHOW_TABLE_BUTTON:
            booking_manager_ptr->show_table();
            break;
        case IDC_SHOW_PRICES_BUTTON:
            booking_manager_ptr->show_prices();
            break;
        }
        return 0;
    }

    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}

LRESULT CALLBACK child_window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
    switch (u_msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        return 0;
    }

    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int n_cmd_show) {
    g_hInst = h_instance;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = window_proc;
    wc.hInstance = h_instance;
    wc.lpszClassName = L"TripPlannerClass";
    RegisterClass(&wc);

    wc.lpfnWndProc = child_window_proc;
    wc.lpszClassName = L"BookingsTable";
    RegisterClass(&wc);

    wc.lpszClassName = L"PricesList";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"TripPlannerClass", L"Планировщик поездок", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, h_instance, NULL);

    ShowWindow(hwnd, n_cmd_show);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
