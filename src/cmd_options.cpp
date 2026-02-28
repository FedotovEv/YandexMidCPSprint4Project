#include "cmd_options.hpp"

#include <exception>
#include <iostream>
#include <print>
#include <string>
#include <filesystem>

#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace analyzer::cmd
{
    namespace po = boost::program_options;

    ProgramOptions::ProgramOptions() : desc_("Формат команды вызова программы")
    {
        desc_.add_options()
            ("help,h", "вывод краткой помощи по формату команды")
            ("ts-path,t", po::value<std::string>(&tree_sitter_path_), "Маршрут размещения утилиты tree-sitter")
            ("ts-config_path,c", po::value<std::string>(&tree_sitter_config_path_), "Маршрут расположения файла конфигурации tree-sitter")
            ("file-path,p", po::value<std::string>(&common_files_path_), "Общий префикс маршрута расположения обрабатываемых файлов")
            ("file,f", po::value<std::vector<std::string>>(&files_)->required()->multitoken(), "Список файлов для обработки (обязательны)");
    }

    ProgramOptions::~ProgramOptions() = default;

    bool ProgramOptions::Parse(int argc, char *argv[])
    {
        try
        {
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv).options(desc_).run(), vm);

            if (vm.count("help"))
            {
                desc_.print(std::cout);
                return false;
            }

            po::notify(vm);
            // Пробуем обнаружить исполняемый файл древолаза.
            bool is_tree_sitter_path_in_cmdline = !tree_sitter_path_.empty();
            if (!is_tree_sitter_path_in_cmdline)
                // Если маршрут к нему явно не указан в командной строке, попробуем обнаружить его также прямо там же, где и сам анализатор.
                tree_sitter_path_ = fs::path(argv[0]).replace_filename(TreeSitterOpt::TREE_SITTER_DEFAULT_EXE).string();
            tree_sitter_path_ = FindTreeSitterExec
                (tree_sitter_path_, TreeSitterOpt::TREE_SITTER_DEFAULT_EXE, is_tree_sitter_path_in_cmdline);
            if (tree_sitter_path_.empty())
            {
                std::cerr << "Ошибка: не удалось обнаружить исполняемый файл построителя синтаксических деревьев\n";
                return false;
            }

            // Далее нужно найти файл рабочей конфигурации древолаза. Список мест его поиска аналогичен исполняемому файлу построителя
            // синтаксических деревьев, но в path поиск конфигурации проводиться не будет.
            bool is_tree_sitter_cfg_in_cmdline = !tree_sitter_config_path_.empty();
            if (!is_tree_sitter_cfg_in_cmdline)
                // Если маршрут к нему явно не указан в командной строке, попробуем обнаружить его также прямо там же, где и сам анализатор.
                tree_sitter_config_path_ = fs::path(argv[0]).replace_filename(TreeSitterOpt::TREE_SITTER_DEFAULT_CONFIG).string();
            tree_sitter_config_path_ = FindTreeSitterCfg
                (tree_sitter_config_path_, tree_sitter_path_, is_tree_sitter_cfg_in_cmdline);
            if (tree_sitter_config_path_.empty())
            {
                std::cerr << "Ошибка: не удалось обнаружить файл конфигурации построителя синтаксических деревьев\n";
                return false;
            }

            // Наконец, обработаем общий префикс пути файлов-исходников, если он задан.
            if (!common_files_path_.empty())
            { // Возможно, оператоор указал шаблон, который нужно заменить на его действительное содержимое.
                if (common_files_path_ == EXE_PATH_PATTERN)
                    common_files_path_ = fs::path(argv[0]).parent_path().string();
                else if (common_files_path_ == TREE_SITTER_PATH_PATTERN)
                    common_files_path_ = fs::path(tree_sitter_path_).parent_path().string();
            }

            return true;
        }
        catch (const po::required_option& ro_errc)
        {
            std::cerr << "Ошибка: Хотя бы один файл для обработки должен быть указан\n";
            desc_.print(std::cout);
            return false;
        }
        catch (const po::unknown_option& uo_errc)
        {
            std::cerr << "Ошибка: Указана неизвестная опция - " << uo_errc.get_option_name() << "\n";
            return false;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Ошибка: Общая ошибка разбора командной строки: " << e.what() << "\n";
            desc_.print(std::cout);
            return false;
        }
    }
}  // namespace analyzer::cmd
