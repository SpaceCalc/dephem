# DEPHEM
Простая C++ библиотека для работы с эфемеридами JPL NASA.  

## Возможности
* Определение положения (и скорости) планет Солнечной Системы, Луны и Солнца.
* Определение значений дополнительных элементов выпусков эфемерид.
* Доступ к общей информации выпуска эфемерид и хранящимся константам. 

Библиотека работает с эфемеридами только **бинарного формата**.

## Быстрый старт
**Внимание**! Для работы с библиотекой требуется файл эфемерид.  
Подробнее: ["О файлах эфемерид"](./docs/about-ephemeris-files.md).  

В примере используется выпуск эфемерид **DE431**.
````c++
#include <iostream>
#include "dephem/EphemerisRelease.h"
    
int main()
{
    // Инициализация файла эфемерид:
    dph::EphemerisRelease de431("lnxm13000p17000.431");

    // Проверка на корректное чтение файла эфемерид:
    if(de431.isReady())
    {
        // 01.01.2000 00:00:00.000 в формате Юлианской Эфемеридной Даты:
        double JED = 2451544.5;

        if(JED >= de431.startDate() && JED <= de431.endDate())
        {
            // Массив для записи результата вычислений (x, y, z):
            double resultArray[3]{};

            // Вычислить радиус-вектор Луны относительно Земли на момент времени JED и записать
            // результат в массив resultArray:
            de431.calculateBody(dph::Calculate::POSITION,
                dph::Body::MOON, dph::Body::EARTH, JED, resultArray);

            std::cout << "Moon position relative to Earth:" << std::endl
            std::cout << "x =\t" << resultArray[0]  << "\t[km]" << std::endl
            std::cout << "y =\t" << resultArray[1]  << "\t[km]" << std::endl
            std::cout << "z =\t" << resultArray[2]  << "\t[km]" << std::endl;
        }        

        // Получить значение астрономической единицы, хранящейся в выпуске DE431:
        const double AU = de431.constant("AU");

        std::cout << "AU =\t" << AU << "\t[km]" << std::endl;    
    }

    return 0;
}
````

## Документация
Пользовательскую документацию можно получить по [ссылке](./docs/index.md).

## Дальнейшая разработка
* Генерация бинарных файлов из эфемерид текстового (ASCII) формата.
* "Обрезка" бинарных файлов для уменьшения их веса и хранения только требуемой информации.
* Работа с testpo-файлами (специальные файлы, идущие вместе с каждым выпуском для сверки результатов вычислений).
* Ускорение работы с бинарными файлами (чтение и вычисления).

## О исходном коде
Библиотека основанна на исходном коде Дэвида Хоффмана:
>David Hoffman/EG5                    
>NASA, Johnson Space Center           
>Houston, TX 77058                    
>e-mail: <david.a.hoffman1@jsc.nasa.gov>

## Контакты
Филиппов Максим (разработчик):  
<vetrume@gmail.com>