#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <list>
#include <map>
#include <functional>
#include <algorithm>
#include <fstream>
#include <utility>
#include <memory>
#include <stdexcept>
#include <windows.h>


/// @brief Перечисление для цвета узлов Красно-Черного дерева.
enum Color { RED, BLACK };

/// @brief Структура для хранения данных объекта.
/// Используется во всех структурах данных (массив, деревья, хеш-таблица).
struct DataObject {
    /// @brief Ключ поиска (предполагается первым нечисловым полем).
    std::string key;
    /// @brief Пример числового поля.
    int value1;
    /// @brief Пример числового поля с плавающей точкой.
    double value2;

    /// @brief Конструктор по умолчанию и с параметрами.
    /// @param k Строковый ключ объекта.
    /// @param v1 Целочисленное значение.
    /// @param v2 Значение с плавающей точкой.
    DataObject(std::string k = "", int v1 = 0, double v2 = 0.0)
        : key(std::move(k)), value1(v1), value2(v2) {}

    /// @brief Оператор сравнения "меньше" по ключу.
    /// Необходим для сортировки и работы деревьев поиска.
    /// @param other Другой объект DataObject для сравнения.
    /// @return true, если ключ текущего объекта меньше ключа other.
    bool operator<(const DataObject& other) const {
        return key < other.key;
    }

    /// @brief Оператор сравнения "равно" по ключу.
    /// Необходим для поиска.
    /// @param other Другой объект DataObject для сравнения.
    /// @return true, если ключи объектов равны.
    bool operator==(const DataObject& other) const {
        return key == other.key;
    }

    /// @brief Оператор сравнения "больше" по ключу.
    /// @param other Другой объект DataObject для сравнения.
    /// @return true, если ключ текущего объекта больше ключа other.
    bool operator>(const DataObject& other) const {
        return key > other.key;
    }

    /// @brief Оператор вывода объекта в поток.
    /// @param os Поток вывода.
    /// @param obj Объект DataObject для вывода.
    /// @return Ссылка на поток вывода.
    friend std::ostream& operator<<(std::ostream& os, const DataObject& obj) {
        os << "Key: " << obj.key << ", Val1: " << obj.value1 << ", Val2: " << obj.value2;
        return os;
    }
};


/// @brief Генерирует вектор объектов DataObject заданного размера.
/// @param size Количество объектов для генерации.
/// @return Вектор сгенерированных объектов DataObject.
/// @note Ключи генерируются таким образом, чтобы допустить дубликаты.
std::vector<DataObject> generateData(size_t size) {
    std::vector<DataObject> data;
    if (size == 0) return data;

    data.reserve(size);
    std::mt19937 gen(std::random_device{}());

    size_t num_unique_keys = std::max(static_cast<size_t>(10), size / 5);
    std::vector<std::string> possible_keys;
    possible_keys.reserve(num_unique_keys);
    std::uniform_int_distribution<> char_dist('a', 'z');
    for (size_t i = 0; i < num_unique_keys; ++i) {
        std::string key = "";
        size_t key_len = std::uniform_int_distribution<size_t>(3, 10)(gen);
        for(size_t j = 0; j < key_len; ++j) {
            key += char_dist(gen);
        }
        possible_keys.push_back(key);
    }

    if (possible_keys.empty()) {
        possible_keys.push_back("defaultkey");
    }


    std::uniform_int_distribution<> key_dist(0, possible_keys.size() - 1);
    std::uniform_int_distribution<> val1_dist(1, 1000);
    std::uniform_real_distribution<> val2_dist(0.0, 100.0);

    for (size_t i = 0; i < size; ++i) {
        std::string random_key = possible_keys[key_dist(gen)];
        data.emplace_back(random_key, val1_dist(gen), val2_dist(gen));
    }
    return data;
}


/// @brief Выполняет линейный поиск всех объектов с заданным ключом в векторе.
/// @param data Вектор объектов DataObject для поиска.
/// @param searchKey Ключ, по которому осуществляется поиск.
/// @return Вектор найденных объектов DataObject. Сложность O(N).
std::vector<DataObject> linearSearch(const std::vector<DataObject>& data, const std::string& searchKey) {
    std::vector<DataObject> results;
    for (const auto& obj : data) {
        if (obj.key == searchKey) {
            results.push_back(obj);
        }
    }
    return results;
}


/// @brief Узел простого бинарного дерева поиска.
struct BSTNode {
    /// @brief Данные, хранящиеся в узле.
    DataObject data;
    /// @brief Указатель на левого потомка.
    BSTNode *left = nullptr;
    /// @brief Указатель на правого потомка.
    BSTNode *right = nullptr;

