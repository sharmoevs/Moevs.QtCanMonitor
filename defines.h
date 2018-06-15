#ifndef DEFINES
#define DEFINES

// Версия ПО
#define APP_VERSION                         ("28.03.17")


// Настройки
#define ORGANIZATION_NAME                   ("MNITI")
#define APPLICATION_NAME                    ("DIGITAL_MOEVS_CAN-monitor")
#define SETTINGS_SELECTED_TAB               ("startup/selectedTab")     // выбранная вкладка запуске
#define SETTINGS_SELECTED_TAB_DEFAULT       0                           // выбранная вкладка по умолчанию
// Настройки для расширенной команды
#define EXTCMD_MAX_DATA_BYTES       6       // максимальное кол-во байт данных в одном пакете расширенной команды
#define EXTCMD_START_MSG            0       // первый пакет расширенной команды
#define EXTCMD_CONT_MSG             1       // продолжение расширенной команды
#define EXTCMD_END_MSG              2       // последний пакет расширенной команды

// Цвета
#define ACTIVE_SETTING_STYLE            ("font: 8pt;\ncolor: rgb(0,   130, 0);")       // все ок
#define TIMEOUT_SETTING_STYLE           ("font: 8pt;\ncolor: rgb(180, 180, 50);")      // таймаут
#define WARNING_SETTING_STYLE           ("font: 8pt;\ncolor: rgb(255, 106, 0);")       // работа с ограничениями
#define ERROR_SETTING_STYLE             ("font: 8pt;\ncolor: rgb(255, 0,   0);")       // ошибка



// Причины неисправности ГС
typedef enum
{
  GgFailure_None                = 0x00, // нет ошибок
  GgFailure_FuncTestError       = 0x01, // провален функциональный тест
  GgFailure_CourseSpeedProtect  = 0x02, // сработала защита по скорости курсового контура
  GgFailure_TangageSpeedProtect = 0x04, // сработала защита по скорости тангажного контура
  GgFailure_StartUpCalibration  = 0x08, // калибровка по включению питания не завершена за отведенное время
} GSFailure_t;




#endif // DEFINES

