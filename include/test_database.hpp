#pragma once

#include "utils.hpp"
#include <string>
#include <unordered_map>

class TestDatabase
{
public:
    ~TestDatabase();
    // Функция возвращает маршрут каталога, в котором будут создаваться временные файлы тестовых объектов. Как правило,
    // это будет системная папка временных файлов
    std::string GetTestDirectory() const;
    // Функция формирования блока параметров процедуры анализа, если эта процедура выполняется в составе модульных тестов,
    // то есть автономно, без связи с основным запускающим модулем. this_exec_filename - полное имя исполняемого файла тестов.
    TreeSitterOpt MakeTestTriSitterOpt(const std::string& this_exec_filename = {});
    // Метод создаёт в каталоге временных файлов специальный тестовый вариант конфигурации древолаза, указывающий на каталог
    // грамматик grammar_path.
    std::string CreateTreeSitterConfig(const std::string& grammar_path);
    // Метод создания во временном каталоге тестового программного файла, который далее может быть использован как опытный объект.
    std::string CreateTestFile(const std::string& creating_test_name);
    // Методы удаления ранее созданных временных файлов. Если конкретная цель операции не указана, удаляются все файлы такого типа.
    void DeleteTreeSitterConfig(const std::string& delete_config_file = {});
    void DeleteTestFile(const std::string& delete_test_file = {});

private:
    static const std::unordered_map<std::string, std::string> test_sources;

    // Список инстанцированных файлов (как конфигурационных, так и тестовых примеров), созданных объектом в процессе работы.
    std::vector<std::pair<std::string, bool>> m_file_created_list;

    // Универсальный метод удаления ранее созданных временных файлов. Если конкретная цель операции не указана, удаляются все файлы
    // указанного в is_test_file типа.
    void DeleteFileObject(const std::string& delete_file, bool is_test_file);
};