    /// @brief Конструктор узла BST.
    /// @param d Объект DataObject для хранения в узле.
    BSTNode(DataObject d) : data(std::move(d)) {}
};

/// @brief Рекурсивно вставляет объект в BST.
/// @param node Ссылка на указатель на текущий узел (может измениться).
/// @param obj Объект DataObject для вставки.
/// @note Дубликаты ключей разрешены и вставляются в правое поддерево.
void insertBST(BSTNode*& node, DataObject obj) {
    if (node == nullptr) {
        node = new BSTNode(std::move(obj));
        return;
    }
    if (obj.key < node->data.key) {
        insertBST(node->left, std::move(obj));
    } else {
        insertBST(node->right, std::move(obj));
    }
}

/// @brief Рекурсивно ищет все объекты с заданным ключом в BST.
/// @param node Текущий узел для проверки.
/// @param searchKey Ключ для поиска.
/// @param results Вектор для накопления найденных объектов.
void searchBSTRecursive(BSTNode* node, const std::string& searchKey, std::vector<DataObject>& results) {
    if (node == nullptr) {
        return;
    }

    if (searchKey == node->data.key) {
        results.push_back(node->data);
        searchBSTRecursive(node->right, searchKey, results);
    } else if (searchKey < node->data.key) {
        searchBSTRecursive(node->left, searchKey, results);
    } else {
        searchBSTRecursive(node->right, searchKey, results);
    }
}

/// @brief Функция-обертка для поиска всех вхождений ключа в BST.
/// @param root Корневой узел BST.
/// @param searchKey Ключ для поиска.
/// @return Вектор найденных объектов DataObject. Сложность O(log N) в среднем, O(N) в худшем + O(k), где k - число найденных.
std::vector<DataObject> searchBST(BSTNode* root, const std::string& searchKey) {
    std::vector<DataObject> results;
    searchBSTRecursive(root, searchKey, results);
    return results;
}

/// @brief Рекурсивно удаляет узлы BST, освобождая память.
/// @param node Узел для удаления.
void destroyBST(BSTNode* node) {
    if (node) {
        destroyBST(node->left);
        destroyBST(node->right);
        delete node;
    }
}


/// @brief Узел Красно-Черного Дерева.
struct RBTNode {
    /// @brief Данные, хранящиеся в узле.
    DataObject data;
    /// @brief Цвет узла (RED или BLACK).
    Color color;
    /// @brief Указатель на родительский узел.
    RBTNode *parent;
    /// @brief Указатель на левого потомка.
    RBTNode *left;
    /// @brief Указатель на правого потомка.
    RBTNode *right;

    /// @brief Конструктор узла RBT.
    /// @param d Данные для узла.
    /// @param c Цвет узла (по умолчанию RED).
    /// @param p Родительский узел (по умолчанию nullptr).
    /// @param l Левый потомок (по умолчанию nullptr).
    /// @param r Правый потомок (по умолчанию nullptr).
    RBTNode(DataObject d, Color c = RED, RBTNode* p = nullptr, RBTNode* l = nullptr, RBTNode* r = nullptr)
        : data(std::move(d)), color(c), parent(p), left(l), right(r) {}

    /// @brief Проверяет, является ли узел левым потомком.
    /// @return true, если узел является левым потомком, иначе false.
    bool isLeftChild() const {
        return parent != nullptr && this == parent->left;
    }

    /// @brief Проверяет, является ли узел правым потомком.
    /// @return true, если узел является правым потомком, иначе false.
    bool isRightChild() const {
        return parent != nullptr && this == parent->right;
    }
};


/// @brief Класс, реализующий Красно-Черное Дерево.
class RedBlackTree {
private:
    /// @brief Указатель на корневой узел дерева.
    RBTNode* root;

    /// @brief Выполняет левый поворот вокруг узла x.
    /// @param x Узел, вокруг которого выполняется поворот.
    /// @pre Правый потомок x не должен быть nullptr.
    void leftRotate(RBTNode* x) {
        RBTNode* y = x->right;
        if (!y) return;

        x->right = y->left;
        if (y->left) {
            y->left->parent = x;
        }

        y->parent = x->parent;
        if (!x->parent) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }

    /// @brief Выполняет правый поворот вокруг узла y.
    /// @param y Узел, вокруг которого выполняется поворот.
    /// @pre Левый потомок y не должен быть nullptr.
    void rightRotate(RBTNode* y) {
        RBTNode* x = y->left;
        if (!x) return;

        y->left = x->right;
        if (x->right) {
            x->right->parent = y;
        }

        x->parent = y->parent;
        if (!y->parent) {
            root = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }

        x->right = y;
        y->parent = x;
    }

