import os
import shutil
from datetime import datetime
from SCons.Script import Import

Import("env")

def copy_spiffs_with_date(source, target, env):
    # Оригинальный файл (spiffs.bin)
    spiffs_bin_path = str(target[0])
    
    # Формируем новое имя
    build_date = datetime.now().strftime("%Y%m%d")
    new_file_name = f"pvk_web_{build_date}.bin"
    
    # Путь назначения (в ту же папку build)
    build_dir = os.path.dirname(spiffs_bin_path)
    new_path = os.path.join(build_dir, new_file_name)
    
    # Копируем файл вместо переименования
    # Это предотвращает ошибку "Access denied", так как оригинал остается на месте
    shutil.copy2(spiffs_bin_path, new_path)
    # os.rename(spiffs_bin_path, new_path)
    # print(f"\n--- SUCCESS: Moved SPIFFS to {new_path} ---\n")

# Важно: используем AddPostAction
env.AddPostAction("$BUILD_DIR/spiffs.bin", copy_spiffs_with_date)