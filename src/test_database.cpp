
#include "test_database.hpp"
#include <string>
#include <filesystem>
#include <fstream>
#include <utils.hpp>

using namespace std::literals;
namespace fs = std::filesystem;

// База данных, содержащая исходные файлы для анализа древолазом, а также конфиурация, используемая для запуска модульных тестов его компонент.
const std::unordered_map<std::string, std::string> TestDatabase::test_sources = 
{
    {"comments.py"s, 
R"--(
def Func_comments(result, a, b):
    # Это комментарий
    a = 10
    # Это ещё комментарий
    # Это ещё комментарий
    b = 20
    result = a + b
)--"},

    {"exceptions.py"s, 
R"--(
def Try_Exceptions():
    try:
        x = 1 / 0
        assert x == NaN
    except ZeroDivisionError:
        print("Ошибка")
    finally:
        print("Завершено")
)--"},

    {"if.py"s, 
R"--(
def testIf(x):
    if x > 0:
        return True
    return False
)--"},

    {"loops.py"s, 
R"--(
def TestLoops(n):
    for i in range(n):
        while (i < n):
            if i % 2 == 0:
                print(i)
            i += 1
    return True
)--"},

    {"many_lines.py"s,
R"--(
def testmultiline():
    data = [
        1, 2, 3,
        4, 5, 6
    ]
    total = sum(data)
    
    print("Больше 10")
    print("Меньше или равно 10")
        
    call_unexisting_function1(total)
    call_unexisting_function2(total)
    call_unexisting_function3(total)

    assert total == 6
)--"},

    {"many_parameters.py"s,
R"--(
def __test_multiparameters__(a, b, c=5, *args, **kwargs):
    assert a + b == c
)--"},

    {"match_case.py"s,
R"--(
def test_Match_case(x):
    match x:
        case 1:
            return "one"
        case 2:
            return "two"
        case _:
            return "many"
)--"},

    {"nested_if.py"s,
R"--(
def Testnestedif(x, y):
    if x > 0:
        if y > 0:
            assert x == y
        elif x < 0:
            return 0
        else:
            return 1
    return -1
)--"},

    {"simple.py"s,
R"--(
def test_simple():
    x = 1
    y = 2
    z = x + y
    print(z)

    assert z == 3
)--"},

    {"ternary.py"s,
R"--(
def teSt_ternary(x):
    return "positive" if (42 if x > 0 else -3) > 0 else "non-positive"
)--"},

    {"sample.py"s,
R"--(
class AdvancedProcessor(SimpleProcessor):
    """Продвинутый процессор данных с дополнительной функциональностью"""
    
    def __init__(self, multiplier: float = 1.0, offset: float = 0.0):
        super().__init__(multiplier)
        self.offset = offset
    
    @log_execution(log_level=LogLevel.DEBUG)
    def process(self, data: List[Union[int, float]]) -> List[float]:
        """Расширенная обработка данных"""
        processed = super().process(data)
        return [x + self.offset for x in processed]
    
    def __call__(self, data: List[float]) -> List[float]:
        """Поддержка вызова как функции"""
        return self.process(data)


def lambda_demo():
    """Демонстрация использования лямбда-функций"""
    numbers = [1, 2, 3, 4, 5]
    
    # Лямбда для фильтрации
    even = filter(lambda x: x % 2 == 0, numbers)
    
    # Лямбда для преобразования
    squared = map(lambda x: x ** 2, numbers)
    
    # Лямбда для сортировки
    sorted_nums = sorted(numbers, key=lambda x: -x)
    
    return list(even), list(squared), sorted_nums
)--"},

    {"config.json"s,
R"--(
{
  "parser-directories": [
    %1
  ],
  "theme": {
    "attribute": {
      "color": 124,
      "italic": true
    },
    "comment": {
      "color": 245,
      "italic": true
    },
    "constant": 94,
    "constant.builtin": {
      "bold": true,
      "color": 94
    },
    "constructor": 136,
    "embedded": null,
    "function": 26,
    "function.builtin": {
      "bold": true,
      "color": 26
    },
    "keyword": 56,
    "module": 136,
    "number": {
      "bold": true,
      "color": 94
    },
    "operator": {
      "bold": true,
      "color": 239
    },
    "property": 124,
    "property.builtin": {
      "bold": true,
      "color": 124
    },
    "punctuation": 239,
    "punctuation.bracket": 239,
    "punctuation.delimiter": 239,
    "punctuation.special": 239,
    "string": 28,
    "string.special": 30,
    "tag": 18,
    "type": 23,
    "type.builtin": {
      "bold": true,
      "color": 23
    },
    "variable": 252,
    "variable.builtin": {
      "bold": true,
      "color": 252
    },
    "variable.parameter": {
      "color": 252,
      "underline": true
    }
  }
}
)--"}
// Конец базы данных исходных файлов модульных тестов.
};

TestDatabase::~TestDatabase()
{
    DeleteTreeSitterConfig();
    DeleteTestFile();
}

