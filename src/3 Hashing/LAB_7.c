#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <windows.h>

#define MAX_BUILDING      10
#define MAX_APARTMENT     80
#define TOTAL_UNIQUE_KEYS (MAX_BUILDING * MAX_APARTMENT)

typedef struct {
    int Building;
    int Apartment;
} Address;

typedef struct HashNode {
    Address key;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode** buckets;
    int M;                  // Размер хэш-таблицы
    int K;                  // Количество ключей
} HashTable;

typedef struct {
    int maxCollisions;      // Максимальная длина цепочки
    int emptyBuckets;       // Количество пустых корзин
    int totalCollisions;    // Общее число коллизий (сумма (len-1) по всем корзинам)
    double avgLoad;         // Средняя нагрузка
    double stdDev;          // Стандартное отклонение
    double chiSquare;       // Критерий хи-квадрат
} Statistcs;

void generateUniqueKeys(Address* keys, int K);
void testTable(int M, const Address* keys, int K);
HashTable* createHashTable(int M);
void insertKey(HashTable* hashTable, const Address* key);
int hashAddress(const Address* key, int M);
void freeHashTable(HashTable* hashTable);

Statistcs analyzeHashTable(const HashTable* hashTable);
void printHistogram(const HashTable* hashTable);

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    const int M1 = 64;
    const int M2 = 67;
    const int K = 500;

    printf("Параметры:\n");
    printf("  M1 = %d\n", M1);
    printf("  M2 = %d\n", M2);
    printf("  K = %d\n", K);
    printf("  Building: [1; %d]\n", MAX_BUILDING);
    printf("  Apartment: [1; %d]\n", MAX_APARTMENT);
    printf("  Всего уникальных ключей: %d\n\n", TOTAL_UNIQUE_KEYS);

    // Unique keys generation
    Address* keys = (Address*)malloc(K * sizeof(Address));
    generateUniqueKeys(keys, K);

    // Hash function testing
    testTable(M1, keys, K);
    testTable(M2, keys, K);

    free(keys);
    return 0;
}

void generateUniqueKeys(Address* keys, int K)
{
    if (!keys) exit(1);

    if (K > TOTAL_UNIQUE_KEYS) {
        fprintf(stderr, "Error: запрошено %d ключей, но возможно только %d\n", K, TOTAL_UNIQUE_KEYS);
        exit(1);
    }

    Address* all = (Address*)malloc(TOTAL_UNIQUE_KEYS * sizeof(Address));
    int total = 0;
    for (int b = 1; b <= MAX_BUILDING; b++) {
        for (int a = 1; a <= MAX_APARTMENT; a++) {
            all[total].Building = b;
            all[total].Apartment = a;
            total++;
        }
    }

    /* Fieher-Yates shuffle */
    srand(42);
    for (int i = total - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Address temp = all[i];
        all[i] = all[j];
        all[j] = temp;
    }

    for (int i = 0; i < K; i++) {
        keys[i] = all[i];
    }

    free(all);
}

void testTable(int M, const Address* keys, int K)
{
    HashTable* hashTable = createHashTable(M);

    for (int i = 0; i < K; i++) {
        insertKey(hashTable, &keys[i]);
    }

    printf("Разработанная хеш-функция (M = %d)\n", M);
    printf("Актуальных ключей в таблице: %d\n", hashTable->K);

    Statistcs stats = analyzeHashTable(hashTable);
    printf("\nСтатистика:\n");
    printf("  Средняя нагрузка на корзину: %.2f\n", stats.avgLoad);
    printf("  Максимальная длина цепочки:  %d\n", stats.maxCollisions);
    printf("  Пустых корзин:               %d (%.1f%%)\n",
           stats.emptyBuckets, 100.0 * stats.emptyBuckets / M);
    printf("  Общее число коллизий:        %d\n", stats.totalCollisions);
    printf("  Стандартное отклонение:      %.2f\n", stats.stdDev);
    printf("  Хи-квадрат:                  %.2f\n", stats.chiSquare);

    if (M <= 80) {
        printHistogram(hashTable);
    }

    freeHashTable(hashTable);
}

