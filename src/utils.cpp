
#include <utils.hpp>

// Модуль для размещения некоторых вспомогательных процедур, требуемых как анализатору, так и набору модульных тестов его компонент.
// -------------------

std::string ScanPathHelper(const fs::path& find_name)
{
    if (char* path_env = std::getenv("PATH"))
    {
        std::string path_env_value(path_env);

        size_t sep_pos = 0;
        while (sep_pos < path_env_value.size())
        {
            size_t next_sep_pos = path_env_value.find(TreeSitterOpt::PATH_DIVISOR, sep_pos);
            if (next_sep_pos == std::string::npos)
                next_sep_pos = path_env_value.size();

            std::string next_find_cat = path_env_value.substr(sep_pos, next_sep_pos - sep_pos);
            if (fs::path full_find_pathname = fs::path(next_find_cat) / find_name; fs::exists(full_find_pathname))
                return full_find_pathname.string();

            sep_pos = next_sep_pos + 1;
        }
    }
    return {};
}

std::string FindTreeSitterExec
    (const std::string& trst_fullexe_hint, const std::string& trst_exename_only, bool is_hint_strict)
{
    if (!trst_fullexe_hint.empty())
    {
        if (fs::exists(trst_fullexe_hint) && fs::is_regular_file(trst_fullexe_hint))
            return trst_fullexe_hint;
        if (is_hint_strict)
            return {};
    }

    if (char* tree_sitter_env = std::getenv(TreeSitterOpt::TREE_SITTER_ENV_VAR))
    {
        if (fs::exists(tree_sitter_env) && fs::is_regular_file(tree_sitter_env))
            return tree_sitter_env;
    }

    // Предыдущие два этапа провалились - остаётся только поиск по PATH.
    fs::path trst_exec_name = !trst_exename_only.empty() ?
        fs::path(trst_exename_only) : fs::path(TreeSitterOpt::TREE_SITTER_DEFAULT_EXE);
    return ScanPathHelper(trst_exec_name);
}

std::string FindTreeSitterCfg(const std::string& trst_cfgname_hint, const std::string& trst_fullexe, bool is_hint_strict)
{
    if (!trst_cfgname_hint.empty())
    {
        if (fs::exists(trst_cfgname_hint) && fs::is_regular_file(trst_cfgname_hint))
            return trst_cfgname_hint;
        if (is_hint_strict)
            return {};
    }

    if (char* tree_sitter_cfg_env = std::getenv(TreeSitterOpt::TREE_SITTER_CFG_ENV_VAR))
    {
        if (fs::exists(tree_sitter_cfg_env) && fs::is_regular_file(tree_sitter_cfg_env))
            return tree_sitter_cfg_env;
    }

    if (!trst_fullexe.empty())
    { // Поиск файла конфигурации непосредственно в каталоге размещения исполняемого файла древолаза.
        fs::path trst_exe_cat = fs::path(trst_fullexe).parent_path() / fs::path(TreeSitterOpt::TREE_SITTER_DEFAULT_CONFIG);
        if (fs::exists(trst_exe_cat) && fs::is_regular_file(trst_exe_cat))
            return trst_exe_cat.string();
    }

    // Все возможные и ожидаемые места расположения конфигурации древолаза осмотрены безрезультатно, выходим с ошибкой.
    return {};
}

std::vector<std::string> ExpandFileNamesWithPath(const std::vector<std::string>& file_names, const TreeSitterOpt& tree_sitter_opt)
{
    auto file_path_names = file_names | rv::transform([&tree_sitter_opt](const std::string& filename) -> std::string
        {
            if (!tree_sitter_opt.common_files_path_prefix.empty())
                return (fs::path(tree_sitter_opt.common_files_path_prefix) / fs::path(filename)).string();
            else
                return filename;
        });
    return rs::to<std::vector<std::string>>(file_path_names);
}

std::string_view TrimView(std::string_view value)
{
    std::string_view result = value;

    while (result.size() && std::isspace(result.front()))
        result.remove_prefix(1);

    while (result.size() && std::isspace(result.back()))
        result.remove_suffix(1);

    return result;
}

std::string TrimString(const std::string& value)
{
    return std::string(TrimView(value));
}

int ToInt(std::string_view value)
{
    std::string_view trimmed_val = TrimView(value);
    int result{};
    auto [parse_end_ptr, error_code] = std::from_chars(trimmed_val.data(), trimmed_val.data() + trimmed_val.size(), result);
    if (error_code != std::errc{} || parse_end_ptr != trimmed_val.data() + trimmed_val.size())
        throw std::invalid_argument("Не могу преобразовать '" + std::string(value) + "' в целочисленное значение");

    return result;
}
