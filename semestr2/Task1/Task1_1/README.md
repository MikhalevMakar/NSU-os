**Создание потоков с использованием POSIX API:**

a. Склонируйте репозиторий git@github.com:mrutman/os.git. Изучите и запустите программу threads/thread.c. Добейтесь, чтобы гарантированно выполнялись оба потока.

b. Измените программу, добавив создание 5-ти потоков с одинаковой поточной функцией.

c. В поточной функции распечатайте:

    i. Идентификаторы процесса, родительского процесса и потока. Для получения идентификатора потока используйте функции pthread_self() и gettid(). Сравните с тем, что функция pthread_create() вернула через первый аргумент. Объясните результат. Почему для сравнения идентификаторов POSIX потоков надо использовать функцию pthread_equal()?

    ii. Адреса локальной, локальной статической, локальной константной и глобальной переменных. Объясните результат.

d. В поточной функции попробуйте изменить локальные и глобальные переменные. Видны ли изменения из других потоков? Объясните результат.

e. Изучите /proc/pid/maps для полученного процесса. Найдите в нем стеки потоков.

f. Запустите программу из-под strace. Найдите системные вызовы, которые создали ваши потоки.
