#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import time

class IRCTestClient:
    def __init__(self, host="127.0.0.1", port=6667, timeout=2):
        """
        Создаёт клиентский сокет и пытается подключиться к IRC-серверу.
        """
        self.host = host
        self.port = port
        self.sock = None
        self._read_buffer = b""  # Буфер для приёма данных

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(timeout)
            self.sock.connect((host, port))
            print(f"[✅] Connected to {host}:{port}")
        except socket.error as e:
            print(f"[❌] Connection failed: {e}")
            self.sock = None

    def send_line(self, message: str):
        """
        Отправляет строку с \r\n в конце, как того требует IRC-протокол.
        """
        if not self.sock:
            print("[⚠️] Not connected, cannot send data.")
            return
        data = (message + "\r\n").encode("utf-8")
        try:
            self.sock.sendall(data)
            print(f"[➡️] Sent: {message}")
        except socket.error as e:
            print(f"[❌] Send failed: {e}")
            self.close()

    def receive_line(self):
        """
        Читает одну строку до \r\n или \n. Возвращает строку без завершающих \r\n.
        Если сервер закрыл соединение или произошла ошибка, возвращает None.
        """
        if not self.sock:
            return None

        while True:
            if b"\n" in self._read_buffer:
                line, _, rest = self._read_buffer.partition(b"\n")
                self._read_buffer = rest
                line = line.rstrip(b"\r").decode("utf-8", errors="replace")
                print(f"[⬅️] Received line: {line}")
                return line

            try:
                chunk = self.sock.recv(1024)
                if not chunk:
                    print("[🔴] Server closed connection.")
                    self.close()
                    return None
                self._read_buffer += chunk
            except socket.timeout:
                return None
            except socket.error as e:
                print(f"[❌] Receive failed: {e}")
                self.close()
                return None

    def auth(self, password: str, nick: str, user: str):
        """
        Упрощённая авторизация: отправляем PASS/NICK/USER поочерёдно.
        """
        if password:
            self.send_line(f"PASS {password}")
        self.send_line(f"NICK {nick}")
        self.send_line(f"USER {user} 0 * :RealNameOf{user}")

    def close(self):
        """Закрывает соединение, если оно было открыто."""
        if self.sock:
            try:
                self.sock.close()
                print("[🔴] Connection closed")
            except socket.error as e:
                print(f"[⚠️] Error closing socket: {e}")
            finally:
                self.sock = None

    def __del__(self):
        self.close()
