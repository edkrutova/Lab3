#include <iostream>
#include <vector>
#include "tinyxml2.h"
#include <string>
#include<algorithm>
#include <valarray>

using namespace std;
using namespace tinyxml2;


class Lab3 {
public:
    vector<vector<float>> coord; // координата образа
    vector<vector<float>> coord_difference{{0}}; // разность координат нового объекта и ядра класса
    vector<vector<float>> cov_matrixx{{0}}; // матрица ковариации. в результате будет иметь вид (S+E)^-1
    vector<vector<float>> cov_mul{{0}}; // (x-y)^T * (S+E)^(-1)
    vector<vector<float>> core; // ядро класса
    float final{}; // финальное расстояние
};

//Вычисляем обратную матрицу для сумм ковариации и единичной матриц
void reverse_matrix(vector<vector<float>> &M) {
    double t;
    vector<vector<float>> E(M.size(), vector<float>(M.size(), 0.0));
    //определяем единичную матрицу и складываем с матрицей ковариации
    for (int i = 0; i < M.size(); i++)
        for (int j = 0; j < M.size(); j++) {
            if (i == j) E[i][j] = 1.0;
            M[i][j] += E[i][j];
        }

    //Задаём номер ведущей строки
    for (int a = 0; a < M.size(); a++)
    {

        t = M[a][a];
        //все элементы a-ой строки матрицы M, кроме a-ого и до него, делим на разрешающий элемент
        for (int b = a + 1; b < M.size(); b++)
        {
            M[a][b] = M[a][b] / t;
        }
        //все элементы a-ой строки матрицы E делим на разрешающий элемент
        for (int b = 0; b < M.size();b++) {
            E[a][b] = E[a][b] / t;
        }
        //элемент соответствующий  разрещающему - делим на самого себя, чтобы получить 1
        M[a][a] /= t;

        if (a > 0) {
            for (int i = 0;i < a;i++) {
                for (int j = 0;j < M.size();j++) {
                    E[i][j] = E[i][j] - E[a][j] * M[i][a]; //Вычисляем элементы матрицы E,идя по столбцам с 0 -ого  к последнему
                }
                for (int j = M.size() - 1;j >= a;j--) {
                    M[i][j] = M[i][j] - M[a][j] * M[i][a]; //Вычисляем элементы матрицы M,идя по столбцам с последнего к a-ому
                }
            }
        }
        for (int i = a + 1;i < M.size();i++) {
            for (int j = 0;j < M.size();j++) {
                E[i][j] = E[i][j] - E[a][j] * M[i][a];
            }
            for (int j = M.size() - 1;j >= a;j--) {
                M[i][j] = M[i][j] - M[a][j] * M[i][a];
            }
        }


    }
    //На месте исходной матрицы должна получиться единичная а на месте единичной - обратная.
    for (int i = 0; i < M.size(); i++) {
        for (int b = 0; b < M.size(); b++) {
            M[i][b] = E[i][b];
        }
    }
}

class Loader {
public:
    std::vector<std::vector<std::vector<int>>> examples;
    std::vector<string> classes; //каждый элементы вектора - id класса сответсвующего образца
    std::vector<std::vector<std::vector<int>>> tasks;

    bool load_instance(const char *fileName) {
        XMLDocument doc;
        if (doc.LoadFile(fileName) != XMLError::XML_SUCCESS) {
            std::cout << "Error openning input XML file." << std::endl;
            return false;
        }
        XMLElement *root;
        root = doc.FirstChildElement("root");
        XMLElement *objects = root->FirstChildElement("examples");
        for (auto object = objects->FirstChildElement("object"); object; object = object->NextSiblingElement(
                "object")) {
            std::vector<std::vector<int>> example;
            classes.emplace_back(object->Attribute("class"));
            for (auto row = object->FirstChildElement("row"); row; row = row->NextSiblingElement("row")) {
                std::vector<int> line;
                std::string values = row->GetText();
                for (char value: values) {
                    if (value == '1')
                        line.push_back(1);
                    else if (value == '0')
                        line.push_back(0);
                }
                example.push_back(line);
            }
            examples.push_back(example);
        }
        XMLElement *task = root->FirstChildElement("tasks");
        for (auto object = task->FirstChildElement(); object; object = object->NextSiblingElement("object")) {
            std::vector<std::vector<int>> example;
            for (auto row = object->FirstChildElement("row"); row; row = row->NextSiblingElement("row")) {
                std::vector<int> line;
                std::string values = row->GetText();
                for (char value: values) {
                    if (value == '1')
                        line.push_back(1);
                    else if (value == '0')
                        line.push_back(0);
                }
                example.push_back(line);
            }
            tasks.push_back(example);
        }
        return true;
    }

    void print_examples() {
        for (int i = 0; i < examples.size(); i++) {
            std::cout << "\nObject " << i + 1 << " class=" << classes[i] << "\n";
            for (auto &j: examples[i]) {
                for (int k: j) {
                    if (k == 0) std::cout << " ";
                    else std::cout << "\x0B1";
                }
                std::cout << "\n";
            }
        }
    }

