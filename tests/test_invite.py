#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import time

GREEN = "\033[1;32m"
RED = "\033[1;31m"
YELLOW = "\033[1;33m"
BLUE = "\033[1;34m"
RESET = "\033[0m"

SERVER_PORT = 6667
SERVER_PASSWORD = "password"

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

def run_test(test_num, test_name, client, command, expected_substr):
    flush_recv(client)
    send_command(client, command)
    time.sleep(0.5)
    response = recv_response(client)

    print(f"{BLUE}-------------------------------------{RESET}")
    print(f"{YELLOW}Test #{test_num}:  {RESET}{test_name}")
    print(f"{YELLOW}Command:          {RESET}{command}")
    print(f"{YELLOW}Expected:         {RESET}{expected_substr}")
    print(f"{YELLOW}Actual response:  {RESET}{response if response else '<no response>'}")

    success = (expected_substr in response)
    print_result(success, f"Test #{test_num}: {test_name}", response)
    print(f"{BLUE}-------------------------------------{RESET}\n")

def test_invite_command(alisa, masha, oleg):

    global failed_tests_count
    failed_tests_count = 0

    print_header("ТЕСТИРОВАНИЕ КОМАНДЫ INVITE")

    channel = "#invite_test"

    run_test(1, "Unregistered Client", create_client(), f"INVITE Masha {channel}", "451 :You have not registered")
    run_test(2, "Not enough parameters", alisa, "INVITE", "461 INVITE :Not enough parameters")
    run_test(3, "No such channel", alisa, "INVITE Masha #nochannel", "403 #nochannel :No such channel")

    join_channel(alisa, channel)
    run_test(4, "No such nick", alisa, f"INVITE Nobody {channel}", "401 Nobody :No such nick/channel")
    run_test(5, "Not on channel", masha, f"INVITE Oleg {channel}", f"442 {channel} :You're not on that channel")

    join_channel(masha, channel)
    run_test(6, "Not a channel operator", masha, f"INVITE Oleg {channel}", f"482 {channel} :You're not a channel operator")
    run_test(7, "Target user already in channel", alisa, f"INVITE Masha {channel}", f"443 Masha {channel} :is already on channel")
    run_test(8, "Inviting yourself", alisa, f"INVITE Alisa {channel}", f"443 Alisa {channel} :is already on channel")

    run_test(9, "Successful INVITE", alisa, f"INVITE Oleg {channel}", f"341 Alisa Oleg {channel}")

    join_channel(oleg, channel)
    run_test(10, "Re-invite Oleg", alisa, f"INVITE Oleg {channel}", f"443 Oleg {channel} :is already on channel")

    send_command(alisa, f"MODE {channel} +i")
    time.sleep(0.3)
    flush_recv(alisa)
    run_test(11, "Invite-only without operator (Masha)", masha, f"INVITE Oleg {channel}", f"482 {channel} :You're not a channel operator")

    bob = create_client()
    register_client(bob, "Bob", "Bob", "Bob the Invisible")
    time.sleep(0.2)
    run_test(12, "Invite-only with operator (non-channel user)", alisa, f"INVITE Bob {channel}", f"341 Alisa Bob {channel}")
    bob.close()


    if failed_tests_count == 0:
        print(f"{GREEN}\n🎉 Все тесты INVITE пройдены успешно!{RESET}")
    else:
        print(f"{RED}\n💀 {failed_tests_count} тест(ов) INVITE провалены!{RESET}")

def run_tests():

    alisa, masha, oleg = setup_environment()
    test_invite_command(alisa, masha, oleg)
    alisa.close()
    masha.close()
    oleg.close()

if __name__ == "__main__":
    run_tests()
