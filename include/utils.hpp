#pragma once

#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <ranges>
#include <filesystem>

namespace rv = std::ranges::views;
namespace rs = std::ranges;
namespace fs = std::filesystem;

#define ZERO_TOLERANCE 1E-7

struct TreeSitterOpt
{
    #ifdef _WIN32
        static constexpr const char TREE_SITTER_DEFAULT_EXE[] = "tree_sitter.exe";
        static constexpr char PATH_DIVISOR = ';';
    #else
        static constexpr const char TREE_SITTER_DEFAULT_EXE[] = "tree_sitter";
        static constexpr char PATH_DIVISOR = ':';
    #endif
    static constexpr const char TREE_SITTER_DEFAULT_CONFIG[] = "config.json";
    static constexpr const char TREE_SITTER_ENV_VAR[] = "TREE-SITTER";
    static constexpr const char TREE_SITTER_CFG_ENV_VAR[] = "TREE-SITTER-CFG";
    static constexpr const char TREE_SITTER_PARSERS_ENV_VAR[] = "TREE-SITTER-PARSERS";

    std::string tree_sitter_exec;           // Полный маршрут и имя исполняемого файла древолаза.
    std::string tree_sitter_config;         // Маршрут и имя, под которым находится файл конфигурации древолаза.
    std::string common_files_path_prefix;   // Общий префикс пути всех обрабатываемых файлов.
};

// Функция поиска исполняемого модуля утилиты-построителя синтаксических деревьев.
// Сначала его пытаются обнаружить по маршруту trst_fullexe_hint, если он задан. Затем делается попытка выявить переменную
// окружения TREE_SITTER_ENV_VARIABLE, ответственную за хранение этого пути. Последний этап - поиск в PATH. Для такой операции
// требуется знать имя исполняемого файла, которое принимается равным tree_sitter_exename_only или TREE_SITTER_DEFAULT_EXE,
// если trst_exename_only не указан. В случае неудачи на всех этапах возвращается пустая строка.
std::string FindTreeSitterExec
    (const std::string& trst_fullexe_hint = {}, const std::string& trst_exename_only = {}, bool is_hint_strict = false);

// Функция поиска файла конфигурации древолаза. Сначала делается попытка проверить её наличие по маршруту trst_cfgname_hint,
// затем по указаниям переменной окружения TREE_SITTER_CFG_ENV_VAR, а затем - в PATH.
// В случае неудачи на всех этапах опять возвращается пустая строка.
std::string FindTreeSitterCfg
    (const std::string& trst_cfgname_hint = {}, const std::string& trst_fullexe = {}, bool is_hint_strict = false);

// Функция дополняет краткие имена обрабатываемых файлов file_names до полных в соответствии с параметрами tree_sitter_opt.
std::vector<std::string> ExpandFileNamesWithPath(const std::vector<std::string>& file_names, const TreeSitterOpt& tree_sitter_opt);

// Удаление пробельных символов с начала и конца аргумента value.
std::string_view TrimView(std::string_view value);
std::string TrimString(const std::string& value);

// Преобразование к целочисленному значению внутренного содержания строки value.
int ToInt(std::string_view value);
