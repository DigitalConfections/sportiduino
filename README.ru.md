![](https://raw.githubusercontent.com/alexandervolikov/sportiduino/master/Images/logo.png)

#### Version 1.5.0

![](https://raw.githubusercontent.com/alexandervolikov/sportIDuino/master/Images/Sportiduino.JPG)

[Read this in English](https://github.com/alexandervolikov/sportiduino/blob/master/README.md)

Данный репозиторий посвящен созданию дешевой системы электронной отметки для ориентирования. Она также может быть использована при проведении рогейнов, мультигонок, трейлов, везде, где требуется фикасация времени и порядка прохождения дистанции. Здесь находится аппаратная часть системы. Ссылки на программное обеспечение помещены [ниже](https://github.com/alexandervolikov/sportiduino/blob/master/README.ru.md#%D0%9E%D0%B1%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%BA%D0%B0-%D0%B4%D0%B0%D0%BD%D0%BD%D1%8B%D1%85)

[Скачать последний релиз](https://github.com/alexandervolikov/sportiduino/releases)

[Руководство](https://github.com/alexandervolikov/sportiduino/blob/master/Doc/ru.md)

Проект свободный. В данный момент изготовление на продажу не осуществляется. Но, кто не боится трудностей, может попоробовать сделать это самостоятельно следуя инструкциям из wiki. Дешевизна компонентов системы может окупить затраченные труды (порядка 500 рублей за базовую станцию, 20 рублей за чип отметки).

Предупреждаю, что разработка носит характер хобби, автор не является профессионалом в области электроники и программирования. Поэтому никаких гарантий не даётся, возможны разного рода проблеммы при воспроизведении. Поддержка также не обещается. Так что, действуйте на свой риск. 

## Вопросы и предложения

Обнаруженные баги, предложения по улучшению системы а также вопросы касательные работы железной части системы просьба писать в [issue](https://github.com/alexandervolikov/sportiduino/issues)

## Вклад в разработку

Вы можете внести вклад в развитие проекта с помощью написания программ для работы с системой на ПК или Андройде. Протокол передачи данных и команды описан в [руководстве](https://github.com/alexandervolikov/sportiduino/blob/master/Doc/ru/MasterStation.md)

Пул реквесты приветствуются. Также с удовольствием добавим ссылку на Ваши разработки, работающие с Sportiduino.

Также поддерживается создание форков, развивающие какие-либо новые идеи.
Например, [форк](https://github.com/halny123/sportiduino) , в которой схема базовой станции дополнена радио-модулем:

Можно помочь с переводом документации. На данный момент он выполнен весьма грубо.

# Компоненты системы

## Чипы

В системе используются чипы Mifare Classic S50. Эти чипы идут в комплекте с модулем RC522. Памяти данных чипов хватает на 42 отметки.

<b>Чипы NTAG в новой версии 1.5.0 пока не поддерживаются!!!</b>

[Подробнее тут](https://github.com/alexandervolikov/sportiduino/blob/master/Doc/ru/Card.md)

## Станции отметки.

Основные компоненты станции - микроконтроллер Atmega328 и модуль MFRC522, работающий на частоте 13,56MHz. Часы DS3231SN. Питание от 3-х пальчиковых батареек через стабилизатор MCP1700T-33. Одного комплекта должно хватать на год активного использования. Протестирована работа станций при различных погодных условиях от -20С до +50.

Суммарно исходные компоненты станции и расходники стоят порядка 500 рублей.

[Подробнее тут](https://github.com/alexandervolikov/sportiduino/blob/master/Doc/ru/BaseStation.md)

## Станция сопряжения

Станция сопряжения попроще станции отметки, состоит из Arduino Nano, RFID-модуля, светодиода и пищалки.
К компьютеру подключение через USB. С помощью станции сопряжения можно выполнять ряд судейских задач.

[Подробнее тут](https://github.com/alexandervolikov/sportiduino/blob/master/Doc/ru/MasterStation.md)

Также есть беспроводная станция сопряжени с модулем bluetooth. Ведётся разарботка программного обеспечения под Android.

## Обработка данных

### SportiduinoPQ

Настройка чипов и станций производится в программе [SportiduinoPQ](https://github.com/alexandervolikov/SportiduinoPQ)

Программа основана на [модуле на python](https://github.com/alexandervolikov/sportiduinoPython) а также на пакете PyQt для создания оконных приложений

### SportOrg

Чтение чипов реализовано в программе [SportOrg ](https://github.com/sportorg/pysport)

[Сайт с руководством](http://sportorg.o-ural.ru/)


***********
[Проведенные на системе соревнования](https://github.com/alexandervolikov/sportiduino/wiki/%D0%9F%D1%80%D0%BE%D0%B2%D0%B5%D0%B4%D0%B5%D0%BD%D0%BD%D1%8B%D0%B5-%D1%81%D0%BE%D1%80%D0%B5%D0%B2%D0%BD%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D1%8F)

***********
Публикация на geektimes [первая](https://geektimes.ru/post/290057/) [вторая](https://geektimes.ru/post/294277/)

***********
Available from:  https://github.com/alexandervolikov/sportiduino
 
License:         GNU GPLv3
