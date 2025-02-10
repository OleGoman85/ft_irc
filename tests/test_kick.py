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
        return client.recv(4096).decode(errors="ignore").strip()
    except socket.timeout:
        return ""

def flush_recv(client):
    try:
        while True:
            client.recv(4096)
    except:
        pass

def register_client(client, nick, user, realname):
    send_command(client, f"NICK {nick}")
    send_command(client, f"USER {user} 0 * :{realname}")
    time.sleep(0.5)
    flush_recv(client)

def join_channel(client, channel):
    send_command(client, f"JOIN {channel}")
    time.sleep(0.2)
    flush_recv(client)

def run_kick_test(test_num, test_name, client, command, expected_substr):

    flush_recv(client)
    send_command(client, command)
    time.sleep(0.5)
    response = recv_response(client)

    print(f"{BLUE}-------------------------------------{RESET}")
    print(f"{YELLOW}Test #{test_num}: {RESET}{test_name}")
    print(f"{YELLOW}Command:         {RESET}{command}")
    print(f"{YELLOW}Expected:        {RESET}{expected_substr}")
    print(f"{YELLOW}Actual response: {RESET}{response if response else '<no response>'}")

    success = (expected_substr in response)
    print_result(success, f"Test #{test_num}: {test_name}", response)
    print(f"{BLUE}-------------------------------------{RESET}\n")

def make_operator(client, channel, nick):
    """
    Делает указанного пользователя оператором канала.
    Предполагается, что `client` уже является оператором.
    """
    send_command(client, f"MODE {channel} +o {nick}")
    time.sleep(0.3)
    flush_recv(client)

def setup_environment():
    alisa = create_client()
    masha = create_client()
    oleg  = create_client()

    register_client(alisa, "Alisa", "Alisa", "Alisa the Great")
    register_client(masha, "Masha", "Masha", "Masha the Cat")
    register_client(oleg,  "Oleg",  "Oleg",  "Oleg the Bold Guy")

    return alisa, masha, oleg

def test_kick_command(alisa, masha, oleg):
    global failed_tests_count
    failed_tests_count = 0

    channel = "#kicktest"
    join_channel(alisa, channel)
    join_channel(masha, channel)
    join_channel(oleg, channel)

    print_header("ТЕСТИРОВАНИЕ КОМАНДЫ KICK")

    # 1) Нерегистрированный клиент пытается кикнуть => 451
    temp_client = create_client()  # не даём ему NICK/USER
    run_kick_test(1,
                  "Unregistered client KICK",
                  temp_client,
                  f"KICK {channel} Masha",
                  "451 :You have not registered")
    temp_client.close()

    # 2) Недостаточно параметров => 461 (KICK <channel> <nick>)
    run_kick_test(2,
                  "Not enough params (no channel & nick)",
                  alisa,
                  "KICK",
                  "461 KICK :Not enough parameters")

    # 3) Канал не существует => 403
    run_kick_test(3,
                  "KICK from non-existent channel",
                  alisa,
                  "KICK #unknown Masha",
                  "403 #unknown :No such channel")

    # 4) Нет прав (обычный пользователь пытается кикнуть) => 482
    #    По классике IRC: "You're not channel operator"
    #    в нашем коде пока этого нет, НУЖНО ДОБАВИТЬ
    run_kick_test(4,
                  "Non-operator tries to KICK",
                  masha,
                  f"KICK {channel} Oleg",
                  "482")

    
    # 5) Кик пользователя, которого нет => 401
    run_kick_test(5,
                  "Kick unknown user",
                  alisa,
                  f"KICK {channel} NonExistentGuy",
                  "401 NonExistentGuy :No such nick/channel")

    # 6) Валидный кик: Alisa кикает Masha => должен прийти "KICK" всем в канале
    run_kick_test(6,
                  "Valid KICK",
                  alisa,
                  f"KICK {channel} Masha",
                  "KICK #kicktest Masha :Kicked")

    # 7) Повторный кик Masha => 401, потому что Masha уже нет в канале
    run_kick_test(7,
                  "Kick user already removed",
                  alisa,
                  f"KICK {channel} Masha",
                  "401 Masha :No such nick/channel")

    # 8) Кик самого себя: Alisa кикает Alisa. Нужно добавить варианты, где Алиса - единственный оператор и нет
    run_kick_test(8,
                  "Kick self",
                  alisa,
                  f"KICK {channel} Alisa",
                  "KICK #kicktest Alisa :Kicked")

    # 9) Канал пустой после кика последнего пользователя => должен удалиться
    #    проверяем, что повторный KICK => "403 #kicktest :No such channel"
    run_kick_test(9,
                  "Kick from an empty channel (should not exist)",
                  alisa,
                  f"KICK {channel} Oleg",
                  "403 #kicktest :No such channel")

    if failed_tests_count == 0:
        print(f"{GREEN}\n🎉 Все тесты KICK пройдены успешно!{RESET}")
    else:
        print(f"{RED}\n💀 {failed_tests_count} тест(ов) KICK провалены!{RESET}")

def run_tests():
    alisa, masha, oleg = setup_environment()
    test_kick_command(alisa, masha, oleg)
    alisa.close()
    masha.close()
    oleg.close()

if __name__ == "__main__":
    run_tests()
