# ENG

"""
    Operating systems and environments - individual practical work №2
    Aleksandr Dubeykovsky

    Implement parallel copying of file using two buffers.
    One for reading, another one for writing. After each step you should swap buffers.

    Any boost could be seen only on huge files and slow processors.
"""

# RU

"""
    ОСИС ИПР 2
    Александр Дубейковский - зачётная книжка 75350046, группа 893551, Вариант 6

    Копирование файла двумя параллельными процессами: один осуществляет чтение из
файла-источника, второй – запись в файл-приемник. Используются 2 независимых буфера:
для чтения и для записи, которые меняются местами (передаются между процессами) по мере
соответственно заполнения и опустошения. В качестве буферов используются блоки разделяемой 
памяти (shared memory), для синхронизации процессов и управления доступом к
буферам – семафоры (semaphore).
    Примечание. Различие скоростей чтения и записи может приводить к простоям процессов, 
что необходимо учесть: заполненный «буфер чтения» можно передать как «буфер
записи» в распоряжение «процесса записи» независимо от его состояния, 
но «процесс чтения» все равно вынужден будет ждать освобождения бывшего «буфера записи», 
чтобы использовать его как «буфер чтения». Представляет интерес оценка эффекта от 
распараллеливания процессов, но заметной она будет лишь на больших размерах файлов и медленных
устройствах
"""


import os
from multiprocessing.dummy import Pool as ThreadPool
from copy import deepcopy


def fileRead(file, size_of_file_to_copy, index_from_where_to_read, buffer, size_of_buffer):
    buffer[0] = ""
    if index_from_where_to_read >= size_of_file_to_copy:
        return True
    file.seek(index_from_where_to_read)
    buffer[0] = file.read(size_of_buffer)
    return True


def fileWrite(file, buffer):
    if buffer[0] == "":
        return True
    if file.mode != "a":
        print("File isn't opened in right mode to write in it")
        return False
    file.write(buffer[0])
    print("Buffer that is currently writing to file:")
    print(buffer[0])
    print("---")
    return True


def distributor(args):
    function, arguments = args
    value = function(*arguments)
    return value


def parallelFileCopying(filename_to_copy, filename_where_to_copy, size_of_buffer):
    # check existance and open file to only read from it
    try:
        file_to_read = open(filename_to_copy, "r")
    except FileNotFoundError:
        print(f"File {filename_to_copy} doesn't exist")
        return False
    # get size of that file
    size_of_file_to_copy = os.path.getsize(f"{filename_to_copy}")

    # check that file with that filename doesn't exist
    try:
        open(filename_where_to_copy, "r")
        print(f"You can't copy your file to a file with name as {filename_where_to_copy},"
              f" because it is already exists")
        return False
    except FileNotFoundError:
        print(f"File {filename_where_to_copy} doesn't exist")
        print("We can create such file and continue our copying task to this new file")

    # create file
    open(filename_where_to_copy, "x")
    # open file to only append to it
    file_to_write = open(filename_where_to_copy, "a")

    buffer_for_reading = [""]
    buffer_for_writing = [""]
    index_from_where_to_read = 0
    # parallel reading and writing for file copying
    # we run both functions in parallel till we've read all the file AND buffer that we need to write
    # is not empty
    while index_from_where_to_read < size_of_file_to_copy or (buffer_for_writing != [""]):
        function_names = [fileRead, fileWrite]
        args_list = [[file_to_read, size_of_file_to_copy, index_from_where_to_read,
                      buffer_for_reading, size_of_buffer],
                     [file_to_write, buffer_for_writing]]

        results = ThreadPool(2).map(distributor, zip(function_names, args_list))
        if not all(results):
            print("We couldn't read file or write to file")
            return False
        buffer_for_writing[0] = deepcopy(buffer_for_reading[0])
        index_from_where_to_read += size_of_buffer
    return True


if __name__ == '__main__':
    size_of_buffer = 32
    filename_to_copy = "text.txt"
    print(f"Size of {filename_to_copy} = " + str(os.path.getsize(f"{filename_to_copy}")) + " bytes")
    filename_where_to_copy = "new_file.txt"
    was_it_successful = parallelFileCopying(filename_to_copy, filename_where_to_copy, size_of_buffer)
    print("Was it successful: " + str(was_it_successful))
    print(f"Size of {filename_where_to_copy} = " + str(os.path.getsize(f"{filename_where_to_copy}"))
          + " bytes")
