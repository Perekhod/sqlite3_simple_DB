#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <sqlite3.h>

// Функция для создания таблицы сотрудников
void createEmployeeTable(sqlite3* db) {
    std::string query = "CREATE TABLE IF NOT EXISTS employees ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "last_name TEXT NOT NULL,"
        "first_name TEXT NOT NULL,"
        "middle_name TEXT,"
        "department TEXT NOT NULL,"
        "position TEXT NOT NULL,"
        "vacation_month TEXT NOT NULL"
        ");";

    int rc = sqlite3_exec(db, query.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка при создании таблицы: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);   // Закрытие базы данных перед завершением программы
        exit(EXIT_FAILURE);  // Завершение программы с кодом ошибки
    }
}

// Функция проверки корректности месяца
int getValidMonthInput() {
    int month;
    do {
        std::cout << "Введите месяц отпуска (число от 1 до 12): ";
        std::cin >> month;

        if (std::cin.fail() || month < 1 || month > 12) {
            std::cin.clear (); // Сброс флага ошибки
            std::cin.ignore(); // Очистка буфера ввода
            std::cerr << "Некорректный ввод. Пожалуйста, введите число от 1 до 12.\n";
        }
        else {
            break;
        }
    } while (true);

    return month;
}

// Функция для добавления сотрудника в базу данных
void addEmployee(sqlite3* db, const std::string& lastName, const std::string& firstName,
    const std::string& middleName, const std::string& department,
    const std::string& position, std::string& vacationMonth) {
    int month;
    do {
        month = getValidMonthInput();
    } while (month < 1 || month > 12);

    vacationMonth = std::to_string(month);

    std::string query = "INSERT INTO employees (last_name, first_name, middle_name, department, position, vacation_month) "
        "VALUES ('" + lastName + "', '" + firstName + "', '" + middleName + "', '"
        + department + "', '" + position + "', '" + vacationMonth + "');";

    int rc = sqlite3_exec(db, query.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка при добавлении сотрудника: " << sqlite3_errmsg(db) << std::endl;
    }
}

// Функция для удаления сотрудника из базы данных
void deleteEmployee(sqlite3* db, int employeeId) {
    std::string query = "SELECT * FROM employees WHERE id = " + std::to_string(employeeId) + ";";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);

    if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Вы уверены, что хотите удалить следующего сотрудника?\n";
        std::cout << "ID: " << sqlite3_column_int(stmt, 0)          << ", "
            << "Фамилия: " << sqlite3_column_text(stmt, 1)          << ", "
            << "Имя: " << sqlite3_column_text(stmt, 2)              << ", "
            << "Отчество: " << sqlite3_column_text(stmt, 3)         << ", "
            << "Отдел: " << sqlite3_column_text(stmt, 4)            << ", "
            << "Должность: " << sqlite3_column_text(stmt, 5)        << ", "
            << "Месяц отпуска: " << sqlite3_column_text(stmt, 6)    << std::endl;

        char confirm;
        std::cout << "Введите 'y' для подтверждения удаления или любой другой символ для отмены: ";
        std::cin >> confirm;

        if (confirm == 'y' || confirm == 'Y') {
            query = "DELETE FROM employees WHERE id = " + std::to_string(employeeId) + ";";

            rc = sqlite3_exec(db, query.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Ошибка при удалении сотрудника: " << sqlite3_errmsg(db) << std::endl;
            }
            else {
                std::cout << "Сотрудник успешно удален.\n";
            }
        }
        else {
            std::cout << "Удаление отменено.\n";
        }
    }
    else {
        std::cerr << "Ошибка при выполнении запроса: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void printEmployeeList(sqlite3_stmt* stmt) {
    std::cout << std::left << std::setw(5) << "ID" << std::setw(15) << "Фамилия" << std::setw(15) << "Имя"
        << std::setw(15) << "Отчество" << std::setw(15) << "Отдел" << std::setw(15) << "Должность"
        << std::setw(15) << "Месяц отпуска" << std::endl;
    std::cout << std::setfill('-') << std::setw(100) << "-" << std::setfill(' ') << std::endl;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << std::left << std::setw(5) << sqlite3_column_int(stmt, 0) << std::setw(15) << sqlite3_column_text(stmt, 1)
            << std::setw(15) << sqlite3_column_text(stmt, 2) << std::setw(15) << sqlite3_column_text(stmt, 3)
            << std::setw(15) << sqlite3_column_text(stmt, 4) << std::setw(15) << sqlite3_column_text(stmt, 5)
            << std::setw(15) << sqlite3_column_text(stmt, 6) << std::endl;
    }
}

