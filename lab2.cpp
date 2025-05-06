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
#include <windows.h> // Для функций SetConsoleOutputCP


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
    // Можно добавить другие поля

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

// --- Генерация Данных ---

/// @brief Генерирует вектор объектов DataObject заданного размера.
/// @param size Количество объектов для генерации.
/// @return Вектор сгенерированных объектов DataObject.
/// @note Ключи генерируются таким образом, чтобы допустить дубликаты.
std::vector<DataObject> generateData(size_t size) {
    std::vector<DataObject> data;
    if (size == 0) return data; // Обработка пустого размера явно

    data.reserve(size);
    std::mt19937 gen(std::random_device{}()); // Генератор случайных чисел

    // Генерируем ключи (допускаем повторения)
    size_t num_unique_keys = std::max(static_cast<size_t>(10), size / 5); // Ограничиваем число уникальных ключей
    std::vector<std::string> possible_keys;
    possible_keys.reserve(num_unique_keys);
    std::uniform_int_distribution<> char_dist('a', 'z');
    for (size_t i = 0; i < num_unique_keys; ++i) {
        std::string key = "";
        size_t key_len = std::uniform_int_distribution<size_t>(3, 10)(gen); // Длина ключей от 3 до 10 символов
        for(size_t j = 0; j < key_len; ++j) {
            key += char_dist(gen);
        }
        possible_keys.push_back(key);
    }

    // Проверка на случай, если num_unique_keys было 0 (например, при size < 50, если использовать только size/5)
    if (possible_keys.empty()) {
         // Добавим хотя бы один ключ, если вдруг possible_keys пуст
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

// --- 1. Методы Поиска ---

// 1.1 Линейный Поиск

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

// 1.2 Бинарное Дерево Поиска (BST)

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
    // Вставляем даже дубликаты ключей
    if (obj.key < node->data.key) {
        insertBST(node->left, std::move(obj));
    } else { // Ключ >= ключа узла, идем направо (включая дубликаты)
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

    // Сравниваем ключ текущего узла с искомым
    if (searchKey == node->data.key) {
        results.push_back(node->data);
        // --- ИЗМЕНЕНИЕ НАЧАЛО ---
        // Ключ найден. Ищем ДУБЛИКАТЫ ТОЛЬКО в ПРАВОМ поддереве,
        // так как вставка ">= направо" гарантирует, что слева их быть не может.
        // searchBSTRecursive(node->left, searchKey, results); // <-- УДАЛЕНО: Поиск влево при равенстве избыточен
        searchBSTRecursive(node->right, searchKey, results); // <-- ОСТАВЛЕНО: Продолжаем поиск дубликатов только справа
        // --- ИЗМЕНЕНИЕ КОНЕЦ ---
    } else if (searchKey < node->data.key) {
        // Искомый ключ меньше, идем только влево
        searchBSTRecursive(node->left, searchKey, results);
    } else { // searchKey > node->data.key
        // Искомый ключ больше, идем только вправо
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

// 1.3 Красно-Черное Дерево (Red-Black Tree - RBT)

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
    // RBTNode* NIL; // Классическая реализация использует узел-часовой NIL, мы используем nullptr

    /// @brief Выполняет левый поворот вокруг узла x.
    /// @param x Узел, вокруг которого выполняется поворот.
    /// @pre Правый потомок x не должен быть nullptr.
    void leftRotate(RBTNode* x) {
        RBTNode* y = x->right; // y - правый потомок x
        if (!y) return;

        x->right = y->left;   // 1. Левое поддерево y становится правым поддеревом x
        if (y->left) {
            y->left->parent = x;
        }

        y->parent = x->parent; // 2. Родитель x становится родителем y
        if (!x->parent) {      // Если x был корнем...
            root = y;          // ...то y становится новым корнем
        } else if (x == x->parent->left) { // Если x был левым потомком...
            x->parent->left = y;          // ...то y становится левым потомком
        } else {                         // Если x был правым потомком...
            x->parent->right = y;         // ...то y становится правым потомком
        }

        y->left = x;          // 3. x становится левым потомком y
        x->parent = y;          // 4. y становится родителем x
    }

    /// @brief Выполняет правый поворот вокруг узла y.
    /// @param y Узел, вокруг которого выполняется поворот.
    /// @pre Левый потомок y не должен быть nullptr.
    void rightRotate(RBTNode* y) {
        RBTNode* x = y->left; // x - левый потомок y
        if (!x) return;

        y->left = x->right;   // 1. Правое поддерево x становится левым поддеревом y
        if (x->right) {
            x->right->parent = y;
        }

        x->parent = y->parent; // 2. Родитель y становится родителем x
        if (!y->parent) {      // Если y был корнем...
            root = x;          // ...то x становится новым корнем
        } else if (y == y->parent->left) { // Если y был левым потомком...
            y->parent->left = x;          // ...то x становится левым потомком
        } else {                         // Если y был правым потомком...
            y->parent->right = x;         // ...то x становится правым потомком
        }

        x->right = y;         // 3. y становится правым потомком x
        y->parent = x;         // 4. x становится родителем y
    }

    /// @brief Восстанавливает свойства Красно-Черного дерева после вставки узла.
    /// @param z Вставленный узел (изначально красный).
    void insertFixup(RBTNode* z) {
        // Пока родитель z красный (нарушение свойства 4)
        while (z->parent && z->parent->color == RED) {
            RBTNode* parent = z->parent;
            RBTNode* grandparent = parent->parent;
            // Если дедушки нет, то родитель - корень, он должен стать черным в конце.
            if (!grandparent) break;

            if (parent == grandparent->left) { // Родитель z - левый потомок дедушки
                RBTNode* uncle = grandparent->right; // Дядя z
                if (uncle && uncle->color == RED) {
                    // --- Случай 1: Дядя красный ---
                    parent->color = BLACK;       // Перекрасить родителя в черный
                    uncle->color = BLACK;        // Перекрасить дядю в черный
                    grandparent->color = RED;      // Перекрасить дедушку в красный
                    z = grandparent;             // Переместить z на дедушку и продолжить проверку вверх
                } else { // Дядя черный (или nullptr)
                    if (z == parent->right) {
                        // --- Случай 2: Дядя черный, z - правый потомок (треугольник) ---
                        z = parent; // Переместить z на родителя
                        leftRotate(z); // Левый поворот вокруг z (бывшего родителя)
                        parent = z->parent; // Обновить указатель на родителя после поворота
                        grandparent = parent ? parent->parent : nullptr; // Обновить указатель на дедушку (с проверкой)
                    }
                    // --- Случай 3: Дядя черный, z - левый потомок (линия) ---
                    // Важно: проверяем grandparent снова, т.к. он мог стать nullptr после поворота в случае 2
                    if(parent) parent->color = BLACK;        // Перекрасить родителя (возможно, нового после поворота) в черный
                    if(grandparent) {
                         grandparent->color = RED;       // Перекрасить дедушку в красный
                         rightRotate(grandparent); // Правый поворот вокруг дедушки
                    }
                }
            } else { // Родитель z - правый потомок дедушки (зеркальные случаи)
                RBTNode* uncle = grandparent->left; // Дядя z
                if (uncle && uncle->color == RED) {
                    // --- Случай 1 (зеркальный): Дядя красный ---
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    grandparent->color = RED;
                    z = grandparent;
                } else { // Дядя черный
                    if (z == parent->left) {
                        // --- Случай 2 (зеркальный): Дядя черный, z - левый потомок (треугольник) ---
                        z = parent;
                        rightRotate(z);
                        parent = z->parent; // Обновить указатель на родителя
                        grandparent = parent ? parent->parent : nullptr; // Обновить указатель на дедушку
                    }
                    // --- Случай 3 (зеркальный): Дядя черный, z - правый потомок (линия) ---
                    if(parent) parent->color = BLACK;
                    if(grandparent) {
                        grandparent->color = RED;
                        leftRotate(grandparent);
                    }
                }
            }
        }
        if(root) root->color = BLACK; // Корень всегда должен быть черным (Свойство 2)
    }

    /// @brief Рекурсивно ищет все объекты с заданным ключом в RBT.
    /// @param node Текущий узел для проверки.
    /// @param searchKey Ключ для поиска.
    /// @param results Вектор для накопления найденных объектов.
    void searchRecursive(RBTNode* node, const std::string& searchKey, std::vector<DataObject>& results) const {
        if (node == nullptr) {
            return;
        }

        // --- ИЗМЕНЕНИЕ НАЧАЛО ---
        // Логика поиска скорректирована для большей ясности и эффективности
        if (searchKey == node->data.key) {
            results.push_back(node->data);
            // Ключ найден. Ищем ДУБЛИКАТЫ ТОЛЬКО в ПРАВОМ поддереве,
            // так как вставка ">= направо" гарантирует, что слева их быть не может.
            // searchRecursive(node->left, searchKey, results); // <-- УДАЛЕНО: Поиск влево при равенстве избыточен
            searchRecursive(node->right, searchKey, results); // <-- ОСТАВЛЕНО: Продолжаем поиск дубликатов только справа
        } else if (searchKey < node->data.key) {
            // Искомый ключ меньше -> идем только влево
            searchRecursive(node->left, searchKey, results);
        } else { // searchKey > node->data.key
            // Искомый ключ больше -> идем только вправо
            searchRecursive(node->right, searchKey, results);
        }
        // --- ИЗМЕНЕНИЕ КОНЕЦ ---
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
        RBTNode* z = new RBTNode(std::move(obj)); // Новый узел всегда RED
        RBTNode* y = nullptr; // Будущий родитель z
        RBTNode* x = root;   // Текущий узел для поиска места вставки

        // 1. Обычная вставка как в BST
        while (x) {
            y = x; // Запоминаем родителя
            if (z->data.key < x->data.key) {
                x = x->left;
            } else { // >= идет направо (разрешаем дубликаты)
                x = x->right;
            }
        }

        z->parent = y; // Устанавливаем родителя для z
        if (!y) {
            root = z; // Дерево было пустым, z становится корнем
        } else if (z->data.key < y->data.key) {
            y->left = z;
        } else {
            y->right = z;
        }

        // z->left и z->right уже инициализированы как nullptr (наши "NIL")
        // z->color инициализирован как RED

        // 2. Восстанавливаем свойства RBT
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
        // Очищаем старое дерево, если оно было
        destroyRecursive(root);
        root = nullptr;
        // Вставляем элементы
        for(const auto& obj : data) {
            insert(obj);
        }
    }
};


// 1.4 Хеш-таблица (с методом цепочек)

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
        // Простое хеширование модулем размера таблицы
        // Добавим проверку на table_size > 0, чтобы избежать деления на ноль
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
        if (n <= 2) return 2; // Минимальный размер простого числа
        // Увеличиваем n, чтобы обеспечить хороший коэффициент загрузки (load factor < 1)
        // Например, сделаем размер таблицы примерно в 1.5-2 раза больше ожидаемого числа элементов
        n = static_cast<size_t>(static_cast<double>(n) * 1.5);
        if (n % 2 == 0) n++; // Начать с нечетного, если нужно
        while (!isPrime(n)) {
            n += 2; // Проверяем только нечетные числа
        }
        return n;
    }


public:
    /// @brief Конструктор хеш-таблицы.
    /// @param expected_elements Ожидаемое количество элементов (для выбора размера таблицы).
    HashTable(size_t expected_elements) : collision_count(0) {
        // Выбираем размер таблицы как простое число, большее ожидаемого количества элементов
        // Минимальный размер для случая 0 или 1 элемента
        table_size = findNextPrime(std::max(static_cast<size_t>(1), expected_elements));
        table.resize(table_size);
    }

    /// @brief Вставляет объект DataObject в хеш-таблицу.
    /// @param obj Объект для вставки. Сложность в среднем O(1), в худшем O(N).
    void insert(const DataObject& obj) {
        // Добавим проверку, что table_size больше 0 перед вставкой
        if (table_size == 0) {
             // Возможно, стоит пересоздать таблицу с минимальным размером
             *this = HashTable(1); // Пересоздаем с минимальным размером
        }

        size_t index = hashFunction(obj.key);

        // Проверка на выход за границы (маловероятно, но безопасно)
        if (index >= table_size) {
            std::cerr << "Ошибка хеш-функции: Индекс " << index << " вне диапазона [0, " << table_size - 1 << "]" << std::endl;
            return;
        }


        // Подсчет коллизий (один из способов):
        // Коллизия происходит, если мы добавляем элемент в непустую цепочку,
        // при этом добавляемый ключ ОТЛИЧАЕТСЯ от ключей уже имеющихся там элементов.
        if (!table[index].empty()) {
             bool key_already_present_in_bucket = false;
             for(const auto& existing_obj : table[index]) {
                 if (existing_obj.key == obj.key) {
                     key_already_present_in_bucket = true;
                     break; // Нашли такой же ключ, это не новая коллизия хешей ДЛЯ РАЗНЫХ КЛЮЧЕЙ
                 }
             }
             // Если список не пуст И мы добавляем элемент с НОВЫМ для этой ячейки ключом
             if (!key_already_present_in_bucket) {
                 collision_count++; // Регистрируем коллизию хешей для разных ключей
             }
        }
        // Непосредственно вставка в цепочку (список)
        table[index].push_back(obj);
    }

    /// @brief Ищет все объекты с заданным ключом в хеш-таблице.
    /// @param searchKey Ключ для поиска.
    /// @return Вектор найденных объектов DataObject. Сложность в среднем O(1 + k), в худшем O(N + k), где k - число найденных.
    std::vector<DataObject> search(const std::string& searchKey) const {
        std::vector<DataObject> results;
        if (table_size == 0) return results; // Если таблица пуста/не инициализирована

        size_t index = hashFunction(searchKey); // Находим "корзину"
        if (index >= table_size) return results; // Защита на случай очень странного хеширования

        const auto& bucket = table[index]; // Получаем ссылку на список (цепочку)
        // Линейный поиск внутри цепочки
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
        // Пересоздаем таблицу с размером, подходящим под новые данные
        // (это сбрасывает коллизии и очищает старые данные)
        *this = HashTable(data.size()); // Используем оператор присваивания для инициализации

        // Вставляем элементы
        for(const auto& obj : data) {
            insert(obj);
        }
    }
};

// 4. Ассоциативный массив std::multimap (для сравнения)
// std::multimap обычно реализуется на основе сбалансированного дерева (часто RBT)

// --- Измерение времени ---

/// @brief Шаблонная функция для измерения времени выполнения функции в наносекундах.
/// @tparam Func Тип вызываемой функции (или лямбда-выражения).
/// @tparam Args Типы аргументов функции.
/// @param func Функция для измерения.
/// @param args Аргументы, передаваемые в функцию func.
/// @return Время выполнения функции в наносекундах (long long).
template <typename Func, typename... Args>
long long measureTime(Func func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...); // Выполняем функцию с переданными аргументами
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(); // Используем nanoseconds
}


