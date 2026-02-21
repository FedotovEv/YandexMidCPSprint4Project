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
            ("use_file_path,u", "Использовать общий маршрутный префикс для обнаружения входных файлов")
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
            if (!vm.contains("ts-path"))
            {  // Маршрут до древожителя явно не задан. Сначала попробуем обнаружить руководящую переменную окружения.
                if (char* tree_sitter_path = std::getenv("TREE-SITTER"))
                    tree_sitter_path_ = tree_sitter_path;
                else 
                    // Если же и переменной окружения не существует, то положим по умолчанию, что исполняемый модуль древолаза
                    // расположен там же, где и исполняемый модуль самого анализатора.
                    tree_sitter_path_ = fs::path(argv[0]).parent_path().string();
            }
            if (!vm.contains("ts-config_path"))
                // Маршрут до конфигурации древожителя явно не задан. Положим по умолчанию, что его файл конфигурации
                // находится там же, где и его исполняемый модуль.
                tree_sitter_config_path_ = tree_sitter_path_;
            if (!vm.contains("file-path"))
                // Аналогично поступаем с общим префиксом маршрута для обрабатываемых файлов.
                common_files_path_ = tree_sitter_path_;
            use_common_files_path_ = vm.contains("use_file_path");

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
            std::cerr << "Указана неизвестная опция - " << uo_errc.get_option_name() << "\n";
            return false;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Общая ошибка разбора командной строки: " << e.what() << "\n";
            desc_.print(std::cout);
            return false;
        }
    }
}  // namespace analyzer::cmd
