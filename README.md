# DEPHEM (Development Ephemeris)
Простая C++ библиотека для работы с DE-эфемеридами NASA JPL.  

## Краткое описание
DEPHEM является простым и удобным решением для интеграции вычислений положений небесных тел (и не только) на основе эфемерид, выпускаемыми NASA JPL.  

Библиотека состоит из одного модуля - _EphemerisRelease_. Данный модуль включает в себя одноимённый класс, являющийся оболочкой над файлом DE-эфемерид **бинарного** формата (см. пункт "_Загрузить бинарный файл эфемерид_"), а также нескольких вспомогательных классов, упрощающих работу с основным.  

Весь код библиотеки снабжён поясняющими комментариями.

## Возможности
- Вычисление значений "_элементов_", хранящихся в выпуске эфемерид:
	- Положение (X, Y, Z) или состояние (X, Y, Z, Vx, Vy, Vz) для одного тела относительно другого на заданный момент времени.
	- Оригинальное значение или (ориг. значение + первая производная) для прочих элементов, не являющихся телами.  
	
- Получение информации о выпуске:
	- Индекс выпуска.
	- Общая информация о выпуске в текстовом формате.
	- Даты начала и конца промежутка, в которм разрешается проводить вычисления.
	- Значения хранящихся констант.

**_Возможности библиотеки ограничены работой только с бинарным форматом эфемерид._**

## Загрузить бинарный файл эфемерид
Загрузить требуемый выпуск эфемерид можно с официального ftp-сервиса NASA JPL SSD:  
<ftp://ssd.jpl.nasa.gov/pub/eph/planets/Linux/>.

По ссылке будут доступны несколько каталогов с названиями, соответствующими выпуску эфемерид. В каждом таком каталоге хранятся несколько файлов, одним из которых является бинарный файл эфемерид. Его название начинается с "_lnx_", а расширение является номерной частью индекса выпуска.  

Например, выпуску DE-эфемерид DE431 соответствует каталог "_de431_". Бинарный файл данного выпуска имеет название "_lnxm13000p17000.431_". Загрузите его для последующей работы библиотеки с ним.

## Быстрый старт
````c++
#include <iostream>
#include "dephem/EphemerisRelease.h"
    
int main()
{
    dph::EphemerisRelease de431("lnxm13000p17000.431");

    // Проверка на корректное чтение бинарного файла эфемерид:
    if(de431.isReady())
    {
        std::cout << "Ephemeris object is ready to work" << std::endl;

        // 1 января 2000 года 00:00:00.000 :
        double JED = 2451544.5;

        // Массив для записи результата вычислений:
        double resultArray[3]{};

        // Вычислить радиус-вектор Луны относительно Земли на момент времени JED и записать
        // результат в массив resultArray:
        de431.calculateBody(dph::Calculate::POSITION, dph::Body::MOON, dph::Body::EARTH, JED, resultArray);

        std::cout << "Moon position relative to Earth:" << std::endl;
        std::cout << "x =\t" << resultArray[0]  << "\t[km]" << std::endl;
        std::cout << "y =\t" << resultArray[1]  << "\t[km]" << std::endl;
        std::cout << "z =\t" << resultArray[2]  << "\t[km]" << std::endl;

        // Получить значение астрономической единицы, хранящейся в выпуске DE431:
        const double AU = de431.constant("AU");

        std::cout << "AU =\t" << AU << "\t[km]" << std::endl;    
    }
    else
    {
        std::cout << "Ephemeris file reading error" << std::endl;
    }

    return 0;
}
````

## Стандарт C++ и особенности репозитория
DEPHEM разрабатывается сразу для двух стандартов C++. Основная ветка *master* написана на C++14, дополнительная - на С++03 (см. соответствующую ветку в репозитории).

## Дальнейшая разработка
С течением времени функционал библиотеки будет расширяться и совершенствоваться.  

Задачи:
* Генерация бинарных файлов из эфемерид текстового (ASCII) формата.
* "Обрезка" бинарных файлов для уменьшения их веса и хранения только требуемой информации.
* Работа с testpo-файлами (специальные файлы, идущие вместе с каждым выпуском для сверки результатов вычислений).
* Ускорение работы с бинарными файлами (чтение и вычисления).

## Контакты
Филиппов Максим (разработчик):  
<vetrume@gmail.com>