    /// @brief Восстанавливает свойства Красно-Черного дерева после вставки узла.
    /// @param z Вставленный узел (изначально красный).
    void insertFixup(RBTNode* z) {
        while (z->parent && z->parent->color == RED) {
            RBTNode* parent = z->parent;
            RBTNode* grandparent = parent->parent;
            if (!grandparent) break;

            if (parent == grandparent->left) {
                RBTNode* uncle = grandparent->right;
                if (uncle && uncle->color == RED) {
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    grandparent->color = RED;
                    z = grandparent;
                } else {
                    if (z == parent->right) {
                        z = parent;
                        leftRotate(z);
                        parent = z->parent;
                        grandparent = parent ? parent->parent : nullptr;
                    }

                    if(parent) parent->color = BLACK;
                    if(grandparent) {
                         grandparent->color = RED;
                         rightRotate(grandparent);
                    }
                }
            } else {
                RBTNode* uncle = grandparent->left;
                if (uncle && uncle->color == RED) {
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    grandparent->color = RED;
                    z = grandparent;
                } else {
                    if (z == parent->left) {
                        z = parent;
                        rightRotate(z);
                        parent = z->parent;
                        grandparent = parent ? parent->parent : nullptr;
                    }
                    if(parent) parent->color = BLACK;
                    if(grandparent) {
                        grandparent->color = RED;
                        leftRotate(grandparent);
                    }
                }
            }
        }
        if(root) root->color = BLACK;
    }

    /// @brief Рекурсивно ищет все объекты с заданным ключом в RBT.
    /// @param node Текущий узел для проверки.
    /// @param searchKey Ключ для поиска.
    /// @param results Вектор для накопления найденных объектов.
    void searchRecursive(RBTNode* node, const std::string& searchKey, std::vector<DataObject>& results) const {
        if (node == nullptr) {
            return;
        }

        if (searchKey == node->data.key) {
            results.push_back(node->data);
            searchRecursive(node->right, searchKey, results);
        } else if (searchKey < node->data.key) {
            searchRecursive(node->left, searchKey, results);
        } else {
            searchRecursive(node->right, searchKey, results);
        }
    }


    /// @brief Рекурсивно удаляет узлы дерева, освобождая память.
    /// @param node Узел для удаления.
    void destroyRecursive(RBTNode* node) {
        if (node) {
            destroyRecursive(node->left);
            destroyRecursive(node->right);
            delete node;
        }
    }

public:
    /// @brief Конструктор RBT. Инициализирует дерево пустым.
    RedBlackTree() : root(nullptr) {}

    /// @brief Деструктор RBT. Освобождает всю память, занятую узлами.
    ~RedBlackTree() {
        destroyRecursive(root);
    }

    /// @brief Вставляет новый объект DataObject в Красно-Черное дерево.
    /// @param obj Объект для вставки. Сложность O(log N).
    void insert(DataObject obj) {
        RBTNode* z = new RBTNode(std::move(obj));
        RBTNode* y = nullptr;
        RBTNode* x = root;

        while (x) {
            y = x;
            if (z->data.key < x->data.key) {
                x = x->left;
            } else {
                x = x->right;
            }
        }

        z->parent = y;
        if (!y) {
            root = z;
        } else if (z->data.key < y->data.key) {
            y->left = z;
        } else {
            y->right = z;
        }

        insertFixup(z);
    }


    /// @brief Ищет все объекты с заданным ключом в RBT.
    /// @param searchKey Ключ для поиска.
    /// @return Вектор найденных объектов DataObject. Сложность O(log N + k), где k - число найденных.
    std::vector<DataObject> search(const std::string& searchKey) const {
        std::vector<DataObject> results;
        searchRecursive(root, searchKey, results);
        return results;
    }

    /// @brief Строит RBT из существующего вектора данных.
    /// @param data Вектор объектов DataObject.
    void build(const std::vector<DataObject>& data) {
        destroyRecursive(root);
        root = nullptr;
        for(const auto& obj : data) {
            insert(obj);
        }
    }
};


/// @brief Класс, реализующий хеш-таблицу с методом цепочек для разрешения коллизий.
class HashTable {
private:
    /// @brief Основное хранилище хеш-таблицы: вектор списков (цепочек).
    std::vector<std::list<DataObject>> table;
    /// @brief Текущий размер вектора table (количество "корзин").
    size_t table_size;
    /// @brief Счетчик коллизий, возникших при вставке.
    size_t collision_count;