HashTable* createHashTable(int M)
{
    HashTable* hashTable = (HashTable*)malloc(sizeof(HashTable));
    hashTable->M = M;
    hashTable->K = 0;
    hashTable->buckets = (HashNode**)calloc(M, sizeof(HashNode*));
    return hashTable;
}

void insertKey(HashTable* hashTable, const Address* key)
{
    if (!hashTable || !hashTable->buckets || !key) exit(1);

    int index = hashAddress(key, hashTable->M);

    // check existing key
    HashNode* current = hashTable->buckets[index];
    while (current) {
        if (current->key.Building == key->Building &&
            current->key.Apartment == key->Apartment) return;
        current = current->next;
    }

    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    newNode->key = *key;
    newNode->next = hashTable->buckets[index];
    hashTable->buckets[index] = newNode;
    hashTable->K++;
}

int hashAddress(const Address* key, int M)
{
    int combined = key->Building * 1000 + key->Apartment;
    return (int)(0.616161 * (float)combined) % M;
}

void freeHashTable(HashTable* hashTable)
{
    if (!hashTable || !hashTable->buckets) exit(1);

    for (int i = 0; i < hashTable->M; i++) {
        HashNode* current = hashTable->buckets[i];
        while (current) {
            HashNode* temp = current;
            current = current->next;
            free(temp);
        }
    }

    free(hashTable->buckets);
    free(hashTable);
}

Statistcs analyzeHashTable(const HashTable* hashTable) {
    if (!hashTable || !hashTable->buckets) exit(1);

    Statistcs stats = {0, 0, 0, 0.0, 0.0, 0.0};

    if (hashTable->K == 0) return stats;

    int* lengths = (int*)calloc(hashTable->M, sizeof(int));

    for (int i = 0; i < hashTable->M; i++) {
        int len = 0;
        HashNode* current = hashTable->buckets[i];
        while (current) {
            len++;
            current = current->next;
        }
        lengths[i] = len;

        if (len == 0) stats.emptyBuckets++;
        if (len > stats.maxCollisions) stats.maxCollisions = len;
        if (len > 1) stats.totalCollisions += (len - 1);
    }

    // Средняя нагрузка
    stats.avgLoad = (double)hashTable->K / hashTable->M;

    // Стандартное отклонение
    double sumSq = 0.0;
    for (int i = 0; i < hashTable->M; i++) {
        double diff = lengths[i] - stats.avgLoad;
        sumSq += diff * diff;
    }
    stats.stdDev = sqrt(sumSq / hashTable->M);

    // Хи-квадрат (ожидаемое число элементов в корзине = K/M)
    double expected = stats.avgLoad;
    double chi = 0.0;
    for (int i = 0; i < hashTable->M; i++) {
        double diff = lengths[i] - expected;
        chi += (diff * diff) / expected;
    }
    stats.chiSquare = chi;

    free(lengths);
    return stats;
}

void printHistogram(const HashTable* hashTable) {
    if (!hashTable || !hashTable->buckets) exit(1);
    if (hashTable->M > 80) {
        printf("Размер таблицы слишком велик для детальной гистограммы.\n");
        exit(1);
    }

    int* lengths = (int*)calloc(hashTable->M, sizeof(int));
    int maxLen = 0;

    for (int i = 0; i < hashTable->M; i++) {
        HashNode* current = hashTable->buckets[i];
        while (current) {
            lengths[i]++;
            current = current->next;
        }
        if (lengths[i] > maxLen) maxLen = lengths[i];
    }

    printf("\nГистограмма загрузки корзин:\n");
    printf("(каждый '#' представляет 1 ключ)\n\n");

    for (int i = 0; i < hashTable->M; i++) {
        printf("%3d | ", i);
        for (int j = 0; j < lengths[i]; j++) {
            printf("#");
        }
        printf(" (%d)\n", lengths[i]);
    }

    printf("\n");

    free(lengths);
}