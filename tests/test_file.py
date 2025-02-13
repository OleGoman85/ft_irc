#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import time

GREEN = "\033[1;32m"
RED   = "\033[1;31m"
YELLOW= "\033[1;33m"
BLUE  = "\033[1;34m"
RESET = "\033[0m"

SERVER_HOST = "127.0.0.1"
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
    s.connect((SERVER_HOST, SERVER_PORT))
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
            data = client.recv(4096)
            if not data:
                break
    except:
        pass


def register_client(client, nick, user, realname):
    send_command(client, f"PASS {SERVER_PASSWORD}")
    time.sleep(0.1)
    flush_recv(client)

    send_command(client, f"NICK {nick}")
    send_command(client, f"USER {user} 0 * :{realname}")
    time.sleep(0.5)
    flush_recv(client)


def run_file_test(test_num, test_name, client, command, expected_substr):
    global failed_tests_count
    flush_recv(client)
    send_command(client, command)
    time.sleep(0.4)
    response = recv_response(client)

    print(f"{BLUE}-------------------------------------{RESET}")
    print(f"{YELLOW}Test #{test_num}: {RESET}{test_name}")
    print(f"{YELLOW}Command:          {RESET}{command}")
    print(f"{YELLOW}Expected:         {RESET}{expected_substr}")
    print(f"{YELLOW}Actual response:  {RESET}{response if response else '<no response>'}")

    success = (expected_substr in response)
    print_result(success, f"Test #{test_num}: {test_name}", response)
    print(f"{BLUE}-------------------------------------{RESET}\n")


def test_file_command():
    global failed_tests_count
    failed_tests_count = 0

    print_header("ТЕСТИРОВАНИЕ FILE-КОМАНДЫ")


    alice = create_client()
    bob   = create_client()

    register_client(alice, "Alisa", "Alisa", "Alisa the Great")
    register_client(bob,   "Bob",   "Bob",   "Bob the Great")

    # Test #1: Недостаточно параметров
    run_file_test(
        1,
        "FILE без параметров",
        alice,
        "FILE",
        "461 FILE :Not enough parameters"
    )

    # Test #2: FILE SEND c недостатком параметров
    run_file_test(
        2,
        "FILE SEND without enough args",
        alice,
        "FILE SEND Bob",
        "461 FILE SEND :Not enough parameters"
    )

    # Test #3: Отправка файлу несуществующему пользователю
    run_file_test(
        3,
        "FILE SEND to non-existent user",
        alice,
        "FILE SEND NonExist file.txt 12",
        "401 NonExist :No such nick"
    )

    # Test #4: Правильная отправка (FILE SEND)
    run_file_test(
        4,
        "FILE SEND valid",
        alice,
        "FILE SEND Bob testfile.txt 12",
        "Ready to receive file 'testfile.txt'"
    )

    # Test #5: Передаем кусок данных (FILE DATA)
    run_file_test(
        5,
        "FILE DATA partial",
        alice,
        "FILE DATA testfile.txt SGVsbG8sIFdvcmxk",
        "Uploaded"
    )

    # Test #6: Завершаем отправку (FILE END)
    run_file_test(
        6,
        "FILE END",
        alice,
        "FILE END testfile.txt",
        "File transfer completed"
    )

    alice.close()
    bob.close()

    if failed_tests_count == 0:
        print(f"{GREEN}\n🎉 Все тесты FILE пройдены успешно!{RESET}")
    else:
        print(f"{RED}\n💀 {failed_tests_count} тест(ов) провалены!{RESET}")


def run_tests():
    test_file_command()

if __name__ == "__main__":
    run_tests()
