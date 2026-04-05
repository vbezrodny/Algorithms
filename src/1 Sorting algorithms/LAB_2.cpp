#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <cstring>

#define M_CONST 15 // [5; 20]

using namespace std;
using namespace std::chrono;

// Глобальный счетчик операций сравнений для оценки сложности
unsigned long long comparison_count = 0;

void run_complexity_analysis();
template <typename T> void generate_ordered_seq(T* array, size_t size, T min_value, T max_value, T interval);
template <typename T> void generate_reordered_seq(T* array, size_t size, T min_value, T max_value, T interval);
template <typename T> void generate_random_seq(T* array, size_t size, T min_value, T max_value, T interval);
template <typename T> void generate_quasi_seq(T* array, size_t size, T min_value, T max_value, T interval);

template <typename T> void insertion_sort(T* array, size_t size);
template <typename T> void counting_sort_float(T* array, size_t size);
template <typename T> void modified_quick_sort(T* array, size_t size, size_t M = M_CONST);
template <typename T> bool compare(const T& a, const T& b);
int compare_qsort(const void* a, const void* b);

template <typename T, typename Func> void test_algorithm(Func sort_func, const string& algo_name,
                                                         const string& seq_type, T* array, size_t size);
void reset_counters();


int main()
{
    run_complexity_analysis();
    cout << "Analysis complete. Results saved to results.csv" << endl;
    return 0;
}

// Основная функция анализа
void run_complexity_analysis()
{
    const size_t sizes[] = {5000, 10000, 15000, 20000, 25000, 30000, 35000, 40000, 45000, 50000};

    // Создаем CSV-файл
    ofstream out("results.csv");
    out << "Algorithm,SequenceType,Size,Time(ms),Comparisons\n";
    out.close();

    for (const size_t size : sizes) {
        auto* array = new float[size];

        // Тестируем на разных типах последовательностей
        const char* seq_types[] = {"ordered", "reordered", "random", "quasi"};

        for (const char* seq_type : seq_types) {
            if (strcmp(seq_type, "ordered") == 0) {
                generate_ordered_seq(array, size, -1000.0f, 1000.0f, 0.0f);
            } else if (strcmp(seq_type, "reordered") == 0) {
                generate_reordered_seq(array, size, -1000.0f, 1000.0f, 0.0f);
            } else if (strcmp(seq_type, "random") == 0) {
                generate_random_seq(array, size, -1000.0f, 1000.0f, 0.0f);
            } else if (strcmp(seq_type, "quasi") == 0) {
                generate_quasi_seq(array, size, -1000.0f, 1000.0f, 500.0f);
            }

            // Создаем копию для каждого алгоритма
            auto* array_copy = new float[size];

            // Тестируем разные алгоритмы
            vector<pair<string, void(*)(float*, size_t)>> algorithms = {
                {"InsertionSort", insertion_sort<float>},
                {"QuickSort", [](float* a, size_t s) { modified_quick_sort(a, s, M_CONST); }},
                {"CountingSort", counting_sort_float<float>},
                {"Qsort", [](float* a, size_t s) { qsort(a, s,
                                                                sizeof(float), compare_qsort); }}
            };

            for (auto& algo : algorithms) {
                copy(array, array + size, array_copy);
                test_algorithm(algo.second, algo.first, seq_type, array_copy, size);
            }

            delete[] array_copy;
        }

        delete[] array;
    }
}

/* Sequences generation */
// (re-)ordered sequences
template <typename T>
void generate_ordered_seq(T* array, size_t size, T min_value, T max_value, T interval)
{
    if (!size) return;

    if constexpr (is_floating_point_v<T>) {
        const T step = (max_value - min_value) / static_cast<T>(size - 1);
        for (size_t i = 0; i < size; ++i) {
            array[i] = min_value + static_cast<T>(i) * step;
        }
    } else {
        for (size_t i = 0; i < size; ++i) {
            array[i] = min_value + static_cast<T>(i);
        }
    }
}

template <typename T>
void generate_reordered_seq(T* array, size_t size, T min_value, T max_value, T interval)
{
    if (!size) return;

    if constexpr (is_floating_point_v<T>) {
        const T step = (max_value - min_value) / static_cast<T>(size - 1);
        for (size_t i = 0; i < size; ++i) {
            array[i] = max_value - static_cast<T>(i) * step;
        }
    } else {
        for (size_t i = 0; i < size; ++i) {
            array[i] = max_value - static_cast<T>(i);
        }
    }
}