/// @brief Основная функция программы.
/// Выполняет генерацию данных, построение структур, поиск и измерение времени.
/// Сохраняет результаты в CSV-файлы для построения графиков.
/// @param argc Количество аргументов командной строки.
/// @param argv Массив аргументов командной строки.
/// @return 0 в случае успешного выполнения.
int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);


    // Размеры массивов для тестирования
    std::vector<size_t> sizes = {100, 300, 500, 1000, 3000, 5000, 10000, 30000, 50000, 100000, 300000, 500000, 1000000};
    const int SEARCH_ITERATIONS = 10000;

    // Открываем файлы для записи результатов
    std::ofstream time_results_file("results/search_times_ns.csv");
    std::ofstream collision_results_file("results/hash_collisions.csv");

    // Записываем заголовки CSV файлов
    time_results_file << "Size,Linear_Search_ns,BST_Search_ns,RBT_Search_ns,HashTable_Search_ns,Multimap_Search_ns\n";
    collision_results_file << "Size,Collisions\n";

    // Генератор случайных чисел для выбора ключа поиска
    std::mt19937 gen(std::random_device{}());

    // Цикл по разным размерам входных данных
    for (size_t size : sizes) {
        std::cout << "Обрабатываемый размер: " << size << std::endl;

        // 1. Генерация данных
        std::vector<DataObject> data = generateData(size);
        if (data.empty() && size > 0) {
            std::cerr << "Предупреждение: Сгенерированы пустые данные для размера " << size << std::endl;
            // Записываем 0, чтобы строки в CSV соответствовали
            time_results_file << size << ",0,0,0,0,0\n";
            collision_results_file << size << ",0\n";
            std::cout << "-------------------------------------\n";
            continue; // Пропускаем итерацию, если данные пусты (кроме случая size=0)
        }
        if (data.empty() && size == 0) {
             // Для размера 0 просто записываем 0 и переходим к следующему
             time_results_file << size << ",0,0,0,0,0\n";
             collision_results_file << size << ",0\n";
             std::cout << "-------------------------------------\n";
             continue;
        }


        // Выбираем случайный, гарантированно существующий ключ для поиска из сгенерированных данных
        std::uniform_int_distribution<> data_idx_dist(0, data.size() - 1);
        std::string searchKey = data[data_idx_dist(gen)].key;
        std::cout << "  Поиск по ключу: \"" << searchKey << "\"" << std::endl;

        // Переменные для суммарного времени поиска по итерациям
        long long total_linear_time = 0;
        long long total_bst_time = 0;
        long long total_rbt_time = 0;
        long long total_hashtable_time = 0;
        long long total_multimap_time = 0;

        // --- Измерения для данного размера ---

        // 2.1 Линейный поиск
        // Линейный поиск не требует предварительного построения специальной структуры.
        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_linear_time += measureTime([&]() {
                volatile auto results = linearSearch(data, searchKey);
                // (void)results; // Используем volatile, чтобы точно не оптимизировалось
            });
        }
        long long avg_linear_time = (SEARCH_ITERATIONS > 0) ? total_linear_time / SEARCH_ITERATIONS : 0;
        std::cout << "  Линейный поиск Среднее время:     " << avg_linear_time << " нс" << std::endl;

        // 2.2 Бинарное дерево поиска (BST)
        BSTNode* bstRoot = nullptr;
        // Время построения BST (не включаем в замеры поиска)
         measureTime([&]() {
             for (const auto& obj : data) {
                 insertBST(bstRoot, obj);
             }
         });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_bst_time += measureTime([&]() {
                volatile auto results = searchBST(bstRoot, searchKey);
                // (void)results;
            });
        }
        destroyBST(bstRoot); // Очищаем память BST
        long long avg_bst_time = (SEARCH_ITERATIONS > 0) ? total_bst_time / SEARCH_ITERATIONS : 0;
        std::cout << "  BST поиск Среднее время:          " << avg_bst_time << " нс" << std::endl;

        // 2.3 Красно-Черное дерево (RBT)
        RedBlackTree rbt;
        // Время построения RBT (не включаем в замеры поиска)
        measureTime([&]() {
            rbt.build(data);
        });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_rbt_time += measureTime([&]() {
                volatile auto results = rbt.search(searchKey);
                // (void)results;
            });
        }
        // Деструктор rbt сработает автоматически
        long long avg_rbt_time = (SEARCH_ITERATIONS > 0) ? total_rbt_time / SEARCH_ITERATIONS : 0;
        std::cout << "  RBT поиск Среднее время:          " << avg_rbt_time << " нс" << std::endl;


        // 2.4 Хеш-таблица
        HashTable hashTable(size); // Создаем таблицу под нужный размер
        // Время построения хеш-таблицы (не включаем в замеры поиска)
        measureTime([&]() {
            hashTable.build(data);
        });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_hashtable_time += measureTime([&]() {
                volatile auto results = hashTable.search(searchKey);
                // (void)results;
            });
        }
        long long avg_hashtable_time = (SEARCH_ITERATIONS > 0) ? total_hashtable_time / SEARCH_ITERATIONS : 0;
        size_t collisions = hashTable.getCollisionCount();
        std::cout << "  Хеш-таблица поиск Среднее время:  " << avg_hashtable_time << " нс" << std::endl;
        std::cout << "  Хеш-таблица Коллизии:             " << collisions << std::endl;


        // 2.5 std::multimap (как эталонное сбалансированное дерево)
        std::multimap<std::string, DataObject> multiMap;
         // Время построения multimap (не включаем в замеры поиска)
         measureTime([&]() {
             for (const auto& obj : data) {
                 multiMap.insert({obj.key, obj});
             }
         });

        for (int i = 0; i < SEARCH_ITERATIONS; ++i) {
            total_multimap_time += measureTime([&]() {
                // Используем equal_range для поиска всех элементов с ключом
                volatile auto range = multiMap.equal_range(searchKey);
                // Просто измеряем время нахождения диапазона.
                // volatile size_t count = std::distance(range.first, range.second); // Можно добавить для надежности
                (void)range; // Явно используем переменную range
            });
        }
        long long avg_multimap_time = (SEARCH_ITERATIONS > 0) ? total_multimap_time / SEARCH_ITERATIONS : 0;
        std::cout << "  std::multimap поиск Среднее время: " << avg_multimap_time << " нс" << std::endl;


        // Запись усредненных результатов в CSV файлы
        time_results_file << size << ","
                          << avg_linear_time << ","
                          << avg_bst_time << ","
                          << avg_rbt_time << ","
                          << avg_hashtable_time << ","
                          << avg_multimap_time << "\n";

        collision_results_file << size << "," << collisions << "\n";
        std::cout << "-------------------------------------\n";
    }

    // Закрываем файлы
    time_results_file.close();
    collision_results_file.close();

    std::cout << "\nРезультаты сохранены в search_times_ns.csv и hash_collisions.csv" << std::endl;

    return 0;
}
