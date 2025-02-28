#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <random>
#include <numeric>
#include <tuple>

using namespace std;

struct SessionResult {
    int semester;
    string subject;
    int grade;
};

struct Student {
    string name;
    string group;
    vector<SessionResult> sessions;
};

double calculateAverageGrade(const Student& student, int semester) {
    double totalGrade = 0;
    int count = 0;

    for (const auto& session : student.sessions) {
        if (session.semester == semester) {
            totalGrade += session.grade;
            count++;
        }
    }

    return count > 0 ? totalGrade / count : 0;
}

double processSequential(const vector<Student>& students, const string& group, int semester) {
    double totalGrade = 0;
    int studentCount = 0;

    for (const auto& student : students) {
        if (student.group == group) {
            totalGrade += calculateAverageGrade(student, semester);
            if (calculateAverageGrade(student, semester) > 0) {
                studentCount++;
            }
        }
    }

    return studentCount > 0 ? totalGrade / studentCount : 0;
}

double processParallel(const vector<Student>& students, const string& group, int semester, int threadCount) {
    vector<future<tuple<double, int>>> futures;
    size_t chunkSize = students.size() / threadCount;
    size_t remainder = students.size() % threadCount;

    auto processChunk = [&](size_t start, size_t end) {
        double totalGrade = 0;
        int count = 0;

        for (size_t i = start; i < end; ++i) {
            if (students[i].group == group) {
                double avgGrade = calculateAverageGrade(students[i], semester);
                totalGrade += avgGrade;
                if (avgGrade > 0) {
                    count++;
                }
            }
        }
        return make_tuple(totalGrade, count);
        };

    size_t startIndex = 0;
    for (int i = 0; i < threadCount; ++i) {
        size_t endIndex = startIndex + chunkSize + (i < remainder ? 1 : 0);
        futures.push_back(async(launch::async, processChunk, startIndex, endIndex));
        startIndex = endIndex;
    }

    double totalGrade = 0;
    int totalCount = 0;
    for (auto& fut : futures) {
        auto result = fut.get();
        totalGrade += get<0>(result);
        totalCount += get<1>(result);
    }

    return totalCount > 0 ? totalGrade / totalCount : 0;
}

vector<Student> generateRandomStudents(int n) {
    vector<Student> students(n);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> gradeGen(2, 5);
    uniform_int_distribution<> semesterGen(1, 2);
    uniform_int_distribution<> groupGen(0, 2);

    vector<string> names = { "Попов Иван", "Волчек Алина", "Шакин Максим", "Качурина Марина", "Шакин Алексей", "Цвях Степан" };
    vector<string> groups = { "A", "B","C" };

    for (int i = 0; i < n; ++i) {
        Student student;
        student.name = names[i % names.size()];
        student.group = groups[groupGen(gen)];

        for (int s = 1; s <= 2; ++s) {
            student.sessions.push_back(SessionResult{ s, "Математика", gradeGen(gen) });
            student.sessions.push_back(SessionResult{ s, "Физика", gradeGen(gen) });
        }

        students[i] = student;
    }

    return students;
}

void printStudent(const Student& student, int semester) {
    cout << "Имя: " << student.name << ", Группа: " << student.group << "\n";
    cout << "Результаты за семестр " << semester << ":\n";
    for (const auto& session : student.sessions) {
        if (session.semester == semester) {
            cout << "  Предмет: " << session.subject << ", Оценка: " << session.grade << "\n";
        }
    }
}


//малое колво
void test1(const string& group, int semester, int threadCount) {
    vector<Student> smallStudents = {
        {"Глумов Иван", "A", {{1, "Математика", 5}, {1, "Физика", 4}, {2, "История", 3}}},
        {"Жданова Анна", "A", {{1, "Математика", 4}, {1, "Физика", 4}, {2, "История", 5}}},
        {"Титовец Алексей", "B", {{1, "Математика", 5}, {1, "Физика", 5}, {2, "История", 5}}},
        {"Артемова Ольга", "A", {{1, "Математика", 5}, {1, "Физика", 5}, {2, "История", 5}}},
        {"Бекетов Антон", "A", {{1, "Математика", 3}, {1, "Физика", 2}, {2, "История", 4}}}
    };

    // Время обработки без многопоточности
    auto start = chrono::high_resolution_clock::now();
    double sequentialResult = processSequential(smallStudents, group, semester);
    auto end = chrono::high_resolution_clock::now();
    cout << "Время без многопоточности: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
    cout << "Средний балл студентов (без многопоточности): " << sequentialResult << endl;

    // Время обработки с многопоточностью
    start = chrono::high_resolution_clock::now();
    double parallelResult = processParallel(smallStudents, group, semester, threadCount);
    end = chrono::high_resolution_clock::now();
    cout << "Время с многопоточностью: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
    cout << "Средний балл студентов (с многопоточностью): " << parallelResult << endl;

    // Вывод результатов
    cout << "\nРезультаты (группа " << group << ", семестр " << semester << "):\n";
    for (const auto& student : smallStudents) {
        if (student.group == group) {
            printStudent(student, semester);
            cout << "----------------------\n";
        }
    }
}

// Для теста с большим количеством студентов
void test2(const string& group, int semester, int threadCount) {
    vector<Student> largeStudents = generateRandomStudents(1000564);

    // Время обработки без многопоточности
    auto start = chrono::high_resolution_clock::now();
    double sequentialResult = processSequential(largeStudents, group, semester);
    auto end = chrono::high_resolution_clock::now();
    cout << "Время без многопоточности: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
    cout << "Средний балл студентов (без многопоточности): " << sequentialResult << endl;

    // Время обработки с многопоточностью
    start = chrono::high_resolution_clock::now();
    double parallelResult = processParallel(largeStudents, group, semester, threadCount);
    end = chrono::high_resolution_clock::now();
    cout << "Время с многопоточностью: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
    cout << "Средний балл студентов (с многопоточностью): " << parallelResult << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");
    string group = "A";
    int semester = 1;
    int threadCount = 4;

    test1(group, semester, threadCount);
    test2(group, semester, threadCount);

    return 0;
}

