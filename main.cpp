#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Base.hpp"
 
#define PORT 8080
#define BUFFER_SIZE 1500

struct CoutRedirect
{
    std::streambuf *old_buf;
 
    explicit CoutRedirect(std::streambuf *new_buf) : old_buf(std::cout.rdbuf(new_buf))
    {
    }
 
    ~CoutRedirect()
    {
        std::cout.rdbuf(old_buf);
    }
};
 
int main()
{
    std::cout << "[Сервер] Инициализация базы данных..." << std::endl;
    Base db("Data.txt", "Data_out.txt");
    std::cout << "[Сервер] База данных готова к работе!" << std::endl;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Ошибка создания сокета!" << std::endl;
        return 1;
    }
 
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Ошибка bind (возможно, порт " << PORT << " занят)!" << std::endl;
        close(server_fd);
        return 1;
    }
 
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "Ошибка listen!" << std::endl;
        close(server_fd);
        return 1;
    }
 
    std::cout << "[Сервер] Ожидание подключений на порту " << PORT << "..." << std::endl;

    while (true)
    {
        socklen_t addrlen = sizeof(address);
        int client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0)
        {
            std::cerr << "Ошибка accept!" << std::endl;
            continue;
        }
 
        std::cout << "\n[Сервер] Клиент подключился!" << std::endl;
 
        char buffer[BUFFER_SIZE] = {0};
        int valread = read(client_socket, buffer, BUFFER_SIZE - 1);
 
        if (valread > 0)
        {
            std::string query_str(buffer);
            while (!query_str.empty() && (query_str.back() == '\n' || query_str.back() == '\r'))
                query_str.pop_back();
 
            std::cout << "[Сервер] Получен запрос: " << query_str << std::endl;

            std::stringstream ss;
            Result res;
            {
                CoutRedirect guard(ss.rdbuf());
                res = db.get_query(query_str);
            }
 
            std::string response;
            if (res == OK)
            {
                response = ss.str();
                if (response.empty())
                    response = "Success (OK)\n";
            }
            else if (res == NO_RES)
            {
                response = "Error: No results found.\n";
            }
            else
            {
                response = "Error: Syntax error or execution failed.\n";
            }
 
            send(client_socket, response.c_str(), response.size(), 0);
            std::cout << "[Сервер] Ответ отправлен клиенту." << std::endl;

            db.to_file();
        }
        else if (valread < 0)
        {
            std::cerr << "[Сервер] Ошибка чтения из сокета." << std::endl;
        }
 
        close(client_socket);
    }
 
    close(server_fd);
    return 0;
}
