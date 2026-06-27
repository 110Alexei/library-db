import socket
import sys
 
sys.stdin.reconfigure(encoding="utf-8", errors="surrogateescape")
sys.stdout.reconfigure(encoding="utf-8", errors="surrogateescape")
 
HOST = "127.0.0.1"
PORT = 8080
RECV_BUFFER_SIZE = 65536
 
def send_query(query: str) -> str:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((HOST, PORT))
        sock.sendall(query.encode("utf-8", errors="surrogateescape"))
        data = sock.recv(RECV_BUFFER_SIZE)
        return data.decode("utf-8", errors="replace")
 
def print_help() -> None:
    print(
        "\nПримеры запросов (формат должен совпадать с тем, что ждёт Query.hpp):\n"
        '  SELECT surname="Иванов"; end;\n'
        '  SELECT classification="Художественная литература/Поэзия"; end;\n'
        '  RESELECT language="Русский"; end;\n'
        '  REMOVE surname="Сидоров"; end;\n'
        '  PRINT selected; end;\n'
        '  PRINT all; end;\n'
        '  INSERT surname="Кузнецов"; name="Алексей"; lastname="Сергеевич"; '
        'title="Зима Весна"; classification="Художественная литература/Проза/Классика"; '
        'language="Русский"; publisher="Питер"; circulation="1250"; end;\n'
        "\nКоманды клиента: help — показать эту справку, exit — выйти.\n"
    )
 
def main() -> None:
    print(f"Клиент библиотечной БД. Подключение к {HOST}:{PORT} при каждом запросе.")
    print_help()
 
    while True:
        try:
            query = input(">>> ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\nВыход.")
            break
 
        if not query:
            continue
        if query.lower() == "exit":
            break
        if query.lower() == "help":
            print_help()
            continue
 
        try:
            response = send_query(query)
        except ConnectionRefusedError:
            print("Не удалось подключиться к серверу")
            continue
        except OSError as e:
            print(f"Ошибка сети: {e}")
            continue
 
        print(response, end="" if response.endswith("\n") else "\n")
 
if __name__ == "__main__":
    main()