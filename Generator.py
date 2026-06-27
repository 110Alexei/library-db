import os
import random

DATA_DIR = 'DataGen'
OUTPUT_FILE = 'Data.txt'
NUM_RECORDS = 10000 

DEFAULT_DATA = {
    'BookNames.txt': ['Ночь', 'Улица', 'Фонарь', 'Аптека', 'Утро', 'День', 'Вечер',
    'Рассвет', 'Закат', 'Осень', 'Зима', 'Весна', 'Лето'],
    'SurName.txt': ['Иванов', 'Петров', 'Сидоров', 'Кузнецов', 'Попов', 'Васильев', 'Соколов', 'Михайлов'],
    'Name.txt': ['Александр', 'Дмитрий', 'Сергей', 'Алексей', 'Андрей', 'Иван', 'Максим', 'Михаил'],
    'LastName.txt': ['Александрович', 'Дмитриевич', 'Сергеевич', 'Алексеевич', 'Андреевич', 'Иванович', 'Максимович', 'Михайлович'],
    'Classification.txt': [
        'Художественная литература/Проза/Классика', 
        'Художественная литература/Поэзия/Лирика',
        'Художественная литература/Фантастика',
        'Художественная литература/Детектив',
        'Наука/Математика/Алгебра',
        'Наука/Информатика/Алгоритмы', 
        'Наука/Физика/Квантовая',
        'Учебная литература/Справочник',
        'Учебная литература/Методическое пособие'
    ],
    'Publisher.txt': ['Наука', 'Просвещение', 'АСТ', 'Эксмо', 'Питер'],
    'Language.txt': ['Русский', 'Английский', 'Немецкий', 'Испанский', "Французский"]
}

def load_source_file(filename):
    """Загружает данные из файла в папке DataGen. Если файла нет — создает его с дефолтными значениями."""
    filepath = os.path.join(DATA_DIR, filename)

    if not os.path.exists(filepath) or os.path.getsize(filepath) == 0:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(DEFAULT_DATA[filename]))
        return DEFAULT_DATA[filename]

    with open(filepath, 'r', encoding='utf-8') as f:
        lines = [line.strip() for line in f if line.strip()]
    return lines

if not os.path.exists(DATA_DIR):
    os.makedirs(DATA_DIR)
    print(f"Создана папка '{DATA_DIR}' для исходных данных.")

book = load_source_file('BookNames.txt')
surname = load_source_file('SurName.txt')
name = load_source_file('Name.txt')
lastname = load_source_file('LastName.txt')
classification = load_source_file('Classification.txt')
publisher = load_source_file('Publisher.txt')
language = load_source_file('Language.txt')

print("Все исходные файлы успешно загружены.")

with open(OUTPUT_FILE, 'w', encoding='utf-8') as fout:
    for i in range(NUM_RECORDS):
        generated_title = f'"{random.choice(book)} {random.choice(book)}"'
        
        record = [
            random.choice(surname),
            random.choice(name),
            random.choice(lastname),
            generated_title,
            random.choice(classification),
            random.choice(language),
            f'"{random.choice(publisher)}"',
            str(random.randint(10, 5000))  # Например, количество страниц
        ]

        line = '; '.join(record)

        if i < NUM_RECORDS - 1:
            fout.write(line + '\n')
        else:
            fout.write(line)

print(f"Успех! Сгенерирован файл '{OUTPUT_FILE}' ({NUM_RECORDS} записей).")