    /// @brief Хеш-функция для ключа (строки).
    /// Использует стандартную std::hash<std::string>.
    /// @param key Строковый ключ для хеширования.
    /// @return Хеш-индекс в диапазоне [0, table_size - 1].
    size_t hashFunction(const std::string& key) const {
        return table_size > 0 ? (std::hash<std::string>{}(key) % table_size) : 0;
    }

    /// @brief Проверяет, является ли число простым.
    /// @param n Число для проверки.
    /// @return true, если n простое, иначе false.
    bool isPrime(size_t n) const {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        for (size_t i = 5; i * i <= n; i = i + 6) {
            if (n % i == 0 || n % (i + 2) == 0) return false;
        }
        return true;
    }

    /// @brief Находит следующее простое число, не меньшее n.
    /// Используется для выбора оптимального размера хеш-таблицы.
    /// @param n Начальное число.
    /// @return Ближайшее простое число >= n.
    size_t findNextPrime(size_t n) const {
        if (n <= 2) return 2;
        n = static_cast<size_t>(static_cast<double>(n) * 1.5);
        if (n % 2 == 0) n++;
        while (!isPrime(n)) {
            n += 2;
        }
        return n;
    }


public:
    /// @brief Конструктор хеш-таблицы.
    /// @param expected_elements Ожидаемое количество элементов (для выбора размера таблицы).
    HashTable(size_t expected_elements) : collision_count(0) {
        table_size = findNextPrime(std::max(static_cast<size_t>(1), expected_elements));
        table.resize(table_size);
    }

    /// @brief Вставляет объект DataObject в хеш-таблицу.
    /// @param obj Объект для вставки. Сложность в среднем O(1), в худшем O(N).
    void insert(const DataObject& obj) {
        if (table_size == 0) {
             *this = HashTable(1);
        }

        size_t index = hashFunction(obj.key);

        if (index >= table_size) {
            std::cerr << "Ошибка хеш-функции: Индекс " << index << " вне диапазона [0, " << table_size - 1 << "]" << std::endl;
            return;
        }


        if (!table[index].empty()) {
             bool key_already_present_in_bucket = false;
             for(const auto& existing_obj : table[index]) {
                 if (existing_obj.key == obj.key) {
                     key_already_present_in_bucket = true;
                     break;
                 }
             }
             if (!key_already_present_in_bucket) {
                 collision_count++;
             }
        }
        table[index].push_back(obj);
    }

    /// @brief Ищет все объекты с заданным ключом в хеш-таблице.
    /// @param searchKey Ключ для поиска.
    /// @return Вектор найденных объектов DataObject. Сложность в среднем O(1 + k), в худшем O(N + k), где k - число найденных.
    std::vector<DataObject> search(const std::string& searchKey) const {
        std::vector<DataObject> results;
        if (table_size == 0) return results;

        size_t index = hashFunction(searchKey);
        if (index >= table_size) return results;

        const auto& bucket = table[index];
        for (const auto& obj : bucket) {
            if (obj.key == searchKey) {
                results.push_back(obj);
            }
        }
        return results;
    }

    /// @brief Возвращает количество коллизий, зафиксированных при вставках.
    /// @return Число коллизий.
    size_t getCollisionCount() const {
        return collision_count;
    }

    /// @brief Строит хеш-таблицу из существующего вектора данных.
    /// @param data Вектор объектов DataObject.
    void build(const std::vector<DataObject>& data) {
        *this = HashTable(data.size());

        for(const auto& obj : data) {
            insert(obj);
        }
    }
};