    void print_tasks(int i, const string &Lab3) {
        for (auto &j: tasks[i]) {
            for (int k: j) {
                if (k == 0) std::cout << " ";
                else std::cout << "\x0B1";
            }
            std::cout << "\n";
        }
        std::cout << "\nTask " << i + 1 << " selected class = " << Lab3 << "\n";
    }
};

void step_one(int n, vector<vector<Lab3>>& field, int row, int col, int samples, Loader loader, int classes) {
    for (int j = 0; j < samples; j++) {
        field[n][j].coord.resize(row);
        field[n][j].core.resize(col);
        field[n][j].cov_matrixx.resize(row * col);
        field[n][j].coord_difference.resize(row);
        field[n][j].cov_mul.resize(row);
        field[n][j].final = 0;

        for (int i = 0; i < row; i++) {
            field[n][j].coord[i].resize(col);
            field[n][j].core[i].resize(col);
            field[n][j].coord_difference[i].resize(col);
            field[n][j].cov_mul[i].resize(col);
        }
        for (int i = 0; i < row * col; i++) {
            field[n][j].cov_matrixx[i].resize(row * col);

        }
    }

    // считываем образы, распределяем коэффициенты
    for (int obr = 0; obr < samples; obr++) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                field[n][obr].coord[i][j] = loader.examples[n * classes + obr][i][j];
            }
        }
    }
    cout << "Coeffs " << n + 1 << " object :\n";

    // вычисляем ядра классов и выводим коэффициентов в консоль
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            for (int obr = 0; obr < samples; obr++) {
                field[n][0].core[i][j] += field[n][obr].coord[i][j];
            }
            field[n][0].core[i][j] /= samples;
            cout << field[n][0].core[i][j] << "\t";
        }
        cout << endl;
    }
    for (int col = 0; col < 50; col++) cout << "-";
    cout << endl;
    // вычисляем матрицу ковариации
    for (int i = 0; i < row ; i++)
        for (int j = 0; j < row; j++) {
            for (int obr = 0; obr < samples; obr++) {
                field[n][0].cov_matrixx[i][j] +=
                        (field[n][obr].coord[i / row][i % row] - field[n][0].core[i / row][i % row]) *
                        (field[n][0].coord[j / col][j % col] - field[n][0].core[j / col][j % col]);
            }
            field[n][0].cov_matrixx[i][j] /= (row * col - 1);
        }

}

int main(int argc, char *argv[]) //argc - argumnet counter, argv - argument values
{
    for (int i = 0; i < argc; i++)
        std::cout << argv[i] << "\n";
    if (argc < 2) {
        std::cout << "Name of the input XML file is not specified." << std::endl;
        return 1;
    }

    Loader loader;
    loader.load_instance(argv[1]);
    loader.print_examples();

    int numb_of_classes = 4;
    int numb_of_samples = 4;
    int numb_of_tasks = 4;
    int row = 10, col = 10;

    vector<vector<Lab3>> field(numb_of_classes, vector<Lab3>(numb_of_samples)); // список всех образов

    for (int n = 0; n < numb_of_classes; n++) {
        step_one(n, field, row, col, numb_of_samples, loader, numb_of_classes);
        // вычисляем матрицу ковариации
        for (int i = 0; i < field[0][0].cov_matrixx.size();i++) {
            for (int j =0; j < field[0][0].cov_matrixx[0].size(); j++ ) {
            }
        }
        reverse_matrix(field[n][0].cov_matrixx);

    }
    // вычисляем разницу между образцом и примером
    for (int st = 0; st < numb_of_tasks; st++) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                for (int step = 0; step < numb_of_classes; step++)
                    field[step][0].coord_difference[i][j] = loader.tasks[st][i][j] - field[step][0].core[i][j];
            }
        }


        //вычисляем первую часть расстояния
        for (int i = 0; i < row * col; i++) {
            for (int j = 0; j < row * col; j++) {
                for (int step = 0; step < numb_of_classes; step++) {
                    field[step][0].cov_mul[i / col][i % col] += (field[step][0].coord_difference[j / col][j % col] *
                                                                 field[step][0].cov_matrixx[i][j]);
                }
            }
        }

        //вычисляем расстояние Махланобиса-Евклида
        for (int step = 0; step < numb_of_classes; step++) {
            field[step][0].final=0;
            for (int i = 0; i < row; i++) {
                for (int j = 0; j < col; j++) {
                        field[step][0].final += (field[step][0].cov_mul[i][j] * field[step][0].coord_difference[i][j]);
                    }
                }
        }
        vector<pair<float, int>> best;
        for (int step = 0; step < numb_of_classes; step++) {
            field[step][0].final = sqrt(fabs(field[step][0].final));
            best.emplace_back(field[step][0].final, step);
        }
        sort(best.begin(), best.end());
        loader.print_tasks(st, loader.classes[best[0].second * 4]);

        for (int step = 0; step < numb_of_classes; step++) {
            cout << "Distance " << loader.classes[step * numb_of_classes] << " : " << field[step][0].final << endl;
        }
        cout << "-------------------------------------------\n";
    }

    return 0;
}