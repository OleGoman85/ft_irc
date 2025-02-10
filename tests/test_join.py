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
        data = client.recv(4096)
        return data.decode().strip()
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

def setup_environment():
    alisa = create_client()
    masha = create_client()
    oleg  = create_client()

    register_client(alisa, "Alisa", "Alisa", "Alisa the Great")
    register_client(masha, "Masha", "Masha", "Masha the Cat")
    register_client(oleg,  "Oleg",  "Oleg",  "Oleg the Bold Guy")

    return alisa, masha, oleg

def run_join_test(test_num, test_name, client, command, expected_substr):
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

def test_join_command(alisa, masha, oleg):
    """
    Сценарии, которые пока проверяем:
    - 451 unregistered
    - 461 not enough params
    - 479 illegal channel name
    - 443 already in channel
    - 473 invite-only
    - 471 channel is full
    - 475 wrong key
    - Создание нового канала
    """

    global failed_tests_count
    failed_tests_count = 0  # сбросим счётчик

    print_header("ТЕСТИРОВАНИЕ КОМАНДЫ JOIN")

    # [Test #1] Unregistered client => 451
    temp_client = create_client()
    run_join_test(1, "Unregistered Client", temp_client, "JOIN #whatever", "451 :You have not registered")
    temp_client.close()

    # [Test #2] Not enough parameters => 461
    run_join_test(2, "Not enough parameters", alisa, "JOIN", "461 JOIN :Not enough parameters")

    # [Test #3] Illegal channel name => 479 (не начинается с '#')
    run_join_test(3, "Illegal channel name", alisa, "JOIN mychannel", "479 mychannel :Illegal channel name. Channel names must start with '#'")

    # [Test #4] Already in channel => 443
    #   Пусть Alisa создаст канал #join_test
    send_command(alisa, "JOIN #join_test")
    time.sleep(0.2)
    flush_recv(alisa)
    # Повторная попытка JOIN от Alisa => 443
    run_join_test(4, "Already in channel", alisa, "JOIN #join_test", "443 #join_test :You are already in the channel")

    # [Test #5] Invite-only => 473
    #   Установим +i на #join_test (сделаем это от лица Alisa — оп, т.к. первая)
    send_command(alisa, "MODE #join_test +i")
    time.sleep(0.2)
    flush_recv(alisa)
    # Masha пробует присоединиться => 473
    run_join_test(5, "Invite-only channel", masha, "JOIN #join_test", "473 #join_test :Cannot join channel (+i mode set)")

    # [Test #6] Channel is full => 471
    #   Допустим, сделаем лимит = 1 на #join_test
    send_command(alisa, "MODE #join_test +l 1")
    time.sleep(0.2)
    flush_recv(alisa)
    # Oleg пробует присоединиться => 471
    run_join_test(6, "Channel is full", oleg, "JOIN #join_test", "471 #join_test :Channel is full")

    # [Test #7] Wrong key => 475
    #   Снимем +l, поставим +k "secret"
    send_command(alisa, "MODE #join_test -l")
    time.sleep(0.1)
    flush_recv(alisa)
    send_command(alisa, "MODE #join_test +k secret")
    time.sleep(0.2)
    flush_recv(alisa)
    # Masha пробует JOIN без ключа => 475
    run_join_test(7, "Wrong key (no param)", masha, "JOIN #join_test", "475 #join_test :Cannot join channel (+k mode set)")

    # [Test #8] Correct key => JOIN succeeds
    #   Oleg пробует JOIN с ключом "secret"
    flush_recv(oleg)
    run_join_test(8, "Correct key", oleg, "JOIN #join_test secret", "JOIN #join_test")

    # [Test #9] Создание нового канала => Alisa => первый пользователь => оператор
    #   Для проверки, что сервер шлёт "MODE #<chan> +o Alisa" или что-то вроде NOTICE
    #   Запрашиваем несуществующий #freshchannel
    flush_recv(alisa)
    run_join_test(9, "Create new channel", alisa, "JOIN #freshchannel", "MODE #freshchannel +o Alisa")

    # [Test #10] Check "already in channel" для #freshchannel
    #   (Alisa снова JOIN #freshchannel => 443)
    run_join_test(10, "Already in channel (freshchannel)", alisa, "JOIN #freshchannel", "443 #freshchannel :You are already in the channel")

    # Итог
    if failed_tests_count == 0:
        print(f"{GREEN}\n🎉 Все тесты JOIN пройдены успешно!{RESET}")
    else:
        print(f"{RED}\n💀 {failed_tests_count} тест(ов) JOIN провалены!{RESET}")

def run_tests():
    alisa, masha, oleg = setup_environment()
    test_join_command(alisa, masha, oleg)

    alisa.close()
    masha.close()
    oleg.close()

if __name__ == "__main__":
    run_tests()
