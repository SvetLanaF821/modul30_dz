#include <future>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <future>
#include "OptimizeThreadPool.h"

constexpr auto ARR_SIZE = 8'000'000;                 //размер массива
bool make_thread = true;

RequestHandler rh;

//быстрая сортировка
void quickSort(std::vector<int>& array, long left, long right)
{
    if (left >= right)
        return;
    long leftBound = left;
    long rightBound = right;
    long middle = left + (right - left) / 2;
    long piv = array[middle];

    while (leftBound < right || rightBound > left) {
        while (array[leftBound] < piv)
            leftBound++;
        while (array[rightBound] > piv)
            rightBound--;

        if (leftBound <= rightBound) {
            std::swap(array[leftBound], array[rightBound]);
            leftBound++;
            rightBound--;
        }
        else {
            if (make_thread && (right - left > 1'000'000)) {
                if (rightBound > left) {
                    std::vector<res_type> results;
                    results.push_back(rh.pushRequest(quickSort, array, left, rightBound));
                    for (auto& r : results) {
                        r.wait();
                    }
                }
                if (leftBound < right)
                    quickSort(array, leftBound, right);
                return;
            }
            else {
                if (rightBound > left)
                    quickSort(array, left, rightBound);
                if (leftBound < right)
                    quickSort(array, leftBound, right);
                return;
            }
        }
    }
}

//проверка сортировки
void sortCheck(std::vector<int>& arr) {
    bool arrSort = true;
    for (int i = 0; i < ARR_SIZE - 1; i++) {
        if (arr[i] > arr[static_cast<ptrdiff_t>(i) + 1]) {
            std::cout << "Вектор не отсортирован\n\n";
            arrSort = false;
            break;
        }
    }
    if (arrSort)
        std::cout << "Вектор отсортирован\n\n";
}

int main() {
    setlocale(LC_ALL, "");

    std::vector<int> arr1(ARR_SIZE);
    std::vector<int> arr2(ARR_SIZE);

    std::cout << "Заполнение вектора (размер " << ARR_SIZE << " элементов)\n";
    srand(time(nullptr));
    for (int i = 0; i < ARR_SIZE; i++) {
        arr1[i] = arr2[i] = rand() % ARR_SIZE;
    }
    std::cout << "Bектор заполнен\n\n";

    make_thread = false;
    std::cout << "Сортировка в основном потоке\n";
    auto start = std::chrono::high_resolution_clock::now();
    quickSort(arr1, 0, ARR_SIZE - 1);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = finish - start;
    std::cout << "Затраченное время: " << elapsedTime.count() << " секунд\n";

    sortCheck(arr1);                             //проверка правильности сортировки 

    make_thread = true;
    std::cout << "Сортировка в пуле\n";
    start = std::chrono::high_resolution_clock::now();
    quickSort(arr2, 0, ARR_SIZE - 1);
    finish = std::chrono::high_resolution_clock::now();
    elapsedTime = finish - start;
    std::cout << "Затраченное время: " << elapsedTime.count() << " секунд\n";

    sortCheck(arr2);                             //проверка правильности сортировки 

    return 0;
}