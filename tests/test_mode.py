#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import time


GREEN = "\033[1;32m"
RED   = "\033[1;31m"
YELLOW= "\033[1;33m"
BLUE  = "\033[1;34m"
RESET = "\033[0m"


SERVER_PORT = 6667
SERVER_PASSWORD = "password"

failed_tests_count = 0 

def print_header(text):
    print(f"\n{YELLOW}{'='*50}\n📌 {text}\n{'='*50}{RESET}")

def print_result(success, message, received=""):
    global failed_tests_count
    if success:
        print(f"{GREEN}[✅] {message} PASSED!{RESET}")
    else:
        print(f"{RED}[❌] {message} FAILED!{RESET}")
        if received:
            print(f"{RED}     ⏪ Server response: {received}{RESET}")
        failed_tests_count += 1

def create_client():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('127.0.0.1', SERVER_PORT))
    s.settimeout(2)
    return s

def send_command(client, command):
    full_command = command + "\r\n"
    client.sendall(full_command.encode())

def recv_response(client):
    try:
        return client.recv(4096).decode().strip()
    except socket.timeout:
        return ""

def flush_recv(client):
    try:
        while True:
            client.recv(4096)
    except:
        pass

def register_client(client, nick, user, realname):
    send_command(client, f"PASS {SERVER_PASSWORD}")
    send_command(client, f"NICK {nick}")
    send_command(client, f"USER {user} 0 * :{realname}")
    time.sleep(0.5)
    flush_recv(client)

def join_channel(client, channel):
    send_command(client, f"JOIN {channel}")
    time.sleep(0.2)
    flush_recv(client)

def setup_environment():
    alisa = create_client()
    masha = create_client()
    oleg  = create_client()

    register_client(alisa, "Alisa", "Alisa", "Alisa the Great")
    register_client(masha, "Masha", "Masha", "Masha the Cat")
    register_client(oleg,  "Oleg",  "Oleg",  "Oleg the Bold Guy")

    return alisa, masha, oleg

def run_mode_test(test_num, test_name, client, command, expected_substr):
    """
    Выполняет один тест:
      1. Очищаем входящие данные.
      2. Отправляем команду.
      3. Ждём.
      4. Получаем ответ.
      5. Проверяем, содержится ли expected_substr в ответе.
      6. Выводим результат (учитывая глобальный счётчик).
    """
    flush_recv(client)
    send_command(client, command)
    time.sleep(0.5)
    response = recv_response(client)

    print(f"{BLUE}-------------------------------------{RESET}")
    print(f"{YELLOW}Test #{test_num}: {RESET}{test_name}")
    print(f"{YELLOW}Command:          {RESET}{command}")
    print(f"{YELLOW}Expected:         {RESET}{expected_substr}")
    print(f"{YELLOW}Actual response:  {RESET}{response if response else '<no response>'}")

    success = (expected_substr in response)
    print_result(success, f"Test #{test_num}: {test_name}", response)
    print(f"{BLUE}-------------------------------------{RESET}\n")


