## Задача 2.1: Проблема конкурентного доступа к разделяемому ресурсу

### a. Изучение и обновление реализации очереди

1. В каталоге `sync` репозитория [mrutman/os](git@github.com:mrutman/os.git) находится простая реализация очереди на основе списка. Выполните следующие действия:

   - Соберите и запустите программу `queue-example.c`.
   - Посмотрите вывод программы и убедитесь, что он соответствует вашему пониманию работы данной реализации очереди.

2. Добавьте реализацию функции `queue_destroy()`.  

### b. Изучение программы `queue-threads.c`

1. Изучите код программы `queue-threads.c` и разберитесь, что она делает.  

2. Запустите программу несколько раз. Если появляются ошибки выполнения, попытайтесь их объяснить и определить, что именно вызывает ошибку. Запишите наблюдаемые ошибки.

3. Поиграйте с следующими параметрами:  

   - Размер очереди (задается в `queue_init()`) - изменяйте размер очереди от 1000 до 1000000.
   - Привязка потоков к процессору (ядру) с помощью функции `set_cpu()`. Привяжите потоки к одному процессору и к разным.
   - Планирование потоков - попробуйте убрать функцию `sched_yield()` перед созданием второго потока.  

4. Объясните наблюдаемые результаты.
