Копирование файла двумя параллельными процессами: один осуществляет чтение из
файла-источника, второй – запись в файл-приемник. Используются 2 независимых буфера:
для чтения и для записи, которые меняются местами (передаются между процессами) по мере соответственно заполнения и опустошения. В качестве буферов используются блоки разделяемой памяти (shared memory), для синхронизации процессов и управления доступом к
буферам – семафоры (semaphore).
Примечание. Различие скоростей чтения и записи может приводить к простоям процессов, что необходимо учесть: заполненный «буфер чтения» можно передать как «буфер
записи» в распоряжение «процесса записи» независимо от его состояния, но «процесс чтения» все равно вынужден будет ждать освобождения бывшего «буфера записи», чтобы использовать его как «буфер чтения». Представляет интерес оценка эффекта от распараллеливания процессов, но заметной она будет лишь на больших размерах файлов и медленных
устройствах