def test_mode_command(alisa, masha, oleg):

    global failed_tests_count
    failed_tests_count = 0

    channel = "#test"
    join_channel(alisa, channel)
    join_channel(masha, channel)
    join_channel(oleg, channel)

    print_header("ТЕСТИРОВАНИЕ КОМАНДЫ MODE")

    # [Test #1] Не зарегистрированный клиент
    temp_client = create_client()
    run_mode_test(1, "Unregistered Client", temp_client, f"MODE {channel}", "451 :You have not registered")
    temp_client.close()

    # [Test #2] Недостаточно параметров
    run_mode_test(2, "Not enough parameters", alisa, "MODE", "461 MODE :Not enough parameters")

    # [Test #3] Несуществующий канал
    run_mode_test(3, "Non-existent channel", alisa, "MODE #nonexistent", "403 #nonexistent :No such channel")

    # [Test #4] Печать текущих режимов
    run_mode_test(4, "Print current modes", alisa, f"MODE {channel}", f"324 Alisa {channel}")

    # [Test #5] Неоператор пытается менять режим
    run_mode_test(5, "Non-operator changing mode", masha, f"MODE {channel} +i", f"482 {channel} :You're not a channel operator")

    # [Test #6] Неверная строка режима
    run_mode_test(6, "Invalid mode string", alisa, f"MODE {channel} i", "472 Alisa :Invalid mode string")

    # [Test #7] Добавляем +it
    run_mode_test(7, "Adding modes +it", alisa, f"MODE {channel} +it", f"MODE {channel} +it")

    # [Test #8] Добавление +k без параметра
    run_mode_test(8, "Adding +k without parameter", alisa, f"MODE {channel} +k", "461 MODE :Not enough parameters for +k")

    # [Test #9] Добавление +k с параметром
    run_mode_test(9, "Adding +k with parameter", alisa, f"MODE {channel} +k secretpass", f"MODE {channel} +k secretpass")

    # [Test #10] +l с нечисловым параметром
    run_mode_test(10, "Adding +l with non-numeric", alisa, f"MODE {channel} +l notanumber", "461 MODE l :Invalid limit parameter")

    # [Test #11] +l с валидным числом
    run_mode_test(11, "Adding +l with parameter", alisa, f"MODE {channel} +l 10", f"MODE {channel} +l 10")

    # [Test #12] Убираем -k
    run_mode_test(12, "Removing -k", alisa, f"MODE {channel} -k", f"MODE {channel} -k")

    # [Test #13] Убираем -l
    run_mode_test(13, "Removing -l", alisa, f"MODE {channel} -l", f"MODE {channel} -l")

    # [Test #14] Делаем Машу оператором
    run_mode_test(14, "Adding operator +o", alisa, f"MODE {channel} +o Masha", f"MODE {channel} +o Masha")

    # [Test #15] Снимаем операторку у Маши (не последний оператор)
    run_mode_test(15, "Removing operator -o Masha (non-last)", alisa, f"MODE {channel} -o Masha", f"MODE {channel} -o Masha")

    # [Test #16] Сложные флаги (валидный порядок) +ilk 15 multiKey
    run_mode_test(16, "Multiple flags +ilk (valid order)",
                  alisa,
                  f"MODE {channel} +ilk 15 multiKey",
                  f"MODE {channel} +ilk")

    # [Test #17] Сложные флаги (невалидный порядок) +ilk multiKey 15
    run_mode_test(17, "Multiple flags +ilk (invalid param order)",
                  alisa,
                  f"MODE {channel} +ilk multiKey 15",
                  "461 MODE l :Invalid limit parameter")

    # [Test #18] Unsupported mode
    run_mode_test(18, "Unsupported mode", alisa, f"MODE {channel} +x", "472 Alisa x :is unknown mode char to me")

    # [Test #19] Complex mode string
    run_mode_test(19, "Complex mode string", alisa, f"MODE {channel} +ikl-t secretpass 10", f"MODE {channel} +ikl -t")

    # [Test #20] Проверка -o Alisa => 482 (последний оператор)
    run_mode_test(20, "Removing operator -o last", alisa, f"MODE {channel} -o Alisa", f"482 {channel} :Cannot remove the last operator")

    # [Test #21] Беспорядочный ввод: -i+lt-k 50
    # По логике IRC, это означает: remove i, add l (param=50), add t, remove k.
    # Если сервер правильно парсит, должен вернуться один ответ без ошибки,
    # например: MODE #test -i+lt-k 50 (или что-то аналогичное).
    # ну или фиг знает, что здесь должно вернуться, нужно проверить на нормальном сервере
    run_mode_test(21, "Jumbled flags -i+lt-k 50",
                  alisa,
                  f"MODE {channel} -i+lt-k 50",
                  f"MODE {channel} -i+lt-k 50")

    if failed_tests_count == 0:
        print(f"{GREEN}\n🎉 Все тесты MODE пройдены успешно!{RESET}")
    else:
        print(f"{RED}\n💀 {failed_tests_count} тест(ов) MODE провалены!{RESET}")

def run_tests():
    alisa, masha, oleg = setup_environment()
    test_mode_command(alisa, masha, oleg)
    alisa.close()
    masha.close()
    oleg.close()

if __name__ == "__main__":
    run_tests()
