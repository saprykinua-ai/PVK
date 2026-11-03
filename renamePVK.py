import os
from datetime import datetime

Import("env")

# Форматируем дату и время для безопасного использования в имени файла
# build_time = datetime.now().strftime("%Y%m%d_%H%M%S")
# env.Replace(PROGNAME=f"RECT_{build_time}")

# Альтернативный вариант - только дата (без дефисов)
build_date = datetime.now().strftime("%Y%m%d")
env.Replace(PROGNAME=f"pvk_{build_date}")

# Добавляем макросы для C++ кода
env.Append(BUILD_FLAGS=[
    f'-DPROJECT_NAME=\\"pvk\\"',
    f'-DBUILD_DATE=\\"{build_date}\\"'
])