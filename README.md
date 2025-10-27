# Simple Messaging

Небольшой проект обмена сообщениями, созданный в учебных целях.
Реализована client-server архитектура, с использованием boost::asio для передачи данных по сети.

## Установка
```bash
git clone https://github.com/SergeySoprykin/Messaging.git
cd Messaging
mkdir build
cd build
cmake ..
cmake --build .
```

## Структура
```
messaging/
├── CMakeLists.txt
├── src/
│   ├── client.cpp
│   ├── connection.cpp
│   └── messages_storage.cpp
│   └── server.cpp
│   └── test_client.cpp
│   └── test_server.cpp
└── include/
    ├── client.hpp
    ├── connection.hpp
    └── i_storage.hpp
    └── message.hpp
    └── messages_storage.hpp
    └── queue_with_lock.hpp
    └── server.hpp
```

## Запуск тестового сервера
```bash
cd build
./Server
```

## Запуск тестовых клиетнов
```bash
cd build
./Client [client_name] [dest_client_name]
```

	client_name - имя запускаемого клиента
	dest_client_name - имя клиента, которому будут отправляться тестовые сообщения