// Метод создаёт в каатлоге временных файлов специальный тестовый вариант конфигурации древолаза, указывающий на каталог грамматик grammar_path.
std::string TestDatabase::CreateTreeSitterConfig(const std::string& grammar_path)
{
    static const std::string GRAMMAR_PATTERN = "%1";

    DeleteTreeSitterConfig();
    auto creating_file_it = test_sources.find(TreeSitterOpt::TREE_SITTER_DEFAULT_CONFIG);
    if (creating_file_it == test_sources.end())
        return {}; // В базе объектов нет объекта-прототипа для создания конфигурации древолаза.

    // Замена местоблюстителя, указывающего положение маршрута грамматики, на его истинное значение.
    std::string config_proto = creating_file_it->second;
    if (size_t grammar_pattern_pos = config_proto.find(GRAMMAR_PATTERN); grammar_pattern_pos != std::string::npos)
        config_proto.replace(grammar_pattern_pos, GRAMMAR_PATTERN.size(), grammar_path);

        fs::path full_file_path = fs::temp_directory_path() / fs::path(TreeSitterOpt::TREE_SITTER_DEFAULT_CONFIG);
        std::ofstream ofstr(full_file_path);
        ofstr.write(config_proto.data(), config_proto.size());
        if (!ofstr)
            return {}; // Ошибка при создании файла.
        m_file_created_list.emplace_back(full_file_path.string(), false);
        // Файл конфигурации древолаза создан и зарегистрирован в словаре m_file_created_list.
        return full_file_path.string();
}

std::string TestDatabase::GetTestDirectory() const
{
    return fs::temp_directory_path().string();
}

// Метод создания во временном каталоге тестового Питонофайла, который далее может быть использован как опытный объект.
std::string TestDatabase::CreateTestFile(const std::string& creating_test_name)
{
    if (auto creating_file_it = test_sources.find(creating_test_name); creating_file_it != test_sources.end())
    {
        auto already_created_it = std::find_if (m_file_created_list.begin(), m_file_created_list.end(),
            [&creating_test_name](const std::pair<std::string, bool>& created_file_pair) -> bool
            {
                // Если существующий файл является тестовым и его имя совпадает с заказанным, считаем, что он уже существует.
                return created_file_pair.second && fs::path(created_file_pair.first).stem().string() == creating_test_name;
            });
        if (already_created_it != m_file_created_list.end())
            return already_created_it->first; // Объект уже существует. Возвращаем имя его файла и выходим.

        // Заказанного объекта пока не существует, его нужно создать.
        fs::path full_file_path = fs::temp_directory_path() / fs::path(creating_test_name);
        std::ofstream ofstr(full_file_path);
        ofstr.write(creating_file_it->second.data(), creating_file_it->second.size());
        if (!ofstr)
            return {}; // Ошибка при создании файла.
        m_file_created_list.emplace_back(full_file_path.string(), true);
        // Новый тестовый файл создан и зарегистрирован в словаре m_file_created_list.
        return full_file_path.string();
    }
    return {};  // Такого теста в базе данных нет.
}

// Методы удаления ранее созданных временных файлов. Если конкретная цель операции не указана, удаляются все файлы такого типа.
void TestDatabase::DeleteFileObject(const std::string& delete_file, bool is_test_file)
{
    auto removed_it = std::remove_if(m_file_created_list.begin(), m_file_created_list.end(),
        [&delete_file, &is_test_file](const std::pair<std::string, bool>& created_file_pair) -> bool
        {
            // Если тип файла соответствует is_test_file и его имя совпадает с delete_file, то такой файл нужно удалить.
            bool do_delete = (created_file_pair.second == is_test_file) &&
                             (delete_file.empty() || delete_file == created_file_pair.first);
            if (do_delete)
                fs::remove(created_file_pair.first);
            return do_delete;
        });
    m_file_created_list.erase(removed_it, m_file_created_list.end());
}

void TestDatabase::DeleteTreeSitterConfig(const std::string& delete_config_file)
{
    DeleteFileObject(delete_config_file, false);
}

void TestDatabase::DeleteTestFile(const std::string& delete_test_file)
{
    DeleteFileObject(delete_test_file, true);
}

TreeSitterOpt TestDatabase::MakeTestTriSitterOpt(const std::string& this_exec_filename)
{
    TreeSitterOpt result;
    result.tree_sitter_exec = FindTreeSitterExec();  // Полный маршрут и имя исполняемого файла древолаза.
    if (result.tree_sitter_exec.empty())
        throw std::runtime_error("Tree-sitter не найден. Экспедиция на дерево отменяется, сушите весла...");

    // Далее предстоит поиск рабочего файла конфигурации.
    // Сначала ищем его обычным способом, как и при нормальной работе анализатора.
    std::string tree_sitter_cfg_hint_path = !this_exec_filename.empty() ?
        (fs::path(this_exec_filename).parent_path() / fs::path(TreeSitterOpt::TREE_SITTER_DEFAULT_CONFIG)).string() : std::string();

    result.tree_sitter_config = FindTreeSitterCfg(tree_sitter_cfg_hint_path, result.tree_sitter_exec);
    if (result.tree_sitter_config.empty())
    { // Если же уже готовую конфигурацию найти не удалось, сгенерируем специальную версию конфигурации древолаза,
      // подходящей для исполнения тестов. Для этого используем ещё одну переменную окружения, указывающую на каталог,
      // в котором размещаются описания грамматик и ранее сгенерированные синтактичесие анализаторы, которые будут
      // применяться сейчас древолазом для генерации АСТ тестовых примеров.
        if (char* tree_sitter_env = std::getenv(TreeSitterOpt::TREE_SITTER_PARSERS_ENV_VAR))
            result.tree_sitter_config = CreateTreeSitterConfig(tree_sitter_env);
        else
            throw std::runtime_error("Не указан каталог расположения грамматик для Tree-sitter. Пойди туда, не знаю куда - мы так не умеем...");
    }
    result.common_files_path_prefix = GetTestDirectory();   // Общий префикс пути всех обрабатываемых файлов.
    return result;
}