// random sequence
template <typename T>
void generate_random_seq(T* array, size_t size, T min_value, T max_value, T interval)
{
    if (!size) return;

    random_device rd;
    mt19937 gen(rd());

    if constexpr (is_integral_v<T>) {
        uniform_int_distribution<T> dis(min_value, max_value);
        for (size_t i = 0; i < size; ++i) {
            array[i] = dis(gen);
        }
    } else if constexpr (is_floating_point_v<T>) {
        uniform_real_distribution<T> dis(min_value, max_value);
        for (size_t i = 0; i < size; ++i) {
            array[i] = dis(gen);
        }
    }
}

// quasi sequence
template <typename T>
void generate_quasi_seq(T* array, size_t size, T min_value, T max_value, T interval)
{
    if (!size || !interval) return;

    random_device rd;
    mt19937 gen(rd());

    if constexpr (is_integral_v<T>) {
        uniform_int_distribution<T> dis(min_value, interval);
        for (size_t i = 0; i < size; ++i) {
            array[i] = (i % 2 == 1) ? static_cast<T>(i) - dis(gen)
                                    : static_cast<T>(i) + dis(gen);
        }
    } else if constexpr (is_floating_point_v<T>) {
        uniform_real_distribution<T> dis(min_value, interval);
        for (size_t i = 0; i < size; ++i) {
            array[i] = (i % 2 == 1) ? static_cast<T>(i) - dis(gen)
                                    : static_cast<T>(i) + dis(gen);
        }
    }
}

/* Sorting algoritms */
// Модифицированная сортировка вставками с подсчетом операций
template <typename T>
void insertion_sort(T* array, size_t size)
{
    for (size_t i = 1; i < size; ++i) {
        T key = array[i];
        int j = static_cast<int>(i) - 1;

        while (j >= 0) {
            comparison_count++;
            if (!compare(array[j], key)) {
                array[j + 1] = array[j];
                j--;
            } else {
                break;
            }
        }
        array[j + 1] = key;
    }
}

// Модифицированная сортировка подсчетом с подсчетом операций
template <typename T>
void counting_sort_float(T* array, size_t size)
{
    if (size == 0) return;

    T min_val = array[0];
    T max_val = array[0];

    for (size_t i = 1; i < size; ++i) {
        if (compare(array[i], min_val)) {
            min_val = array[i];
        }
        if (array[i] > max_val) {
            comparison_count++;
            max_val = array[i];
        }
    }

    const size_t range = static_cast<size_t>(max_val - min_val) + 1;
    if (range > 1000000) {
        insertion_sort(array, size);
        return;
    }

    vector<size_t> count(range, 0);

    for (size_t i = 0; i < size; ++i) {
        ++count[static_cast<size_t>(array[i] - min_val)];
    }

    size_t index = 0;
    for (size_t i = 0; i < range; ++i) {
        while (count[i] > 0) {
            array[index++] = static_cast<T>(i) + min_val;
            count[i]--;
        }
    }
}

// Модифицированная быстрая сортировка с подсчетом операций
template <typename T>
void modified_quick_sort(T* array, size_t size, size_t M)
{
    if (size <= M) {
        insertion_sort(array, size);
        return;
    }

    // Выбор медианы из трех
    size_t mid = size / 2;
    if (compare(array[size-1], array[0])) swap(array[0], array[size-1]);
    if (compare(array[mid], array[0])) swap(array[0], array[mid]);
    if (compare(array[size-1], array[mid])) swap(array[mid], array[size-1]);

    T pivot = array[mid];

    // Разделение
    size_t i = 0, j = size - 1;

    while (i <= j) {
        while (compare(array[i], pivot)) {
            i++;
        }
        while (compare(pivot, array[j])) {
            j--;
        }
        if (i <= j) {
            swap(array[i], array[j]);
            i++;
            j--;
        }
    }

    if (j > 0) modified_quick_sort(array, j + 1, M);
    if (i < size) modified_quick_sort(array + i, size - i, M);
}

// Функция для сравнения с подсчетом
template <typename T>
bool compare(const T& a, const T& b)
{
    comparison_count++;
    return a < b;
}

// Компаратор для qsort
int compare_qsort(const void* a, const void* b)
{
    comparison_count++;
    float fa = *((float*)a);
    float fb = *((float*)b);
    return (fa > fb) - (fa < fb);
}

/* Testing algoritms */
// Тестирование алгоритма
template <typename T, typename Func>
void test_algorithm(Func sort_func, const string& algo_name, const string& seq_type, T* array, size_t size)
{
    reset_counters();

    auto start = high_resolution_clock::now();
    sort_func(array, size);
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);

    // Запись результатов в файл
    ofstream out("results.csv", ios::app);
    out << algo_name << "," << seq_type << "," << size << ","
        << duration.count() << "," << comparison_count << "\n";
    out.close();
}

void reset_counters()
{
    comparison_count = 0;
}