// Функция для вывода списка всех сотрудников
void showEmployeeList(sqlite3* db) {
    std::string query = "SELECT * FROM employees;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);

    if (rc == SQLITE_OK) {
        printEmployeeList(stmt);
        sqlite3_finalize(stmt);
    }
    else {
        std::cerr << "Ошибка при выполнении запроса: " << sqlite3_errmsg(db) << std::endl;
    }
}

// Функция для фильтрации сотрудников по месяцу отпуска
void filterByVacationMonth(sqlite3* db, const std::string& vacationMonth) {
    std::string query = "SELECT * FROM employees WHERE vacation_month = '" + vacationMonth + "';";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);

    if (rc == SQLITE_OK) {
        printEmployeeList(stmt);
        sqlite3_finalize(stmt);
    }
    else {
        std::cerr << "Ошибка при выполнении запроса: " << sqlite3_errmsg(db) << std::endl;
    }
}

// Функция для очистки таблицы сотрудников
void clearEmployeeTable(sqlite3* db) {
    char confirm;
    std::cout << "Вы уверены, что хотите очистить всю таблицу сотрудников? (y/n): ";
    std::cin >> confirm;

    if (confirm == 'y' || confirm == 'Y') {
        std::string query = "DELETE FROM employees;";

        int rc = sqlite3_exec(db, query.c_str(), 0, 0, 0);
        if (rc != SQLITE_OK) {
            std::cerr << "Ошибка при очистке таблицы: " << sqlite3_errmsg(db) << std::endl;
        }
        else {
            std::cout << "Таблица сотрудников успешно очищена.\n";
        }
    }
    else {
        std::cout << "Очистка таблицы отменена.\n";
    }
}

void printMenu() {
    std::cout << "Выберите действие:\n"
        << "1. Посмотреть список сотрудников\n"
        << "2. Добавить сотрудника\n"
        << "3. Выбрать сотрудников по месяцу отпуска\n"
        << "4. Удалить сотрудника\n"
        << "5. Очистить таблицу\n"
        << "0. Выйти\n";
}

int main() {
    setlocale(LC_ALL, "ru");

    sqlite3* db;
    int rc = sqlite3_open("employees.db", &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка при открытии/создании базы данных: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    createEmployeeTable(db);

    int choice;
    do {
        printMenu();
        std::cout << "Ваш выбор: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            showEmployeeList(db);
            break;
        case 2: {
            std::string lastName, firstName, middleName, department, position, vacationMonth;
            std::cout << "Введите данные нового сотрудника:\n";
            std::cout << "Чтобы избежать проблем с кодировкой, вводите данные на английском языке\n";

            // Фамилия
            std::cout << "Фамилия: ";
            std::cin.ignore(); // Очищаем буфер ввода
            std::getline(std::cin, lastName);
            if (lastName.empty() || lastName.find(' ') != std::string::npos) {
                std::cerr << "Ошибка: Введите только одно слово для Фамилии.\n";
                break;
            }

            // Имя
            std::cout << "Имя: ";
            std::getline(std::cin, firstName);
            if (firstName.empty() || firstName.find(' ') != std::string::npos) {
                std::cerr << "Ошибка: Введите только одно слово для Имени.\n";
                break;
            }

            // Отчество
            std::cout << "Отчество: ";
            std::getline(std::cin, middleName);
            if (middleName.empty() || middleName.find(' ') != std::string::npos) {
                std::cerr << "Ошибка: Введите только одно слово для Отчества.\n";
                break;
            }

            // Отдел
            std::cout << "Отдел: ";
            std::getline(std::cin, department);
            if (department.empty() || department.find(' ') != std::string::npos) {
                std::cerr << "Ошибка: Введите только одно слово для Отдела.\n";
                break;
            }

            // Должность
            std::cout << "Должность: ";
            std::getline(std::cin, position);
            if (position.empty() || position.find(' ') != std::string::npos) {
                std::cerr << "Ошибка: Введите только одно слово для Должности.\n";
                break;
            }

            // Вызываем функцию addEmployee
            addEmployee(db, lastName, firstName, middleName, department, position, vacationMonth);
            break;
        }
        case 3: {
            std::string month;
            std::cout << "Введите месяц отпуска для фильтрации: ";
            std::cin >> month;
            filterByVacationMonth(db, month);
            break;
        }
        case 4: {
            int employeeId;
            std::cout << "Введите ID сотрудника для удаления: ";
            std::cin >> employeeId;
            deleteEmployee(db, employeeId);
            break;
        case 5:
            clearEmployeeTable(db);
            break;
        }
        case 0:
            break;
        default:
            std::cout << "Некорректный ввод. Повторите попытку.\n";
        }
    } while (choice != 0);

    // Закрытие базы данных после завершения цикла
    sqlite3_close(db);

    return 0;
}