/// @brief Шаблонная функция для измерения времени выполнения функции в наносекундах.
/// @tparam Func Тип вызываемой функции (или лямбда-выражения).
/// @tparam Args Типы аргументов функции.
/// @param func Функция для измерения.
/// @param args Аргументы, передаваемые в функцию func.
/// @return Время выполнения функции в наносекундах (long long).
template <typename Func, typename... Args>
long long measureTime(Func func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


/// @brief Основная функция программы.
/// Выполняет генерацию данных, построение структур, поиск и измерение времени.
/// Сохраняет результаты в CSV-файлы для построения графиков.
/// @param argc Количество аргументов командной строки.
/// @param argv Массив аргументов командной строки.
/// @return 0 в случае успешного выполнения.
int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);

    std::vector<size_t> sizes = {100, 300, 500, 1000, 3000, 5000, 10000, 30000, 50000, 100000, 300000, 500000, 1000000};
    const int SEARCH_ITERATIONS = 10000;

    std::ofstream time_results_file("results/search_times_ns.csv");
    std::ofstream collision_results_file("results/hash_collisions.csv");

    time_results_file << "Size,Linear_Search_ns,BST_Search_ns,RBT_Search_ns,HashTable_Search_ns,Multimap_Search_ns\n";
    collision_results_file << "Size,Collisions\n";

    std::mt19937 gen(std::random_device{}());

    for (size_t size : sizes) {
        std::cout << "Обрабатываемый размер: " << size << std::endl;

        std::vector<DataObject> data = generateData(size);
        if (data.empty() && size > 0) {
            std::cerr << "Предупреждение: Сгенерированы пустые данные для размера " << size << std::endl;
            time_results_file << size << ",0,0,0,0,0\n";
            collision_results_file << size << ",0\n";
            std::cout << "-------------------------------------\n";
            continue;
        }
        if (data.empty() && size == 0) {
             time_results_file << size << ",0,0,0,0,0\n";
             collision_results_file << size << ",0\n";
             std::cout << "-------------------------------------\n";
             continue;
        }

        std::uniform_int_distribution<> data_idx_dist(0, data.size() - 1);
        std::string searchKey = data[data_idx_dist(gen)].key;
        std::cout << "  Поиск по ключу: \"" << searchKey << "\"" << std::endl;

        long long total_linear_time = 0;
        long long total_bst_time = 0;
        long long total_rbt_time = 0;
        long long total_hashtable_time = 0;
        long long total_multimap_time = 0;

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_linear_time += measureTime([&]() {
                volatile auto results = linearSearch(data, searchKey);
            });
        }
        long long avg_linear_time = (SEARCH_ITERATIONS > 0) ? total_linear_time / SEARCH_ITERATIONS : 0;
        std::cout << "  Линейный поиск Среднее время:     " << avg_linear_time << " нс" << std::endl;

        BSTNode* bstRoot = nullptr;
        measureTime([&]() {
             for (const auto& obj : data) {
                 insertBST(bstRoot, obj);
             }
         });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_bst_time += measureTime([&]() {
                volatile auto results = searchBST(bstRoot, searchKey);
            });
        }
        destroyBST(bstRoot);
        long long avg_bst_time = (SEARCH_ITERATIONS > 0) ? total_bst_time / SEARCH_ITERATIONS : 0;
        std::cout << "  BST поиск Среднее время:          " << avg_bst_time << " нс" << std::endl;

        RedBlackTree rbt;
        measureTime([&]() {
            rbt.build(data);
        });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_rbt_time += measureTime([&]() {
                volatile auto results = rbt.search(searchKey);
            });
        }
        long long avg_rbt_time = (SEARCH_ITERATIONS > 0) ? total_rbt_time / SEARCH_ITERATIONS : 0;
        std::cout << "  RBT поиск Среднее время:          " << avg_rbt_time << " нс" << std::endl;

        HashTable hashTable(size);
        measureTime([&]() {
            hashTable.build(data);
        });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_hashtable_time += measureTime([&]() {
                volatile auto results = hashTable.search(searchKey);
            });
        }
        long long avg_hashtable_time = (SEARCH_ITERATIONS > 0) ? total_hashtable_time / SEARCH_ITERATIONS : 0;
        size_t collisions = hashTable.getCollisionCount();
        std::cout << "  Хеш-таблица поиск Среднее время:  " << avg_hashtable_time << " нс" << std::endl;
        std::cout << "  Хеш-таблица Коллизии:             " << collisions << std::endl;

        std::multimap<std::string, DataObject> multiMap;
        measureTime([&]() {
             for (const auto& obj : data) {
                 multiMap.insert({obj.key, obj});
             }
         });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_multimap_time += measureTime([&]() {
                volatile auto range = multiMap.equal_range(searchKey);
            });
        }
        long long avg_multimap_time = (SEARCH_ITERATIONS > 0) ? total_multimap_time / SEARCH_ITERATIONS : 0;
        std::cout << "  std::multimap поиск Среднее время: " << avg_multimap_time << " нс" << std::endl;

        time_results_file << size << ","
                          << avg_linear_time << ","
                          << avg_bst_time << ","
                          << avg_rbt_time << ","
                          << avg_hashtable_time << ","
                          << avg_multimap_time << "\n";

        collision_results_file << size << "," << collisions << "\n";
        std::cout << "-------------------------------------\n";
    }

    time_results_file.close();
    collision_results_file.close();

    std::cout << "\nРезультаты сохранены в search_times_ns.csv и hash_collisions.csv" << std::endl;

    return 0;
}
