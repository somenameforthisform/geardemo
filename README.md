Сборка через emscripten 
---
- если до этого была сборка под десктоп, то удалить папку build `rm -rf build`

- [собрать emscripten](https://emscripten.org/docs/getting_started/downloads.html)
- [собрать raylib](https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)#2-compile-raylib-library)

- указать переменные окружения emsdk `source {путь_до_билда_emscripten}/emsdk_env.sh`
- настроить проект для сборки с помощью emcmake `emcmake cmake -S . -B build`
- подождать какое-то время исходники raylib подтянутся из github
- собрать проект `cmake --build build`
- будет сгенерирован файл "geardemo.html" в папке build - его нужно заменить на файл с названием "index.html" из папки src.
  Или оставить всё как есть, не будет работать только скачивание STL модели
- запустить dev-сервер `emrun build/index.html` или `emrun build/geardemo.html` - если файл не был заменён
- в `/docs` лежит уже собранная версия (да, там не документация, но так есть возможность развернуть проект через github pages)



десктопная сборка
---
- настроить проект для сборки с помощью cmake `cmake -S . -B build`
- подождать какое-то время исходники raylib подтянутся из github

  если возникли проблемы со скачиванием исходников raylib через cmake и команда выше не подаёт признаков жизни, вероятно проблема с интернет-соединением. Можно скачать zip-архив через браузер (подробнее в CMakeLists.txt)

- собрать проект `cmake --build build`
- перейти в папку со сборкой (это важно, чтобы подтянулись шрифты и шейдеры) `cd build`
- запустить проект `./geardemo`
- при сохранении STL модели, файл модели создаётся рядом с исполняемым файлом



