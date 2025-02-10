#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import time

GREEN  = "\033[1;32m"
RED    = "\033[1;31m"
YELLOW = "\033[1;33m"
BLUE   = "\033[1;34m"
RESET  = "\033[0m"


SERVER_PORT     = 6667
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
        return client.recv(4096).decode(errors='ignore').strip()
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

    register_client(alisa, "Alisa", "Alisa", "Alisa The Great")
    register_client(masha, "Masha", "Masha", "Masha The Cat")
    register_client(oleg,  "Oleg",  "Oleg",  "Oleg The Bold")

    return alisa, masha, oleg

def run_bot_test(test_num, test_name, client, command, expected_substr):
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


def test_bot_command(alisa, masha, oleg):
    global failed_tests_count
    failed_tests_count = 0

    print_header("ТЕСТИРОВАНИЕ КОМАНДЫ BOT")

    channel_name = "#botTest"

    # [Test #1] Недостаточно параметров
    run_bot_test(1, "Not enough parameters", alisa, "BOT", "461 BOT :Not enough parameters")

    # [Test #2] BOT JOIN
    run_bot_test(2, "BOT JOIN channel", alisa, f"BOT JOIN {channel_name}", f"Bot joined channel {channel_name}")

    # [Test #3] BOT 8BALL без вопроса
    run_bot_test(3, "BOT 8BALL no question", alisa, "BOT 8BALL", "461 BOT 8BALL :Not enough parameters")

    # [Test #4] BOT 8BALL с вопросом
    # Ожидаем "Magic 8-Ball says:"
    run_bot_test(4, "BOT 8BALL with question", alisa, "BOT 8BALL Will I pass the exam?", "Magic 8-Ball says:")

    # [Test #5] BOT SAY
    run_bot_test(5, "BOT SAY message", alisa, f"BOT SAY {channel_name} Hello from test!", f"Bot message sent to {channel_name}")

    # [Test #6] BOT LEAVE
    run_bot_test(6, "BOT LEAVE channel", alisa, f"BOT LEAVE {channel_name}", f"Bot left channel {channel_name}")

    # [Test #7] Неизвестная подкоманда
    run_bot_test(7, "Unknown subcommand", alisa, "BOT DANCE polka", "421 BOT DANCE :Unknown BOT subcommand")

    if failed_tests_count == 0:
        print(f"{GREEN}\n🎉 Все тесты BOT пройдены успешно!{RESET}")
    else:
        print(f"{RED}\n💀 {failed_tests_count} тест(ов) BOT провалены!{RESET}")

def run_tests():
    alisa, masha, oleg = setup_environment()
    test_bot_command(alisa, masha, oleg)
    alisa.close()
    masha.close()
    oleg.close()

if __name__ == "__main__":
    run_tests()
